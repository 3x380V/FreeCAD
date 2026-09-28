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

#pragma once

#include <cstddef>
#include <filesystem>
#include <functional>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <Base/BaseClass.h>

#include <Mod/Material/MaterialGlobal.h>

namespace Materials
{

using IconData = std::vector<std::byte>;

/**
 * A material or model library.
 *
 * Two kinds of paths are used throughout the material code:
 *  - library paths are '/' separated strings such as "/System/Metal/Steel.FCMat",
 *    optionally prefixed with the library name. They name entries inside a
 *    library and never depend on the operating system.
 *  - file system paths are std::filesystem::path values and only exist for
 *    local libraries.
 * Library converts between the two.
 *
 * The icon of a library is a reference: a file or Qt resource path that the
 * Gui resolves. Icon bytes are provided only by remote libraries and Python.
 */
class MaterialsExport Library: public Base::BaseClass
{
    TYPESYSTEM_HEADER_WITH_OVERRIDE();

public:
    Library() = default;
    Library(const Library &other) = default;
    Library(const std::string& libraryName, const std::string& icon, bool readOnly = true);
    Library(const std::string& libraryName, IconData icon, bool readOnly);
    Library(const std::string& libraryName,
            const std::filesystem::path& dir,
            const std::string& iconPath,
            bool readOnly = true);
    ~Library() override = default;

    bool isLocal() const;
    void setLocal(bool local);

    const std::string& getName() const
    {
        return _name;
    }
    void setName(std::string newName)
    {
        _name = std::move(newName);
    }
    bool isName(const std::string& name)
    {
        return (_name == name);
    }

    const std::string& getIconPath() const
    {
        return _iconPath;
    }
    void setIconPath(std::string iconPath);
    const IconData& getIconData() const
    {
        return _iconData;
    }
    void setIconData(IconData icon);
    bool hasIconData() const
    {
        return !_iconData.empty();
    }
    bool hasIcon() const
    {
        return !_iconPath.empty() || !_iconData.empty();
    }
    IconData loadIconData() const;
    bool isReadOnly() const
    {
        return _readOnly;
    }
    void setReadOnly(bool readOnly)
    {
        _readOnly = readOnly;
    }

    /// The file system directory of a local library, empty for remote ones
    const std::filesystem::path& getDirectory() const
    {
        return _directory;
    }
    void setDirectory(const std::filesystem::path& directory);

    bool operator==(const Library& library) const;
    bool operator!=(const Library& library) const
    {
        return !operator==(library);
    }

    /// The file system location of the library path \a path
    std::filesystem::path getLocalPath(const std::string& path) const;
    /// The library path of \a path, relative to the library root
    std::string getRelativePath(const std::string& path) const;
    /// The library path of the file system path \a path, relative to the library root
    std::string getRelativePath(const std::filesystem::path& path) const;
    std::string getLibraryPath(const std::string& path, const std::string& filename) const;
    bool isRoot(const std::string& path) const;

    // Validate a remote library against this one (a local library)
    void validate(const Library& remote) const;

    /// Normalize a library path, removing empty, "." and ".." components
    static std::string cleanPath(const std::string& path);
    /// Split \a text on \a separator, keeping empty parts
    static std::vector<std::string> split(const std::string& text, char separator);

    /// Is the file system path \a path inside this library's directory?
    bool startsWithDirectory(const std::filesystem::path& path) const;

    /// Does the last component of \a path start with a dot?
    static bool isHidden(const std::filesystem::path& path);
    /// Call \a callback for every entry below \a directory, skipping hidden ones
    static void
    forEachEntry(const std::filesystem::path& directory,
                 const std::function<void(const std::filesystem::directory_entry&)>& callback);

private:
    /// \a path relative to the library directory, or nothing when it is outside of it
    std::optional<std::filesystem::path>
    relativeToDirectory(const std::filesystem::path& path) const;

    std::string _name;
    std::filesystem::path _directory;
    std::string _iconPath;
    IconData _iconData;
    bool _readOnly;

    bool _local;
};

class MaterialsExport LibraryObject
{
public:
    LibraryObject(std::string uuid, std::string path, std::string name)
        : _uuid(std::move(uuid))
        , _path(std::move(path))
        , _name(std::move(name))
    {}
    ~LibraryObject() = default;

    void setUUID(std::string uuid)
    {
        _uuid = std::move(uuid);
    }
    const std::string& getUUID() const
    {
        return _uuid;
    }

    void setPath(std::string path)
    {
        _path = std::move(path);
    }
    const std::string& getPath() const
    {
        return _path;
    }

    void setName(std::string name)
    {
        _name = std::move(name);
    }
    const std::string& getName() const
    {
        return _name;
    }

private:
    std::string _uuid;
    std::string _path;
    std::string _name;
};

}  // namespace Materials
