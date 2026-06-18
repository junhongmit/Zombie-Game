#include "WeaponRelight.h"

#include "AssetPaths.h"
#include "Texture.h"

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

namespace zg {

namespace {

static const float kLightPosX = 0.8f;
static const float kLightPosY = -0.3f;
static const float kLightPosZ = 0.1f;
static const float kLightFalloff = 1.0f;
static const float kAmbient = 0.5f;
static const float kDirectional = 0.25f;
static const float kSpecularStrength = 1.10f;
static const float kSpecularPower = 14.0f;
static const float kShadowStrength = 0.25f;
static const float kHeightScale = 7.0f;
static const int kBlurRadius = 3;
static const int kMaskErode = 2;
static const int kMaskBlur = 2;
static const int kShadowSteps = 20;
static const float kShadowBias = 0.01f;
static const float kShadowDirX = -0.42f;
static const float kShadowDirY = 0.18f;
static const float kShadowDirZ = 1.35f;
static const float kShadowCanvasScale = 2.3f;
static const float kShadowTableHeightScale = 0.32f;
static const float kShadowThicknessScale = 1.4f;
static const float kShadowOpacity = 0.56f;
static const float kShadowBlurBase = 2.9f;
static const float kShadowBlurHeightScale = 0.10f;
static const float kShadowStretchX = 1.55f;
static const float kShadowStretchY = 0.72f;
static const int kShadowDownsample = 5;
static const int kContactGapPixels = 10;

struct Vec3 {
    float x;
    float y;
    float z;
};

float clamp01(float value)
{
    return std::max(0.0f, std::min(1.0f, value));
}

Uint8 clamp255(float value)
{
    return static_cast<Uint8>(std::max(0.0f, std::min(255.0f, std::round(value))));
}

Vec3 normalize3(float x, float y, float z)
{
    const float length = std::sqrt(x * x + y * y + z * z);
    if (length <= 1e-6f) {
        return Vec3{0.0f, 0.0f, 1.0f};
    }
    return Vec3{x / length, y / length, z / length};
}

float dot3(const Vec3& a, const Vec3& b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

Vec3 subtract3(const Vec3& a, const Vec3& b)
{
    return Vec3{a.x - b.x, a.y - b.y, a.z - b.z};
}

bool file_exists(const std::string& path)
{
    std::FILE* file = std::fopen(path.c_str(), "rb");
    if (file == nullptr) {
        return false;
    }
    std::fclose(file);
    return true;
}

SDL_Surface* load_surface_rgba(const char* path)
{
    const std::string resolved = resolve_asset_path(path);
    SDL_Surface* loaded = IMG_Load(resolved.c_str());
    if (loaded == nullptr) {
        return nullptr;
    }
    SDL_Surface* converted = SDL_ConvertSurface(loaded, SDL_PIXELFORMAT_RGBA32);
    SDL_DestroySurface(loaded);
    return converted;
}

float luminance(Uint8 r, Uint8 g, Uint8 b)
{
    return (0.299f * r + 0.587f * g + 0.114f * b) / 255.0f;
}

float bilinear_sample(const std::vector<float>& values, int width, int height, float x, float y)
{
    x = std::max(0.0f, std::min(static_cast<float>(width - 1), x));
    y = std::max(0.0f, std::min(static_cast<float>(height - 1), y));
    const int x0 = static_cast<int>(std::floor(x));
    const int y0 = static_cast<int>(std::floor(y));
    const int x1 = std::min(width - 1, x0 + 1);
    const int y1 = std::min(height - 1, y0 + 1);
    const float tx = x - x0;
    const float ty = y - y0;

    const float v00 = values[y0 * width + x0];
    const float v10 = values[y0 * width + x1];
    const float v01 = values[y1 * width + x0];
    const float v11 = values[y1 * width + x1];
    const float a = v00 * (1.0f - tx) + v10 * tx;
    const float b = v01 * (1.0f - tx) + v11 * tx;
    return a * (1.0f - ty) + b * ty;
}

std::vector<float> blur_scalar_field(const std::vector<float>& values, int width, int height, int radius)
{
    if (radius <= 0) {
        return values;
    }

    std::vector<float> result(width * height, 0.0f);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float total = 0.0f;
            int count = 0;
            for (int oy = -radius; oy <= radius; ++oy) {
                const int sy = y + oy;
                if (sy < 0 || sy >= height) {
                    continue;
                }
                for (int ox = -radius; ox <= radius; ++ox) {
                    const int sx = x + ox;
                    if (sx < 0 || sx >= width) {
                        continue;
                    }
                    total += values[sy * width + sx];
                    ++count;
                }
            }
            result[y * width + x] = total / std::max(1, count);
        }
    }
    return result;
}

std::vector<float> erode_mask(const std::vector<float>& values, int width, int height, int radius)
{
    if (radius <= 0) {
        return values;
    }

    std::vector<float> result(width * height, 0.0f);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float minimum = 1.0f;
            for (int oy = -radius; oy <= radius; ++oy) {
                const int sy = y + oy;
                if (sy < 0 || sy >= height) {
                    minimum = 0.0f;
                    continue;
                }
                for (int ox = -radius; ox <= radius; ++ox) {
                    const int sx = x + ox;
                    if (sx < 0 || sx >= width) {
                        minimum = 0.0f;
                        continue;
                    }
                    minimum = std::min(minimum, values[sy * width + sx]);
                }
            }
            result[y * width + x] = minimum;
        }
    }
    return result;
}

