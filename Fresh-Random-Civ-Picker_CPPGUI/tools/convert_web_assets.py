"""Copies the web version's UI images into this project as lossless PNG files.

The web version (frcp.net, repository frcp_html) keeps its images as WebP. Windows can't be relied on to decode
WebP (the codec is an optional Store extension), so the desktop app ships the same pixels as PNG, which WIC
always decodes. Run after the web version's images change:
    python tools/convert_web_assets.py <path to frcp_html>
Needs Pillow (pip install pillow).
"""
import sys
from pathlib import Path

from PIL import Image

PROJECT_IMAGES = Path(__file__).resolve().parent.parent / "images"
WEB_IMAGES = [
    "ui/background", "ui/button", "ui/button_pressed", "ui/button_9slice", "ui/button_pressed_9slice",
    "ui/checkbox", "ui/cursor", "ui/cursor_pointing", "ui/dropdown_button", "ui/external", "ui/log_9slice",
    "ui/off", "ui/options_button", "ui/panel_9slice", "ui/scrollbar_3slice", "ui/scrollbar_knob", "ui/slider",
    "ui/share_icon", "ui/slider_knob", "ui/tabs", "ui/techtree_button", "ui/textfield_9slice", "ui/x_button",
    "civ/civicon_atlas", "dlc/dlc_atlas", "dlc/edition_banners_atlas",
]


def main():
    if len(sys.argv) != 2:
        raise SystemExit("usage: python tools/convert_web_assets.py <path to frcp_html>")
    web_root = Path(sys.argv[1]) / "images"
    for name in WEB_IMAGES:
        source = web_root / (name + ".webp")
        target = PROJECT_IMAGES / (name + ".png")
        target.parent.mkdir(parents=True, exist_ok=True)
        Image.open(source).convert("RGBA").save(target, optimize=True)
        print(f"{source.name} -> {target.relative_to(PROJECT_IMAGES.parent)}")


if __name__ == "__main__":
    main()
