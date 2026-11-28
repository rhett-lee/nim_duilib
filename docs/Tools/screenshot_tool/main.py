# -*- coding: utf-8 -*-
"""nim_duilib 示例窗口截图工具。

为 docs/Images 生成“窗口 + 纯色背景 + 真实阴影”的截图：
1. 截图前临时显示一层覆盖整个桌面的纯色窗口，并把目标窗口置顶；
2. 使用 DWM 扩展边框获取真实窗口范围，兼容高 DPI 与多显示器；
3. 按进程名输出固定文件名，便于文档直接引用。
"""

from __future__ import annotations

import ctypes
import os
import sys
import time
import traceback
from ctypes import wintypes
from dataclasses import dataclass
from pathlib import Path

import psutil
import win32api
import win32con
import win32gui
import win32process
from PIL import Image, ImageGrab
from PyQt5.QtCore import QSize, Qt, QThread, QTimer, pyqtSignal
from PyQt5.QtGui import QColor, QFont, QIcon, QPixmap
from PyQt5.QtWidgets import (
    QApplication,
    QColorDialog,
    QFileDialog,
    QGroupBox,
    QHBoxLayout,
    QHeaderView,
    QLabel,
    QLineEdit,
    QListWidget,
    QListWidgetItem,
    QMainWindow,
    QMessageBox,
    QPlainTextEdit,
    QProgressBar,
    QPushButton,
    QSpinBox,
    QTreeWidget,
    QTreeWidgetItem,
    QVBoxLayout,
    QWidget,
)

DWMWA_EXTENDED_FRAME_BOUNDS = 9
DWMWA_CLOAKED = 14

# 窗口外侧额外截取的像素（用于容纳系统阴影），会按窗口 DPI 缩放
BACKGROUND_PADDING = 20

EXCLUDED_WINDOW_CLASSES = {"Progman", "WorkerW", "Shell_TrayWnd"}
INVALID_FILENAME_CHARS = set('<>:"/\\|?*')
WINDOWS_RESERVED_NAMES = {
    "CON", "PRN", "AUX", "NUL",
    *(f"COM{i}" for i in range(1, 10)),
    *(f"LPT{i}" for i in range(1, 10)),
}


class RECT(ctypes.Structure):
    _fields_ = [
        ("left", wintypes.LONG),
        ("top", wintypes.LONG),
        ("right", wintypes.LONG),
        ("bottom", wintypes.LONG),
    ]


def enable_dpi_awareness() -> None:
    """必须在创建 QApplication 之前调用，否则 Qt 与 Win32 坐标会不一致。"""
    try:
        if ctypes.windll.user32.SetProcessDpiAwarenessContext(ctypes.c_void_p(-4)):
            return
    except Exception:
        pass
    try:
        ctypes.windll.shcore.SetProcessDpiAwareness(2)  # PROCESS_PER_MONITOR_DPI_AWARE
        return
    except Exception:
        pass
    try:
        ctypes.windll.user32.SetProcessDPIAware()
    except Exception:
        pass


def dwm_get_window_attribute(hwnd: int, attribute: int, buffer, size: int):
    try:
        return ctypes.windll.dwmapi.DwmGetWindowAttribute(
            ctypes.c_void_p(hwnd), attribute, buffer, size)
    except Exception:
        return -1


def dwm_flush() -> None:
    try:
        ctypes.windll.dwmapi.DwmFlush()
    except Exception:
        pass


def is_window_cloaked(hwnd: int) -> bool:
    value = ctypes.c_int(0)
    if dwm_get_window_attribute(
            hwnd, DWMWA_CLOAKED, ctypes.byref(value), ctypes.sizeof(value)) != 0:
        return False
    return value.value != 0


def get_window_frame_rect(hwnd: int) -> tuple[int, int, int, int]:
    rect = RECT()
    if dwm_get_window_attribute(
            hwnd, DWMWA_EXTENDED_FRAME_BOUNDS,
            ctypes.byref(rect), ctypes.sizeof(rect)) == 0:
        if rect.right > rect.left and rect.bottom > rect.top:
            return rect.left, rect.top, rect.right, rect.bottom
    return win32gui.GetWindowRect(hwnd)


def get_window_dpi(hwnd: int) -> int:
    try:
        dpi = ctypes.windll.user32.GetDpiForWindow(hwnd)
        if dpi > 0:
            return dpi
    except Exception:
        pass
    return 96


def get_virtual_screen_rect() -> tuple[int, int, int, int]:
    return (
        win32api.GetSystemMetrics(win32con.SM_XVIRTUALSCREEN),
        win32api.GetSystemMetrics(win32con.SM_YVIRTUALSCREEN),
        win32api.GetSystemMetrics(win32con.SM_CXVIRTUALSCREEN),
        win32api.GetSystemMetrics(win32con.SM_CYVIRTUALSCREEN),
    )


def sanitize_filename(name: str) -> str:
    name = "".join(ch for ch in name if ch not in INVALID_FILENAME_CHARS and ord(ch) >= 32)
    name = name.strip(" .")
    if not name:
        return "window"
    if name.upper() in WINDOWS_RESERVED_NAMES:
        name = "_" + name
    return name[:80]