std::vector<float> dilate_scalar_field(const std::vector<float>& values, int width, int height, int radius)
{
    if (radius <= 0) {
        return values;
    }

    std::vector<float> result(width * height, 0.0f);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float maximum = 0.0f;
            for (int oy = -radius; oy <= radius; ++oy) {
                const int sy = y + oy;
                if (sy < 0 || sy >= height) {
                    continue;
                }
                for (int ox = -radius; ox <= radius; ++ox) {
                    const int sx = x + ox;
                    if (sx < 0 || sx >= width) {
                        continue;
                    }
                    maximum = std::max(maximum, values[sy * width + sx]);
                }
            }
            result[y * width + x] = maximum;
        }
    }
    return result;
}

std::vector<float> resize_scalar_field(const std::vector<float>& values, int width, int height, int target_width, int target_height)
{
    if (width == target_width && height == target_height) {
        return values;
    }

    std::vector<float> result(target_width * target_height, 0.0f);
    for (int y = 0; y < target_height; ++y) {
        const float src_y = ((static_cast<float>(y) + 0.5f) * height / target_height) - 0.5f;
        for (int x = 0; x < target_width; ++x) {
            const float src_x = ((static_cast<float>(x) + 0.5f) * width / target_width) - 0.5f;
            result[y * target_width + x] = bilinear_sample(values, width, height, src_x, src_y);
        }
    }
    return result;
}

std::vector<Vec3> compute_normals(
    const std::vector<float>& values,
    const std::vector<float>& alpha,
    int width,
    int height,
    float height_scale)
{
    std::vector<Vec3> normals(width * height, Vec3{0.0f, 0.0f, 1.0f});
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const int index = y * width + x;
            if (alpha[index] <= 0.0f) {
                continue;
            }

            const float left = values[y * width + std::max(0, x - 1)];
            const float right = values[y * width + std::min(width - 1, x + 1)];
            const float up = values[std::max(0, y - 1) * width + x];
            const float down = values[std::min(height - 1, y + 1) * width + x];
            const float dx = right - left;
            const float dy = down - up;
            normals[index] = normalize3(-dx * height_scale, -dy * height_scale, 1.0f);
        }
    }
    return normals;
}

