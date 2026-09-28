// SPDX-License-Identifier: LGPL-2.1-or-later

/***************************************************************************
 *   Copyright (c) 2023 David Carter <dcarter@david.carter.ca>             *
 *                                                                         *
 *   This file is part of FreeCAD.                                         *
 *                                                                         *
 *   FreeCAD is free software: you can redistribute it and/or modify it    *
 *   under the terms of the GNU Lesser General Public License as           *
 *   published by the Free Software Foundation, either version 2.1 of the  *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   FreeCAD is distributed in the hope that it will be useful, but        *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU      *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with FreeCAD. If not, see                               *
 *   <https://www.gnu.org/licenses/>.                                      *
 *                                                                         *
 **************************************************************************/

#include <cstddef>
#include <filesystem>
#include <functional>
#include <ios>
#include <iterator>
#include <optional>
#include <span>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

#include <Base/Console.h>
#include <Base/FileInfo.h>
#include "PathUtils.h"
#include <Base/Stream.h>

#include "Exceptions.h"
#include "Library.h"

namespace fs = std::filesystem;

using namespace Materials;

TYPESYSTEM_SOURCE(Materials::Library, Base::BaseClass)

namespace
{

/// The canonical form of a directory, without a trailing separator
fs::path normalizeDirectory(const fs::path& directory)
{
    fs::path normal = directory.lexically_normal();
    if (!normal.has_filename() && normal.has_parent_path()) {
        normal = normal.parent_path();
    }
    return Materials::canonicalIfExists(normal);
}

}  // namespace

Library::Library(const std::string& libraryName, const std::string& iconPath, bool readOnly)
    : _name(libraryName)
    , _readOnly(readOnly)
    , _local(false)
{
    setIconPath(iconPath);
}

Library::Library(const std::string& libraryName, IconData icon, bool readOnly)
    : _name(libraryName)
    , _iconData(std::move(icon))
    , _readOnly(readOnly)
    , _local(false)
{}

Library::Library(const std::string& libraryName,
                 const fs::path& dir,
                 const std::string& iconPath,
                 bool readOnly)
    : _name(libraryName)
    , _directory(normalizeDirectory(dir))
    , _readOnly(readOnly)
    , _local(false)
{
    setIconPath(iconPath);
}

void Library::setIconPath(std::string iconPath)
{
    _iconPath = std::move(iconPath);
    _iconData.clear();
}

void Library::setIconData(IconData icon)
{
    _iconData = std::move(icon);
    _iconPath.clear();
}

