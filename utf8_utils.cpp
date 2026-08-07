#include "utf8_utils.h"

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <Windows.h>
#endif

namespace Utf8 {

#ifdef _WIN32

std::string PathToUtf8(const fs::path& p) {
    const std::wstring& wide = p.native();
    if (wide.empty()) return {};

    int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), (int)wide.size(),
                                          nullptr, 0, nullptr, nullptr);
    std::string out(sizeNeeded, 0);
    WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), (int)wide.size(),
                         out.data(), sizeNeeded, nullptr, nullptr);
    return out;
}

fs::path Utf8ToPath(const std::string& utf8) {
    if (utf8.empty()) return {};

    int sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), (int)utf8.size(),
                                          nullptr, 0);
    std::wstring wide(sizeNeeded, 0);
    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), (int)utf8.size(),
                         wide.data(), sizeNeeded);
    return fs::path(wide);
}

fs::path GetExeDir() {
    wchar_t buf[MAX_PATH];
    DWORD len = GetModuleFileNameW(nullptr, buf, MAX_PATH);
    if (len == 0 || len == MAX_PATH) return fs::current_path();
    return fs::path(buf).parent_path();
}

#else

    std::string PathToUtf8(const fs::path& p) {
    return p.string();
}

fs::path Utf8ToPath(const std::string& utf8) {
    return fs::path(utf8);
}

fs::path GetExeDir() {
#if defined(__linux__)
    std::error_code ec;
    fs::path self = fs::read_symlink("/proc/self/exe", ec);
    if (!ec && !self.empty()) return self.parent_path();
#endif
    return fs::current_path();
}

#endif

}