std::vector<float> compute_point_shadow(
    const std::vector<float>& heights,
    const std::vector<float>& alpha,
    int width,
    int height,
    const Vec3& light_pos,
    int shadow_steps,
    float height_scale,
    float shadow_bias)
{
    std::vector<float> result(width * height, 0.0f);
    const float max_dim = static_cast<float>(std::max(width, height));
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const int index = y * width + x;
            if (alpha[index] <= 0.0f) {
                continue;
            }

            const Vec3 point{static_cast<float>(x), static_cast<float>(y), heights[index] * height_scale};
            const Vec3 ray = subtract3(light_pos, point);
            const float ray_xy_len = std::sqrt(ray.x * ray.x + ray.y * ray.y);
            if (ray_xy_len <= 1e-5f) {
                continue;
            }

            float occlusion = 0.0f;
            for (int step_index = 1; step_index <= shadow_steps; ++step_index) {
                const float t = static_cast<float>(step_index) / static_cast<float>(shadow_steps);
                const float sample_x = point.x + ray.x * t;
                const float sample_y = point.y + ray.y * t;
                const float sample_alpha = bilinear_sample(alpha, width, height, sample_x, sample_y);
                if (sample_alpha <= 0.01f) {
                    continue;
                }

                const float sample_height = bilinear_sample(heights, width, height, sample_x, sample_y) * height_scale;
                const float ray_height = point.z + ray.z * t;
                if (sample_height > ray_height + shadow_bias * max_dim) {
                    occlusion = std::max(
                        occlusion,
                        clamp01((sample_height - ray_height) / std::max(1.0f, 0.12f * max_dim)));
                }
            }
            result[index] = occlusion;
        }
    }
    return result;
}

void scatter_soft_disk(
    std::vector<float>* field,
    int width,
    int height,
    float center_x,
    float center_y,
    float radius_x,
    float radius_y,
    float opacity)
{
    if (field == nullptr || opacity <= 0.0f || radius_x <= 0.0f || radius_y <= 0.0f) {
        return;
    }

    const int min_x = std::max(0, static_cast<int>(std::floor(center_x - radius_x - 1.0f)));
    const int max_x = std::min(width - 1, static_cast<int>(std::ceil(center_x + radius_x + 1.0f)));
    const int min_y = std::max(0, static_cast<int>(std::floor(center_y - radius_y - 1.0f)));
    const int max_y = std::min(height - 1, static_cast<int>(std::ceil(center_y + radius_y + 1.0f)));
    const float inv_rx = 1.0f / std::max(1e-5f, radius_x);
    const float inv_ry = 1.0f / std::max(1e-5f, radius_y);

    for (int py = min_y; py <= max_y; ++py) {
        const float dy = (py - center_y) * inv_ry;
        for (int px = min_x; px <= max_x; ++px) {
            const float dx = (px - center_x) * inv_rx;
            const float dist2 = dx * dx + dy * dy;
            if (dist2 >= 1.0f) {
                continue;
            }
            const float falloff = std::pow(1.0f - dist2, 1.6f);
            const int index = py * width + px;
            (*field)[index] = std::max((*field)[index], opacity * falloff);
        }
    }
}

