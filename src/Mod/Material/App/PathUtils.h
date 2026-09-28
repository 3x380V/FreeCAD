// SPDX-License-Identifier: LGPL-2.1-or-later
// SPDX-FileCopyrightText: 2026 Ladislav Michl
// SPDX-FileNotice: Part of the FreeCAD project.

#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>

#include <Mod/Material/MaterialGlobal.h>

// Path conversions with the interface of Base/PathUtils.h, which is not in the
// core yet. Once it is, delete this header and include the Base one instead.

namespace Materials
{

/// Convert a UTF-8 encoded string to a platform file system path
MaterialsExport std::filesystem::path pathFromUtf8(std::string_view utf8);

/// Convert a file system path to UTF-8 using '/' as the separator
MaterialsExport std::string pathToPortableUtf8(const std::filesystem::path& path);

/// Returns canonical(path) if it exists, otherwise the input unchanged
MaterialsExport std::filesystem::path canonicalIfExists(const std::filesystem::path& path);

}  // namespace Materials
