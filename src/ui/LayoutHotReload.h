#pragma once

#include <cstdint>
#include <string>

namespace zg {
namespace ui {

class LayoutHotReload {
public:
    LayoutHotReload() = default;

    void set_path(const char* path);
    void mark_loaded();
    bool poll_changed();

private:
    struct FileStamp {
        std::int64_t write_ticks = 0;
        std::uintmax_t file_size = 0;

        bool valid() const
        {
            return write_ticks != 0;
        }

        bool operator==(const FileStamp& other) const
        {
            return write_ticks == other.write_ticks && file_size == other.file_size;
        }

        bool operator!=(const FileStamp& other) const
        {
            return !(*this == other);
        }
    };

    static FileStamp read_file_stamp(const char* path);

    std::string path_;
    FileStamp last_stamp_{};
};

} // namespace ui
} // namespace zg
