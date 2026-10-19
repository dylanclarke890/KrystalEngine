"""Captures Chrome's rendering of golden scenes: Render2D's fidelity goldens, or Krystal.UI's page goldens.

Each scene in <golden>/scenes/<name>.html is rendered by headless Chrome at a device pixel ratio of 1, in sRGB,
with software rasterisation, into <golden>/<name>.chrome.png. The golden directory is Krystal.Gfx.Tests/golden
unless --golden names another (Krystal.UI.Tests/golden for the page goldens). The viewport's size comes from
the scene's <html data-width="..." data-height="...">.

Usage: python tools/golden/capture_chrome.py [scene names...] [--golden DIR] [--chrome PATH]
"""

import argparse
import pathlib
import re
import shutil
import subprocess
import sys
import tempfile

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
    with tempfile.TemporaryDirectory() as profile:
        subprocess.run(
            [
                str(chrome),
                "--headless=new",
                "--disable-gpu",
                "--no-first-run",
                "--hide-scrollbars",
                "--force-device-scale-factor=1",
                "--force-color-profile=srgb",
                # Scenes load fonts from the repository by relative file URLs, and wait for them.
                "--allow-file-access-from-files",
                "--virtual-time-budget=5000",
                f"--user-data-dir={profile}",
                f"--window-size={width},{height}",
                f"--screenshot={output}",
                scene.as_uri(),
            ],
            check=True,
            capture_output=True,
        )
    print(f"{scene.stem}: {output.relative_to(ROOT)} ({width}x{height})")


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
        capture(chrome, golden, scene)


if __name__ == "__main__":
    main()
