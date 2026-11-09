"""Captures Chrome's rendering of golden scenes: Render2D's fidelity goldens, or Krystal.UI's page goldens.

Each scene in <golden>/scenes/<name>.html is rendered by headless Chrome at a device pixel ratio of 1, in sRGB,
with software rasterisation, into <golden>/<name>.chrome.png. The golden directory is Krystal.Gfx.Tests/golden
unless --golden names another (Krystal.UI.Tests/golden for the page goldens). The viewport's size comes from
the scene's <html data-width="..." data-height="...">.

A scene whose script records something when loaded with "?record" (setting data-record-<kind> on its <html>, to
lines of text) also gets <golden>/<name>.chrome-<kind>.txt for each, read from Chrome's DOM: the client rects
of its elements with ids ("rects", lines of "id x y width height"), say.

A scene with data-hand-captured on its <html> is left alone: it needs something this can't do, which it describes.

A scene that lists input actions (see cdp.py) also gets <golden>/<name>.chrome-input.txt: the actions, sent to
Chrome as real input, each with the state the scene's recordState() gives after it.

Usage: python tools/golden/capture_chrome.py [scene names...] [--golden DIR] [--chrome PATH]
"""

import argparse
import html as html_module
import pathlib
import re
import shutil
import sys

import cdp

ROOT = pathlib.Path(__file__).resolve().parents[2]
DEFAULT_GOLDEN = ROOT / "Krystal.Gfx.Tests" / "golden"
CHROME_CANDIDATES = [
    pathlib.Path(r"C:\Program Files\Google\Chrome\Application\chrome.exe"),
    pathlib.Path(r"C:\Program Files (x86)\Google\Chrome\Application\chrome.exe"),
]


def find_chrome(explicit):
    if explicit:
        return pathlib.Path(explicit)
    for candidate in CHROME_CANDIDATES:
        if candidate.exists():
            return candidate
    found = shutil.which("chrome") or shutil.which("google-chrome")
    if found:
        return pathlib.Path(found)
    sys.exit("Chrome wasn't found; pass --chrome.")


def scene_size(html):
    width = re.search(r'data-width="(\d+)"', html)
    height = re.search(r'data-height="(\d+)"', html)
    if not width or not height:
        sys.exit("A scene needs data-width and data-height on its <html>.")
    return int(width.group(1)), int(height.group(1))


def capture(chrome, golden, scene):
    width, height = scene_size(scene.read_text(encoding="utf-8-sig"))
    output = golden / f"{scene.stem}.chrome.png"
    # Through DevTools rather than --screenshot, whose --window-size isn't the viewport: under 516 wide the viewport is
    # 500, above that it's 16 narrower, and it's always about 90 shorter, with the image then cropped or padded to size.
    browser = cdp.Chrome(chrome, width, height)
    try:
        browser.load_settled(scene.as_uri())
        output.write_bytes(browser.screenshot())
    finally:
        browser.close()
    print(f"{scene.stem}: {output.relative_to(ROOT)} ({width}x{height})")


def capture_records(chrome, golden, scene):
    width, height = scene_size(scene.read_text(encoding="utf-8-sig"))
    browser = cdp.Chrome(chrome, width, height)
    try:
        browser.load_settled(scene.as_uri() + "?record")
        dom = browser.evaluate("document.documentElement.outerHTML")
    finally:
        browser.close()
    records = re.findall(r'data-record-([a-z]+)="([^"]*)"', dom)
    if not records:
        sys.exit(f"{scene.stem}: Chrome's DOM has no data-record-* attributes.")
    for kind, value in records:
        output = golden / f"{scene.stem}.chrome-{kind}.txt"
        text = html_module.unescape(value).strip()
        output.write_text(text + "\n", encoding="utf-8", newline="\n")
        print(f"{scene.stem}: {output.relative_to(ROOT)} ({len(text.splitlines())} lines)")


def capture_input(chrome, golden, scene):
    width, height = scene_size(scene.read_text(encoding="utf-8-sig"))
    lines = cdp.run_actions(chrome, scene, width, height)
    output = golden / f"{scene.stem}.chrome-input.txt"
    output.write_text("\n".join(lines) + "\n", encoding="utf-8", newline="\n")
    print(f"{scene.stem}: {output.relative_to(ROOT)} ({len(lines)} actions)")


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("scenes", nargs="*", help="scene names (default: all)")
    parser.add_argument("--golden", help="the golden directory (default: Krystal.Gfx.Tests/golden)")
    parser.add_argument("--chrome", help="the Chrome executable")
    args = parser.parse_args()

    chrome = find_chrome(args.chrome)
    golden = (ROOT / args.golden).resolve() if args.golden else DEFAULT_GOLDEN
    scenes = golden / "scenes"
    names = args.scenes or sorted(path.stem for path in scenes.glob("*.html"))
    for name in names:
        scene = scenes / f"{name}.html"
        if not scene.exists():
            sys.exit(f"No scene {scene.relative_to(ROOT)}.")
        html = scene.read_text(encoding="utf-8-sig")
        if "data-hand-captured" in html:
            print(f"{name}: left alone, as its Chrome golden is captured by hand")
            continue
        capture(chrome, golden, scene)
        if '"?record"' in html:
            capture_records(chrome, golden, scene)
        if cdp.actions_of(html):
            capture_input(chrome, golden, scene)


if __name__ == "__main__":
    main()
