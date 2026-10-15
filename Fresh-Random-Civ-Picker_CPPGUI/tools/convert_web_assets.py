"""Copies the web version's UI images into this project as lossless PNG files.

The web version (frcp.net, repository frcp_html) keeps its images as WebP. Windows can't be relied on to decode
WebP (the codec is an optional Store extension), so the desktop app ships the same pixels as PNG, which WIC
always decodes. Run after the web version's images change:
    python tools/convert_web_assets.py <path to frcp_html>
Needs Pillow (pip install pillow).

The program is moving to the game's original images. Those it already has (the parchment, the cursors, the slider
and the dialogs' panel) are not listed here, so this never overwrites them. The civ, edition and DLC icons are one
file each; a web icon is only copied where the project has no icon of that name yet, so an original is never
replaced. The web version's atlases are left out: the program builds its own from the icon files at startup.
"""
import sys
from pathlib import Path

from PIL import Image

PROJECT_IMAGES = Path(__file__).resolve().parent.parent / "images"
WEB_IMAGES = [
    "ui/button", "ui/button_pressed", "ui/button_9slice", "ui/button_pressed_9slice", "ui/checkbox",
    "ui/dropdown_button", "ui/external", "ui/log_9slice", "ui/off", "ui/options_button", "ui/scrollbar_3slice",
    "ui/scrollbar_knob", "ui/share_icon", "ui/tabs", "ui/techtree_button", "ui/textfield_9slice", "ui/x_button",
]
WEB_ICON_FOLDERS = ["civ", "dlc"]


def convert(source, target):
    target.parent.mkdir(parents=True, exist_ok=True)
    Image.open(source).convert("RGBA").save(target, optimize=True)
    print(f"{source.name} -> {target.relative_to(PROJECT_IMAGES.parent)}")


def main():
    if len(sys.argv) != 2:
        raise SystemExit("usage: python tools/convert_web_assets.py <path to frcp_html>")
    web_root = Path(sys.argv[1]) / "images"
    for name in WEB_IMAGES:
        convert(web_root / (name + ".webp"), PROJECT_IMAGES / (name + ".png"))
    for folder in WEB_ICON_FOLDERS:
        for source in sorted((web_root / folder).glob("*.webp")):
            target = PROJECT_IMAGES / folder / (source.stem + ".png")
            if source.stem.endswith("_atlas") or target.exists():
                continue
            convert(source, target)


if __name__ == "__main__":
    main()