IconData Library::loadIconData() const
{
    if (!_iconData.empty() || _iconPath.empty()) {
        return _iconData;
    }

    const Base::FileInfo info(_iconPath);
    if (!info.isFile()) {
        // Qt resource paths and missing files
        return {};
    }

    Base::ifstream file(info, std::ios::in | std::ios::binary);
    const std::string bytes((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    const auto view = std::as_bytes(std::span(bytes));
    return IconData(view.begin(), view.end());
}

bool Library::isLocal() const
{
    return _local;
}

void Library::setLocal(bool local)
{
    _local = local;
}

void Library::setDirectory(const fs::path& directory)
{
    _directory = normalizeDirectory(directory);
}

bool Library::operator==(const Library& library) const
{
    return (getName() == library.getName()) && (_directory == library._directory);
}

void Library::validate(const Library& remote) const
{
    if (getName() != remote.getName()) {
        throw InvalidLibrary("Library names don't match");
    }
    if (hasIconData() && remote.hasIconData() && getIconData() != remote.getIconData()) {
        throw InvalidLibrary("Library icons don't match");
    }

    // Local and remote paths will differ
    if (!remote.getDirectory().empty()) {
        throw InvalidLibrary("Remote library should not have a path");
    }

    if (isReadOnly() != remote.isReadOnly()) {
        throw InvalidLibrary("Library readonly settings don't match");
    }
}

std::optional<fs::path> Library::relativeToDirectory(const fs::path& path) const
{
    if (_directory.empty()) {
        return std::nullopt;
    }

    // Both paths are usually canonical already, so try the cheap lexical test first
    const fs::path normal = path.lexically_normal();
    const fs::path relative = normal.lexically_relative(_directory);
    if (!relative.empty() && *relative.begin() != "..") {
        return relative;
    }

    // Otherwise let the file system decide, which also covers case insensitive
    // file systems and symbolic links. The path itself may not exist yet.
    std::error_code ec;
    for (fs::path ancestor = normal; !ancestor.empty(); ancestor = ancestor.parent_path()) {
        if (fs::equivalent(ancestor, _directory, ec)) {
            return normal.lexically_relative(ancestor);
        }
        if (!ancestor.has_relative_path()) {
            break;  // the root directory
        }
    }

    return std::nullopt;
}

bool Library::startsWithDirectory(const fs::path& path) const
{
    return relativeToDirectory(path).has_value();
}

std::string Library::getRelativePath(const fs::path& path) const
{
    const fs::path relative = relativeToDirectory(path).value_or(path);
    if (relative == ".") {
        return {};
    }

    std::string result = Materials::pathToPortableUtf8(relative);
    if (result.starts_with('/')) {
        result.erase(0, 1);
    }
    return result;
}

std::string Library::getRelativePath(const std::string& path) const
{
    const std::string clean = cleanPath(path);
    const std::string prefix = "/" + getName();
    if (clean == prefix || clean.starts_with(prefix + "/")) {
        // Remove the library name from the path
        std::string relative = clean.substr(prefix.length());
        if (relative.starts_with('/')) {
            relative.erase(0, 1);
        }
        return relative;
    }

    // Not a library path, so it is a file system path
    return getRelativePath(Materials::pathFromUtf8(clean));
}

fs::path Library::getLocalPath(const std::string& path) const
{
    return _directory / Materials::pathFromUtf8(getRelativePath(path));
}

bool Library::isRoot(const std::string& path) const
{
    return getLocalPath("") == getLocalPath(path);
}

std::string Library::getLibraryPath(const std::string& path, const std::string& filename) const
{
    std::string filePath {path};
    if (filePath.ends_with(filename)) {
        filePath.erase(filePath.length() - filename.length());
    }
    if (filePath.ends_with('/')) {
        filePath.pop_back();
    }

    return filePath;
}

bool Library::isHidden(const fs::path& path)
{
    const auto name = path.filename().native();
    return !name.empty() && name.front() == static_cast<fs::path::value_type>('.');
}

void Library::forEachEntry(const fs::path& directory,
                           const std::function<void(const fs::directory_entry&)>& callback)
{
    std::error_code ec;
    for (auto it = fs::recursive_directory_iterator(directory,
                                                    fs::directory_options::skip_permission_denied,
                                                    ec);
         !ec && it != fs::recursive_directory_iterator();
         it.increment(ec)) {
        if (isHidden(it->path())) {
            it.disable_recursion_pending();
            continue;
        }
        callback(*it);
    }

    if (ec) {
        Base::Console().log("Unable to read directory '{}': {}\n",
                            Materials::pathToPortableUtf8(directory),
                            ec.message());
    }
}

std::string Library::cleanPath(const std::string& path)
{
    std::vector<std::string> parts;
    for (const auto& part : split(path, '/')) {
        if (part.empty() || part == ".") {
            continue;
        }
        if (part == "..") {
            if (!parts.empty() && parts.back() != "..") {
                parts.pop_back();
            }
            else if (!path.starts_with('/')) {
                parts.push_back(part);  // keep leading ".." of a relative path
            }
            continue;
        }
        parts.push_back(part);
    }

    std::string clean = path.starts_with('/') ? "/" : "";
    for (std::size_t i = 0; i < parts.size(); ++i) {
        if (i > 0) {
            clean += '/';
        }
        clean += parts[i];
    }
    return clean;
}

std::vector<std::string> Library::split(const std::string& text, char separator)
{
    std::vector<std::string> parts;

    for (std::size_t pos = 0;;) {
        const auto end = text.find(separator, pos);
        parts.push_back(text.substr(pos, end == std::string::npos ? end : end - pos));
        if (end == std::string::npos) {
            return parts;
        }
        pos = end + 1;
    }
}
