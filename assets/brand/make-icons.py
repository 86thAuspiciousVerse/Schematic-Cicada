#!/usr/bin/env python3
"""Regenerate the Schematic-Cicada brand assets (docs/04 §4).

Sources (repo `assets/`):
  白线logo透明底.png   white-line glyph on transparency — the mark + app icon glyph
Outputs (`assets/brand/`, git-tracked):
  cicada-mark.png   transparent white-line glyph, square, 128px (sidebar brand)
  cicada-64.png     app icon tile, 64px (page favicon / window icon)
  cicada.ico        multi-size 16..256 (Windows shortcut + window icon)

The app icon composites the white glyph on a dark rounded tile so it stays
legible on light and dark Windows themes; the sidebar mark stays transparent
because the product sidebar is dark.

Usage: python3 assets/brand/make-icons.py   (run from the repository root)
"""
from pathlib import Path

from PIL import Image, ImageDraw, ImageFilter

ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / 'assets' / '白线logo透明底.png'
OUT = ROOT / 'assets' / 'brand'
TILE = (23, 23, 23, 255)          # near-black tile (matches the product sidebar)
ICO_SIZES = [16, 24, 32, 48, 64, 128, 256]


def glyph() -> Image.Image:
    """The white-line glyph, cropped to its ink and padded to a square."""
    image = Image.open(SOURCE).convert('RGBA')
    ink = image.getbbox()
    image = image.crop(ink)
    side = max(image.size)
    square = Image.new('RGBA', (side, side), (0, 0, 0, 0))
    square.paste(image, ((side - image.width) // 2, (side - image.height) // 2), image)
    return square


def tile(glyph_image: Image.Image, size: int, padding: float = 0.14) -> Image.Image:
    """Composite the glyph on a dark rounded tile (visible on any theme)."""
    canvas = Image.new('RGBA', (size, size), (0, 0, 0, 0))
    draw = ImageDraw.Draw(canvas)
    draw.rounded_rectangle([0, 0, size - 1, size - 1], radius=int(size * 0.22), fill=TILE)
    inner = int(size * (1 - 2 * padding))
    art = glyph_image.resize((inner, inner), Image.LANCZOS)
    art = art.filter(ImageFilter.UnsharpMask(radius=1.2, percent=140, threshold=2))
    canvas.paste(art, ((size - inner) // 2, (size - inner) // 2), art)
    return canvas


def main() -> None:
    OUT.mkdir(parents=True, exist_ok=True)
    art = glyph()
    mark = art.resize((128, 128), Image.LANCZOS).filter(
        ImageFilter.UnsharpMask(radius=1.2, percent=140, threshold=2)
    )
    mark.save(OUT / 'cicada-mark.png')
    icon_frames = [tile(art, size) for size in ICO_SIZES]
    icon_frames[-1].save(OUT / 'cicada-256.png')
    icon_frames[ICO_SIZES.index(64)].save(OUT / 'cicada-64.png')
    icon_frames[-1].save(OUT / 'cicada.ico', format='ICO', sizes=[(s, s) for s in ICO_SIZES])
    for name in ('cicada-mark.png', 'cicada-64.png', 'cicada-256.png', 'cicada.ico'):
        print(f'{name}: {(OUT / name).stat().st_size} bytes')


if __name__ == '__main__':
    main()
