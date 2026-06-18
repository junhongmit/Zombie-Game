#include "LayoutJson.h"

#include "../AssetPaths.h"

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>

namespace zg {

bool read_layout_text_file(const char* path, std::string* out)
{
    const std::string resolved = resolve_asset_path(path);
    std::ifstream input(resolved.c_str(), std::ios::in | std::ios::binary);
    if (!input.is_open()) {
        std::fprintf(stderr, "Failed to open %s\n", resolved.c_str());
        return false;
    }

    std::ostringstream buffer;
    buffer << input.rdbuf();
    *out = buffer.str();
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

bool extract_string_value(const std::string& text, const char* key, std::string* out)
{
    const std::string token = std::string("\"") + key + "\"";
    const size_t key_pos = text.find(token);
    if (key_pos == std::string::npos) {
        return false;
    }
    size_t colon_pos = text.find(':', key_pos + token.size());
    if (colon_pos == std::string::npos) {
        return false;
    }
    size_t value_start = text.find('"', colon_pos + 1);
    if (value_start == std::string::npos) {
        return false;
    }
    size_t value_end = text.find('"', value_start + 1);
    if (value_end == std::string::npos) {
        return false;
    }
    *out = text.substr(value_start + 1, value_end - value_start - 1);
    return true;
}

bool extract_int_value(const std::string& text, const char* key, int* out)
{
    const std::string token = std::string("\"") + key + "\"";
    const size_t key_pos = text.find(token);
    if (key_pos == std::string::npos) {
        return false;
    }

    size_t colon_pos = text.find(':', key_pos + token.size());
    if (colon_pos == std::string::npos) {
        return false;
    }

    size_t value_pos = colon_pos + 1;
    while (value_pos < text.size() && std::isspace(static_cast<unsigned char>(text[value_pos]))) {
        ++value_pos;
    }

    size_t end_pos = value_pos;
    if (end_pos < text.size() && (text[end_pos] == '-' || text[end_pos] == '+')) {
        ++end_pos;
    }
    while (end_pos < text.size() && std::isdigit(static_cast<unsigned char>(text[end_pos]))) {
        ++end_pos;
    }
    if (end_pos == value_pos) {
        return false;
    }

    *out = std::atoi(text.substr(value_pos, end_pos - value_pos).c_str());
    return true;
}

bool extract_float_value(const std::string& text, const char* key, float* out)
{
    const std::string token = std::string("\"") + key + "\"";
    const size_t key_pos = text.find(token);
    if (key_pos == std::string::npos) {
        return false;
    }

    size_t colon_pos = text.find(':', key_pos + token.size());
    if (colon_pos == std::string::npos) {
        return false;
    }

    size_t value_pos = colon_pos + 1;
    while (value_pos < text.size() && std::isspace(static_cast<unsigned char>(text[value_pos]))) {
        ++value_pos;
    }

    size_t end_pos = value_pos;
    if (end_pos < text.size() && (text[end_pos] == '-' || text[end_pos] == '+')) {
        ++end_pos;
    }

    bool saw_digit = false;
    while (end_pos < text.size()) {
        const unsigned char c = static_cast<unsigned char>(text[end_pos]);
        if (std::isdigit(c)) {
            saw_digit = true;
            ++end_pos;
            continue;
        }
        if (text[end_pos] == '.') {
            ++end_pos;
            continue;
        }
        break;
    }

    if (!saw_digit) {
        return false;
    }

    *out = static_cast<float>(std::atof(text.substr(value_pos, end_pos - value_pos).c_str()));
    return true;
}

bool parse_normalized_rect(const std::string& object_text, NormalizedRect* rect)
{
    return extract_float_value(object_text, "x", &rect->x) &&
        extract_float_value(object_text, "y", &rect->y) &&
        extract_float_value(object_text, "w", &rect->w) &&
        extract_float_value(object_text, "h", &rect->h);
}

bool parse_named_rect(const std::string& object_text, const char* key, NormalizedRect* rect)
{
    std::string rect_object;
    if (!extract_object_value(object_text, key, &rect_object)) {
        return false;
    }
    return parse_normalized_rect(rect_object, rect);
}

void parse_container_node(const std::string& object_text, const char* key, LayoutContainerNode* node)
{
    std::string container_object;
    if (!extract_object_value(object_text, key, &container_object)) {
        return;
    }
    parse_normalized_rect(container_object, &node->rect);

    std::string children_object;
    if (!extract_object_value(container_object, "children", &children_object)) {
        return;
    }

    std::string title_object;
    if (extract_object_value(children_object, "title", &title_object)) {
        parse_normalized_rect(title_object, &node->title.rect);
        extract_string_value(title_object, "text", &node->title.text);
    }

    std::string list_view_object;
    if (extract_object_value(children_object, "list_view", &list_view_object)) {
        parse_normalized_rect(list_view_object, &node->list_view);
    }
}

} // namespace zg
