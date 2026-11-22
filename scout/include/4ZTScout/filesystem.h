#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>
#include <stdexcept>

#if defined(_WIN32)
    #include <windows.h>
#elif defined(__APPLE__)
    #include <mach-o/dyld.h>
    #include <vector>
#else 
    #include <unistd.h>
    #include <climits>
#endif

// This file comes from the Origin Engine, only the namespace was changed
namespace Scout {

    class FileSystem {
    public:
        static void Mount(const std::string& alias, const std::filesystem::path& physicalPath) {
            s_MountPoints[alias] = std::filesystem::absolute(physicalPath);
        }

        static void Mount(const std::string& alias) {
            s_MountPoints[alias] = GetExecutableDir();
        }

        static void Unmount(const std::string& alias) {
            s_MountPoints.erase(alias);
        }

        static std::filesystem::path Resolve(const std::string& virtualPath) {
            size_t pos = virtualPath.find("://");
            if (pos != std::string::npos) {
                std::string alias = virtualPath.substr(0, pos);
                std::string relative = virtualPath.substr(pos + 3);
                
                auto it = s_MountPoints.find(alias);
                if (it != s_MountPoints.end()) {
                    return (it->second / relative).lexically_normal();
                }
            }
            
            return std::filesystem::path(virtualPath).lexically_normal();
        }

    private:
        static std::filesystem::path GetExecutableDir() {
            return GetExecutablePath().parent_path();
        }

        static std::filesystem::path GetExecutablePath() {
    #if defined(_WIN32)
            wchar_t buffer[MAX_PATH];
            DWORD len = GetModuleFileNameW(nullptr, buffer, MAX_PATH);
            if (len == 0 || len == MAX_PATH)
                throw std::runtime_error("FileSystem: failed to get executable path (Windows)");
            return std::filesystem::path(buffer);
    #elif defined(__APPLE__)
            uint32_t size = 0;
            _NSGetExecutablePath(nullptr, &size);
            std::vector<char> buffer(size);
            if (_NSGetExecutablePath(buffer.data(), &size) != 0)
                throw std::runtime_error("FileSystem: failed to get executable path (macOS)");
            return std::filesystem::canonical(std::filesystem::path(buffer.data()));
    #else
            char buffer[PATH_MAX];
            ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
            if (len == -1)
                throw std::runtime_error("FileSystem: failed to get executable path (Linux)");
            buffer[len] = '\0';
            return std::filesystem::path(buffer);
    #endif
        }

        inline static std::unordered_map<std::string, std::filesystem::path> s_MountPoints;
    };

}