# ---------------------------------------------------------------------------
# 权限诊断
# ---------------------------------------------------------------------------

_kernel32 = ctypes.windll.kernel32
_advapi32 = ctypes.windll.advapi32
_kernel32.OpenProcess.argtypes = [wintypes.DWORD, wintypes.BOOL, wintypes.DWORD]
_kernel32.OpenProcess.restype = wintypes.HANDLE
_kernel32.CloseHandle.argtypes = [wintypes.HANDLE]
_kernel32.CloseHandle.restype = wintypes.BOOL
_advapi32.OpenProcessToken.argtypes = [
    wintypes.HANDLE, wintypes.DWORD, ctypes.POINTER(wintypes.HANDLE)]
_advapi32.OpenProcessToken.restype = wintypes.BOOL
_advapi32.GetTokenInformation.argtypes = [
    wintypes.HANDLE, ctypes.c_int, ctypes.c_void_p, wintypes.DWORD,
    ctypes.POINTER(wintypes.DWORD)]
_advapi32.GetTokenInformation.restype = wintypes.BOOL

PROCESS_QUERY_LIMITED_INFORMATION = 0x1000
TOKEN_QUERY = 0x0008
TOKEN_ELEVATION = 20


def is_process_elevated(pid: int) -> bool | None:
    """判断进程是否以管理员权限运行；无法查询时返回 None。"""
    if pid <= 0:
        return None
    process = _kernel32.OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, False, pid)
    if not process:
        return None
    try:
        token = wintypes.HANDLE()
        if not _advapi32.OpenProcessToken(process, TOKEN_QUERY, ctypes.byref(token)):
            return None
        try:
            elevation = wintypes.DWORD(0)
            returned = wintypes.DWORD(0)
            if not _advapi32.GetTokenInformation(
                    token, TOKEN_ELEVATION, ctypes.byref(elevation),
                    ctypes.sizeof(elevation), ctypes.byref(returned)):
                return None
            return bool(elevation.value)
        finally:
            _kernel32.CloseHandle(token)
    finally:
        _kernel32.CloseHandle(process)


# ---------------------------------------------------------------------------
# 纯色背景窗口：给 duilib 的分层窗口阴影提供干净底色
# ---------------------------------------------------------------------------

_BACKDROP_CLASS_NAME = "NimDuilibScreenshotBackdrop"
_backdrop_color = (255, 255, 255)


def _fill_backdrop(hwnd: int, hdc) -> None:
    brush = win32gui.CreateSolidBrush(win32api.RGB(*_backdrop_color))
    if not brush:
        return
    try:
        win32gui.FillRect(hdc, win32gui.GetClientRect(hwnd), brush)
    finally:
        win32gui.DeleteObject(brush)


def _backdrop_wndproc(hwnd, msg, wparam, lparam):
    try:
        if msg == win32con.WM_ERASEBKGND:
            _fill_backdrop(hwnd, wparam)
            return 1
        if msg == win32con.WM_PAINT:
            hdc, paint_struct = win32gui.BeginPaint(hwnd)
            try:
                _fill_backdrop(hwnd, hdc)
            finally:
                win32gui.EndPaint(hwnd, paint_struct)
            return 0
    except Exception:
        pass
    return win32gui.DefWindowProc(hwnd, msg, wparam, lparam)


class SolidBackdrop:
    """纯色背景窗口，会被插到目标窗口正后方，用来承接窗口阴影。"""

    def __init__(self) -> None:
        self.hwnd = 0

    def ensure_created(self) -> bool:
        if self.hwnd and win32gui.IsWindow(self.hwnd):
            return True
        try:
            hinst = win32api.GetModuleHandle(None)
            wc = win32gui.WNDCLASS()
            wc.hInstance = hinst
            wc.lpszClassName = _BACKDROP_CLASS_NAME
            wc.hbrBackground = 0
            wc.lpfnWndProc = _backdrop_wndproc
            try:
                win32gui.RegisterClass(wc)
            except Exception:
                pass  # 类名已注册，直接复用
            self.hwnd = win32gui.CreateWindowEx(
                win32con.WS_EX_TOOLWINDOW | win32con.WS_EX_NOACTIVATE,
                _BACKDROP_CLASS_NAME, "", win32con.WS_POPUP,
                0, 0, 64, 64, 0, 0, hinst, None)
            return bool(self.hwnd) and bool(win32gui.IsWindow(self.hwnd))
        except Exception:
            self.hwnd = 0
            return False

    def set_color(self, color: tuple[int, int, int]) -> None:
        global _backdrop_color
        _backdrop_color = color

    def show_behind(self, target_hwnd: int, rect: tuple[int, int, int, int]) -> bool:
        """把背景窗口插到目标窗口正后方，返回是否成功排在目标窗口下方。"""
        if not self.ensure_created():
            return False
        left, top, right, bottom = rect
        win32gui.SetWindowPos(
            self.hwnd, target_hwnd, left, top,
            max(1, right - left), max(1, bottom - top),
            win32con.SWP_NOACTIVATE | win32con.SWP_SHOWWINDOW)
        win32gui.InvalidateRect(self.hwnd, None, True)

        if not win32gui.IsWindowVisible(self.hwnd):
            return False
        if win32gui.GetWindowRect(self.hwnd) != (left, top, right, bottom):
            return False
        # 确认目标窗口位于背景窗口上方（中间可能夹着输入法等隐藏窗口）
        window = win32gui.GetWindow(self.hwnd, win32con.GW_HWNDPREV)
        while window:
            if window == target_hwnd:
                return True
            window = win32gui.GetWindow(window, win32con.GW_HWNDPREV)
        return False

    def hide(self) -> None:
        if self.hwnd and win32gui.IsWindow(self.hwnd):
            win32gui.ShowWindow(self.hwnd, win32con.SW_HIDE)