bool build_workbench_shadow_surface(
    SDL_Surface* albedo_surface,
    const std::vector<float>& alpha_src,
    const std::vector<float>& heights_src,
    SDL_Surface** out_surface,
    WorkbenchShadowPlacement* out_placement)
{
    if (albedo_surface == nullptr || out_surface == nullptr || out_placement == nullptr) {
        return false;
    }

    const int src_width = albedo_surface->w;
    const int src_height = albedo_surface->h;
    const int canvas_width = std::max(static_cast<int>(std::round(src_width * kShadowCanvasScale)), src_width + 120);
    const int canvas_height = std::max(static_cast<int>(std::round(src_height * kShadowCanvasScale)), src_height + 140);
    const int table_plane_y = std::max(src_height + 18, static_cast<int>(std::round(canvas_height * 0.74f)));
    const int weapon_offset_x = std::max(24, static_cast<int>(std::round((canvas_width - src_width) * 0.32f)));

    int alpha_bottom = src_height - 1;
    bool found_bottom = false;
    for (int y = src_height - 1; y >= 0; --y) {
        for (int x = 0; x < src_width; ++x) {
            if (alpha_src[y * src_width + x] > 0.01f) {
                alpha_bottom = y;
                found_bottom = true;
                break;
            }
        }
        if (found_bottom) {
            break;
        }
    }
    const int weapon_offset_y = std::max(16, table_plane_y - alpha_bottom - kContactGapPixels);

    out_placement->anchor_x = static_cast<float>(weapon_offset_x);
    out_placement->anchor_y = static_cast<float>(weapon_offset_y);

    const int low_scale = std::max(2, kShadowDownsample * 2);
    const int low_width = std::max(1, canvas_width / low_scale);
    const int low_height = std::max(1, canvas_height / low_scale);
    std::vector<float> low_shadow(low_width * low_height, 0.0f);

    const Vec3 dir = normalize3(kShadowDirX, kShadowDirY, kShadowDirZ);
    const float inv_dir_z = 1.0f / std::max(0.08f, dir.z);

    float min_x = static_cast<float>(src_width);
    float max_x = 0.0f;
    float total_weight = 0.0f;
    float sum_x = 0.0f;
    float sum_z = 0.0f;
    for (int y = 0; y < src_height; ++y) {
        for (int x = 0; x < src_width; ++x) {
            const int index = y * src_width + x;
            const float a = alpha_src[index];
            if (a <= 0.02f) {
                continue;
            }
            const float world_y = static_cast<float>(weapon_offset_y + y);
            const float lift = std::max(0.0f, (table_plane_y - world_y) * kShadowTableHeightScale);
            const float thickness = heights_src[index] * kHeightScale * kShadowThicknessScale;
            const float point_z = lift + thickness;
            const float weight = a * std::max(0.18f, 1.0f - 0.6f * clamp01(point_z / std::max(1.0f, static_cast<float>(src_height))));
            total_weight += weight;
            sum_x += (weapon_offset_x + x) * weight;
            sum_z += point_z * weight;
            min_x = std::min(min_x, static_cast<float>(x));
            max_x = std::max(max_x, static_cast<float>(x));
        }
    }

    if (total_weight > 1e-5f) {
        const float blob_x = sum_x / total_weight + dir.x * (sum_z / total_weight) * inv_dir_z * 1.18f;
        const float blob_y = table_plane_y + dir.y * (sum_z / total_weight) * inv_dir_z;
        const float blob_rx = ((max_x - min_x) * 0.55f + (sum_z / total_weight) * 0.8f) / low_scale;
        const float blob_ry = (src_height * 0.10f + (sum_z / total_weight) * 0.18f) / low_scale;
        scatter_soft_disk(
            &low_shadow,
            low_width,
            low_height,
            blob_x / low_scale,
            blob_y / low_scale,
            std::max(2.2f, blob_rx * kShadowStretchX),
            std::max(1.1f, blob_ry * kShadowStretchY),
            kShadowOpacity * 0.42f);
    }

    for (int y = 0; y < src_height; ++y) {
        for (int x = 0; x < src_width; ++x) {
            const int index = y * src_width + x;
            const float a = alpha_src[index];
            if (a <= 0.03f) {
                continue;
            }

            const float world_x = static_cast<float>(weapon_offset_x + x);
            const float world_y = static_cast<float>(weapon_offset_y + y);
            const float lift = std::max(0.0f, (table_plane_y - world_y) * kShadowTableHeightScale);
            const float thickness = heights_src[index] * kHeightScale * kShadowThicknessScale;
            const float point_z = lift + thickness;
            const float shadow_x = world_x + dir.x * point_z * inv_dir_z;
            const float shadow_y = static_cast<float>(table_plane_y) + dir.y * point_z * inv_dir_z;
            const float height_norm = clamp01(lift / std::max(1.0f, static_cast<float>(src_height)));
            const float opacity = kShadowOpacity * a * (1.0f - 0.72f * height_norm);
            const float radius_x = std::max(0.9f, (kShadowBlurBase + point_z * kShadowBlurHeightScale * 0.45f) * kShadowStretchX / low_scale);
            const float radius_y = std::max(0.5f, (kShadowBlurBase * 0.42f + point_z * kShadowBlurHeightScale * 0.12f) * kShadowStretchY / low_scale);
            scatter_soft_disk(&low_shadow, low_width, low_height, shadow_x / low_scale, shadow_y / low_scale, radius_x, radius_y, opacity);

            const float contact_weight = clamp01(1.0f - lift / 22.0f);
            if (contact_weight > 0.01f) {
                const float contact_shift_x = dir.x * std::max(0.6f, lift * 0.38f);
                scatter_soft_disk(
                    &low_shadow,
                    low_width,
                    low_height,
                    (world_x + contact_shift_x) / low_scale,
                    table_plane_y / static_cast<float>(low_scale),
                    std::max(0.9f, kShadowBlurBase * 0.35f / low_scale),
                    std::max(0.45f, kShadowBlurBase * 0.14f / low_scale),
                    kShadowOpacity * a * 0.30f * contact_weight);
            }
        }
    }

    low_shadow = blur_scalar_field(low_shadow, low_width, low_height, 2);
    low_shadow = dilate_scalar_field(low_shadow, low_width, low_height, 1);
    std::vector<float> shadow_values = resize_scalar_field(low_shadow, low_width, low_height, canvas_width, canvas_height);
    shadow_values = blur_scalar_field(shadow_values, canvas_width, canvas_height, 2);

    SDL_Surface* surface = SDL_CreateSurface(canvas_width, canvas_height, SDL_PIXELFORMAT_RGBA32);
    if (surface == nullptr) {
        return false;
    }
    for (int y = 0; y < canvas_height; ++y) {
        for (int x = 0; x < canvas_width; ++x) {
            const Uint8 alpha_out = clamp255(shadow_values[y * canvas_width + x] * 255.0f);
            SDL_WriteSurfacePixel(surface, x, y, 0, 0, 0, alpha_out);
        }
    }
    *out_surface = surface;
    return true;
}

