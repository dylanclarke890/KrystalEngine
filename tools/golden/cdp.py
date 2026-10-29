"""Drives headless Chrome through the DevTools protocol, for scenes that record what real input does.

A scene lists its input in a <script type="text/x-actions"> block, one action a line:

    click X Y             a left click
    shiftclick X Y        a left click with Shift held
    dblclick X Y          a double click
    drag X1 Y1 X2 Y2      press at the first point, move to the second, release there
    type TEXT             text typed, as an input method commits it (the rest of the line)
    key NAME [shift]      a key pressed and released: Backspace, Delete, ArrowLeft, ArrowRight, Home or End

and defines window.recordState(), which returns a line describing the page after each action. Each action is
sent to Chrome as real input, and the record is a line per action: "<action> => <state>".

The client is the standard library only: a WebSocket with no extensions, enough for the protocol's JSON.
"""

import base64
import http.client
import json
import os
import pathlib
import socket
import struct
import subprocess
import tempfile
import time
import urllib.parse

KEYS = {
    "Backspace": ("Backspace", 8),
    "Delete": ("Delete", 46),
    "ArrowLeft": ("ArrowLeft", 37),
    "ArrowRight": ("ArrowRight", 39),
    "Home": ("Home", 36),
    "End": ("End", 35),
}


class WebSocket:
    def __init__(self, url):
        parts = urllib.parse.urlparse(url)
        self.sock = socket.create_connection((parts.hostname, parts.port))
        key = base64.b64encode(os.urandom(16)).decode()
        request = (
            f"GET {parts.path} HTTP/1.1\r\nHost: {parts.hostname}:{parts.port}\r\nUpgrade: websocket\r\n"
            f"Connection: Upgrade\r\nSec-WebSocket-Key: {key}\r\nSec-WebSocket-Version: 13\r\n\r\n"
        )
        self.sock.sendall(request.encode())
        response = b""
        while b"\r\n\r\n" not in response:
            chunk = self.sock.recv(4096)
            if not chunk:
                raise RuntimeError("The DevTools socket closed during the handshake.")
            response += chunk
        if b" 101 " not in response.split(b"\r\n", 1)[0]:
            raise RuntimeError(f"The DevTools socket refused the upgrade: {response[:200]!r}")
        self.buffer = response.split(b"\r\n\r\n", 1)[1]

    def _read(self, count):
        while len(self.buffer) < count:
            chunk = self.sock.recv(65536)
            if not chunk:
                raise RuntimeError("The DevTools socket closed.")
            self.buffer += chunk
        data, self.buffer = self.buffer[:count], self.buffer[count:]
        return data

    def send(self, text):
        payload = text.encode()
        header = bytearray([0x81])
        if len(payload) < 126:
            header.append(0x80 | len(payload))
        elif len(payload) < 65536:
            header.append(0x80 | 126)
            header += struct.pack(">H", len(payload))
        else:
            header.append(0x80 | 127)
            header += struct.pack(">Q", len(payload))
        mask = os.urandom(4)
        self.sock.sendall(bytes(header) + mask + bytes(b ^ mask[i % 4] for i, b in enumerate(payload)))

    def receive(self):
        message = b""
        while True:
            first, second = self._read(2)
            length = second & 0x7F
            if length == 126:
                length = struct.unpack(">H", self._read(2))[0]
            elif length == 127:
                length = struct.unpack(">Q", self._read(8))[0]
            payload = self._read(length)
            opcode = first & 0x0F
            if opcode == 0x8:
                raise RuntimeError("The DevTools socket closed.")
            if opcode in (0x9, 0xA):
                continue
            message += payload
            if first & 0x80:
                return message.decode()

    def close(self):
        self.sock.close()


