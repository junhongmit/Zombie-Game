from __future__ import annotations

import argparse
import math
from pathlib import Path
from typing import Iterable

from PIL import Image, ImageDraw


def parse_vec3(text: str) -> tuple[float, float, float]:
    parts = [part.strip() for part in text.split(",")]
    if len(parts) != 3:
        raise argparse.ArgumentTypeError("Expected light direction as x,y,z")
    try:
        x, y, z = (float(part) for part in parts)
    except ValueError as exc:
        raise argparse.ArgumentTypeError("Light direction must contain numeric values") from exc
    return normalize3(x, y, z)


def parse_raw_vec3(text: str) -> tuple[float, float, float]:
    parts = [part.strip() for part in text.split(",")]
    if len(parts) != 3:
        raise argparse.ArgumentTypeError("Expected vector as x,y,z")
    try:
        return tuple(float(part) for part in parts)  # type: ignore[return-value]
    except ValueError as exc:
        raise argparse.ArgumentTypeError("Vector must contain numeric values") from exc


def clamp01(value: float) -> float:
    return max(0.0, min(1.0, value))


def clamp255(value: float) -> int:
    return max(0, min(255, int(round(value))))


def normalize3(x: float, y: float, z: float) -> tuple[float, float, float]:
    length = math.sqrt(x * x + y * y + z * z)
    if length <= 1e-6:
        return (0.0, 0.0, 1.0)
    return (x / length, y / length, z / length)


def dot3(a: tuple[float, float, float], b: tuple[float, float, float]) -> float:
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2]


def subtract3(a: tuple[float, float, float], b: tuple[float, float, float]) -> tuple[float, float, float]:
    return (a[0] - b[0], a[1] - b[1], a[2] - b[2])


def bilinear_sample(values: list[float], width: int, height: int, x: float, y: float) -> float:
    x = max(0.0, min(width - 1.0, x))
    y = max(0.0, min(height - 1.0, y))
    x0 = int(math.floor(x))
    y0 = int(math.floor(y))
    x1 = min(width - 1, x0 + 1)
    y1 = min(height - 1, y0 + 1)
    tx = x - x0
    ty = y - y0

    v00 = values[y0 * width + x0]
    v10 = values[y0 * width + x1]
    v01 = values[y1 * width + x0]
    v11 = values[y1 * width + x1]
    a = v00 * (1.0 - tx) + v10 * tx
    b = v01 * (1.0 - tx) + v11 * tx
    return a * (1.0 - ty) + b * ty


def load_rgba(path: Path) -> Image.Image:
    return Image.open(path).convert("RGBA")


def height_values_from_image(image: Image.Image) -> list[float]:
    values: list[float] = []
    pixels = image.load()
    for y in range(image.height):
        for x in range(image.width):
            r, g, b, a = pixels[x, y]
            luminance = (0.299 * r + 0.587 * g + 0.114 * b) / 255.0
            values.append(luminance * (a / 255.0))
    return values


def alpha_values_from_image(image: Image.Image) -> list[float]:
    values: list[float] = []
    pixels = image.load()
    for y in range(image.height):
        for x in range(image.width):
            values.append(pixels[x, y][3] / 255.0)
    return values


def blur_scalar_field(values: list[float], width: int, height: int, radius: int) -> list[float]:
    if radius <= 0:
        return values[:]

    result = [0.0] * (width * height)
    for y in range(height):
        for x in range(width):
            total = 0.0
            count = 0
            for oy in range(-radius, radius + 1):
                sy = y + oy
                if sy < 0 or sy >= height:
                    continue
                for ox in range(-radius, radius + 1):
                    sx = x + ox
                    if sx < 0 or sx >= width:
                        continue
                    total += values[sy * width + sx]
                    count += 1
            result[y * width + x] = total / max(1, count)
    return result


def resize_scalar_field(values: list[float], width: int, height: int, target_width: int, target_height: int) -> list[float]:
    if width == target_width and height == target_height:
        return values[:]

    image = Image.new("L", (width, height), 0)
    image.putdata([clamp255(value * 255.0) for value in values])
    image = image.resize((target_width, target_height), Image.Resampling.BILINEAR)
    return [pixel / 255.0 for pixel in image.getdata()]


def erode_mask(values: list[float], width: int, height: int, radius: int) -> list[float]:
    if radius <= 0:
        return values[:]

    result = [0.0] * (width * height)
    for y in range(height):
        for x in range(width):
            minimum = 1.0
            for oy in range(-radius, radius + 1):
                sy = y + oy
                if sy < 0 or sy >= height:
                    minimum = 0.0
                    continue
                for ox in range(-radius, radius + 1):
                    sx = x + ox
                    if sx < 0 or sx >= width:
                        minimum = 0.0
                        continue
                    minimum = min(minimum, values[sy * width + sx])
            result[y * width + x] = minimum
    return result


def dilate_scalar_field(values: list[float], width: int, height: int, radius: int) -> list[float]:
    if radius <= 0:
        return values[:]

    result = [0.0] * (width * height)
    for y in range(height):
        for x in range(width):
            maximum = 0.0
            for oy in range(-radius, radius + 1):
                sy = y + oy
                if sy < 0 or sy >= height:
                    continue
                for ox in range(-radius, radius + 1):
                    sx = x + ox
                    if sx < 0 or sx >= width:
                        continue
                    maximum = max(maximum, values[sy * width + sx])
            result[y * width + x] = maximum
    return result


def compute_normals(values: list[float], alpha: list[float], width: int, height: int, height_scale: float) -> list[tuple[float, float, float]]:
    normals: list[tuple[float, float, float]] = [(0.0, 0.0, 1.0)] * (width * height)
    for y in range(height):
        for x in range(width):
            idx = y * width + x
            if alpha[idx] <= 0.0:
                normals[idx] = (0.0, 0.0, 1.0)
                continue
            left = values[y * width + max(0, x - 1)]
            right = values[y * width + min(width - 1, x + 1)]
            up = values[max(0, y - 1) * width + x]
            down = values[min(height - 1, y + 1) * width + x]
            dx = right - left
            dy = down - up
            normals[idx] = normalize3(-dx * height_scale, -dy * height_scale, 1.0)
    return normals