# ---------------------------------------------------------------------------
# 窗口枚举与截图线程
# ---------------------------------------------------------------------------

@dataclass
class WindowInfo:
    hwnd: int
    pid: int
    process: str
    title: str
    class_name: str
    width: int
    height: int
    minimized: bool

    def size_text(self) -> str:
        if self.minimized or self.width <= 0 or self.height <= 0:
            return "-"
        return f"{self.width}×{self.height}"


def list_visible_windows(own_pid: int) -> list[WindowInfo]:
    windows: list[WindowInfo] = []
    process_names: dict[int, str] = {}

    def process_name(pid: int) -> str:
        if pid not in process_names:
            try:
                process_names[pid] = psutil.Process(pid).name() or ""
            except Exception:
                process_names[pid] = ""
        return process_names[pid]

    def callback(hwnd, _):
        try:
            if not win32gui.IsWindowVisible(hwnd) or is_window_cloaked(hwnd):
                return True
            title = win32gui.GetWindowText(hwnd)
            if not title:
                return True
            class_name = win32gui.GetClassName(hwnd)
            if class_name in EXCLUDED_WINDOW_CLASSES:
                return True
            _, pid = win32process.GetWindowThreadProcessId(hwnd)
            if not pid or pid == own_pid:
                return True
            minimized = bool(win32gui.IsIconic(hwnd))
            if minimized:
                width = height = 0
            else:
                left, top, right, bottom = get_window_frame_rect(hwnd)
                width = max(0, right - left)
                height = max(0, bottom - top)
                if width <= 0 or height <= 0:
                    return True
            windows.append(WindowInfo(
                hwnd=hwnd,
                pid=pid,
                process=process_name(pid),
                title=title,
                class_name=class_name,
                width=width,
                height=height,
                minimized=minimized,
            ))
        except Exception:
            pass
        return True

    win32gui.EnumWindows(callback, None)
    windows.sort(key=lambda w: ((w.process or "").lower(), w.title.lower()))
    return windows


class SearchWorker(QThread):
    results_ready = pyqtSignal(list)
    failed = pyqtSignal(str)

    def __init__(self, keyword: str, parent=None) -> None:
        super().__init__(parent)
        self.keyword = keyword.strip().lower()

    def run(self) -> None:
        try:
            windows = list_visible_windows(os.getpid())
            if self.keyword:
                windows = [
                    w for w in windows
                    if self.keyword in (w.process or "").lower()
                    or self.keyword in w.title.lower()
                    or self.keyword in (w.class_name or "").lower()
                ]
            self.results_ready.emit(windows)
        except Exception as exc:
            self.failed.emit(f"枚举窗口失败：{exc}")


@dataclass
class CaptureConfig:
    output_dir: str
    target_width: int
    delay_ms: int


