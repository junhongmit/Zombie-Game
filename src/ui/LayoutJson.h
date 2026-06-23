#pragma once

#include "LayoutTypes.h"

#include <string>

namespace zg {

bool read_layout_text_file(const char* path, std::string* out);
bool extract_object_value(const std::string& text, const char* key, std::string* out);
bool extract_string_value(const std::string& text, const char* key, std::string* out);
bool extract_int_value(const std::string& text, const char* key, int* out);
bool extract_float_value(const std::string& text, const char* key, float* out);
bool parse_normalized_rect(const std::string& object_text, NormalizedRect* rect);
bool parse_named_rect(const std::string& object_text, const char* key, NormalizedRect* rect);
void parse_panel_node(const std::string& object_text, const char* key, LayoutPanelNode* node);
void parse_container_node(const std::string& object_text, const char* key, LayoutContainerNode* node);

} // namespace zg
