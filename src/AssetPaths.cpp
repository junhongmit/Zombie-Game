#include "AssetPaths.h"

#include <SDL3/SDL_filesystem.h>

#include <cstdio>
#include <string>
#include <vector>

namespace zg {

namespace {

bool path_exists(const std::string& path)
{
    SDL_PathInfo info;
    if (!SDL_GetPathInfo(path.c_str(), &info)) {
        return false;
    }
    return info.type == SDL_PATHTYPE_FILE || info.type == SDL_PATHTYPE_DIRECTORY;
}

std::string join_path(const std::string& prefix, const std::string& suffix)
{
    if (prefix.empty()) {
        return suffix;
    }
    const char last = prefix[prefix.size() - 1];
    if (last == '/' || last == '\\') {
        return prefix + suffix;
    }
    return prefix + "/" + suffix;
}

} // namespace

std::string resolve_asset_path(const char* relative_path)
{
    if (relative_path == nullptr || relative_path[0] == '\0') {
        return std::string();
    }

    const std::string relative(relative_path);
    if (path_exists(relative)) {
        return relative;
    }

    std::vector<std::string> candidates;
    candidates.push_back(relative);
    candidates.push_back(join_path("..", relative));
    candidates.push_back(join_path("../..", relative));
    candidates.push_back(join_path("../../..", relative));
    candidates.push_back(join_path("../../../..", relative));
    candidates.push_back(join_path("../../../../..", relative));

    if (const char* base_path = SDL_GetBasePath()) {
        const std::string base(base_path);
        candidates.push_back(join_path(base, relative));
        candidates.push_back(join_path(join_path(base, ".."), relative));
        candidates.push_back(join_path(join_path(base, "../.."), relative));
        candidates.push_back(join_path(join_path(base, "../../.."), relative));
        candidates.push_back(join_path(join_path(base, "../../../.."), relative));
        candidates.push_back(join_path(join_path(base, "../../../../.."), relative));
    }

    for (size_t i = 0; i < candidates.size(); ++i) {
        if (path_exists(candidates[i])) {
            return candidates[i];
        }
    }

    return relative;
}

} // namespace zg
