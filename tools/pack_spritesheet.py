from __future__ import annotations

import argparse
from pathlib import Path
import re

from PIL import Image


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Pack loose frames into a horizontal sprite sheet.")
    parser.add_argument("--input-dir", required=True, help="Directory containing source frames.")
    parser.add_argument("--pattern", required=True, help="Glob pattern, e.g. boom1_*.bmp")
    parser.add_argument("--output-image", required=True, help="Output sprite sheet path.")
    parser.add_argument("--output-meta", required=True, help="Output metadata path.")
    parser.add_argument("--trim", action="store_true", help="Trim transparent borders before packing.")
    return parser.parse_args()


def trim_image(image: Image.Image) -> Image.Image:
    bbox = image.getbbox()
    if bbox is None:
        return image
    return image.crop(bbox)


def natural_sort_key(path: Path) -> list[object]:
    parts = re.split(r"(\d+)", path.stem.lower())
    key: list[object] = []
    for part in parts:
        if not part:
            continue
        if part.isdigit():
            key.append(int(part))
        else:
            key.append(part)
    return key


def main() -> int:
    args = parse_args()
    input_dir = Path(args.input_dir)
    output_image = Path(args.output_image)
    output_meta = Path(args.output_meta)

    frame_paths = sorted(input_dir.glob(args.pattern), key=natural_sort_key)
    if not frame_paths:
        raise SystemExit(f"No frames matched: {input_dir / args.pattern}")

    frames = []
    for path in frame_paths:
        image = Image.open(path).convert("RGBA")
        if args.trim:
            image = trim_image(image)
        frames.append(image)

    cell_width = max(frame.width for frame in frames)
    cell_height = max(frame.height for frame in frames)
    frame_count = len(frames)
    sheet = Image.new("RGBA", (cell_width * frame_count, cell_height), (255, 255, 255, 0))

    for index, frame in enumerate(frames):
        offset_x = index * cell_width + (cell_width - frame.width) // 2
        offset_y = (cell_height - frame.height) // 2
        sheet.alpha_composite(frame, (offset_x, offset_y))

    output_image.parent.mkdir(parents=True, exist_ok=True)
    output_meta.parent.mkdir(parents=True, exist_ok=True)
    sheet.save(output_image)

    meta_text = "\n".join(
        [
            f"frame_width={cell_width}",
            f"frame_height={cell_height}",
            f"stride_x={cell_width}",
            f"frame_count={frame_count}",
            "",
        ]
    )
    output_meta.write_text(meta_text, encoding="utf-8")

    print(f"Packed {frame_count} frames into {output_image}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
