#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""CEF JS 桥探针（DevTools 协议，无第三方依赖）。

用于验证 `NimCefWebInstance.call/register`（CefRegisterExtension 移除后的替代实现）
在运行时确实可用。示例：

    Start-Process .\\bin\\cef.exe -ArgumentList '--remote-debugging-port=9335'
    python cef_bridge_probe.py --port 9335 inspect
    python cef_bridge_probe.py --port 9335 invoke
    python cef_bridge_probe.py --port 9335 read

`invoke` 会在页面里调用 NimCefWebInstance.call('ShowMessageBox', ...)，
C++ 侧弹出模态框并阻塞 UI 线程；此时需要关闭该模态框，`read` 才能读到回调结果
（window.__r）。可用 EnumWindows + WM_CLOSE(0x0010) 关闭，见 SKILL.md 第 10 节。

注意：PowerShell 5 没有内置 WebSocket 客户端，这里用裸 socket 实现；
客户端帧必须带掩码（mask），否则服务端会在握手后直接断开。
"""

from __future__ import annotations

import argparse
import json
import socket
import struct
import sys
import time
import urllib.request

MASK = b"\x11\x22\x33\x44"


def fetch_targets(port: int) -> list:
    with urllib.request.urlopen("http://127.0.0.1:%d/json" % port, timeout=5) as resp:
        return json.load(resp)


def find_target(port: int, suffix: str) -> dict:
    targets = fetch_targets(port)
    for item in targets:
        if item.get("url", "").endswith(suffix):
            return item
    print("target not found (suffix=%s); available:" % suffix)
    for item in targets:
        print("  %s  %s" % (item.get("type"), item.get("url")))
    sys.exit(1)


class DevToolsSocket:
    def __init__(self, port: int, path: str) -> None:
        self.sock = socket.create_connection(("127.0.0.1", port), timeout=10)
        self.sock.sendall((
            "GET /devtools%s HTTP/1.1\r\nHost: 127.0.0.1:%d\r\n"
            "Upgrade: websocket\r\nConnection: Upgrade\r\n"
            "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"
            "Sec-WebSocket-Version: 13\r\n\r\n" % (path, port)).encode())
        response = self.sock.recv(4096)
        if b"101" not in response:
            raise RuntimeError("websocket handshake failed: %r" % response[:200])

    def send_frame(self, opcode: int, payload: bytes) -> None:
        header = bytes([0x80 | opcode])
        length = len(payload)
        if length < 126:
            header += struct.pack("!B", 0x80 | length)
        elif length < 65536:
            header += struct.pack("!BH", 0x80 | 126, length)
        else:
            header += struct.pack("!BQ", 0x80 | 127, length)
        masked = bytes(b ^ MASK[i % 4] for i, b in enumerate(payload))
        self.sock.sendall(header + MASK + masked)

    def _recv_exact(self, count: int) -> bytes:
        data = b""
        while len(data) < count:
            chunk = self.sock.recv(count - len(data))
            if not chunk:
                raise ConnectionError("websocket closed")
            data += chunk
        return data

    def recv_frame(self) -> tuple:
        first = self._recv_exact(2)
        opcode = first[0] & 0x0F
        length = first[1] & 0x7F
        if length == 126:
            length = struct.unpack("!H", self._recv_exact(2))[0]
        elif length == 127:
            length = struct.unpack("!Q", self._recv_exact(8))[0]
        payload = self._recv_exact(length) if length else b""
        if opcode == 9:  # ping -> pong
            self.send_frame(10, payload)
            return self.recv_frame()
        return opcode, payload

    def evaluate(self, expression: str, msg_id: int):
        self.send_frame(1, json.dumps({
            "id": msg_id,
            "method": "Runtime.evaluate",
            "params": {"expression": expression, "returnByValue": True},
        }).encode())
        deadline = time.time() + 10
        while time.time() < deadline:
            try:
                opcode, payload = self.recv_frame()
            except (ConnectionError, socket.timeout):
                return None
            if opcode != 1:
                continue
            message = json.loads(payload.decode("utf-8", "replace"))
            if message.get("id") == msg_id:
                result = message.get("result", {}).get("result", {})
                if result.get("subtype") == "error":
                    return "evaluate error: %s" % result.get("description")
                return result.get("value")
        return None


def main() -> int:
    parser = argparse.ArgumentParser(description="CEF JS bridge probe (DevTools protocol)")
    parser.add_argument("--port", type=int, default=9335, help="remote debugging port")
    parser.add_argument("--url-suffix", default="cef.html", help="page url suffix")
    parser.add_argument("--function", default="ShowMessageBox", help="native function name")
    parser.add_argument("--message", default="cef-js-bridge-ok", help="message passed to the function")
    parser.add_argument("mode", choices=("inspect", "invoke", "read"),
                        help="inspect: check bindings; invoke: call native and wait for callback; "
                             "read: read window.__r")
    args = parser.parse_args()

    target = find_target(args.port, args.url_suffix)
    ws_path = target["webSocketDebuggerUrl"].split("/devtools", 1)[1]
    client = DevToolsSocket(args.port, ws_path)

    if args.mode == "inspect":
        checks = (
            ("typeof NimCefWebInstance", "typeof NimCefWebInstance"),
            ("typeof NimCefWebInstance.call", "typeof (NimCefWebInstance && NimCefWebInstance.call)"),
            ("typeof __nimCefNativeCall", "typeof __nimCefNativeCall"),
            ("typeof __nimCefNativeRegister", "typeof __nimCefNativeRegister"),
        )
        for index, (label, expr) in enumerate(checks, start=1):
            print("%s = %s" % (label, client.evaluate(expr, index)))
        return 0

    if args.mode == "invoke":
        expression = (
            "(function(){try{NimCefWebInstance.call(%s,{message:%s},function(err,res){"
            "window.__r=JSON.stringify([err,res]);});return 'invoked';}"
            "catch(e){return 'js-error:'+e;}})()"
            % (json.dumps(args.function), json.dumps(args.message)))
        print("invoke ->", client.evaluate(expression, 1))
        print("now close the modal dialog (WM_CLOSE), then run: --port %d read" % args.port)
        return 0

    print("window.__r ->", client.evaluate("window.__r", 1))
    return 0


if __name__ == "__main__":
    sys.exit(main())
