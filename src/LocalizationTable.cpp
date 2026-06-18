#include "LocalizationTable.h"

#include "AssetPaths.h"

#include <cctype>
#include <fstream>
#include <sstream>

namespace zg {

namespace {

std::string trim(const std::string& value)
{
    size_t start = 0;
    while (start < value.size() && std::isspace(static_cast<unsigned char>(value[start]))) {
        ++start;
    }

    size_t end = value.size();
    while (end > start && std::isspace(static_cast<unsigned char>(value[end - 1]))) {
        --end;
    }

    return value.substr(start, end - start);
}

} // namespace

bool LocalizationTable::load(const char* asset_path)
{
    entries_.clear();

    const std::string resolved = resolve_asset_path(asset_path);
    std::ifstream input(resolved.c_str(), std::ios::in | std::ios::binary);
    if (!input.is_open()) {
        return false;
    }

    std::string line;
    while (std::getline(input, line)) {
        const std::string trimmed = trim(line);
        if (trimmed.empty() || trimmed[0] == '#') {
            continue;
        }

        const size_t separator = trimmed.find('=');
        if (separator == std::string::npos) {
            continue;
        }

        const std::string key = trim(trimmed.substr(0, separator));
        const std::string value = trim(trimmed.substr(separator + 1));
        if (!key.empty()) {
            entries_[key] = value;
        }
    }

    return true;
}

const std::string& LocalizationTable::get(const char* key, const char* fallback) const
{
    static std::string fallback_storage;

    const auto it = entries_.find(key != nullptr ? key : "");
    if (it != entries_.end()) {
        return it->second;
    }

    fallback_storage = fallback != nullptr ? fallback : "";
    return fallback_storage;
}

const std::string& LocalizationTable::resolve_token(const std::string& text) const
{
    if (!text.empty() && text[0] == '$') {
        return get(text.c_str() + 1, text.c_str() + 1);
    }
    static std::string literal_storage;
    literal_storage = text;
    return literal_storage;
}

} // namespace zg