def compute_shadow(
    values: list[float],
    alpha: list[float],
    width: int,
    height: int,
    light_dir_xy: tuple[float, float],
    shadow_steps: int,
    shadow_step: float,
    height_scale: float,
    shadow_bias: float,
) -> list[float]:
    result = [0.0] * (width * height)
    lx, ly = light_dir_xy
    for y in range(height):
        for x in range(width):
            idx = y * width + x
            if alpha[idx] <= 0.0:
                continue
            base_height = values[idx]
            occlusion = 0.0
            for step_index in range(1, shadow_steps + 1):
                sample_x = x + lx * shadow_step * step_index
                sample_y = y + ly * shadow_step * step_index
                sample_alpha = bilinear_sample(alpha, width, height, sample_x, sample_y)
                if sample_alpha <= 0.01:
                    continue
                sample_height = bilinear_sample(values, width, height, sample_x, sample_y)
                threshold = base_height + shadow_bias + (step_index / max(1.0, float(shadow_steps))) * (0.02 / max(0.01, height_scale))
                if sample_height > threshold:
                    occlusion = max(occlusion, clamp01((sample_height - threshold) * height_scale * 1.4))
            result[idx] = clamp01(occlusion)
    return result


def compute_table_anchor_by_column(alpha: list[float], width: int, height: int) -> list[float]:
    anchors = [float(height - 1)] * width
    for x in range(width):
        found = False
        for y in range(height - 1, -1, -1):
            if alpha[y * width + x] > 0.01:
                anchors[x] = float(y)
                found = True
                break
        if not found:
            if x > 0:
                anchors[x] = anchors[x - 1]
            else:
                anchors[x] = float(height - 1)
    return anchors


def compute_table_plane_y(alpha: list[float], width: int, height: int, ground_offset_y: float) -> float:
    bottom_y = 0.0
    found = False
    for y in range(height - 1, -1, -1):
        row_has_alpha = False
        for x in range(width):
            if alpha[y * width + x] > 0.01:
                row_has_alpha = True
                break
        if row_has_alpha:
            bottom_y = float(y)
            found = True
            break
    if not found:
        bottom_y = float(height - 1)
    return bottom_y + ground_offset_y


def compute_shadow_blob_params(
    alpha: list[float],
    heights: list[float],
    width: int,
    height: int,
    support_by_column: list[float],
    ground_offset_y: float,
    table_height_scale: float,
    height_scale: float,
    thickness_scale: float,
    light_dir: tuple[float, float, float],
) -> tuple[float, float, float, float]:
    total_weight = 0.0
    sum_x = 0.0
    sum_y = 0.0
    min_x = float(width)
    max_x = 0.0
    min_support_y = float(height)
    max_support_y = 0.0
    avg_lift = 0.0

    for y in range(height):
        for x in range(width):
            idx = y * width + x
            a = alpha[idx]
            if a <= 0.02:
                continue
            support_y = support_by_column[x] + ground_offset_y
            lift_from_table = max(0.0, support_y - float(y)) * table_height_scale
            thickness = heights[idx] * height_scale * thickness_scale
            point_z = max(0.01, lift_from_table + thickness)
            # Favor the lower half a bit so the blob hugs the table under the gun.
            weight = a * (0.45 + 0.55 * (float(y) / max(1.0, float(height - 1))))
            total_weight += weight
            sum_x += float(x) * weight
            sum_y += support_y * weight
            avg_lift += point_z * weight
            min_x = min(min_x, float(x))
            max_x = max(max_x, float(x))
            min_support_y = min(min_support_y, support_y)
            max_support_y = max(max_support_y, support_y)

    if total_weight <= 1e-5:
        return (width * 0.5, height * 0.8, width * 0.2, height * 0.06)

    cx = sum_x / total_weight
    cy = sum_y / total_weight
    avg_lift /= total_weight
    span_x = max(6.0, max_x - min_x)
    span_y = max(4.0, max_support_y - min_support_y + avg_lift * 0.35)

    dir_x, dir_y, _ = normalize3(light_dir[0], light_dir[1], light_dir[2])
    # Shadow falls opposite the incoming light vector.
    cx -= dir_x * avg_lift * 1.35
    cy -= dir_y * avg_lift * 0.65

    radius_x = span_x * 0.42 + avg_lift * 2.2
    radius_y = span_y * 0.55 + avg_lift * 0.55
    return (cx, cy, radius_x, radius_y)