SDL_Surface* create_surface_from_rgba_buffer(int width, int height, const std::vector<unsigned char>& rgba)
{
    if (width <= 0 || height <= 0 || rgba.size() != static_cast<size_t>(width * height * 4)) {
        return nullptr;
    }
    SDL_Surface* surface = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_RGBA32);
    if (surface == nullptr) {
        return nullptr;
    }
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const size_t index = static_cast<size_t>((y * width + x) * 4);
            SDL_WriteSurfacePixel(surface, x, y, rgba[index + 0], rgba[index + 1], rgba[index + 2], rgba[index + 3]);
        }
    }
    return surface;
}

} // namespace

bool bake_workbench_relight_assets(
    const char* albedo_path,
    const char* depth_path,
    WorkbenchRelightBakeResult* out_result)
{
    if (albedo_path == nullptr || depth_path == nullptr || out_result == nullptr) {
        return false;
    }
    *out_result = WorkbenchRelightBakeResult{};

    const std::string resolved_depth = resolve_asset_path(depth_path);
    if (!file_exists(resolved_depth)) {
        return false;
    }

    SDL_Surface* albedo_surface = load_surface_rgba(albedo_path);
    SDL_Surface* depth_surface = load_surface_rgba(depth_path);
    if (albedo_surface == nullptr || depth_surface == nullptr) {
        if (albedo_surface != nullptr) {
            SDL_DestroySurface(albedo_surface);
        }
        if (depth_surface != nullptr) {
            SDL_DestroySurface(depth_surface);
        }
        return false;
    }

    if (depth_surface->w != albedo_surface->w || depth_surface->h != albedo_surface->h) {
        SDL_Surface* scaled_depth = SDL_ScaleSurface(depth_surface, albedo_surface->w, albedo_surface->h, SDL_SCALEMODE_LINEAR);
        SDL_DestroySurface(depth_surface);
        depth_surface = scaled_depth;
        if (depth_surface == nullptr) {
            SDL_DestroySurface(albedo_surface);
            return false;
        }
    }

    const int width = albedo_surface->w;
    const int height = albedo_surface->h;
    const int pixel_count = width * height;
    std::vector<SDL_Color> albedo(pixel_count);
    std::vector<float> alpha(pixel_count, 0.0f);
    std::vector<float> heights(pixel_count, 0.0f);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const int index = y * width + x;
            Uint8 r = 0, g = 0, b = 0, a = 0;
            SDL_ReadSurfacePixel(albedo_surface, x, y, &r, &g, &b, &a);
            albedo[index] = SDL_Color{r, g, b, a};
            alpha[index] = a / 255.0f;

            Uint8 hr = 0, hg = 0, hb = 0, ha = 0;
            SDL_ReadSurfacePixel(depth_surface, x, y, &hr, &hg, &hb, &ha);
            heights[index] = luminance(hr, hg, hb) * (ha / 255.0f);
        }
    }

    std::vector<float> shading_mask = erode_mask(alpha, width, height, kMaskErode);
    shading_mask = blur_scalar_field(shading_mask, width, height, kMaskBlur);
    heights = blur_scalar_field(heights, width, height, kBlurRadius);
    const std::vector<Vec3> normals = compute_normals(heights, alpha, width, height, kHeightScale);

    const float max_dim = static_cast<float>(std::max(width, height));
    const Vec3 light_pos{
        kLightPosX * width,
        kLightPosY * height,
        kLightPosZ * max_dim
    };
    const std::vector<float> shadow = compute_point_shadow(
        heights,
        alpha,
        width,
        height,
        light_pos,
        kShadowSteps,
        kHeightScale,
        kShadowBias);

    out_result->lit_width = width;
    out_result->lit_height = height;
    out_result->lit_rgba.resize(static_cast<size_t>(width * height * 4), 0);

    const Vec3 view_dir{0.0f, 0.0f, 1.0f};
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const int index = y * width + x;
            const SDL_Color& base = albedo[index];
            const size_t out_index = static_cast<size_t>((index) * 4);
            if (base.a == 0) {
                out_result->lit_rgba[out_index + 0] = 0;
                out_result->lit_rgba[out_index + 1] = 0;
                out_result->lit_rgba[out_index + 2] = 0;
                out_result->lit_rgba[out_index + 3] = 0;
                continue;
            }

            const Vec3 point{static_cast<float>(x), static_cast<float>(y), heights[index] * kHeightScale};
            const Vec3 light_vec = subtract3(light_pos, point);
            const float light_distance = std::sqrt(dot3(light_vec, light_vec));
            const Vec3 light_dir = normalize3(light_vec.x, light_vec.y, light_vec.z);
            const float attenuation = 1.0f / (1.0f + kLightFalloff * std::pow(light_distance / max_dim, 2.0f));
            const Vec3 half_dir = normalize3(
                light_dir.x + view_dir.x,
                light_dir.y + view_dir.y,
                light_dir.z + view_dir.z);

            const float diffuse = std::max(0.0f, dot3(normals[index], light_dir)) * attenuation;
            float specular = std::max(0.0f, dot3(normals[index], half_dir));
            specular = std::pow(specular, kSpecularPower);

            const float mask = shading_mask[index];
            const float shadow_mask = 1.0f - kShadowStrength * shadow[index] * mask;
            const float light_term = clamp01(kAmbient + kDirectional * diffuse * shadow_mask);
            const float highlight_term = clamp01(kSpecularStrength * specular * shadow_mask * attenuation * mask);

            const float lit_r = base.r * light_term + 255.0f * highlight_term;
            const float lit_g = base.g * light_term + 248.0f * highlight_term;
            const float lit_b = base.b * light_term + 236.0f * highlight_term;

            const Uint8 out_r = clamp255(base.r * (1.0f - mask) + lit_r * mask);
            const Uint8 out_g = clamp255(base.g * (1.0f - mask) + lit_g * mask);
            const Uint8 out_b = clamp255(base.b * (1.0f - mask) + lit_b * mask);
            out_result->lit_rgba[out_index + 0] = out_r;
            out_result->lit_rgba[out_index + 1] = out_g;
            out_result->lit_rgba[out_index + 2] = out_b;
            out_result->lit_rgba[out_index + 3] = base.a;
        }
    }

    SDL_Surface* shadow_surface = nullptr;
    bool shadow_ok = build_workbench_shadow_surface(albedo_surface, alpha, heights, &shadow_surface, &out_result->shadow_placement);
    if (shadow_ok && shadow_surface != nullptr) {
        out_result->shadow_width = shadow_surface->w;
        out_result->shadow_height = shadow_surface->h;
        out_result->shadow_rgba.resize(static_cast<size_t>(shadow_surface->w * shadow_surface->h * 4), 0);
        for (int y = 0; y < shadow_surface->h; ++y) {
            for (int x = 0; x < shadow_surface->w; ++x) {
                Uint8 r = 0, g = 0, b = 0, a = 0;
                SDL_ReadSurfacePixel(shadow_surface, x, y, &r, &g, &b, &a);
                const size_t index = static_cast<size_t>((y * shadow_surface->w + x) * 4);
                out_result->shadow_rgba[index + 0] = r;
                out_result->shadow_rgba[index + 1] = g;
                out_result->shadow_rgba[index + 2] = b;
                out_result->shadow_rgba[index + 3] = a;
            }
        }
    }

    if (shadow_surface != nullptr) {
        SDL_DestroySurface(shadow_surface);
    }
    SDL_DestroySurface(albedo_surface);
    SDL_DestroySurface(depth_surface);
    out_result->valid = shadow_ok && !out_result->lit_rgba.empty() && !out_result->shadow_rgba.empty();
    return out_result->valid;
}

