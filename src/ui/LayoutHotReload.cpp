#include "LayoutHotReload.h"

#include "../AssetPaths.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace zg {
namespace ui {

void LayoutHotReload::set_path(const char* path)
{
    path_ = resolve_asset_path(path != nullptr ? path : "");
}

void LayoutHotReload::mark_loaded()
{
    last_stamp_ = read_file_stamp(path_.c_str());
}

bool LayoutHotReload::poll_changed()
{
    if (path_.empty()) {
        return false;
    }

    const FileStamp current_stamp = read_file_stamp(path_.c_str());
    if (!current_stamp.valid()) {
        return false;
    }
    if (!last_stamp_.valid()) {
        last_stamp_ = current_stamp;
        return false;
    }
    if (current_stamp != last_stamp_) {
        last_stamp_ = current_stamp;
        return true;
    }
    return false;
}

LayoutHotReload::FileStamp LayoutHotReload::read_file_stamp(const char* path)
{
    if (path == nullptr || path[0] == '\0') {
        return {};
    }

    WIN32_FILE_ATTRIBUTE_DATA data;
    if (!GetFileAttributesExA(path, GetFileExInfoStandard, &data)) {
        return {};
    }

    const std::uint64_t high = static_cast<std::uint64_t>(data.ftLastWriteTime.dwHighDateTime);
    const std::uint64_t low = static_cast<std::uint64_t>(data.ftLastWriteTime.dwLowDateTime);
    const std::uint64_t ticks = (high << 32) | low;
    if (ticks == 0) {
        return {};
    }

    const std::uint64_t size_high = static_cast<std::uint64_t>(data.nFileSizeHigh);
    const std::uint64_t size_low = static_cast<std::uint64_t>(data.nFileSizeLow);

    FileStamp stamp;
    stamp.write_ticks = static_cast<std::int64_t>(ticks);
    stamp.file_size = static_cast<std::uintmax_t>((size_high << 32) | size_low);
    return stamp;
}

} // namespace ui
} // namespace zg