def compute_directional_table_shadow(
    alpha: list[float],
    heights: list[float],
    width: int,
    height: int,
    support_by_column: list[float],
    light_dir: tuple[float, float, float],
    height_scale: float,
    table_height_scale: float,
    thickness_scale: float,
    shadow_opacity: float,
    shadow_blur_base: float,
    shadow_blur_height_scale: float,
    shadow_stretch_x: float,
    shadow_stretch_y: float,
    ground_offset_y: float,
    mask_erode: int,
    mask_blur: int,
    downsample_factor: int,
) -> list[float]:
    contact_mask = erode_mask(alpha, width, height, mask_erode)
    contact_mask = blur_scalar_field(contact_mask, width, height, mask_blur)
    contact_mask = blur_scalar_field(contact_mask, width, height, 2)
    smooth_heights = blur_scalar_field(heights, width, height, 7)
    table_plane_y = compute_table_plane_y(alpha, width, height, ground_offset_y)

    point_z_field = [0.0] * (width * height)
    support_field = [0.0] * (width * height)
    contact_shadow = [0.0] * (width * height)
    for y in range(height):
        for x in range(width):
            idx = y * width + x
            support_y = table_plane_y
            support_field[idx] = support_y
            alpha_value = contact_mask[idx]
            if alpha_value <= 0.01:
                continue
            lift_from_table = max(0.0, support_y - float(y)) * table_height_scale
            thickness = smooth_heights[idx] * height_scale * thickness_scale
            point_z = max(0.01, lift_from_table + thickness)
            point_z_field[idx] = point_z

            distance_to_plane = max(0.0, support_y - float(y))
            contact_weight = clamp01(1.0 - distance_to_plane / 18.0)
            if contact_weight > 0.01:
                scatter_soft_disk(
                    contact_shadow,
                    width,
                    height,
                    float(x),
                    support_y,
                    max(1.4, shadow_blur_base * 1.2),
                    max(0.8, shadow_blur_base * 0.52),
                    shadow_opacity * alpha_value * 0.36 * contact_weight,
                )

    downsample = max(1, downsample_factor)
    low_width = max(1, width // downsample)
    low_height = max(1, height // downsample)
    alpha_low = resize_scalar_field(contact_mask, width, height, low_width, low_height)
    point_z_low = resize_scalar_field(point_z_field, width, height, low_width, low_height)
    support_low = resize_scalar_field(support_field, width, height, low_width, low_height)
    shadow_low = [0.0] * (low_width * low_height)

    dir_x, dir_y, dir_z = normalize3(light_dir[0], light_dir[1], light_dir[2])
    inv_dir_z = 1.0 / max(0.08, dir_z)
    max_point_z = max(point_z_low, default=1.0)
    if max_point_z <= 1e-5:
        max_point_z = 1.0

    # First lay down a broad mass-shadow blob so the table gets a unified volume shadow.
    total_weight = 0.0
    sum_x = 0.0
    sum_y = 0.0
    sum_z = 0.0
    min_x = float(low_width)
    max_x = 0.0
    min_y = float(low_height)
    max_y = 0.0
    for y in range(low_height):
        for x in range(low_width):
            idx = y * low_width + x
            occ = alpha_low[idx]
            if occ <= 0.03:
                continue
            z = point_z_low[idx]
            height_factor = 1.0 - 0.55 * clamp01(z / max_point_z)
            weight = occ * max(0.15, height_factor)
            total_weight += weight
            sum_x += float(x) * weight
            sum_y += (support_low[idx] / downsample) * weight
            sum_z += z * weight
            min_x = min(min_x, float(x))
            max_x = max(max_x, float(x))
            min_y = min(min_y, support_low[idx] / downsample)
            max_y = max(max_y, support_low[idx] / downsample)

    if total_weight > 1e-5:
        center_x = sum_x / total_weight
        center_y = sum_y / total_weight
        avg_z = sum_z / total_weight
        center_x += dir_x * avg_z * inv_dir_z / downsample
        center_y += dir_y * avg_z * inv_dir_z / downsample
        radius_x = max(3.0, (max_x - min_x) * 0.46 + avg_z * 1.75 / downsample)
        radius_y = max(1.8, (max_y - min_y) * 0.75 + avg_z * 0.42 / downsample)
        scatter_soft_disk(
            shadow_low,
            low_width,
            low_height,
            center_x,
            center_y,
            radius_x * shadow_stretch_x,
            radius_y * shadow_stretch_y,
            shadow_opacity * 0.42,
        )

    # Then project low-frequency mass from each cell. Lower parts cast darker, tighter shadow.
    for y in range(low_height):
        for x in range(low_width):
            idx = y * low_width + x
            occ = alpha_low[idx]
            if occ <= 0.03:
                continue
            z = point_z_low[idx]
            support_y = support_low[idx] / downsample
            shadow_x = float(x) + dir_x * z * inv_dir_z / downsample
            shadow_y = support_y + dir_y * z * inv_dir_z / downsample
            height_norm = clamp01(z / max_point_z)
            opacity = shadow_opacity * occ * (1.05 - 0.72 * height_norm)
            radius_x = max(0.75, (shadow_blur_base + z * shadow_blur_height_scale * 0.55) * shadow_stretch_x / downsample)
            radius_y = max(0.55, (shadow_blur_base * 0.55 + z * shadow_blur_height_scale * 0.16) * shadow_stretch_y / downsample)
            scatter_soft_disk(shadow_low, low_width, low_height, shadow_x, shadow_y, radius_x, radius_y, opacity)

    shadow_low = blur_scalar_field(shadow_low, low_width, low_height, 2)
    shadow_low = dilate_scalar_field(shadow_low, low_width, low_height, 1)
    shadow_high = resize_scalar_field(shadow_low, low_width, low_height, width, height)
    shadow_high = blur_scalar_field(shadow_high, width, height, 2)

    result = [0.0] * (width * height)
    for idx in range(width * height):
        result[idx] = clamp01(max(contact_shadow[idx], shadow_high[idx] * 0.95))
    return result


def scatter_soft_disk(
    field: list[float],
    width: int,
    height: int,
    center_x: float,
    center_y: float,
    radius_x: float,
    radius_y: float,
    opacity: float,
) -> None:
    if opacity <= 0.0 or radius_x <= 0.0 or radius_y <= 0.0:
        return

    min_x = max(0, int(math.floor(center_x - radius_x - 1.0)))
    max_x = min(width - 1, int(math.ceil(center_x + radius_x + 1.0)))
    min_y = max(0, int(math.floor(center_y - radius_y - 1.0)))
    max_y = min(height - 1, int(math.ceil(center_y + radius_y + 1.0)))

    inv_rx = 1.0 / max(1e-5, radius_x)
    inv_ry = 1.0 / max(1e-5, radius_y)
    for py in range(min_y, max_y + 1):
        dy = (py - center_y) * inv_ry
        for px in range(min_x, max_x + 1):
            dx = (px - center_x) * inv_rx
            dist2 = dx * dx + dy * dy
            if dist2 >= 1.0:
                continue
            falloff = math.pow(1.0 - dist2, 1.6)
            idx = py * width + px
            field[idx] = max(field[idx], opacity * falloff)


def compute_table_shadow(
    heights: list[float],
    alpha: list[float],
    width: int,
    height: int,
    light_mode: str,
    light_dir: tuple[float, float, float],
    light_pos: tuple[float, float, float],
    height_scale: float,
    table_height_scale: float,
    thickness_scale: float,
    shadow_opacity: float,
    shadow_blur_base: float,
    shadow_blur_height_scale: float,
    shadow_stretch_x: float,
    shadow_stretch_y: float,
    ground_offset_y: float,
    mask_erode: int,
    mask_blur: int,
    downsample_factor: int,
) -> list[float]:
    support_by_column = compute_table_anchor_by_column(alpha, width, height)
    if light_mode == "directional":
        return compute_directional_table_shadow(
            alpha,
            heights,
            width,
            height,
            support_by_column,
            light_dir,
            height_scale,
            table_height_scale,
            thickness_scale,
            shadow_opacity,
            shadow_blur_base,
            shadow_blur_height_scale,
            shadow_stretch_x,
            shadow_stretch_y,
            ground_offset_y,
            mask_erode,
            mask_blur,
            downsample_factor,
        )

    raw_shadow = [0.0] * (width * height)
    contact_mask = erode_mask(alpha, width, height, mask_erode)
    contact_mask = blur_scalar_field(contact_mask, width, height, mask_blur)
    contact_mask = blur_scalar_field(contact_mask, width, height, 1)
    smooth_heights = blur_scalar_field(heights, width, height, 6)

    blob_cx, blob_cy, blob_rx, blob_ry = compute_shadow_blob_params(
        alpha,
        smooth_heights,
        width,
        height,
        support_by_column,
        ground_offset_y,
        table_height_scale,
        height_scale,
        thickness_scale,
        light_dir,
    )
    scatter_soft_disk(
        raw_shadow,
        width,
        height,
        blob_cx,
        blob_cy,
        blob_rx,
        blob_ry,
        shadow_opacity * 0.6,
    )

    light_x, light_y, light_z = light_pos
    dir_x, dir_y, dir_z = normalize3(light_dir[0], light_dir[1], light_dir[2])
    inv_dir_z = 1.0 / max(0.08, dir_z)
    for y in range(height):
        for x in range(width):
            idx = y * width + x
            alpha_value = contact_mask[idx]
            if alpha_value <= 0.01:
                continue

            support_y = support_by_column[x] + ground_offset_y
            lift_from_table = max(0.0, support_y - float(y)) * table_height_scale
            thickness = smooth_heights[idx] * height_scale * thickness_scale
            point_z = max(0.01, lift_from_table + thickness)

            if light_z <= point_z + 1e-4:
                continue

            # Contact shadow keeps the weapon visually attached to the table.
            if support_y >= float(y) - 0.5:
                contact_opacity = shadow_opacity * alpha_value * 0.42
                scatter_soft_disk(
                    raw_shadow,
                    width,
                    height,
                    float(x),
                    support_y,
                    max(0.8, shadow_blur_base * 0.9),
                    max(0.6, shadow_blur_base * 0.45),
                    contact_opacity,
                )
            if light_mode == "directional":
                shadow_x = float(x) - dir_x * point_z * inv_dir_z
                shadow_y = support_y - dir_y * point_z * inv_dir_z
                blur = shadow_blur_base + point_z * shadow_blur_height_scale * 0.6
            else:
                if light_z <= point_z + 1e-4:
                    continue
                scale = point_z / (light_z - point_z)
                shadow_x = float(x) + (float(x) - light_x) * scale
                shadow_y = support_y + (support_y - light_y) * scale
                blur = shadow_blur_base + point_z * shadow_blur_height_scale
            body_radius_x = max(0.9, blur * shadow_stretch_x * 0.9)
            body_radius_y = max(0.7, blur * shadow_stretch_y * 0.84)
            body_opacity = shadow_opacity * alpha_value * clamp01(0.08 + thickness * 0.10) * (0.35 if light_mode == "directional" else 1.0)
            scatter_soft_disk(raw_shadow, width, height, shadow_x, shadow_y, body_radius_x, body_radius_y, body_opacity)

    downsample = max(1, downsample_factor)
    low_width = max(1, width // downsample)
    low_height = max(1, height // downsample)
    low_shadow = resize_scalar_field(raw_shadow, width, height, low_width, low_height)
    low_shadow = blur_scalar_field(low_shadow, low_width, low_height, 2)
    low_shadow = dilate_scalar_field(low_shadow, low_width, low_height, 1)
    softened_shadow = resize_scalar_field(low_shadow, low_width, low_height, width, height)
    softened_shadow = blur_scalar_field(softened_shadow, width, height, 2)
    return [clamp01(value * 0.9) for value in softened_shadow]


def compute_point_shadow(
    values: list[float],
    alpha: list[float],
    width: int,
    height: int,
    light_pos: tuple[float, float, float],
    shadow_steps: int,
    height_scale: float,
    shadow_bias: float,
) -> list[float]:
    result = [0.0] * (width * height)
    max_dim = float(max(width, height))
    for y in range(height):
        for x in range(width):
            idx = y * width + x
            if alpha[idx] <= 0.0:
                continue

            point_z = values[idx] * height_scale
            point = (float(x), float(y), point_z)
            ray = subtract3(light_pos, point)
            ray_xy_len = math.sqrt(ray[0] * ray[0] + ray[1] * ray[1])
            if ray_xy_len <= 1e-5:
                continue

            occlusion = 0.0
            for step_index in range(1, shadow_steps + 1):
                t = step_index / float(shadow_steps)
                sample_x = point[0] + ray[0] * t
                sample_y = point[1] + ray[1] * t
                sample_alpha = bilinear_sample(alpha, width, height, sample_x, sample_y)
                if sample_alpha <= 0.01:
                    continue

                sample_height = bilinear_sample(values, width, height, sample_x, sample_y) * height_scale
                ray_height = point[2] + ray[2] * t
                if sample_height > ray_height + shadow_bias * max_dim:
                    occlusion = max(
                        occlusion,
                        clamp01((sample_height - ray_height) / max(1.0, 0.12 * max_dim)),
                    )
            result[idx] = occlusion
    return result


def normal_preview_image(normals: Iterable[tuple[float, float, float]], alpha: list[float], size: tuple[int, int]) -> Image.Image:
    width, height = size
    image = Image.new("RGBA", size, (0, 0, 0, 0))
    pixels = []
    for idx, (nx, ny, nz) in enumerate(normals):
        if alpha[idx] <= 0.0:
            pixels.append((0, 0, 0, 0))
            continue
        pixels.append(
            (
                clamp255((nx * 0.5 + 0.5) * 255.0),
                clamp255((ny * 0.5 + 0.5) * 255.0),
                clamp255((nz * 0.5 + 0.5) * 255.0),
                clamp255(alpha[idx] * 255.0),
            )
        )
    image.putdata(pixels)
    return image


def grayscale_preview_image(values: list[float], alpha: list[float], size: tuple[int, int]) -> Image.Image:
    image = Image.new("RGBA", size, (0, 0, 0, 0))
    pixels = []
    for idx, value in enumerate(values):
        intensity = clamp255(value * 255.0)
        pixels.append((intensity, intensity, intensity, clamp255(alpha[idx] * 255.0)))
    image.putdata(pixels)
    return image


def shadow_rgba_image(values: list[float], size: tuple[int, int], tint: tuple[int, int, int] = (0, 0, 0)) -> Image.Image:
    image = Image.new("RGBA", size, (0, 0, 0, 0))
    pixels = []
    for value in values:
        alpha_value = clamp255(value * 255.0)
        pixels.append((tint[0], tint[1], tint[2], alpha_value))
    image.putdata(pixels)
    return image


def paste_scalar_field(
    src: list[float],
    src_width: int,
    src_height: int,
    dst: list[float],
    dst_width: int,
    dst_height: int,
    offset_x: int,
    offset_y: int,
) -> None:
    for y in range(src_height):
        dy = y + offset_y
        if dy < 0 or dy >= dst_height:
            continue
        src_row = y * src_width
        dst_row = dy * dst_width
        for x in range(src_width):
            dx = x + offset_x
            if dx < 0 or dx >= dst_width:
                continue
            dst[dst_row + dx] = src[src_row + x]


def create_canvas_debug_preview(
    albedo: Image.Image,
    height_map: Image.Image,
    light_mode: str,
    light_dir: tuple[float, float, float],
    light_pos: tuple[float, float, float],
    height_scale: float,
    table_height_scale: float,
    thickness_scale: float,
    table_shadow_mode: str,
    table_shadow_opacity: float,
    table_shadow_blur_base: float,
    table_shadow_blur_height_scale: float,
    table_shadow_stretch_x: float,
    table_shadow_stretch_y: float,
    table_shadow_downsample: int,
    mask_erode: int,
    mask_blur: int,
    canvas_scale: float,
) -> Image.Image:
    src_width, src_height = albedo.size
    canvas_width = max(int(round(src_width * canvas_scale)), src_width + 120)
    canvas_height = max(int(round(src_height * canvas_scale)), src_height + 140)
    canvas = Image.new("RGBA", (canvas_width, canvas_height), (255, 255, 255, 255))

    weapon_offset_x = max(24, int(round((canvas_width - src_width) * 0.32)))
    table_plane_y = max(src_height + 18, int(round(canvas_height * 0.74)))

    alpha_src = alpha_values_from_image(albedo)
    heights_src = blur_scalar_field(height_values_from_image(height_map), src_width, src_height, 3)
    alpha_bottom = src_height - 1
    alpha_top = 0
    found_alpha = False
    for y in range(src_height):
        row_has_alpha = False
        for x in range(src_width):
            if alpha_src[y * src_width + x] > 0.01:
                row_has_alpha = True
                found_alpha = True
                break
        if row_has_alpha:
            alpha_top = y
            break
    for y in range(src_height - 1, -1, -1):
        row_has_alpha = False
        for x in range(src_width):
            if alpha_src[y * src_width + x] > 0.01:
                row_has_alpha = True
                alpha_bottom = y
                break
        if row_has_alpha:
            break
    if not found_alpha:
        alpha_top = 0
        alpha_bottom = src_height - 1

    desired_gap = 10
    weapon_offset_y = max(16, table_plane_y - alpha_bottom - desired_gap)
    shadow_mode = light_mode if table_shadow_mode == "match-light" else table_shadow_mode
    low_scale = max(2, table_shadow_downsample * 2)
    low_width = max(1, canvas_width // low_scale)
    low_height = max(1, canvas_height // low_scale)
    low_shadow = [0.0] * (low_width * low_height)
    dir_x, dir_y, dir_z = normalize3(light_dir[0], light_dir[1], light_dir[2])
    inv_dir_z = 1.0 / max(0.08, dir_z)

    # Broad mass blob to anchor the shadow on the tabletop.
    min_x = float(src_width)
    max_x = 0.0
    total_weight = 0.0
    sum_x = 0.0
    sum_z = 0.0
    for y in range(src_height):
        for x in range(src_width):
            idx = y * src_width + x
            a = alpha_src[idx]
            if a <= 0.02:
                continue
            world_y = weapon_offset_y + y
            lift = max(0.0, table_plane_y - world_y) * table_height_scale
            thickness = heights_src[idx] * height_scale * thickness_scale
            point_z = lift + thickness
            weight = a * max(0.18, 1.0 - 0.6 * clamp01(point_z / max(1.0, src_height)))
            total_weight += weight
            sum_x += (weapon_offset_x + x) * weight
            sum_z += point_z * weight
            min_x = min(min_x, float(x))
            max_x = max(max_x, float(x))

    if total_weight > 1e-5:
        blob_x = sum_x / total_weight
        blob_z = sum_z / total_weight
        blob_x += dir_x * blob_z * inv_dir_z * 1.18
        blob_y = table_plane_y + dir_y * blob_z * inv_dir_z
        blob_rx = ((max_x - min_x) * 0.55 + blob_z * 0.8) / low_scale
        blob_ry = (src_height * 0.10 + blob_z * 0.18) / low_scale
        scatter_soft_disk(
            low_shadow,
            low_width,
            low_height,
            blob_x / low_scale,
            blob_y / low_scale,
            max(2.2, blob_rx * table_shadow_stretch_x),
            max(1.1, blob_ry * table_shadow_stretch_y),
            table_shadow_opacity * 0.42,
        )

    # Low-frequency perspective volume shadow.
    for y in range(src_height):
        for x in range(src_width):
            idx = y * src_width + x
            a = alpha_src[idx]
            if a <= 0.03:
                continue
            world_x = weapon_offset_x + x
            world_y = weapon_offset_y + y
            lift = max(0.0, table_plane_y - world_y) * table_height_scale
            thickness = heights_src[idx] * height_scale * thickness_scale
            point_z = lift + thickness
            if shadow_mode == "directional":
                shadow_x = world_x + dir_x * point_z * inv_dir_z
                shadow_y = table_plane_y + dir_y * point_z * inv_dir_z
            else:
                light_x = light_pos[0] * canvas_width
                light_y = light_pos[1] * canvas_height
                light_z = light_pos[2] * float(max(canvas_width, canvas_height))
                if light_z <= point_z + 1e-4:
                    continue
                scale = point_z / (light_z - point_z)
                shadow_x = world_x + (world_x - light_x) * scale
                shadow_y = table_plane_y + (table_plane_y - light_y) * scale
            height_norm = clamp01(lift / max(1.0, src_height))
            opacity = table_shadow_opacity * a * (1.0 - 0.72 * height_norm)
            radius_x = max(0.9, (table_shadow_blur_base + point_z * table_shadow_blur_height_scale * 0.45) * table_shadow_stretch_x / low_scale)
            radius_y = max(0.5, (table_shadow_blur_base * 0.42 + point_z * table_shadow_blur_height_scale * 0.12) * table_shadow_stretch_y / low_scale)
            scatter_soft_disk(
                low_shadow,
                low_width,
                low_height,
                shadow_x / low_scale,
                shadow_y / low_scale,
                radius_x,
                radius_y,
                opacity,
            )

            # Contact shadow: stronger for parts closer to the tabletop.
            contact_weight = clamp01(1.0 - lift / 22.0)
            if contact_weight > 0.01:
                contact_shift_x = 0.0
                if shadow_mode == "directional":
                    # Keep the contact shadow near the tabletop, but nudge it along the light
                    # direction so the square-ish block under the gun does not sit perfectly centered.
                    contact_shift_x = dir_x * max(0.6, lift * 0.38)
                scatter_soft_disk(
                    low_shadow,
                    low_width,
                    low_height,
                    (world_x + contact_shift_x) / low_scale,
                    table_plane_y / low_scale,
                    max(0.9, table_shadow_blur_base * 0.35 / low_scale),
                    max(0.45, table_shadow_blur_base * 0.14 / low_scale),
                    table_shadow_opacity * a * 0.30 * contact_weight,
                )

    low_shadow = blur_scalar_field(low_shadow, low_width, low_height, 2)
    low_shadow = dilate_scalar_field(low_shadow, low_width, low_height, 1)
    shadow_values = resize_scalar_field(low_shadow, low_width, low_height, canvas_width, canvas_height)
    shadow_values = blur_scalar_field(shadow_values, canvas_width, canvas_height, 2)
    shadow_layer = shadow_rgba_image(shadow_values, (canvas_width, canvas_height))
    canvas.alpha_composite(shadow_layer)
    canvas.alpha_composite(albedo, (weapon_offset_x, weapon_offset_y))

    draw = ImageDraw.Draw(canvas)
    draw.line((0, table_plane_y + 0.5, canvas_width, table_plane_y + 0.5), fill=(220, 220, 220, 255), width=1)

    if shadow_mode == "point":
        light_x = light_pos[0] * canvas_width
        light_y = light_pos[1] * canvas_height
        marker_x = min(max(light_x, 12.0), canvas_width - 12.0)
        marker_y = min(max(light_y, 12.0), canvas_height - 12.0)
        outer_radius = max(8, int(round(min(canvas_width, canvas_height) * 0.014)))
        inner_radius = max(3, outer_radius // 3)
        draw.ellipse(
            (marker_x - outer_radius, marker_y - outer_radius, marker_x + outer_radius, marker_y + outer_radius),
            fill=(255, 220, 80, 140),
            outline=(255, 140, 0, 255),
            width=2,
        )
        draw.ellipse(
            (marker_x - inner_radius, marker_y - inner_radius, marker_x + inner_radius, marker_y + inner_radius),
            fill=(255, 245, 210, 255),
        )
        draw.text((min(canvas_width - 60.0, marker_x + outer_radius + 6), max(0.0, marker_y - outer_radius - 4)), "Point", fill=(120, 70, 0, 255))
    else:
        dir_x, dir_y, _ = normalize3(light_dir[0], light_dir[1], light_dir[2])
        start_x = min(canvas_width - 36.0, max(36.0, canvas_width * 0.84))
        start_y = max(28.0, min(canvas_height - 28.0, canvas_height * 0.14))
        arrow_len = max(30.0, min(canvas_width, canvas_height) * 0.08)
        end_x = start_x + dir_x * arrow_len
        end_y = start_y + dir_y * arrow_len
        draw.line((start_x, start_y, end_x, end_y), fill=(255, 140, 0, 255), width=3)
        head_dx = end_x - start_x
        head_dy = end_y - start_y
        head_len = math.sqrt(head_dx * head_dx + head_dy * head_dy)
        if head_len > 1e-5:
            ux = head_dx / head_len
            uy = head_dy / head_len
            px = -uy
            py = ux
            head_size = 9.0
            draw.polygon(
                [
                    (end_x, end_y),
                    (end_x - ux * head_size + px * head_size * 0.6, end_y - uy * head_size + py * head_size * 0.6),
                    (end_x - ux * head_size - px * head_size * 0.6, end_y - uy * head_size - py * head_size * 0.6),
                ],
                fill=(255, 140, 0, 255),
            )
        draw.text((start_x + 8.0, start_y - 20.0), "Directional", fill=(120, 70, 0, 255))

    return canvas


def create_debug_preview(
    albedo: Image.Image,
    table_shadow: Image.Image,
    light_mode: str,
    light_dir: tuple[float, float, float],
    light_pos: tuple[float, float, float],
) -> Image.Image:
    width, height = albedo.size
    preview = Image.new("RGBA", (width, height), (255, 255, 255, 255))
    preview.alpha_composite(table_shadow)
    preview.alpha_composite(albedo)

    if light_mode == "point":
        draw = ImageDraw.Draw(preview)
        light_x = light_pos[0] * width
        light_y = light_pos[1] * height
        marker_x = min(max(light_x, 12.0), width - 12.0)
        marker_y = min(max(light_y, 12.0), height - 12.0)
        outer_radius = max(8, int(round(min(width, height) * 0.018)))
        inner_radius = max(3, outer_radius // 3)
        draw.ellipse(
            (marker_x - outer_radius, marker_y - outer_radius, marker_x + outer_radius, marker_y + outer_radius),
            fill=(255, 220, 80, 140),
            outline=(255, 140, 0, 255),
            width=2,
        )
        draw.ellipse(
            (marker_x - inner_radius, marker_y - inner_radius, marker_x + inner_radius, marker_y + inner_radius),
            fill=(255, 245, 210, 255),
        )
        if abs(marker_x - light_x) > 0.5 or abs(marker_y - light_y) > 0.5:
            draw.line((marker_x, marker_y, min(max(light_x, 0.0), width - 1.0), min(max(light_y, 0.0), height - 1.0)), fill=(255, 140, 0, 255), width=2)
        draw.text((min(width - 50.0, marker_x + outer_radius + 6), max(0.0, marker_y - outer_radius - 4)), "Point", fill=(120, 70, 0, 255))
    else:
        draw = ImageDraw.Draw(preview)
        dir_x, dir_y, _ = normalize3(light_dir[0], light_dir[1], light_dir[2])
        start_x = min(width - 36.0, max(36.0, width * 0.83))
        start_y = max(28.0, min(height - 28.0, height * 0.12))
        arrow_len = max(30.0, min(width, height) * 0.12)
        end_x = start_x + dir_x * arrow_len
        end_y = start_y + dir_y * arrow_len
        draw.line((start_x, start_y, end_x, end_y), fill=(255, 140, 0, 255), width=3)
        head_dx = end_x - start_x
        head_dy = end_y - start_y
        head_len = math.sqrt(head_dx * head_dx + head_dy * head_dy)
        if head_len > 1e-5:
            ux = head_dx / head_len
            uy = head_dy / head_len
            px = -uy
            py = ux
            head_size = 9.0
            draw.polygon(
                [
                    (end_x, end_y),
                    (end_x - ux * head_size + px * head_size * 0.6, end_y - uy * head_size + py * head_size * 0.6),
                    (end_x - ux * head_size - px * head_size * 0.6, end_y - uy * head_size - py * head_size * 0.6),
                ],
                fill=(255, 140, 0, 255),
            )
        draw.text((start_x + 8.0, start_y - 20.0), "Directional", fill=(120, 70, 0, 255))

    return preview


def relight(
    albedo: Image.Image,
    height_map: Image.Image,
    light_mode: str,
    ambient: float,
    directional: float,
    specular_strength: float,
    specular_power: float,
    shadow_strength: float,
    height_scale: float,
    blur_radius: int,
    light_dir: tuple[float, float, float],
    light_pos: tuple[float, float, float],
    light_falloff: float,
    shadow_steps: int,
    shadow_step: float,
    shadow_bias: float,
    mask_erode: int,
    mask_blur: int,
    table_height_scale: float,
    thickness_scale: float,
    table_shadow_opacity: float,
    table_shadow_blur_base: float,
    table_shadow_blur_height_scale: float,
    table_shadow_stretch_x: float,
    table_shadow_stretch_y: float,
    table_ground_offset_y: float,
    table_shadow_downsample: int,
    table_shadow_mode: str,
) -> tuple[Image.Image, Image.Image, Image.Image, Image.Image, Image.Image, Image.Image]:
    width, height = albedo.size
    if height_map.size != albedo.size:
        height_map = height_map.resize(albedo.size, Image.Resampling.BILINEAR)

    albedo_pixels = []
    albedo_source = albedo.load()
    for y in range(albedo.height):
        for x in range(albedo.width):
            albedo_pixels.append(albedo_source[x, y])
    alpha = alpha_values_from_image(albedo)
    shading_mask = erode_mask(alpha, width, height, mask_erode)
    shading_mask = blur_scalar_field(shading_mask, width, height, mask_blur)
    heights = height_values_from_image(height_map)
    heights = blur_scalar_field(heights, width, height, blur_radius)
    normals = compute_normals(heights, alpha, width, height, height_scale)

    max_dim = float(max(width, height))
    world_light_pos = (light_pos[0] * width, light_pos[1] * height, light_pos[2] * max_dim)
    if light_mode == "point":
        shadow = compute_point_shadow(
            heights,
            alpha,
            width,
            height,
            world_light_pos,
            shadow_steps,
            height_scale,
            shadow_bias,
        )
    else:
        lx, ly, lz = light_dir
        shadow = compute_shadow(
            heights,
            alpha,
            width,
            height,
            (-lx, -ly),
            shadow_steps,
            shadow_step,
            height_scale,
            shadow_bias,
        )

    table_shadow = compute_table_shadow(
        heights,
        alpha,
        width,
        height,
        light_mode if table_shadow_mode == "match-light" else table_shadow_mode,
        light_dir,
        world_light_pos,
        height_scale,
        table_height_scale,
        thickness_scale,
        table_shadow_opacity,
        table_shadow_blur_base,
        table_shadow_blur_height_scale,
        table_shadow_stretch_x,
        table_shadow_stretch_y,
        table_ground_offset_y,
        mask_erode,
        mask_blur,
        table_shadow_downsample,
    )

    final_pixels = []
    light_pixels = []
    shadow_pixels = []
    highlight_pixels = []
    for idx, (r, g, b, a) in enumerate(albedo_pixels):
        alpha_value = alpha[idx]
        if alpha_value <= 0.0:
            final_pixels.append((0, 0, 0, 0))
            light_pixels.append((0, 0, 0, 0))
            shadow_pixels.append((0, 0, 0, 0))
            highlight_pixels.append((0, 0, 0, 0))
            continue

        nx, ny, nz = normals[idx]
        normal = (nx, ny, nz)
        mask = shading_mask[idx]
        if light_mode == "point":
            point = (float(idx % width), float(idx // width), heights[idx] * height_scale)
            light_vec = subtract3(world_light_pos, point)
            light_distance = math.sqrt(dot3(light_vec, light_vec))
            light_dir_local = normalize3(light_vec[0], light_vec[1], light_vec[2])
            attenuation = 1.0 / (1.0 + light_falloff * math.pow(light_distance / max_dim, 2.0))
        else:
            light_dir_local = light_dir
            attenuation = 1.0

        view_dir = (0.0, 0.0, 1.0)
        half_dir = normalize3(
            light_dir_local[0] + view_dir[0],
            light_dir_local[1] + view_dir[1],
            light_dir_local[2] + view_dir[2],
        )

        diffuse = max(0.0, dot3(normal, light_dir_local)) * attenuation
        specular = max(0.0, dot3(normal, half_dir))
        specular = math.pow(specular, specular_power)
        shadow_mask = (1.0 - shadow_strength * shadow[idx] * mask)
        light_term = clamp01(ambient + directional * diffuse * shadow_mask)
        highlight_term = clamp01(specular_strength * specular * shadow_mask * attenuation * mask)
        lit_r = clamp255(r * light_term + 255.0 * highlight_term)
        lit_g = clamp255(g * light_term + 248.0 * highlight_term)
        lit_b = clamp255(b * light_term + 236.0 * highlight_term)
        final_pixels.append(
            (
                clamp255(r * (1.0 - mask) + lit_r * mask),
                clamp255(g * (1.0 - mask) + lit_g * mask),
                clamp255(b * (1.0 - mask) + lit_b * mask),
                a,
            )
        )
        light_intensity = clamp255(clamp01(ambient + directional * diffuse) * 255.0)
        light_pixels.append((light_intensity, light_intensity, light_intensity, a))
        shadow_intensity = clamp255(shadow[idx] * 255.0)
        shadow_pixels.append((shadow_intensity, shadow_intensity, shadow_intensity, a))
        highlight_intensity = clamp255(highlight_term * 255.0)
        highlight_pixels.append((highlight_intensity, highlight_intensity, highlight_intensity, a))

    final_image = Image.new("RGBA", albedo.size, (0, 0, 0, 0))
    final_image.putdata(final_pixels)

    light_image = Image.new("RGBA", albedo.size, (0, 0, 0, 0))
    light_image.putdata(light_pixels)

    shadow_image = Image.new("RGBA", albedo.size, (0, 0, 0, 0))
    shadow_image.putdata(shadow_pixels)

    highlight_image = Image.new("RGBA", albedo.size, (0, 0, 0, 0))
    highlight_image.putdata(highlight_pixels)

    return (
        final_image,
        normal_preview_image(normals, alpha, albedo.size),
        light_image,
        shadow_image,
        highlight_image,
        shadow_rgba_image(table_shadow, albedo.size),
    )


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Prototype 2.5D relighting for workbench weapon renders.")
    parser.add_argument("--albedo", required=True, help="Base weapon sprite path.")
    parser.add_argument("--height", required=True, help="Depth / height map path.")
    parser.add_argument("--output", required=True, help="Lit output image path.")
    parser.add_argument("--normal-output", help="Optional normal preview output path.")
    parser.add_argument("--light-output", help="Optional light-term preview output path.")
    parser.add_argument("--shadow-output", help="Optional shadow preview output path.")
    parser.add_argument("--highlight-output", help="Optional specular/highlight preview output path.")
    parser.add_argument("--table-shadow-output", help="Optional projected table-shadow output path.")
    parser.add_argument("--debug-preview-output", help="Optional white-background debug preview with light marker.")
    parser.add_argument("--height-preview-output", help="Optional blurred height preview output path.")
    parser.add_argument("--light-mode", choices=("directional", "point"), default="point", help="Lighting model to use.")
    parser.add_argument("--ambient", type=float, default=0.42, help="Ambient light contribution.")
    parser.add_argument("--directional", type=float, default=0.62, help="Directional light contribution.")
    parser.add_argument("--specular-strength", type=float, default=0.38, help="Additive highlight strength.")
    parser.add_argument("--specular-power", type=float, default=24.0, help="Specular tightness. Higher is sharper.")
    parser.add_argument("--shadow-strength", type=float, default=0.55, help="How strongly occlusion darkens the result.")
    parser.add_argument("--height-scale", type=float, default=8.0, help="Normal reconstruction scale.")
    parser.add_argument("--blur-radius", type=int, default=1, help="Box blur radius applied to the height map.")
    parser.add_argument("--light-dir", type=parse_vec3, default=parse_vec3("1,-1,1.35"), help="Directional light vector as x,y,z.")
    parser.add_argument("--light-pos", type=parse_raw_vec3, default=parse_raw_vec3("0.72,0.12,0.65"), help="Point light position as normalized x,y,z.")
    parser.add_argument("--light-falloff", type=float, default=5.5, help="Point light distance falloff.")
    parser.add_argument("--shadow-steps", type=int, default=20, help="Number of occlusion samples along the light ray.")
    parser.add_argument("--shadow-step", type=float, default=1.35, help="Step length in texels for occlusion sampling.")
    parser.add_argument("--shadow-bias", type=float, default=0.01, help="Bias that reduces self-shadowing.")
    parser.add_argument("--mask-erode", type=int, default=1, help="Erode the lit region inward to preserve dark outlines.")
    parser.add_argument("--mask-blur", type=int, default=1, help="Blur the eroded lighting mask to soften the transition.")
    parser.add_argument("--table-height-scale", type=float, default=0.18, help="How strongly vertical sprite position contributes to table-shadow spread.")
    parser.add_argument("--thickness-scale", type=float, default=2.0, help="Multiplier for converting the one-sided depth map into approximate total weapon thickness.")
    parser.add_argument("--table-shadow-mode", choices=("match-light", "directional", "point"), default="directional", help="Lighting model used only for the projected table shadow.")
    parser.add_argument("--table-shadow-opacity", type=float, default=0.75, help="Base opacity of the projected table shadow.")
    parser.add_argument("--table-shadow-blur-base", type=float, default=1.6, help="Minimum softness of the projected table shadow.")
    parser.add_argument("--table-shadow-blur-height-scale", type=float, default=0.16, help="Additional softness added as the occluder lifts away from the table.")
    parser.add_argument("--table-shadow-stretch-x", type=float, default=1.2, help="Horizontal spread multiplier for the projected table shadow.")
    parser.add_argument("--table-shadow-stretch-y", type=float, default=0.72, help="Vertical spread multiplier for the projected table shadow.")
    parser.add_argument("--table-ground-offset-y", type=float, default=0.0, help="Additional downward offset for the table plane in pixels.")
    parser.add_argument("--table-shadow-downsample", type=int, default=4, help="Downsample factor used to deliberately soften the projected table shadow.")
    parser.add_argument("--debug-canvas-scale", type=float, default=2.2, help="Canvas scale for the workbench-style debug preview.")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    albedo_path = Path(args.albedo)
    height_path = Path(args.height)
    output_path = Path(args.output)

    albedo = load_rgba(albedo_path)
    height_map = load_rgba(height_path)
    final_image, normal_image, light_image, shadow_image, highlight_image, table_shadow_image = relight(
        albedo=albedo,
        height_map=height_map,
        light_mode=args.light_mode,
        ambient=args.ambient,
        directional=args.directional,
        specular_strength=args.specular_strength,
        specular_power=args.specular_power,
        shadow_strength=args.shadow_strength,
        height_scale=args.height_scale,
        blur_radius=args.blur_radius,
        light_dir=args.light_dir,
        light_pos=args.light_pos,
        light_falloff=args.light_falloff,
        shadow_steps=args.shadow_steps,
        shadow_step=args.shadow_step,
        shadow_bias=args.shadow_bias,
        mask_erode=args.mask_erode,
        mask_blur=args.mask_blur,
        table_height_scale=args.table_height_scale,
        thickness_scale=args.thickness_scale,
        table_shadow_opacity=args.table_shadow_opacity,
        table_shadow_blur_base=args.table_shadow_blur_base,
        table_shadow_blur_height_scale=args.table_shadow_blur_height_scale,
        table_shadow_stretch_x=args.table_shadow_stretch_x,
        table_shadow_stretch_y=args.table_shadow_stretch_y,
        table_ground_offset_y=args.table_ground_offset_y,
        table_shadow_downsample=args.table_shadow_downsample,
        table_shadow_mode=args.table_shadow_mode,
    )

    output_path.parent.mkdir(parents=True, exist_ok=True)
    final_image.save(output_path)

    if args.normal_output:
        Path(args.normal_output).parent.mkdir(parents=True, exist_ok=True)
        normal_image.save(args.normal_output)
    if args.light_output:
        Path(args.light_output).parent.mkdir(parents=True, exist_ok=True)
        light_image.save(args.light_output)
    if args.shadow_output:
        Path(args.shadow_output).parent.mkdir(parents=True, exist_ok=True)
        shadow_image.save(args.shadow_output)
    if args.highlight_output:
        Path(args.highlight_output).parent.mkdir(parents=True, exist_ok=True)
        highlight_image.save(args.highlight_output)
    if args.table_shadow_output:
        Path(args.table_shadow_output).parent.mkdir(parents=True, exist_ok=True)
        table_shadow_image.save(args.table_shadow_output)
    if args.debug_preview_output:
        Path(args.debug_preview_output).parent.mkdir(parents=True, exist_ok=True)
        debug_shadow_mode = args.light_mode if args.table_shadow_mode == "match-light" else args.table_shadow_mode
        create_canvas_debug_preview(
            albedo,
            height_map,
            args.light_mode,
            args.light_dir,
            args.light_pos,
            args.height_scale,
            args.table_height_scale,
            args.thickness_scale,
            args.table_shadow_mode,
            args.table_shadow_opacity,
            args.table_shadow_blur_base,
            args.table_shadow_blur_height_scale,
            args.table_shadow_stretch_x,
            args.table_shadow_stretch_y,
            args.table_shadow_downsample,
            args.mask_erode,
            args.mask_blur,
            args.debug_canvas_scale,
        ).save(args.debug_preview_output)
    if args.height_preview_output:
        blurred_height = blur_scalar_field(height_values_from_image(height_map), albedo.width, albedo.height, args.blur_radius)
        grayscale_preview_image(blurred_height, alpha_values_from_image(albedo), albedo.size).save(args.height_preview_output)

    print(f"Saved lit preview to {output_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
