#ifndef UI_CORE_WINDOW_ROOT_H_
#define UI_CORE_WINDOW_ROOT_H_

#include "duilib/Core/ControlPtrT.h"
#include "duilib/Core/Shadow.h"
#include "duilib/Core/UiTypes.h"
#include "duilib/Utils/FilePath.h"

namespace ui
{

class Box;
class Control;
class Window;
class FullscreenBox;
class ControlFinder;
class DpiManager;

/** 窗口根节点管理器类
*
*  负责管理窗口的根容器、窗口阴影以及控件全屏状态的核心组件
*
*  该类封装了Window类中与根节点(root)、阴影(shadow)和控件全屏(control fullscreen)相关的
*  所有成员变量和业务逻辑，用于减少Window类的代码复杂度，提高代码的可维护性
*
*  主要功能包括:
*  - 窗口根容器的管理（AttachBox、GetRoot、GetXmlRoot等）
*  - 窗口阴影的管理（创建、销毁、属性设置等）
*  - 控件全屏功能的管理（进入、退出全屏状态）
*  - 窗口最大化/还原时的边距调整
*
*  使用方式:
*  Window类持有WindowRoot的智能指针，通过WindowRoot间接管理上述功能
*/
class DUILIB_API WindowRoot
{
public:
    /** 构造函数
    * @param [in] pWindow 关联的窗口指针，不能为空
    */
    explicit WindowRoot(Window* pWindow);

    /** 禁用拷贝构造函数
    */
    WindowRoot(const WindowRoot& r) = delete;

    /** 禁用赋值运算符
    */
    WindowRoot& operator=(const WindowRoot& r) = delete;

    /** 析构函数
    *  负责清理根容器和阴影资源
    */
    ~WindowRoot();

public:
    /** 获取窗口的根容器
    * @return 返回窗口根容器的指针，可能是阴影容器（当AttachShadow被调用后），
    *         也可能是XML配置的原始Box容器
    */
    Box* GetRoot() const;

    /** 获取XML配置的原始根容器
    * @return 返回XML文件中配置的Box容器，不包含阴影容器
    */
    Box* GetXmlRoot() const;

    /** 绑定窗口的根容器
    * @param [in] pRoot 要绑定的根容器指针
    * @return 绑定成功返回true，否则返回false
    * @note 会先清理旧的根容器，然后设置新的根容器，并更新控件查找器
    */
    bool AttachBox(Box* pRoot);

public:
    /** 获取窗口阴影对象指针
    * @return 返回阴影对象的指针
    * @note 当处于控件全屏状态时，如果根容器是FullscreenBox类型，返回nullptr
    */
    Shadow* GetShadow() const;

    /** 创建窗口阴影对象
    * @param [in] bLayeredWindow 是否为分层窗口
    *         true - 分层窗口（如透明窗口）
    *         false - 普通窗口
    * @note 如果阴影已存在则不会重复创建
    */
    void CreateShadow(bool bLayeredWindow);

public:
    /** 判断当前是否处于控件全屏状态
    * @return 处于控件全屏状态返回true，否则返回false
    */
    bool IsControlFullscreen() const;

public:
    /** 设置要全屏显示的控件
    * @param [in] pFullscreenControl 需要全屏显示的控件指针
    * @param [in] exitButtonClass 退出全屏按钮的Class名称，如果为空则不显示退出按钮
    * @return 设置成功返回true，失败返回false
    * @note 全屏后可以通过ExitControlFullscreen()函数退出全屏状态
    */
    bool SetFullscreenControl(Control* pFullscreenControl, const DString& exitButtonClass);

    /** 获取当前全屏显示的控件
    * @return 返回当前全屏显示的控件指针，如果没有则返回nullptr
    */
    Control* GetFullscreenControl() const;

    /** 退出控件的全屏显示状态
    *  恢复到控件全屏前的窗口状态
    */
    void ExitControlFullscreen();

public:
    /** 处理窗口最大化事件, 当窗口从正常状态或最小化状态切换到最大化状态时调用
    */
    void ProcessWindowMaximized();

    /** 处理窗口还原事件, 当窗口从最大化状态切换到正常状态时调用
    */
    void ProcessWindowRestored();

    /** 处理窗口大小变化事件
    */
    void ProcessWindowResized();

    /** 处理窗口进入全屏状态事件, 当窗口进入全屏模式时调用 
    */
    void ProcessWindowEnterFullscreen();

    /** 处理窗口退出全屏状态事件, 当窗口退出全屏模式时调用
    */
    void ProcessWindowExitFullscreen();

    /** 处理全屏按钮的鼠标移动事件
    * @param [in] pt 当前鼠标位置，客户区坐标
    * @note 用于实现全屏按钮的动态显示/隐藏效果
    */
    void ProcessFullscreenButtonMouseMove(const UiPoint& pt);

public:
    /** 将阴影附加到窗口的根容器
    * @param [in] pXmlRoot XML配置的原始根容器
    * @return 如果阴影已附加返回阴影Box，否则返回原始根容器
    */
    Box* AttachShadow(Box* pXmlRoot);

    /** 设置是否附加阴影效果
    * @param [in] bShadowAttached true-启用阴影，false-禁用阴影
    * @note 设置后会自动调用ProcessWindowShadowTypeChanged处理绑定逻辑
    */
    void SetShadowAttached(bool bShadowAttached);

    /** 获取是否已附加阴影效果
    * @return 已附加阴影返回true，否则返回false
    */
    bool IsShadowAttached() const;

    /** 设置阴影类型
    * @param [in] nShadowType 阴影类型
    * @see ShadowType 枚举定义了所有可用的阴影类型
    */
    void SetShadowType(ShadowType nShadowType);