bool upload_workbench_relight_assets(
    SDL_Renderer* renderer,
    const WorkbenchRelightBakeResult& baked,
    Texture* out_lit_texture,
    Texture* out_shadow_texture,
    WorkbenchShadowPlacement* out_shadow_placement)
{
    if (renderer == nullptr || out_lit_texture == nullptr || out_shadow_texture == nullptr ||
        out_shadow_placement == nullptr || !baked.valid) {
        return false;
    }

    SDL_Surface* lit_surface = create_surface_from_rgba_buffer(baked.lit_width, baked.lit_height, baked.lit_rgba);
    SDL_Surface* shadow_surface = create_surface_from_rgba_buffer(baked.shadow_width, baked.shadow_height, baked.shadow_rgba);
    if (lit_surface == nullptr || shadow_surface == nullptr) {
        if (lit_surface != nullptr) {
            SDL_DestroySurface(lit_surface);
        }
        if (shadow_surface != nullptr) {
            SDL_DestroySurface(shadow_surface);
        }
        return false;
    }

    const bool lit_ok = out_lit_texture->load_from_surface(renderer, lit_surface);
    const bool shadow_ok = out_shadow_texture->load_from_surface(renderer, shadow_surface);
    SDL_DestroySurface(lit_surface);
    SDL_DestroySurface(shadow_surface);
    if (!lit_ok || !shadow_ok) {
        return false;
    }

    *out_shadow_placement = baked.shadow_placement;
    return true;
}

bool build_workbench_relight_assets(
    SDL_Renderer* renderer,
    const char* albedo_path,
    const char* depth_path,
    Texture* out_lit_texture,
    Texture* out_shadow_texture,
    WorkbenchShadowPlacement* out_shadow_placement)
{
    WorkbenchRelightBakeResult baked;
    if (!bake_workbench_relight_assets(albedo_path, depth_path, &baked)) {
        return false;
    }
    return upload_workbench_relight_assets(renderer, baked, out_lit_texture, out_shadow_texture, out_shadow_placement);
}

} // namespace zg
