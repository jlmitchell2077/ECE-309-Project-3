#include "tools/workspace.h"
#include <stdexcept>
#include <string>
#include <vector>

namespace fs = std::filesystem;

bool Workspace::resolve(std::string_view user_path, fs::path* out, std::string* reason) const {
    if (user_path.empty()) {
        *reason = "empty path";
        return false;
    }

    for (char character : user_path) {
        if (character == '\0') {
            *reason = "path contains NUL";
            return false;
        }
    }

    try {
        std::string path_str(user_path);
        fs::path candidate(path_str);

        if (candidate.is_absolute() || candidate.has_root_path()) {
            *reason = "path must be workspace-relative";
            return false;
        }

        fs::path normalized = candidate.lexically_normal();
        auto first_it = normalized.begin();

        if (first_it != normalized.end() && *first_it == "..") {
            *reason = "path escapes workspace";
            return false;
        }

        if (normalized == ".") {
            *out = root();
        } else {
            *out = root() / normalized;
        }

        reason->clear();
        return true;

    } catch (const std::exception&) {
        *reason = "cannot resolve path";
        return false;
    }
}