    /** 获取当前阴影类型
    * @return 返回当前设置的阴影类型
    */
    ShadowType GetShadowType() const;

    /** 获取阴影图片属性
    * @return 返回阴影图片的路径字符串
    */
    DString GetShadowImage() const;

    /** 设置阴影图片
    * @param [in] shadowImage 阴影图片的路径
    */
    void SetShadowImage(const DString& shadowImage);

    /** 设置阴影边框大小（未经过DPI缩放）
    * @param [in] nShadowBorderSize 阴影边框大小，单位像素
    */
    void SetShadowBorderSize(int32_t nShadowBorderSize);

    /** 获取阴影边框大小（未经过DPI缩放）
    * @return 返回阴影边框大小，单位像素
    */
    int32_t GetShadowBorderSize() const;

    /** 设置阴影边框颜色
    * @param [in] shadowBorderColor 阴影边框颜色，ARGB格式字符串
    */
    void SetShadowBorderColor(const DString& shadowBorderColor);

    /** 获取阴影边框颜色
    * @return 返回阴影边框颜色字符串
    */
    DString GetShadowBorderColor() const;

    /** 获取当前的阴影九宫格属性（已做过DPI缩放）
    * @return 如果阴影未附加或者窗口处于最大化状态，返回空九宫格 UiPadding(0,0,0,0)，
    *         否则返回已缩放的九宫格属性
    */
    UiPadding GetCurrentShadowCorner() const;

    /** 获取已设置的阴影九宫格属性（未经过DPI缩放）
    * @return 返回通过SetShadowCorner设置的九宫格属性
    */
    UiPadding GetShadowCorner() const;

    /** 设置阴影九宫格描述
    * @param [in] rcShadowCorner 阴影图片的九宫格属性，未经DPI缩放
    */
    void SetShadowCorner(const UiPadding& rcShadowCorner);

    /** 设置阴影圆角大小（未经过DPI缩放）
    * @param [in] szBorderRound 阴影的圆角大小
    */
    void SetShadowBorderRound(UiSize szBorderRound);

    /** 获取阴影圆角大小（未经过DPI缩放）
    * @return 返回阴影的圆角大小
    */
    UiSize GetShadowBorderRound() const;

    /** 设置阴影是否支持窗口贴边操作
    * @param [in] bEnable true-启用贴边，false-禁用贴边
    * @note 贴边功能允许窗口吸附在屏幕边缘
    */
    void SetEnableShadowSnap(bool bEnable);

    /** 获取阴影是否支持窗口贴边操作
    * @return 启用贴边返回true，否则返回false
    */
    bool IsEnableShadowSnap() const;

public:
    /** 设置窗口贴边属性
    * @param [in] bLeftSnap 左侧贴边
    * @param [in] bRightSnap 右侧贴边
    * @param [in] bTopSnap 上侧贴边
    * @param [in] bBottomSnap 下侧贴边
    */
    void SetWindowPosSnap(bool bLeftSnap, bool bRightSnap, bool bTopSnap, bool bBottomSnap);

    /** 清理图片缓存
    *  清理根容器及其子控件的图片缓存，释放内存
    * @note 如果存在阴影Box，也会清理阴影Box的图片缓存
    */
    void ClearImageCache();

    /** DPI缩放比例发生变化时，更新相关属性
    * @param [in] dpi DPI管理器引用
    * @param [in] nOldScaleFactor 旧的DPI缩放百分比
    * @param [in] nNewScaleFactor 新的DPI缩放百分比
    */
    void ChangeDpiScale(const DpiManager& dpi, uint32_t nOldScaleFactor, uint32_t nNewScaleFactor);

public:
    /** 设置控件查找器指针
    * @param [in] pControlFinder 控件查找器指针
    * @note 用于在AttachBox时自动更新控件查找器的根节点
    */
    void SetControlFinder(ControlFinder* pControlFinder);

    /** 释放所有资源
    */
    void Clear();

private:
    /** 处理窗口阴影类型变化事件, 当窗口阴影类型发生改变时被调用
    */
    void ProcessWindowShadowTypeChanged();

    /** 进入控件全屏模式（内部函数）
    * @param [in] pFullscreenControl 需要全屏的控件
    * @param [in] exitButtonClass 退出全屏按钮的Class名称
    * @return 成功返回true，失败返回false
    * @note 负责创建FullscreenBox并完成全屏设置
    */
    bool EnterControlFullscreen(Control* pFullscreenControl, const DString& exitButtonClass);

    /** 获取窗口最大化时的外边距（内部函数）
    * @note 根据当前窗口大小和屏幕工作区计算并设置外边距
    */
    UiMargin GetWindowMaximizedMargin() const;

    /** 清理根容器
    *  删除并清空当前的根容器指针
    */
    void ClearRoot();

    /** 清理阴影对象
    *  重置并销毁阴影对象
    */
    void ClearShadow();

    /** 更新系统阴影边框/窗口边框/全屏/最大化时对应的外边距，避免内容被遮盖
    */
    void UpdateXmlRootMargin();

private:
    /** 关联的窗口指针
    */
    Window* m_pWindow;

    /** 窗口根容器指针
    *  可能是XML配置的原始Box，也可能被阴影Box包装
    */
    BoxPtr m_pRoot;

    /** 窗口阴影对象
    *  管理窗口的阴影效果
    */
    std::unique_ptr<Shadow> m_shadow;

    /** 当前是否处于控件全屏状态
    *  true-处于控件全屏状态，false-普通状态
    */
    bool m_bControlFullscreen;

    /** 根容器的原始Margin值
    */
    UiMargin m_rcXmlRootMargin;

    /** 控件查找器指针
    *  用于快速查找控件
    */
    ControlFinder* m_pControlFinder;
};

} // namespace ui

#endif // UI_CORE_WINDOW_ROOT_H_