class ScreenshotWorker(QThread):
    log_msg = pyqtSignal(str)
    progress_changed = pyqtSignal(int)
    image_saved = pyqtSignal(str)
    work_finished = pyqtSignal(bool, str, int, int)

    def __init__(self, windows: list[WindowInfo], config: CaptureConfig,
                 backdrop: SolidBackdrop, parent=None) -> None:
        super().__init__(parent)
        self.windows = windows
        self.config = config
        self.backdrop = backdrop
        self._name_counts: dict[str, int] = {}
        self._self_elevated = is_process_elevated(os.getpid())

    def run(self) -> None:
        try:
            self._run()
        except Exception:
            self.work_finished.emit(
                False, "截图线程异常：\n" + traceback.format_exc(limit=3), 0, len(self.windows))

    def _run(self) -> None:
        total = len(self.windows)
        try:
            os.makedirs(self.config.output_dir, exist_ok=True)
        except Exception as exc:
            self.work_finished.emit(False, f"无法创建输出目录：{exc}", 0, total)
            return

        success = 0
        for index, window in enumerate(self.windows, start=1):
            self.progress_changed.emit(int((index - 1) * 100 / total))
            if self._capture_one(window):
                success += 1
        self.progress_changed.emit(100)

        if success == 0:
            self.work_finished.emit(False, f"截图失败：0/{total}，请查看日志", 0, total)
        else:
            self.work_finished.emit(
                True,
                f"完成：成功 {success}/{total} 张，输出目录：{self.config.output_dir}",
                success,
                total,
            )

    def _capture_one(self, window: WindowInfo) -> bool:
        label = f"{window.process or '未知进程'} - {window.title}"
        if not win32gui.IsWindow(window.hwnd):
            self.log_msg.emit(f"跳过（窗口已关闭）：{label}")
            return False
        if is_window_cloaked(window.hwnd) or not win32gui.IsWindowVisible(window.hwnd):
            self.log_msg.emit(f"跳过（窗口不可见）：{label}")
            return False

        if self._self_elevated is False and is_process_elevated(window.pid):
            self.log_msg.emit(
                "警告：目标窗口以管理员权限运行，置顶或背景插入可能失败；"
                "建议以管理员身份运行本工具。")

        was_minimized = bool(win32gui.IsIconic(window.hwnd))
        try:
            self.log_msg.emit(f"正在截图：{label}")
            self._raise_window(window.hwnd, was_minimized)

            rect = get_window_frame_rect(window.hwnd)
            if rect[2] - rect[0] <= 0 or rect[3] - rect[1] <= 0:
                self.log_msg.emit(f"跳过（窗口尺寸无效）：{label}")
                return False

            padding = max(1, round(BACKGROUND_PADDING * get_window_dpi(window.hwnd) / 96.0))
            bbox = self._capture_bbox(rect, padding)
            if bbox is None:
                self.log_msg.emit(f"跳过（窗口超出屏幕范围）：{label}")
                return False

            # 窗口刚创建时置顶请求可能被系统忽略，这里确认它真的在最前面
            self._ensure_window_on_top(window.hwnd, rect)

            # 把纯色背景插到目标窗口正后方，窗口阴影会自然落在背景上
            backdrop_ready = self.backdrop.show_behind(window.hwnd, bbox)
            if not backdrop_ready:
                self.log_msg.emit("警告：背景窗口未能排到目标窗口下方，改为直接截屏。")
                self.backdrop.hide()

            time.sleep(max(0, self.config.delay_ms) / 1000.0)
            if not self._is_window_on_top(window.hwnd, rect):
                self.log_msg.emit("警告：目标窗口被其他窗口遮挡，正在重新置顶。")
                self._ensure_window_on_top(window.hwnd, rect)
                time.sleep(0.1)
            image = ImageGrab.grab(bbox=bbox, all_screens=True).convert("RGB")
            self.backdrop.hide()

            image = self._resize_to_width(image, self.config.target_width)
            path, existed = self._save_image(window, image)
            self.log_msg.emit(f"已保存{('（覆盖）' if existed else '')}：{path}")
            self.image_saved.emit(path)
            return True
        except Exception as exc:
            self.log_msg.emit(f"失败：{label}（{exc}）")
            return False
        finally:
            self.backdrop.hide()
            self._restore_window(window.hwnd, was_minimized)

    @staticmethod
    def _is_window_on_top(hwnd: int, rect: tuple[int, int, int, int]) -> bool:
        """检查窗口中心点处最上层窗口是否就是目标窗口。"""
        left, top, right, bottom = rect
        point = ((left + right) // 2, (top + bottom) // 2)
        try:
            window = win32gui.WindowFromPoint(point)
            if not window:
                return False
            root = win32gui.GetAncestor(window, win32con.GA_ROOT)
            return root == hwnd or window == hwnd
        except Exception:
            return False

    @classmethod
    def _ensure_window_on_top(cls, hwnd: int, rect, attempts: int = 12) -> bool:
        for _ in range(attempts):
            if cls._is_window_on_top(hwnd, rect):
                return True
            cls._raise_window(hwnd, False)
            time.sleep(0.05)
        return cls._is_window_on_top(hwnd, rect)

    @staticmethod
    def _raise_window(hwnd: int, was_minimized: bool) -> None:
        if was_minimized:
            win32gui.ShowWindow(hwnd, win32con.SW_RESTORE)
        ex_style = win32gui.GetWindowLong(hwnd, win32con.GWL_EXSTYLE)
        # 保持窗口原有的置顶状态，只把它提到所在 Z 序分组的顶部
        insert_after = (win32con.HWND_TOPMOST
                        if ex_style & win32con.WS_EX_TOPMOST
                        else win32con.HWND_TOP)
        win32gui.SetWindowPos(
            hwnd, insert_after, 0, 0, 0, 0,
            win32con.SWP_NOMOVE | win32con.SWP_NOSIZE |
            win32con.SWP_SHOWWINDOW | win32con.SWP_NOACTIVATE)
        dwm_flush()

    @staticmethod
    def _restore_window(hwnd: int, was_minimized: bool) -> None:
        if not was_minimized:
            return
        try:
            if win32gui.IsWindow(hwnd):
                win32gui.ShowWindow(hwnd, win32con.SW_MINIMIZE)
        except Exception:
            pass

    @staticmethod
    def _capture_bbox(rect: tuple[int, int, int, int], padding: int):
        vx, vy, vw, vh = get_virtual_screen_rect()
        left, top, right, bottom = rect
        if left < vx or top < vy or right > vx + vw or bottom > vy + vh:
            return None
        return (
            max(vx, left - padding),
            max(vy, top - padding),
            min(vx + vw, right + padding),
            min(vy + vh, bottom + padding),
        )

    @staticmethod
    def _resize_to_width(image, target_width: int):
        if target_width <= 0 or image.width == target_width:
            return image
        height = max(1, round(image.height * target_width / image.width))
        return image.resize((target_width, height), Image.LANCZOS)

    def _save_image(self, window: WindowInfo, image):
        stem = sanitize_filename(Path(window.process).stem if window.process else "window")
        count = self._name_counts.get(stem.lower(), 0)
        self._name_counts[stem.lower()] = count + 1
        filename = f"{stem}.png" if count == 0 else f"{stem}_{count + 1}.png"
        path = os.path.join(self.config.output_dir, filename)
        existed = os.path.exists(path)
        image.save(path, "PNG")
        return path, existed


# ---------------------------------------------------------------------------
# 主界面
# ---------------------------------------------------------------------------

def default_output_dir() -> str:
    tool_dir = Path(__file__).resolve().parent
    parents = tool_dir.parents
    if len(parents) >= 2:
        images = parents[1] / "Images"  # docs/Tools/screenshot_tool -> docs/Images
        if images.is_dir():
            return str(images)
    return str(Path.cwd() / "screenshots")


class MainWindow(QMainWindow):
    def __init__(self) -> None:
        super().__init__()
        self.setWindowTitle("nim_duilib 截图工具")
        self.resize(900, 660)
        self.setMinimumSize(820, 580)

        self.search_worker: SearchWorker | None = None
        self.worker: ScreenshotWorker | None = None
        self.backdrop = SolidBackdrop()
        self.bg_color = (255, 255, 255)
        self._busy = False

        self._build_ui()
        self.update_color_button()
        if not self.backdrop.ensure_created():
            self.append_log("警告：纯色背景窗口创建失败，截图可能包含桌面内容。")
        self.append_log("选择窗口后点击“开始截图选中窗口”。输出目录默认为 docs/Images。")
        QTimer.singleShot(0, self._fit_title_column)
        QTimer.singleShot(0, self.start_search)

    def _build_ui(self) -> None:
        central = QWidget()
        self.setCentralWidget(central)
        root = QVBoxLayout(central)
        root.setContentsMargins(10, 10, 10, 10)
        root.setSpacing(6)

        # 1. 窗口列表
        group_windows = QGroupBox("1. 选择窗口")
        group_layout = QVBoxLayout(group_windows)
        group_layout.setSpacing(6)

        filter_row = QHBoxLayout()
        self.edit_filter = QLineEdit()
        self.edit_filter.setPlaceholderText("按进程名或窗口标题过滤，留空显示全部可见窗口")
        self.edit_filter.returnPressed.connect(self.start_search)
        self.btn_search = QPushButton("刷新")
        self.btn_search.clicked.connect(self.start_search)
        self.btn_check_all = QPushButton("全选")
        self.btn_check_all.clicked.connect(lambda: self.set_all_checked(True))
        self.btn_check_none = QPushButton("全不选")
        self.btn_check_none.clicked.connect(lambda: self.set_all_checked(False))
        filter_row.addWidget(QLabel("过滤:"))
        filter_row.addWidget(self.edit_filter, 1)
        filter_row.addWidget(self.btn_search)
        filter_row.addWidget(self.btn_check_all)
        filter_row.addWidget(self.btn_check_none)
        group_layout.addLayout(filter_row)

        self.tree = QTreeWidget()
        self.tree.setColumnCount(5)
        self.tree.setHeaderLabels(["选择", "窗口标题", "进程", "类名", "尺寸"])
        self.tree.setRootIsDecorated(False)
        self.tree.setAlternatingRowColors(True)
        self.tree.setUniformRowHeights(True)
        header = self.tree.header()
        header.setMinimumSectionSize(30)
        header.setStretchLastSection(False)
        header.setSectionResizeMode(0, QHeaderView.Fixed)
        for column in range(1, 5):
            header.setSectionResizeMode(column, QHeaderView.Interactive)
        # 双击表头分隔线：按内容自适应该列宽度
        header.sectionHandleDoubleClicked.connect(self._fit_column_to_contents)
        self.tree.setColumnWidth(0, 52)
        self.tree.setColumnWidth(1, 220)
        self.tree.setColumnWidth(2, 130)
        self.tree.setColumnWidth(3, 150)
        self.tree.setColumnWidth(4, 110)
        self.tree.setTextElideMode(Qt.ElideRight)
        self.tree.setWordWrap(False)
        self.tree.setMinimumHeight(200)
        self.tree.itemChanged.connect(self.update_capture_button)
        group_layout.addWidget(self.tree)

        # 2. 截图设置
        group_settings = QGroupBox("2. 截图设置")
        group_settings.setFixedWidth(350)
        group_settings.setToolTip(
            "截图时会在目标窗口后面铺一层背景色，窗口阴影（系统阴影或自绘阴影）会完整保留。")
        form = QVBoxLayout(group_settings)
        form.setContentsMargins(8, 6, 8, 6)
        form.setSpacing(4)

        def add_setting_row(text: str, field: QWidget) -> None:
            row = QHBoxLayout()
            row.setSpacing(8)
            label = QLabel(text)
            label.setFixedWidth(76)
            label.setAlignment(Qt.AlignRight | Qt.AlignVCenter)
            row.addWidget(label)
            row.addWidget(field, 1)
            form.addLayout(row)

        self.spin_width = QSpinBox()
        self.spin_width.setRange(0, 8000)
        self.spin_width.setValue(0)
        self.spin_width.setSuffix(" px")
        self.spin_width.setToolTip("0 表示不缩放；大于 0 时按最终图片宽度缩放。")
        add_setting_row("宽度:", self.spin_width)

        self.spin_delay = QSpinBox()
        self.spin_delay.setRange(100, 3000)
        self.spin_delay.setSingleStep(50)
        self.spin_delay.setValue(400)
        self.spin_delay.setSuffix(" ms")
        self.spin_delay.setToolTip("窗口置顶后等待绘制完成的时间；窗口有动画或加载较慢时可调大。")
        add_setting_row("等待:", self.spin_delay)

        self.btn_color = QPushButton()
        self.btn_color.setFixedHeight(24)
        self.btn_color.setMinimumWidth(96)
        self.btn_color.setMaximumWidth(140)
        self.btn_color.setToolTip("截图时在目标窗口后面铺一层该颜色，用于承接窗口阴影。")
        self.btn_color.clicked.connect(self.choose_background_color)
        color_row = QWidget()
        color_layout = QHBoxLayout(color_row)
        color_layout.setContentsMargins(0, 0, 0, 0)
        color_layout.setSpacing(0)
        color_layout.addWidget(self.btn_color)
        color_layout.addStretch()
        add_setting_row("背景色:", color_row)

        self.edit_dir = QLineEdit(default_output_dir())
        self.edit_dir.setCursorPosition(0)
        dir_row = QWidget()
        dir_layout = QHBoxLayout(dir_row)
        dir_layout.setContentsMargins(0, 0, 0, 0)
        dir_layout.setSpacing(6)
        self.btn_browse = QPushButton("浏览…")
        self.btn_browse.setFixedWidth(90)
        self.btn_browse.clicked.connect(self.browse_output_dir)
        dir_layout.addWidget(self.edit_dir, 1)
        dir_layout.addWidget(self.btn_browse)
        add_setting_row("目录:", dir_row)

        # 3. 截图结果
        group_result = QGroupBox("3. 截图结果（双击打开）")
        result_layout = QVBoxLayout(group_result)
        self.preview = QListWidget()
        self.preview.setViewMode(QListWidget.IconMode)
        self.preview.setResizeMode(QListWidget.Adjust)
        self.preview.setMovement(QListWidget.Static)
        self.preview.setIconSize(QSize(140, 96))
        self.preview.setGridSize(QSize(160, 124))
        self.preview.setSpacing(4)
        self.preview.setWordWrap(True)
        self.preview.itemDoubleClicked.connect(self.open_preview)
        result_layout.addWidget(self.preview)

        open_row = QHBoxLayout()
        open_row.addStretch()
        self.btn_open_dir = QPushButton("打开输出目录")
        self.btn_open_dir.clicked.connect(self.open_output_dir)
        open_row.addWidget(self.btn_open_dir)
        result_layout.addLayout(open_row)
        group_result.setFixedHeight(176)

        bottom_band = QHBoxLayout()
        bottom_band.setSpacing(8)
        bottom_band.addWidget(group_settings)
        bottom_band.addWidget(group_result, 1)

        body = QVBoxLayout()
        body.setSpacing(6)
        body.addWidget(group_windows, 1)
        body.addLayout(bottom_band)
        root.addLayout(body, 1)

        self.log_view = QPlainTextEdit()
        self.log_view.setReadOnly(True)
        self.log_view.setFixedHeight(64)
        self.log_view.setMaximumBlockCount(500)
        self.log_view.setPlaceholderText("日志")
        root.addWidget(self.log_view)

        bottom = QHBoxLayout()
        self.progress = QProgressBar()
        self.progress.setRange(0, 100)
        self.progress.setValue(0)
        self.progress.setTextVisible(False)
        self.progress.setFixedHeight(12)
        bottom.addWidget(self.progress, 1)

        self.btn_capture = QPushButton("开始截图选中窗口")
        self.btn_capture.setMinimumHeight(36)
        self.btn_capture.setEnabled(False)
        self.btn_capture.clicked.connect(self.start_capture)
        self.btn_capture.setStyleSheet(
            "QPushButton { background-color: #2196F3; color: white; padding: 8px 18px; "
            "font-weight: bold; border-radius: 4px; }"
            "QPushButton:hover { background-color: #1976D2; }"
            "QPushButton:disabled { background-color: #BDBDBD; }")
        bottom.addWidget(self.btn_capture)
        root.addLayout(bottom)

        self.statusBar().showMessage("就绪")

    # -- 基础交互 ----------------------------------------------------------

    def resizeEvent(self, event) -> None:
        super().resizeEvent(event)
        QTimer.singleShot(0, self._fit_title_column)

    def _fit_title_column(self) -> None:
        """标题列自动占满剩余宽度，其余列仍可手动拖动调整。"""
        viewport_width = self.tree.viewport().width()
        scrollbar = self.tree.verticalScrollBar()
        if not scrollbar.isVisible():
            # 预留滚动条宽度，避免滚动条出现后撑出横向滚动条
            viewport_width -= max(16, scrollbar.sizeHint().width())
        other_width = sum(self.tree.columnWidth(index) for index in (0, 2, 3, 4))
        self.tree.setColumnWidth(1, max(160, viewport_width - other_width))

    # 各列的最大宽度：内容再长也只在单元格内省略，避免把窗口撑宽
    COLUMN_MAX_WIDTH = {1: 400, 2: 150, 3: 170, 4: 120}

    def _resize_column_to_fit(self, column: int) -> None:
        self.tree.resizeColumnToContents(column)
        max_width = self.COLUMN_MAX_WIDTH.get(column, 400)
        if self.tree.columnWidth(column) > max_width:
            self.tree.setColumnWidth(column, max_width)

    def _apply_column_widths(self) -> None:
        """刷新列表后按内容设定各列宽度，标题列占满剩余空间。"""
        for column in (2, 3, 4):
            self._resize_column_to_fit(column)
        self._fit_title_column()

    def _fit_column_to_contents(self, column: int) -> None:
        """双击表头分隔线时，仅让当前列按内容自适应。"""
        if column <= 0 or column == 1:
            return
        self._resize_column_to_fit(column)
        self._fit_title_column()

    def append_log(self, message: str) -> None:
        self.log_view.appendPlainText(message)
        scrollbar = self.log_view.verticalScrollBar()
        scrollbar.setValue(scrollbar.maximum())

    def update_color_button(self) -> None:
        r, g, b = self.bg_color
        text_color = "#FFFFFF" if (r * 299 + g * 587 + b * 114) / 1000 < 128 else "#000000"
        self.btn_color.setText(f"#{r:02X}{g:02X}{b:02X}")
        self.btn_color.setStyleSheet(
            f"QPushButton {{ background-color: rgb({r},{g},{b}); color: {text_color}; "
            f"border: 1px solid #999; border-radius: 3px; }}")
        self.btn_color.setToolTip(f"RGB({r}, {g}, {b})")

    def choose_background_color(self) -> None:
        color = QColorDialog.getColor(QColor(*self.bg_color), self, "选择背景色")
        if color.isValid():
            self.bg_color = (color.red(), color.green(), color.blue())
            self.update_color_button()

    def browse_output_dir(self) -> None:
        directory = QFileDialog.getExistingDirectory(self, "选择保存目录", self.edit_dir.text())
        if directory:
            self.edit_dir.setText(directory)
            self.edit_dir.setCursorPosition(0)

    def open_output_dir(self) -> None:
        path = self.edit_dir.text().strip()
        if path and os.path.isdir(path):
            os.startfile(path)
        else:
            QMessageBox.information(self, "提示", "输出目录尚未创建。")

    def open_preview(self, item: QListWidgetItem) -> None:
        path = item.data(Qt.UserRole)
        if path and os.path.exists(path):
            os.startfile(path)

    # -- 窗口列表 ----------------------------------------------------------

    def set_busy(self, busy: bool) -> None:
        self._busy = busy
        for widget in (
            self.edit_filter, self.btn_search, self.btn_check_all, self.btn_check_none,
            self.tree, self.spin_width, self.spin_delay, self.edit_dir,
            self.btn_browse, self.btn_color, self.btn_open_dir,
        ):
            widget.setEnabled(not busy)
        self.update_capture_button()

    def start_search(self) -> None:
        if self._busy or self.search_worker is not None:
            return
        keyword = self.edit_filter.text().strip()
        self.set_busy(True)
        self.tree.clear()
        self.statusBar().showMessage("正在枚举窗口…")
        self.search_worker = SearchWorker(keyword)
        self.search_worker.results_ready.connect(self.on_search_results)
        self.search_worker.failed.connect(self.on_search_failed)
        self.search_worker.finished.connect(self.on_search_thread_finished)
        self.search_worker.start()

    def on_search_results(self, windows: list[WindowInfo]) -> None:
        self.tree.blockSignals(True)
        self.tree.clear()
        for info in windows:
            item = QTreeWidgetItem([
                "",
                info.title,
                info.process or "?",
                info.class_name,
                info.size_text(),
            ])
            item.setFlags(item.flags() | Qt.ItemIsUserCheckable)
            item.setCheckState(0, Qt.Unchecked)
            item.setData(0, Qt.UserRole, info)
            item.setToolTip(1, info.title)
            item.setToolTip(2, info.process)
            item.setToolTip(3, info.class_name)
            if info.minimized:
                item.setToolTip(4, "窗口已最小化，截图前会自动还原")
            self.tree.addTopLevelItem(item)
        self.tree.blockSignals(False)
        QTimer.singleShot(0, self._apply_column_widths)

        self.set_busy(False)
        self.statusBar().showMessage(f"找到 {len(windows)} 个可见窗口")
        self.append_log(f"窗口列表已刷新：{len(windows)} 个")
        self.update_capture_button()

    def on_search_failed(self, message: str) -> None:
        self.set_busy(False)
        self.statusBar().showMessage("窗口枚举失败")
        self.append_log(message)
        QMessageBox.critical(self, "错误", message)

    def on_search_thread_finished(self) -> None:
        if self.search_worker is not None:
            self.search_worker.deleteLater()
            self.search_worker = None

    def set_all_checked(self, checked: bool) -> None:
        state = Qt.Checked if checked else Qt.Unchecked
        self.tree.blockSignals(True)
        for index in range(self.tree.topLevelItemCount()):
            self.tree.topLevelItem(index).setCheckState(0, state)
        self.tree.blockSignals(False)
        self.update_capture_button()

    def get_checked_windows(self) -> list[WindowInfo]:
        result: list[WindowInfo] = []
        for index in range(self.tree.topLevelItemCount()):
            item = self.tree.topLevelItem(index)
            if item.checkState(0) == Qt.Checked:
                info = item.data(0, Qt.UserRole)
                if info is not None:
                    result.append(info)
        return result

    def update_capture_button(self, *_args) -> None:
        enabled = (not self._busy) and bool(self.get_checked_windows())
        self.btn_capture.setEnabled(enabled)

    # -- 截图流程 ----------------------------------------------------------

    def start_capture(self) -> None:
        if self._busy or self.worker is not None:
            return
        selected = self.get_checked_windows()
        if not selected:
            QMessageBox.warning(self, "提示", "请至少勾选一个窗口。")
            return
        output_dir = self.edit_dir.text().strip()
        if not output_dir:
            QMessageBox.warning(self, "提示", "请设置保存目录。")
            return

        config = CaptureConfig(
            output_dir=output_dir,
            target_width=self.spin_width.value(),
            delay_ms=self.spin_delay.value(),
        )
        self.preview.clear()
        self.log_view.clear()
        self.progress.setValue(0)
        self.set_busy(True)
        self.append_log(f"开始截图：{len(selected)} 个窗口，等待 {config.delay_ms} ms")

        self.worker = ScreenshotWorker(selected, config, self.backdrop)
        self.worker.log_msg.connect(self.append_log)
        self.worker.progress_changed.connect(self.progress.setValue)
        self.worker.image_saved.connect(self.add_preview)
        self.worker.work_finished.connect(self.on_capture_finished)
        self.worker.finished.connect(self.on_worker_thread_finished)

        self.backdrop.set_color(self.bg_color)
        self.hide()
        QTimer.singleShot(200, self.launch_worker)

    def launch_worker(self) -> None:
        if self.worker is not None and not self.worker.isRunning():
            self.worker.start()

    def on_capture_finished(self, success: bool, message: str, ok: int, total: int) -> None:
        self.backdrop.hide()
        self.set_busy(False)
        self.progress.setValue(100 if ok > 0 else 0)
        self.statusBar().showMessage(message, 15000)
        self.append_log(message)
        self.show()
        self.raise_()
        if not success:
            QMessageBox.warning(self, "截图未完成", message)

    def on_worker_thread_finished(self) -> None:
        worker = self.worker
        self.worker = None
        if worker is not None:
            worker.deleteLater()
        self.update_capture_button()

    def add_preview(self, path: str) -> None:
        pixmap = QPixmap(path)
        if pixmap.isNull():
            return
        icon = QIcon(pixmap.scaled(140, 96, Qt.KeepAspectRatio, Qt.SmoothTransformation))
        item = QListWidgetItem(icon, os.path.basename(path))
        item.setData(Qt.UserRole, path)
        item.setToolTip(path)
        self.preview.addItem(item)
        self.preview.scrollToItem(item)

    def closeEvent(self, event) -> None:
        if self._busy or self.worker is not None or self.search_worker is not None:
            QMessageBox.information(self, "提示", "任务正在执行，请稍候再关闭。")
            event.ignore()
            return
        self.backdrop.hide()
        event.accept()


def main() -> None:
    enable_dpi_awareness()
    try:
        QApplication.setAttribute(Qt.AA_EnableHighDpiScaling, True)
        QApplication.setAttribute(Qt.AA_UseHighDpiPixmaps, True)
    except AttributeError:
        pass

    app = QApplication(sys.argv)
    app.setFont(QFont("Microsoft YaHei UI", 9))
    window = MainWindow()
    window.show()
    sys.exit(app.exec_())


if __name__ == "__main__":
    main()
