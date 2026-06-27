#include "Weapon.h"

#include "AssetPaths.h"
#include "Constants.h"
#include "MathUtil.h"
#include "WeaponRelight.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <chrono>
#include <map>
#include <sstream>

namespace zg {

namespace {

std::string trim_copy(const std::string& value)
{
    size_t start = 0;
    while (start < value.size() && std::isspace(static_cast<unsigned char>(value[start])) != 0) {
        ++start;
    }

    size_t end = value.size();
    while (end > start && std::isspace(static_cast<unsigned char>(value[end - 1])) != 0) {
        --end;
    }

    return value.substr(start, end - start);
}

std::string lower_copy(std::string value)
{
    for (char& ch : value) {
        ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    }
    return value;
}

int parse_int(const std::string& text)
{
    return std::atoi(text.c_str());
}

float parse_float(const std::string& text)
{
    return static_cast<float>(std::atof(text.c_str()));
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

struct JsonValue {
    enum class Type {
        Null,
        Bool,
        Number,
        String,
        Array,
        Object
    };

    Type type = Type::Null;
    bool bool_value = false;
    double number_value = 0.0;
    std::string string_value;
    std::vector<JsonValue> array_value;
    std::map<std::string, JsonValue> object_value;

    bool is_object() const { return type == Type::Object; }
    bool is_array() const { return type == Type::Array; }
    bool is_string() const { return type == Type::String; }
    bool is_number() const { return type == Type::Number; }
    bool is_bool() const { return type == Type::Bool; }
};

class JsonParser {
public:
    explicit JsonParser(std::string text)
        : text_(std::move(text))
    {
    }

    bool parse(JsonValue* out)
    {
        if (out == nullptr) {
            return false;
        }
        skip_ws();
        if (!parse_value(out)) {
            return false;
        }
        skip_ws();
        return pos_ == text_.size();
    }

private:
    void skip_ws()
    {
        while (pos_ < text_.size() && std::isspace(static_cast<unsigned char>(text_[pos_])) != 0) {
            ++pos_;
        }
    }

    bool parse_value(JsonValue* out)
    {
        skip_ws();
        if (pos_ >= text_.size()) {
            return false;
        }
        const char ch = text_[pos_];
        if (ch == '{') {
            return parse_object(out);
        }
        if (ch == '[') {
            return parse_array(out);
        }
        if (ch == '"') {
            out->type = JsonValue::Type::String;
            return parse_string(&out->string_value);
        }
        if (ch == 't' || ch == 'f') {
            return parse_bool(out);
        }
        if (ch == 'n') {
            return parse_null(out);
        }
        if (ch == '-' || std::isdigit(static_cast<unsigned char>(ch)) != 0) {
            return parse_number(out);
        }
        return false;
    }

    bool parse_object(JsonValue* out)
    {
        if (text_[pos_] != '{') {
            return false;
        }
        ++pos_;
        out->type = JsonValue::Type::Object;
        out->object_value.clear();
        skip_ws();
        if (pos_ < text_.size() && text_[pos_] == '}') {
            ++pos_;
            return true;
        }

        while (pos_ < text_.size()) {
            std::string key;
            if (!parse_string(&key)) {
                return false;
            }
            skip_ws();
            if (pos_ >= text_.size() || text_[pos_] != ':') {
                return false;
            }
            ++pos_;
            JsonValue value;
            if (!parse_value(&value)) {
                return false;
            }
            out->object_value[key] = value;
            skip_ws();
            if (pos_ < text_.size() && text_[pos_] == '}') {
                ++pos_;
                return true;
            }
            if (pos_ >= text_.size() || text_[pos_] != ',') {
                return false;
            }
            ++pos_;
            skip_ws();
        }
        return false;
    }

    bool parse_array(JsonValue* out)
    {
        if (text_[pos_] != '[') {
            return false;
        }
        ++pos_;
        out->type = JsonValue::Type::Array;
        out->array_value.clear();
        skip_ws();
        if (pos_ < text_.size() && text_[pos_] == ']') {
            ++pos_;
            return true;
        }
        while (pos_ < text_.size()) {
            JsonValue value;
            if (!parse_value(&value)) {
                return false;
            }
            out->array_value.push_back(value);
            skip_ws();
            if (pos_ < text_.size() && text_[pos_] == ']') {
                ++pos_;
                return true;
            }
            if (pos_ >= text_.size() || text_[pos_] != ',') {
                return false;
            }
            ++pos_;
            skip_ws();
        }
        return false;
    }

    bool parse_string(std::string* out)
    {
        if (out == nullptr || pos_ >= text_.size() || text_[pos_] != '"') {
            return false;
        }
        ++pos_;
        out->clear();
        while (pos_ < text_.size()) {
            const char ch = text_[pos_++];
            if (ch == '"') {
                return true;
            }
            if (ch == '\\') {
                if (pos_ >= text_.size()) {
                    return false;
                }
                const char esc = text_[pos_++];
                switch (esc) {
                case '"': out->push_back('"'); break;
                case '\\': out->push_back('\\'); break;
                case '/': out->push_back('/'); break;
                case 'b': out->push_back('\b'); break;
                case 'f': out->push_back('\f'); break;
                case 'n': out->push_back('\n'); break;
                case 'r': out->push_back('\r'); break;
                case 't': out->push_back('\t'); break;
                default: return false;
                }
                continue;
            }
            out->push_back(ch);
        }
        return false;
    }

    bool parse_number(JsonValue* out)
    {
        const size_t start = pos_;
        if (text_[pos_] == '-') {
            ++pos_;
        }
        while (pos_ < text_.size() && std::isdigit(static_cast<unsigned char>(text_[pos_])) != 0) {
            ++pos_;
        }
        if (pos_ < text_.size() && text_[pos_] == '.') {
            ++pos_;
            while (pos_ < text_.size() && std::isdigit(static_cast<unsigned char>(text_[pos_])) != 0) {
                ++pos_;
            }
        }
        if (pos_ < text_.size() && (text_[pos_] == 'e' || text_[pos_] == 'E')) {
            ++pos_;
            if (pos_ < text_.size() && (text_[pos_] == '+' || text_[pos_] == '-')) {
                ++pos_;
            }
            while (pos_ < text_.size() && std::isdigit(static_cast<unsigned char>(text_[pos_])) != 0) {
                ++pos_;
            }
        }

        out->type = JsonValue::Type::Number;
        out->number_value = std::atof(text_.substr(start, pos_ - start).c_str());
        return true;
    }

    bool parse_bool(JsonValue* out)
    {
        if (text_.compare(pos_, 4, "true") == 0) {
            pos_ += 4;
            out->type = JsonValue::Type::Bool;
            out->bool_value = true;
            return true;
        }
        if (text_.compare(pos_, 5, "false") == 0) {
            pos_ += 5;
            out->type = JsonValue::Type::Bool;
            out->bool_value = false;
            return true;
        }
        return false;
    }

    bool parse_null(JsonValue* out)
    {
        if (text_.compare(pos_, 4, "null") != 0) {
            return false;
        }
        pos_ += 4;
        out->type = JsonValue::Type::Null;
        return true;
    }

    std::string text_;
    size_t pos_ = 0;
};

const JsonValue* json_member(const JsonValue& object, const char* key)
{
    if (!object.is_object()) {
        return nullptr;
    }
    const std::map<std::string, JsonValue>::const_iterator it = object.object_value.find(key);
    return it != object.object_value.end() ? &it->second : nullptr;
}

std::string json_string(const JsonValue& object, const char* key, const std::string& fallback = std::string())
{
    const JsonValue* value = json_member(object, key);
    return value != nullptr && value->is_string() ? value->string_value : fallback;
}

int json_int(const JsonValue& object, const char* key, int fallback = 0)
{
    const JsonValue* value = json_member(object, key);
    return value != nullptr && value->is_number() ? static_cast<int>(std::round(value->number_value)) : fallback;
}

float json_float(const JsonValue& object, const char* key, float fallback = 0.0f)
{
    const JsonValue* value = json_member(object, key);
    return value != nullptr && value->is_number() ? static_cast<float>(value->number_value) : fallback;
}

bool json_bool(const JsonValue& object, const char* key, bool fallback = false)
{
    const JsonValue* value = json_member(object, key);
    return value != nullptr && value->is_bool() ? value->bool_value : fallback;
}

WeaponDefinition::LocalPoint json_point(const JsonValue& object, const char* key, WeaponDefinition::LocalPoint fallback = {})
{
    const JsonValue* value = json_member(object, key);
    if (value == nullptr || !value->is_object()) {
        return fallback;
    }

    WeaponDefinition::LocalPoint point = fallback;
    point.x = json_float(*value, "x", fallback.x);
    point.y = json_float(*value, "y", fallback.y);
    return point;
}

bool load_weapon_hold_metadata(const std::string& metadata_path, WeaponDefinition* definition)
{
    if (definition == nullptr || metadata_path.empty()) {
        return false;
    }

    const std::string resolved = resolve_asset_path(metadata_path.c_str());
    std::FILE* file = std::fopen(resolved.c_str(), "rb");
    if (file == nullptr) {
        return false;
    }
    std::fseek(file, 0, SEEK_END);
    const long size = std::ftell(file);
    std::rewind(file);
    if (size <= 0) {
        std::fclose(file);
        return false;
    }

    std::string text;
    text.resize(static_cast<size_t>(size));
    const size_t read = std::fread(&text[0], 1, text.size(), file);
    std::fclose(file);
    if (read != text.size()) {
        return false;
    }

    JsonValue root;
    JsonParser parser(text);
    if (!parser.parse(&root) || !root.is_object()) {
        return false;
    }

    definition->rear_wrist = json_point(root, "rear_wrist", definition->rear_wrist);
    definition->front_wrist = json_point(root, "front_wrist", definition->front_wrist);
    definition->muzzle = json_point(root, "muzzle", definition->muzzle);
    definition->mirrored_pair_sprite = json_bool(root, "mirrored_pair_sprite", definition->mirrored_pair_sprite);
    definition->hold_scale = json_float(root, "hold_scale", definition->hold_scale);
    return true;
}

std::string derive_preview_image_path(const std::string& name)
{
    const std::string lowered = lower_copy(name);
    if (lowered.find("glock") != std::string::npos) {
        return "assets/weapons/glock/preview.png";
    }
    if (lowered.find("desert eagle") != std::string::npos || lowered.find("desert_eagle") != std::string::npos) {
        return "assets/weapons/desert_eagle/preview.png";
    }
    if (lowered.find("p90") != std::string::npos) {
        return "assets/weapons/p90/preview.png";
    }
    return std::string();
}

std::string derive_icon_image_path_from_preview(const std::string& preview_image_path)
{
    if (preview_image_path.empty()) {
        return std::string();
    }
    const size_t dot = preview_image_path.find_last_of('.');
    if (dot == std::string::npos) {
        return preview_image_path + "_icon.png";
    }
    return preview_image_path.substr(0, dot) + "_icon.png";
}

std::string derive_icon_image_path(const std::string& name)
{
    const std::string lowered = lower_copy(name);
    if (lowered.find("glock") != std::string::npos) {
        return "assets/weapons/glock/icon.png";
    }
    if (lowered.find("desert eagle") != std::string::npos || lowered.find("desert_eagle") != std::string::npos) {
        return "assets/weapons/desert_eagle/icon.png";
    }
    return std::string();
}

std::string derive_depth_image_path(const std::string& preview_image_path)
{
    if (preview_image_path.empty()) {
        return std::string();
    }
    const size_t dot = preview_image_path.find_last_of('.');
    if (dot == std::string::npos) {
        return preview_image_path + "_depth.png";
    }
    return preview_image_path.substr(0, dot) + "_depth.png";
}

bool finalize_weapon_definition(SDL_Renderer* renderer, WeaponDefinition* definition, std::vector<WeaponDefinition>* definitions);

void load_optional_preview_texture(SDL_Renderer* renderer, WeaponDefinition* definition)
{
    if (definition == nullptr) {
        return;
    }

    if (definition->preview_image_path.empty()) {
        definition->preview_image_path = derive_preview_image_path(definition->name);
    }
    if (definition->preview_image_path.empty()) {
        return;
    }

    const std::string resolved = resolve_asset_path(definition->preview_image_path.c_str());
    if (!file_exists(resolved)) {
        return;
    }
    definition->preview_texture.load(renderer, definition->preview_image_path.c_str(), true);

}

void load_optional_icon_texture(SDL_Renderer* renderer, WeaponDefinition* definition)
{
    if (definition == nullptr) {
        return;
    }

    if (definition->icon_image_path.empty()) {
        definition->icon_image_path = derive_icon_image_path_from_preview(definition->preview_image_path);
    }
    if (definition->icon_image_path.empty()) {
        definition->icon_image_path = derive_icon_image_path(definition->name);
    }
    if (definition->icon_image_path.empty()) {
        return;
    }

    const std::string resolved = resolve_asset_path(definition->icon_image_path.c_str());
    if (!file_exists(resolved)) {
        return;
    }
    definition->icon_texture.load(renderer, definition->icon_image_path.c_str(), true);
}

bool load_weapon_catalog_json(SDL_Renderer* renderer, const std::string& resolved_path, std::vector<WeaponDefinition>* definitions)
{
    std::FILE* file = std::fopen(resolved_path.c_str(), "rb");
    if (file == nullptr) {
        return false;
    }
    std::fseek(file, 0, SEEK_END);
    const long size = std::ftell(file);
    std::rewind(file);
    if (size <= 0) {
        std::fclose(file);
        return false;
    }

    std::string text;
    text.resize(static_cast<size_t>(size));
    const size_t read = std::fread(&text[0], 1, text.size(), file);
    std::fclose(file);
    if (read != text.size()) {
        return false;
    }

    JsonValue root;
    JsonParser parser(text);
    if (!parser.parse(&root)) {
        return false;
    }
    const JsonValue* weapons = json_member(root, "weapons");
    if (weapons == nullptr || !weapons->is_array()) {
        return false;
    }

    for (size_t i = 0; i < weapons->array_value.size(); ++i) {
        const JsonValue& item = weapons->array_value[i];
        if (!item.is_object()) {
            continue;
        }
        WeaponDefinition current;
        current.name = json_string(item, "name");
        current.image_path = json_string(item, "image_path");
        current.hold_metadata_path = json_string(item, "hold_metadata_path");
        current.preview_image_path = json_string(item, "preview_image_path");
        current.icon_image_path = json_string(item, "icon_image_path");
        current.ui_card_template = json_string(item, "ui_card_template", "default");
        current.mirrored_pair_sprite = json_bool(item, "mirrored_pair_sprite", true);
        current.hold_scale = json_float(item, "hold_scale", 1.0f);
        current.route_x = json_int(item, "route_x", 4);
        current.route_y = json_int(item, "route_y", 3);
        current.rear_wrist = json_point(item, "rear_wrist", WeaponDefinition::LocalPoint{
            static_cast<float>(current.route_x),
            static_cast<float>(current.route_y)
        });
        current.front_wrist = json_point(item, "front_wrist", current.rear_wrist);
        current.muzzle = json_point(item, "muzzle", current.front_wrist);
        current.type = json_int(item, "type", 101) == 102 ? WeaponType::Grenade : WeaponType::Gun;
        current.magazine_size = json_int(item, "magazine_size", 0);
        current.damage = json_int(item, "damage", 0);
        current.speed_rpm = json_int(item, "speed_rpm", 600);
        current.price = json_int(item, "price", 0);
        current.initial_reserve = json_int(item, "initial_reserve", 0);
        current.full_auto = json_bool(item, "full_auto", false);
        current.reload_duration = json_float(item, "reload_duration", 1.6f);
        current.diameter = json_float(item, "diameter", 0.0f);
        current.up = json_float(item, "up", 0.0f);
        current.shake_duration = json_float(item, "shake_duration", current.shake_duration);
        current.shake_magnitude = json_float(item, "shake_magnitude", current.shake_magnitude);
        current.loudness = json_float(item, "loudness", current.loudness);
        current.shoot_sound_path = json_string(item, "shoot_sound_path");
        if (!finalize_weapon_definition(renderer, &current, definitions)) {
            return false;
        }
    }
    return !definitions->empty();
}

void apply_default_feedback(WeaponDefinition* definition)
{
    if (definition->name == "Glock") {
        definition->shake_duration = 0.10f;
        definition->shake_magnitude = 1.4f;
        definition->loudness = 0.78f;
    } else if (definition->name == "AK47") {
        definition->shake_duration = 0.14f;
        definition->shake_magnitude = 3.4f;
        definition->loudness = 0.96f;
    } else if (definition->name == "AWM") {
        definition->shake_duration = 0.20f;
        definition->shake_magnitude = 5.0f;
        definition->loudness = 1.0f;
    } else if (definition->name == "AUG") {
        definition->shake_duration = 0.13f;
        definition->shake_magnitude = 2.8f;
        definition->loudness = 0.92f;
    } else if (definition->name == "M249") {
        definition->shake_duration = 0.15f;
        definition->shake_magnitude = 3.8f;
        definition->loudness = 0.98f;
    } else if (definition->full_auto) {
        definition->shake_duration = 0.13f;
        definition->shake_magnitude = 3.0f;
        definition->loudness = 0.94f;
    }
}

bool finalize_weapon_definition(SDL_Renderer* renderer, WeaponDefinition* definition, std::vector<WeaponDefinition>* definitions)
{
    if (renderer == nullptr || definition == nullptr || definitions == nullptr ||
        definition->name.empty() || definition->image_path.empty()) {
        return false;
    }

    if (definition->initial_reserve <= 0) {
        definition->initial_reserve = definition->magazine_size * 5;
    }
    apply_default_feedback(definition);
    if (!definition->hold_metadata_path.empty()) {
        load_weapon_hold_metadata(definition->hold_metadata_path, definition);
    }
    if (!definition->texture.load(renderer, definition->image_path.c_str(), true)) {
        return false;
    }
    const float frame_width_pixels = definition->mirrored_pair_sprite
        ? definition->texture.width() * 0.5f
        : static_cast<float>(definition->texture.width());
    const float frame_width = frame_width_pixels * definition->hold_scale;
    if (definition->front_wrist.x == 0.0f && definition->front_wrist.y == 0.0f) {
        definition->front_wrist.x = std::max(3.6f, frame_width * 0.30f);
        definition->front_wrist.y = definition->rear_wrist.y + 0.7f;
    }
    if (definition->muzzle.x == 0.0f && definition->muzzle.y == 0.0f) {
        definition->muzzle.x = std::max(definition->front_wrist.x + 4.0f, frame_width - 1.0f);
        definition->muzzle.y = definition->rear_wrist.y - 0.4f;
    }
    load_optional_preview_texture(renderer, definition);
    load_optional_icon_texture(renderer, definition);
    definitions->push_back(std::move(*definition));
    *definition = WeaponDefinition{};
    return true;
}

} // namespace

float WeaponDefinition::fire_interval_seconds() const
{
    return speed_rpm > 0 ? 60.0f / static_cast<float>(speed_rpm) : kGlockFireIntervalSeconds;
}

bool WeaponCatalog::load(SDL_Renderer* renderer, const char* path)
{
    definitions_.clear();
    relight_jobs_.clear();

    const std::string resolved = resolve_asset_path(path);
    if (lower_copy(resolved).size() >= 5 && lower_copy(resolved).substr(lower_copy(resolved).size() - 5) == ".json") {
        const bool loaded = load_weapon_catalog_json(renderer, resolved, &definitions_);
        if (loaded) {
            relight_jobs_.resize(definitions_.size());
            for (int i = 0; i < static_cast<int>(definitions_.size()); ++i) {
                schedule_relight_job(i);
            }
        }
        return loaded;
    }

    std::FILE* file = std::fopen(resolved.c_str(), "r");
    if (file == nullptr) {
        return false;
    }

    WeaponDefinition current;
    bool has_current = false;
    char buffer[512];
    while (std::fgets(buffer, sizeof(buffer), file) != nullptr) {
        std::string line = trim_copy(buffer);
        if (line.empty()) {
            continue;
        }

        if (!line.empty() && line[0] == '#') {
            if (lower_copy(line).find("#weapon") == 0) {
                if (has_current && !current.name.empty() && !current.image_path.empty()) {
                    if (!finalize_weapon_definition(renderer, &current, &definitions_)) {
                        std::fclose(file);
                        return false;
                    }
                }
                has_current = true;
            }
            continue;
        }

        const size_t equals = line.find('=');
        if (equals == std::string::npos) {
            continue;
        }

        const std::string key = lower_copy(trim_copy(line.substr(0, equals)));
        const std::string value = trim_copy(line.substr(equals + 1));
        if (key == "name") {
            current.name = value;
        } else if (key == "imagepath") {
            current.image_path = value;
        } else if (key == "shopimage" || key == "previewimage" || key == "preview_image_path") {
            current.preview_image_path = value;
        } else if (key == "iconimage" || key == "icon_image_path" || key == "icon") {
            current.icon_image_path = value;
        } else if (key == "uicardtemplate" || key == "ui_card_template" || key == "cardtemplate") {
            current.ui_card_template = value;
        } else if (key == "type") {
            current.type = parse_int(value) == 102 ? WeaponType::Grenade : WeaponType::Gun;
        } else if (key == "magazine") {
            current.magazine_size = parse_int(value);
        } else if (key == "reloadtime") {
            current.reload_duration = parse_int(value) * 0.05f;
        } else if (key == "diameter" || key == "diamter") {
            current.diameter = parse_float(value);
        } else if (key == "up") {
            current.up = parse_float(value);
        } else if (key == "damage") {
            current.damage = parse_int(value);
        } else if (key == "running" || key == "ruuning") {
            current.full_auto = parse_int(value) != 0;
        } else if (key == "speed") {
            current.speed_rpm = parse_int(value);
        } else if (key == "price") {
            current.price = parse_int(value);
        } else if (key == "sound") {
            current.shoot_sound_path = value;
        } else if (key == "routex") {
            current.route_x = parse_int(value);
            current.rear_wrist.x = static_cast<float>(current.route_x);
        } else if (key == "routey") {
            current.route_y = parse_int(value);
            current.rear_wrist.y = static_cast<float>(current.route_y);
        } else if (key == "rearwristx") {
            current.rear_wrist.x = parse_float(value);
        } else if (key == "rearwristy") {
            current.rear_wrist.y = parse_float(value);
        } else if (key == "frontwristx") {
            current.front_wrist.x = parse_float(value);
        } else if (key == "frontwristy") {
            current.front_wrist.y = parse_float(value);
        } else if (key == "muzzlex") {
            current.muzzle.x = parse_float(value);
        } else if (key == "muzzley") {
            current.muzzle.y = parse_float(value);
        } else if (key == "shakeduration") {
            current.shake_duration = parse_float(value);
        } else if (key == "shakemagnitude") {
            current.shake_magnitude = parse_float(value);
        } else if (key == "loudness") {
            current.loudness = parse_float(value);
        } else if (key == "reserveammo") {
            current.initial_reserve = parse_int(value);
        }
    }
    std::fclose(file);

    if (has_current && !current.name.empty() && !current.image_path.empty()) {
        if (!finalize_weapon_definition(renderer, &current, &definitions_)) {
            return false;
        }
    }

    if (!definitions_.empty()) {
        relight_jobs_.resize(definitions_.size());
        for (int i = 0; i < static_cast<int>(definitions_.size()); ++i) {
            schedule_relight_job(i);
        }
    }
    return !definitions_.empty();
}

void WeaponCatalog::schedule_relight_job(int index)
{
    if (index < 0 || index >= static_cast<int>(definitions_.size()) || index >= static_cast<int>(relight_jobs_.size())) {
        return;
    }
    WeaponDefinition& definition = definitions_[static_cast<size_t>(index)];
    if (definition.preview_image_path.empty()) {
        return;
    }

    const std::string resolved = resolve_asset_path(definition.preview_image_path.c_str());
    if (!file_exists(resolved)) {
        return;
    }

    const std::string depth_path = derive_depth_image_path(definition.preview_image_path);
    const std::string resolved_depth = resolve_asset_path(depth_path.c_str());
    if (!file_exists(resolved_depth)) {
        return;
    }

    RelightJob& job = relight_jobs_[static_cast<size_t>(index)];
    job.scheduled = true;
    job.uploaded = false;
    const std::string albedo_path = definition.preview_image_path;
    const std::string depth_copy = depth_path;
    job.future = std::async(
        std::launch::async,
        [albedo_path, depth_copy]() {
            WorkbenchRelightBakeResult result;
            bake_workbench_relight_assets(albedo_path.c_str(), depth_copy.c_str(), &result);
            return result;
        });
}

void WeaponCatalog::update_relight_jobs(SDL_Renderer* renderer)
{
    if (renderer == nullptr) {
        return;
    }
    for (size_t i = 0; i < relight_jobs_.size(); ++i) {
        RelightJob& job = relight_jobs_[i];
        if (!job.scheduled || job.uploaded || !job.future.valid()) {
            continue;
        }
        const std::future_status status = job.future.wait_for(std::chrono::seconds(0));
        if (status != std::future_status::ready) {
            continue;
        }

        WorkbenchRelightBakeResult baked = job.future.get();
        if (baked.valid) {
            upload_workbench_relight_assets(
                renderer,
                baked,
                &definitions_[i].workbench_lit_texture,
                &definitions_[i].workbench_shadow_texture,
                &definitions_[i].workbench_shadow_placement);
        }
        job.uploaded = true;
    }
}

int WeaponCatalog::count() const
{
    return static_cast<int>(definitions_.size());
}

const WeaponDefinition* WeaponCatalog::definition(int index) const
{
    if (index < 0 || index >= static_cast<int>(definitions_.size())) {
        return nullptr;
    }
    return &definitions_[static_cast<size_t>(index)];
}

WeaponState::WeaponState()
{
}

bool WeaponState::load_default_inventory(const WeaponCatalog& catalog)
{
    slot_count_ = 0;
    active_slot_ = 0;

    for (int i = 0; i < catalog.count() && slot_count_ < kMaxSlots; ++i) {
        const WeaponDefinition* definition = catalog.definition(i);
        if (definition == nullptr || definition->type != WeaponType::Gun) {
            continue;
        }

        WeaponSlot& slot = slots_[slot_count_++];
        slot.definition = definition;
        slot.ammo_in_mag = definition->magazine_size;
        slot.ammo_reserve = definition->initial_reserve;
    }

    if (slot_count_ <= 0) {
        return false;
    }

    sync_from_slot();
    return true;
}

bool WeaponState::switch_to_slot(int slot_index)
{
    if (slot_index < 0 || slot_index >= slot_count_ || slot_index == active_slot_) {
        return false;
    }

    sync_to_slot();
    active_slot_ = slot_index;
    reloading = false;
    reload_timer = 0.0f;
    reload_flash_timer = 0.0f;
    reload_flash_on = false;
    sync_from_slot();
    return true;
}

bool WeaponState::cycle(int delta)
{
    if (slot_count_ <= 1 || delta == 0) {
        return false;
    }

    const int next = (active_slot_ + delta + slot_count_) % slot_count_;
    return switch_to_slot(next);
}

bool WeaponState::can_fire() const
{
    return current_definition() != nullptr && !reloading && ammo_in_mag > 0;
}

void WeaponState::consume_round()
{
    if (ammo_in_mag > 0) {
        --ammo_in_mag;
        sync_to_slot();
    }
    if (ammo_in_mag <= 0) {
        ammo_in_mag = 0;
        sync_to_slot();
        start_reload();
    }
}

void WeaponState::start_reload()
{
    if (reloading || ammo_in_mag >= magazine_size || ammo_reserve <= 0) {
        return;
    }

    reloading = true;
    reload_timer = reload_duration;
    reload_flash_on = false;
    reload_flash_timer = kReloadFlashIntervalSeconds;
}

void WeaponState::update(float dt)
{
    if (!reloading) {
        return;
    }

    reload_timer -= dt;
    reload_flash_timer -= dt;
    while (reload_flash_timer <= 0.0f) {
        reload_flash_on = !reload_flash_on;
        reload_flash_timer += kReloadFlashIntervalSeconds;
    }
    if (reload_timer > 0.0f) {
        return;
    }

    const int needed = magazine_size - ammo_in_mag;
    const int loaded = needed < ammo_reserve ? needed : ammo_reserve;
    ammo_in_mag += loaded;
    ammo_reserve -= loaded;
    reload_timer = 0.0f;
    reload_flash_timer = 0.0f;
    reloading = false;
    reload_flash_on = false;
    sync_to_slot();
}

float WeaponState::indicator_ratio() const
{
    if (reloading) {
        return reload_duration > 0.0f ? clamp_float(reload_timer / reload_duration, 0.0f, 1.0f) : 0.0f;
    }
    return magazine_size > 0 ? clamp_float(static_cast<float>(ammo_in_mag) / static_cast<float>(magazine_size), 0.0f, 1.0f) : 0.0f;
}

int WeaponState::slot_count() const
{
    return slot_count_;
}

int WeaponState::active_slot_index() const
{
    return active_slot_;
}

const WeaponDefinition* WeaponState::current_definition() const
{
    return slot_count_ > 0 ? slots_[active_slot_].definition : nullptr;
}

const WeaponSlot* WeaponState::slots() const
{
    return slots_;
}

void WeaponState::sync_from_slot()
{
    const WeaponDefinition* definition = current_definition();
    if (definition == nullptr) {
        ammo_in_mag = 0;
        ammo_reserve = 0;
        magazine_size = 0;
        reload_duration = 0.0f;
        return;
    }

    const WeaponSlot& slot = slots_[active_slot_];
    ammo_in_mag = slot.ammo_in_mag;
    ammo_reserve = slot.ammo_reserve;
    magazine_size = definition->magazine_size;
    reload_duration = definition->reload_duration;
}

void WeaponState::sync_to_slot()
{
    if (slot_count_ <= 0) {
        return;
    }

    WeaponSlot& slot = slots_[active_slot_];
    slot.ammo_in_mag = ammo_in_mag;
    slot.ammo_reserve = ammo_reserve;
}

} // namespace zg