class Chrome:
    """A headless Chrome showing one page, at a viewport of width by height."""

    def __init__(self, chrome, width, height):
        self.profile = tempfile.TemporaryDirectory()
        self.process = subprocess.Popen(
            [
                str(chrome),
                "--headless=new",
                "--disable-gpu",
                "--no-first-run",
                "--hide-scrollbars",
                "--force-device-scale-factor=1",
                "--force-color-profile=srgb",
                "--allow-file-access-from-files",
                "--remote-debugging-port=0",
                f"--user-data-dir={self.profile.name}",
                f"--window-size={width},{height}",
                "about:blank",
            ],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
        )
        port_file = pathlib.Path(self.profile.name) / "DevToolsActivePort"
        for _ in range(200):
            if port_file.exists() and port_file.read_text().strip():
                break
            time.sleep(0.05)
        port = int(port_file.read_text().splitlines()[0])
        connection = http.client.HTTPConnection("127.0.0.1", port)
        connection.request("GET", "/json/list")
        targets = json.loads(connection.getresponse().read())
        page = next(target for target in targets if target["type"] == "page")
        self.socket = WebSocket(page["webSocketDebuggerUrl"])
        self.next_id = 0
        # The window includes room the viewport does not get, so the viewport is set as the scene asks.
        self.call("Emulation.setDeviceMetricsOverride", width=width, height=height, deviceScaleFactor=1, mobile=False)

    def call(self, method, **params):
        self.next_id += 1
        self.socket.send(json.dumps({"id": self.next_id, "method": method, "params": params}))
        while True:
            message = json.loads(self.socket.receive())
            if message.get("id") == self.next_id:
                if "error" in message:
                    raise RuntimeError(f"{method}: {message['error']}")
                return message.get("result", {})

    def evaluate(self, expression):
        result = self.call("Runtime.evaluate", expression=expression, awaitPromise=True, returnByValue=True)
        if "exceptionDetails" in result:
            raise RuntimeError(f"{expression}: {result['exceptionDetails']}")
        return result["result"].get("value")

    def load(self, url):
        self.call("Page.navigate", url=url)
        for _ in range(200):
            if self.evaluate("document.readyState") == "complete":
                break
            time.sleep(0.05)
        self.evaluate("document.fonts.ready.then(() => true)")

    def mouse(self, kind, x, y, click_count=0, modifiers=0, buttons=0):
        params = {"type": kind, "x": x, "y": y, "modifiers": modifiers, "buttons": buttons}
        params.update(button="left" if kind != "mouseMoved" or buttons else "none")
        if kind != "mouseMoved":
            params.update(clickCount=click_count)
        self.call("Input.dispatchMouseEvent", **params)

    def click(self, x, y, count=1, modifiers=0):
        for clicks in range(1, count + 1):
            self.mouse("mousePressed", x, y, clicks, modifiers, buttons=1)
            self.mouse("mouseReleased", x, y, clicks, modifiers)

    def key(self, name, modifiers=0):
        key, code = KEYS[name]
        for kind in ("rawKeyDown", "keyUp"):
            self.call("Input.dispatchKeyEvent", type=kind, key=key, code=key, windowsVirtualKeyCode=code,
                      nativeVirtualKeyCode=code, modifiers=modifiers)

    def close(self):
        self.socket.close()
        self.process.kill()
        self.process.wait()
        self.profile.cleanup()


def actions_of(html):
    start = html.find('<script type="text/x-actions">')
    if start < 0:
        return []
    start = html.index(">", start) + 1
    end = html.index("</script>", start)
    return [line.strip() for line in html[start:end].splitlines() if line.strip()]


def run_actions(chrome_path, scene, width, height):
    """The record of the scene's actions in Chrome: a line per action, "<action> => <state>"."""
    html = scene.read_text(encoding="utf-8-sig")
    chrome = Chrome(chrome_path, width, height)
    try:
        chrome.load(scene.as_uri())
        lines = []
        for action in actions_of(html):
            verb, _, rest = action.partition(" ")
            args = rest.split()
            if verb in ("click", "shiftclick", "dblclick"):
                x, y = float(args[0]), float(args[1])
                chrome.click(x, y, 2 if verb == "dblclick" else 1, 8 if verb == "shiftclick" else 0)
            elif verb == "drag":
                x1, y1, x2, y2 = map(float, args)
                chrome.mouse("mousePressed", x1, y1, 1, buttons=1)
                for step in range(1, 5):
                    chrome.mouse("mouseMoved", x1 + (x2 - x1) * step / 4, y1 + (y2 - y1) * step / 4, buttons=1)
                chrome.mouse("mouseReleased", x2, y2, 1)
            elif verb == "type":
                chrome.call("Input.insertText", text=rest)
            elif verb == "key":
                chrome.key(args[0], 8 if "shift" in args[1:] else 0)
            else:
                raise RuntimeError(f"{scene.stem}: unknown action {action!r}")
            lines.append(f"{action} => {chrome.evaluate('recordState()')}")
        return lines
    finally:
        chrome.close()
