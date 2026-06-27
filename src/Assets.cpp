#include "Assets.h"

#include "AssetPaths.h"

#include <SDL3/SDL_filesystem.h>

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <utility>
#include <vector>

namespace zg {

namespace {

bool load_optional_sheet(SDL_Renderer* renderer, Texture* texture, SpriteSheet* meta, bool* loaded, const char* image_path, const char* meta_path)
{
    *loaded = texture->load(renderer, image_path, true) && meta->load_metadata(meta_path);
    if (*loaded) {
        return true;
    }

    texture->reset();
    *meta = SpriteSheet{};
    *loaded = false;
    return false;
}

bool path_exists(const std::string& path)
{
    SDL_PathInfo info;
    return SDL_GetPathInfo(path.c_str(), &info) &&
        (info.type == SDL_PATHTYPE_FILE || info.type == SDL_PATHTYPE_DIRECTORY);
}

bool read_text_file(const std::string& path, std::string* out)
{
    std::ifstream input(path.c_str(), std::ios::in | std::ios::binary);
    if (!input.is_open()) {
        return false;
    }
    std::ostringstream buffer;
    buffer << input.rdbuf();
    *out = buffer.str();
    return true;
}

bool extract_string_value(const std::string& text, const char* key, std::string* out)
{
    const std::string token = std::string("\"") + key + "\"";
    const size_t key_pos = text.find(token);
    if (key_pos == std::string::npos) {
        return false;
    }

    const size_t colon_pos = text.find(':', key_pos + token.size());
    if (colon_pos == std::string::npos) {
        return false;
    }

    const size_t value_start = text.find('"', colon_pos + 1);
    if (value_start == std::string::npos) {
        return false;
    }

    const size_t value_end = text.find('"', value_start + 1);
    if (value_end == std::string::npos) {
        return false;
    }

    *out = text.substr(value_start + 1, value_end - value_start - 1);
    return true;
}

bool extract_float_value(const std::string& text, const char* key, float* out)
{
    const std::string token = std::string("\"") + key + "\"";
    const size_t key_pos = text.find(token);
    if (key_pos == std::string::npos) {
        return false;
    }

    const size_t colon_pos = text.find(':', key_pos + token.size());
    if (colon_pos == std::string::npos) {
        return false;
    }

    const size_t value_start = text.find_first_of("-0123456789", colon_pos + 1);
    if (value_start == std::string::npos) {
        return false;
    }

    const size_t value_end = text.find_first_not_of("0123456789.-", value_start);
    const std::string value = text.substr(
        value_start,
        value_end == std::string::npos ? std::string::npos : value_end - value_start);
    *out = std::strtof(value.c_str(), nullptr);
    return true;
}

bool extract_int_value(const std::string& text, const char* key, int* out)
{
    float value = 0.0f;
    if (!extract_float_value(text, key, &value)) {
        return false;
    }
    *out = static_cast<int>(value);
    return true;
}

bool extract_object_value(const std::string& text, const char* key, std::string* out)
{
    const std::string token = std::string("\"") + key + "\"";
    const size_t key_pos = text.find(token);
    if (key_pos == std::string::npos) {
        return false;
    }

    size_t brace_pos = text.find('{', key_pos + token.size());
    if (brace_pos == std::string::npos) {
        return false;
    }

    int depth = 0;
    for (size_t i = brace_pos; i < text.size(); ++i) {
        if (text[i] == '{') {
            ++depth;
        } else if (text[i] == '}') {
            --depth;
            if (depth == 0) {
                *out = text.substr(brace_pos, i - brace_pos + 1);
                return true;
            }
        }
    }

    return false;
}

bool parse_rect_value(const std::string& text, const char* key, SDL_Rect* out)
{
    std::string object_text;
    if (!extract_object_value(text, key, &object_text)) {
        return false;
    }
    return extract_int_value(object_text, "x", &out->x) &&
        extract_int_value(object_text, "y", &out->y) &&
        extract_int_value(object_text, "w", &out->w) &&
        extract_int_value(object_text, "h", &out->h);
}

bool parse_point_value(const std::string& text, const char* key, SDL_FPoint* out)
{
    std::string object_text;
    if (!extract_object_value(text, key, &object_text)) {
        return false;
    }
    return extract_float_value(object_text, "x", &out->x) &&
        extract_float_value(object_text, "y", &out->y);
}

std::vector<std::string> extract_object_keys(const std::string& object_text)
{
    std::vector<std::string> keys;
    int depth = 0;
    for (size_t i = 0; i < object_text.size(); ++i) {
        const char c = object_text[i];
        if (c == '{') {
            ++depth;
            continue;
        }
        if (c == '}') {
            --depth;
            continue;
        }
        if (depth != 1 || c != '"') {
            continue;
        }

        const size_t end = object_text.find('"', i + 1);
        if (end == std::string::npos) {
            break;
        }

        const size_t colon = object_text.find(':', end + 1);
        if (colon == std::string::npos) {
            break;
        }

        keys.push_back(object_text.substr(i + 1, end - i - 1));
        i = end;
    }
    return keys;
}

bool resolve_control_style_sheet(const char* metadata_path, std::string* image_path)
{
    const std::string resolved_metadata = resolve_asset_path(metadata_path);
    std::string text;
    if (!read_text_file(resolved_metadata, &text)) {
        std::fprintf(stderr, "Failed to read UI metadata: %s\n", resolved_metadata.c_str());
        return false;
    }

    std::string sheet_name;
    if (!extract_string_value(text, "sheet", &sheet_name) || sheet_name.empty()) {
        std::fprintf(stderr, "UI metadata missing sheet: %s\n", resolved_metadata.c_str());
        return false;
    }

    const std::string metadata_string(metadata_path);
    const size_t slash_pos = metadata_string.find_last_of("/\\");
    if (slash_pos == std::string::npos) {
        *image_path = sheet_name;
    } else {
        *image_path = metadata_string.substr(0, slash_pos + 1) + sheet_name;
    }
    return true;
}

bool load_control_style_from_metadata(SDL_Renderer* renderer, const char* metadata_path, const char* control_name, ControlStyle* out_style)
{
    std::string image_path;
    if (!resolve_control_style_sheet(metadata_path, &image_path)) {
        return false;
    }
    return out_style->load(renderer, image_path.c_str(), metadata_path, control_name);
}

bool parse_character_rig_part(const std::string& parts_text, const char* key, CharacterRigPart* out_part)
{
    std::string part_text;
    if (!extract_object_value(parts_text, key, &part_text)) {
        return false;
    }
    if (!parse_rect_value(part_text, "frame", &out_part->frame) ||
        !parse_point_value(part_text, "pivot", &out_part->pivot) ||
        !extract_string_value(part_text, "anchor", &out_part->anchor) ||
        !extract_int_value(part_text, "z_order", &out_part->z_order)) {
        return false;
    }
    extract_float_value(part_text, "solver_length", &out_part->solver_length);
    out_part->has_distal_joint = parse_point_value(part_text, "distal_joint", &out_part->distal_joint);

    // Pivots are authored in sheet-space coordinates so they are easy to measure
    // directly in tools like Aseprite. Convert them to part-local coordinates here.
    out_part->pivot.x -= static_cast<float>(out_part->frame.x);
    out_part->pivot.y -= static_cast<float>(out_part->frame.y);
    if (out_part->has_distal_joint) {
        out_part->distal_joint.x -= static_cast<float>(out_part->frame.x);
        out_part->distal_joint.y -= static_cast<float>(out_part->frame.y);
    }
    return true;
}

bool load_character_rig_from_metadata(SDL_Renderer* renderer, const char* metadata_path, CharacterRigAsset* out_rig)
{
    *out_rig = CharacterRigAsset{};

    const std::string resolved_metadata = resolve_asset_path(metadata_path);
    std::string text;
    if (!read_text_file(resolved_metadata, &text)) {
        return false;
    }

    std::string sheet_name;
    if (!extract_string_value(text, "sheet", &sheet_name) || sheet_name.empty()) {
        return false;
    }

    const std::string metadata_string(metadata_path);
    const size_t slash_pos = metadata_string.find_last_of("/\\");
    const std::string sheet_asset_path = slash_pos == std::string::npos
        ? sheet_name
        : metadata_string.substr(0, slash_pos + 1) + sheet_name;
    if (!out_rig->sheet.load(renderer, sheet_asset_path.c_str(), true)) {
        return false;
    }

    std::string torso_behavior_text;
    if (extract_object_value(text, "torso_behavior", &torso_behavior_text)) {
        extract_float_value(torso_behavior_text, "scale_x_min", &out_rig->torso_scale_x_min);
        extract_float_value(torso_behavior_text, "scale_x_max", &out_rig->torso_scale_x_max);
        extract_float_value(torso_behavior_text, "scale_y_min", &out_rig->torso_scale_y_min);
        extract_float_value(torso_behavior_text, "scale_y_max", &out_rig->torso_scale_y_max);
    }

    std::string parts_text;
    if (!extract_object_value(text, "parts", &parts_text)) {
        return false;
    }

    if (!parse_character_rig_part(parts_text, "head", &out_rig->head) ||
        !parse_character_rig_part(parts_text, "torso", &out_rig->torso) ||
        !parse_character_rig_part(parts_text, "front_upper_arm", &out_rig->front_upper_arm) ||
        !parse_character_rig_part(parts_text, "front_forearm", &out_rig->front_forearm) ||
        !parse_character_rig_part(parts_text, "back_upper_arm", &out_rig->back_upper_arm) ||
        !parse_character_rig_part(parts_text, "back_forearm", &out_rig->back_forearm)) {
        return false;
    }

    std::string torso_text;
    std::string anchors_text;
    if (!extract_object_value(parts_text, "torso", &torso_text) ||
        !extract_object_value(torso_text, "anchors", &anchors_text)) {
        return false;
    }
    if (!parse_point_value(anchors_text, "front_shoulder", &out_rig->torso_front_shoulder) ||
        !parse_point_value(anchors_text, "back_shoulder", &out_rig->torso_back_shoulder) ||
        !parse_point_value(anchors_text, "pelvis", &out_rig->torso_pelvis)) {
        return false;
    }

    out_rig->loaded = true;
    return true;
}

struct UiSkinScanContext {
    SDL_Renderer* renderer;
    std::unordered_map<std::string, ControlStyle>* out_skins;
};

bool load_scene_manifest(std::unordered_map<std::string, std::string>* out_paths)
{
    out_paths->clear();

    const std::string resolved_manifest = resolve_asset_path("assets/scenes/scenes.json");
    std::string text;
    if (!read_text_file(resolved_manifest, &text)) {
        return false;
    }

    std::string scenes_object;
    if (!extract_object_value(text, "scenes", &scenes_object)) {
        std::fprintf(stderr, "Scene manifest missing scenes object: %s\n", resolved_manifest.c_str());
        return false;
    }

    const std::vector<std::string> scene_names = extract_object_keys(scenes_object);
    for (size_t i = 0; i < scene_names.size(); ++i) {
        const std::string key_token = std::string("\"") + scene_names[i] + "\"";
        const size_t key_pos = scenes_object.find(key_token);
        if (key_pos == std::string::npos) {
            continue;
        }

        std::string scene_object;
        if (!extract_object_value(scenes_object.substr(key_pos), scene_names[i].c_str(), &scene_object)) {
            continue;
        }

        std::string relative_path;
        if (!extract_string_value(scene_object, "path", &relative_path) || relative_path.empty()) {
            std::fprintf(stderr, "Scene manifest entry missing path: %s\n", scene_names[i].c_str());
            return false;
        }

        (*out_paths)[scene_names[i]] = std::string("assets/scenes/") + relative_path;
    }

    return !out_paths->empty();
}

const char* scene_asset_path(
    const std::unordered_map<std::string, std::string>& scene_paths,
    const char* scene_name,
    const char* fallback_asset_path)
{
    const auto it = scene_paths.find(scene_name);
    if (it == scene_paths.end()) {
        return fallback_asset_path;
    }
    return it->second.c_str();
}

bool scan_ui_skin_file(UiSkinScanContext* context, const char* metadata_asset_path)
{
    const std::string metadata_name(metadata_asset_path);
    if (metadata_name.find(".example.json") != std::string::npos) {
        return true;
    }

    const std::string resolved_metadata = resolve_asset_path(metadata_asset_path);
    std::string text;
    if (!read_text_file(resolved_metadata, &text)) {
        return true;
    }

    std::string controls_object;
    if (!extract_object_value(text, "controls", &controls_object)) {
        return true;
    }

    std::string sheet_asset_path;
    if (!resolve_control_style_sheet(metadata_asset_path, &sheet_asset_path)) {
        return false;
    }

    const std::string resolved_sheet = resolve_asset_path(sheet_asset_path.c_str());
    if (!path_exists(resolved_sheet)) {
        std::fprintf(stderr, "Skipping UI control metadata with missing sheet: %s -> %s\n", metadata_asset_path, sheet_asset_path.c_str());
        return true;
    }

    const std::vector<std::string> control_names = extract_object_keys(controls_object);
    for (size_t i = 0; i < control_names.size(); ++i) {
        const std::string& control_name = control_names[i];
        ControlStyle style;
        if (!style.load(context->renderer, sheet_asset_path.c_str(), metadata_asset_path, control_name.c_str())) {
            std::fprintf(stderr, "Failed to load UI control style %s from %s\n", control_name.c_str(), metadata_asset_path);
            return false;
        }
        (*context->out_skins)[control_name] = std::move(style);
    }

    return true;
}

SDL_EnumerationResult SDLCALL enumerate_ui_skin_paths(void* userdata, const char* dirname, const char* fname)
{
    UiSkinScanContext* context = static_cast<UiSkinScanContext*>(userdata);
    const std::string full_path = std::string(dirname) + fname;

    SDL_PathInfo info;
    if (!SDL_GetPathInfo(full_path.c_str(), &info)) {
        return SDL_ENUM_FAILURE;
    }

    if (info.type == SDL_PATHTYPE_DIRECTORY) {
        if (!SDL_EnumerateDirectory(full_path.c_str(), enumerate_ui_skin_paths, userdata)) {
            return SDL_ENUM_FAILURE;
        }
        return SDL_ENUM_CONTINUE;
    }

    const std::string name(fname);
    if (name.size() < 5 || name.substr(name.size() - 5) != ".json") {
        return SDL_ENUM_CONTINUE;
    }

    std::string asset_path = full_path;
    std::replace(asset_path.begin(), asset_path.end(), '\\', '/');
    const std::string cwd_prefix = std::string("./");
    if (asset_path.compare(0, cwd_prefix.size(), cwd_prefix) == 0) {
        asset_path = asset_path.substr(cwd_prefix.size());
    }

    if (!scan_ui_skin_file(context, asset_path.c_str())) {
        return SDL_ENUM_FAILURE;
    }
    return SDL_ENUM_CONTINUE;
}

bool load_all_ui_skins(SDL_Renderer* renderer, std::unordered_map<std::string, ControlStyle>* out_skins)
{
    out_skins->clear();

    UiSkinScanContext context{renderer, out_skins};
    const std::string root = resolve_asset_path("assets/ui");
    if (!SDL_EnumerateDirectory(root.c_str(), enumerate_ui_skin_paths, &context)) {
        std::fprintf(stderr, "Failed to enumerate UI assets in %s: %s\n", root.c_str(), SDL_GetError());
        return false;
    }

    return true;
}

} // namespace

bool Assets::load(SDL_Renderer* renderer)
{
    if (!load_scene_manifest(&scene_asset_paths)) {
        scene_asset_paths.clear();
    }

    bool ok = sky.load(renderer, scene_asset_path(scene_asset_paths, "sky", "assets/scenes/sky1.png"), true) &&
        backcity1.load(renderer, scene_asset_path(scene_asset_paths, "backcity1", "assets/scenes/backcity1.png"), true) &&
        backcity2.load(renderer, scene_asset_path(scene_asset_paths, "backcity2", "assets/scenes/backcity2.png"), true) &&
        backcity3.load(renderer, scene_asset_path(scene_asset_paths, "backcity3", "assets/scenes/backcity3.png"), true) &&
        building.load(renderer, scene_asset_path(scene_asset_paths, "building", "assets/scenes/building1.png"), true) &&
        bench.load(renderer, scene_asset_path(scene_asset_paths, "bench", "assets/scenes/bench.png"), true) &&
        market.load(renderer, scene_asset_path(scene_asset_paths, "market", "assets/scenes/market.png"), true) &&
        notebook.load(renderer, scene_asset_path(scene_asset_paths, "notebook", "assets/scenes/notebook.png"), true) &&
        hero.load(renderer, "assets/characters/man1.png", true) &&
        bullet_icon.load(renderer, "assets/ui/icons/bull.png", true) &&
        load_control_style_from_metadata(renderer, "assets/ui/buttons/ui_button1.json", "button_square_bronze", &title_button_skin) &&
        load_control_style_from_metadata(renderer, "assets/ui/cards/ui_card1.json", "card1_weapon_row", &weapon_card_style) &&
        load_control_style_from_metadata(renderer, "assets/ui/cards/ui_card2.json", "card2_inventory_slot", &inventory_card_style) &&
        load_control_style_from_metadata(renderer, "assets/ui/scrolls/ui_scroll1.json", "scrollbar_vertical_track", &scrollbar_vertical_track_style) &&
        load_control_style_from_metadata(renderer, "assets/ui/scrolls/ui_scroll1.json", "scrollbar_vertical_fill", &scrollbar_vertical_fill_style) &&
        load_control_style_from_metadata(renderer, "assets/ui/scrolls/ui_scroll1.json", "scrollbar_vertical_thumb", &scrollbar_vertical_thumb_style) &&
        load_control_style_from_metadata(renderer, "assets/ui/scrolls/ui_scroll1.json", "scrollbar_horizontal_track", &scrollbar_horizontal_track_style) &&
        load_control_style_from_metadata(renderer, "assets/ui/scrolls/ui_scroll1.json", "scrollbar_horizontal_fill", &scrollbar_horizontal_fill_style) &&
        load_control_style_from_metadata(renderer, "assets/ui/scrolls/ui_scroll1.json", "scrollbar_horizontal_thumb", &scrollbar_horizontal_thumb_style) &&
        load_control_style_from_metadata(renderer, "assets/ui/progress/ui_progress1.json", "progressbar_horizontal_track", &progressbar_horizontal_track_style) &&
        load_control_style_from_metadata(renderer, "assets/ui/progress/ui_progress1.json", "progressbar_horizontal_fill", &progressbar_horizontal_fill_style) &&
        load_control_style_from_metadata(renderer, "assets/ui/progress/ui_progress1.json", "progressbar_vertical_track", &progressbar_vertical_track_style) &&
        load_control_style_from_metadata(renderer, "assets/ui/progress/ui_progress1.json", "progressbar_vertical_fill", &progressbar_vertical_fill_style) &&
        load_control_style_from_metadata(renderer, "assets/ui/panels/ui_panel1.json", "panel_square_bronze", &panel_skin) &&
        smoke.load(renderer, "assets/effects/smog.png", true) &&
        zombie.load(renderer, "assets/characters/zom1.png", true) &&
        zombie_mask.load("assets/characters/zom1.png");

    if (!ok) {
        return false;
    }

    load_character_rig_from_metadata(renderer, "assets/characters/man1_rig.json", &hero_rig);

    if (!load_all_ui_skins(renderer, &ui_skins)) {
        return false;
    }

    load_optional_sheet(
        renderer,
        &explosion_sheet,
        &explosion_sheet_meta,
        &has_explosion_sheet,
        "assets/effects/boom1_strip.png",
        "assets/effects/boom1_strip.sheet");
    load_optional_sheet(
        renderer,
        &heavy_explosion_sheet,
        &heavy_explosion_sheet_meta,
        &has_heavy_explosion_sheet,
        "assets/effects/boom2_strip.png",
        "assets/effects/boom2_strip.sheet");
    load_optional_sheet(
        renderer,
        &grenade_effect_sheet,
        &grenade_effect_sheet_meta,
        &has_grenade_effect_sheet,
        "assets/effects/gre_strip.png",
        "assets/effects/gre_strip.sheet");

    return ok && has_explosion_sheet;
}

const ControlStyle* Assets::find_ui_skin(const std::string& name) const
{
    const auto it = ui_skins.find(name);
    if (it == ui_skins.end()) {
        return nullptr;
    }
    return &it->second;
}

const std::string* Assets::find_scene_asset_path(const std::string& name) const
{
    const auto it = scene_asset_paths.find(name);
    if (it == scene_asset_paths.end()) {
        return nullptr;
    }
    return &it->second;
}

} // namespace zg
