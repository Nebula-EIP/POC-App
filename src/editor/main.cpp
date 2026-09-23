#include <exception>
#include <iostream>

#include "application.hpp"

namespace {
constexpr int kExitFailure = 1;
constexpr int kExitUsage = 2;
}  // namespace

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "usage: " << (argc > 0 ? argv[0] : "editor")
                  << " <module-path>\n";
        return kExitUsage;
    }

    try {
        editor::Application app{argv[1]};
        return app.Run();
    } catch (const std::exception &error) {
        std::cerr << "fatal: " << error.what() << '\n';
        return kExitFailure;
    }
}