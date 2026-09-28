// SPDX-License-Identifier: LGPL-2.1-or-later
// SPDX-FileCopyrightText: 2026 Ladislav Michl
// SPDX-FileNotice: Part of the FreeCAD project.

#include <algorithm>
#include <filesystem>
#include <string>
#include <system_error>

#include <Base/FileInfo.h>

#include "PathUtils.h"

namespace fs = std::filesystem;

fs::path Materials::pathFromUtf8(std::string_view utf8)
{
    if (utf8.empty()) {
        return {};
    }
    return Base::FileInfo::stringToPath(std::string(utf8));
}

std::string Materials::pathToPortableUtf8(const fs::path& path)
{
    std::string result = Base::FileInfo::pathToString(path);
#if defined(FC_OS_WIN32)
    std::replace(result.begin(), result.end(), '\\', '/');
#endif
    return result;
}

fs::path Materials::canonicalIfExists(const fs::path& path)
{
    std::error_code error;
    if (!fs::exists(path, error) || error) {
        return path;
    }

    fs::path canon = fs::canonical(path, error);
    if (error) {
        return path;
    }
    return canon;
}
