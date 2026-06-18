#pragma once

#include <string>
#include <unordered_map>

namespace zg {

class LocalizationTable {
public:
    bool load(const char* asset_path);
    const std::string& get(const char* key, const char* fallback) const;
    const std::string& resolve_token(const std::string& text) const;

private:
    std::unordered_map<std::string, std::string> entries_;
};

} // namespace zg
