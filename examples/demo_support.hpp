#pragma once

#include <array>
#include <cstdio>
#include <filesystem>
#include <string>

inline std::string RunCommandCaptureOutput(const std::string &command,
                                           int &exit_code) {
    std::array<char, 256> buffer{};
    std::string output;

    FILE *pipe = popen(command.c_str(), "r");
    if (pipe == nullptr) {
        exit_code = -1;
        return "";
    }

    while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe)) {
        output += buffer.data();
    }

    exit_code = pclose(pipe);
    return output;
}

inline std::filesystem::path FindRepoRoot() {
    try {
        std::filesystem::path exe = std::filesystem::read_symlink("/proc/self/exe");
        if (exe.is_relative()) exe = std::filesystem::absolute(exe);
        std::filesystem::path p = exe.parent_path();
        while (!p.empty() && p != p.root_path()) {
            if (std::filesystem::exists(p / "CMakeLists.txt")) return p;
            p = p.parent_path();
        }
    } catch (...) {
    }

    std::filesystem::path cur = std::filesystem::current_path();
    while (!cur.empty() && cur != cur.root_path()) {
        if (std::filesystem::exists(cur / "CMakeLists.txt")) return cur;
        cur = cur.parent_path();
    }

    return std::filesystem::current_path();
}
