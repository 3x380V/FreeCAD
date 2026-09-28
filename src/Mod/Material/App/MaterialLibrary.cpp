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

#include <filesystem>
#include <fstream>
#include <system_error>
#include <vector>

#include <App/Application.h>
#include <Base/Console.h>
#include "PathUtils.h"

#include "MaterialFilter.h"
#include "MaterialLibrary.h"
#include "MaterialLoader.h"
#include "MaterialManager.h"
#include "Materials.h"
#include "ModelManager.h"

namespace fs = std::filesystem;

using namespace Materials;

/* TRANSLATOR Material::Materials */

TYPESYSTEM_SOURCE(Materials::MaterialLibrary, Base::BaseClass)

MaterialLibrary::MaterialLibrary(const std::string& libraryName, const std::string& icon, bool readOnly)
    : Library(libraryName, icon, readOnly)
{}

MaterialLibrary::MaterialLibrary(const std::string& libraryName,
                                 const fs::path& dir,
                                 const std::string& icon,
                                 bool readOnly)
    : Library(libraryName, dir, icon, readOnly)
{}

MaterialLibrary::MaterialLibrary(const Library& library)
    : Library(library)
{}

std::shared_ptr<std::map<std::string, std::shared_ptr<MaterialTreeNode>>>
MaterialLibrary::getMaterialTree(const Materials::MaterialFilter& filter,
                                 const Materials::MaterialFilterOptions& options) const
{
    std::shared_ptr<std::map<std::string, std::shared_ptr<MaterialTreeNode>>> materialTree =
        std::make_shared<std::map<std::string, std::shared_ptr<MaterialTreeNode>>>();

    auto materials = MaterialManager::getManager().libraryMaterials(getName(), filter, options, isLocal());
    for (auto& it : *materials) {
        auto uuid = it.getUUID();
        auto path = it.getPath();
        auto filename = it.getName();

        std::vector<std::string> list = split(path, '/');

        // Start at the root
        std::shared_ptr<std::map<std::string, std::shared_ptr<MaterialTreeNode>>> node =
            materialTree;
        for (auto& itp : list) {
            if (!itp.empty()) {
                // Add the folder only if it's not already there
                if (!node->contains(itp)) {
                    auto mapPtr = std::make_shared<
                        std::map<std::string, std::shared_ptr<MaterialTreeNode>>>();
                    std::shared_ptr<MaterialTreeNode> child =
                        std::make_shared<MaterialTreeNode>();
                    child->setFolder(mapPtr);
                    child->setReadOnly(isReadOnly());
                    (*node)[itp] = child;
                    node = mapPtr;
                }
                else {
                    node = (*node)[itp]->getFolder();
                }
            }
        }
        std::shared_ptr<MaterialTreeNode> child = std::make_shared<MaterialTreeNode>();
        child->setUUID(uuid);
        child->setReadOnly(isReadOnly());
        if (isLocal()) {
            auto material = MaterialManager::getManager().getMaterial(uuid);
            child->setOldFormat(material->isOldFormat());
        }
        (*node)[filename] = child;
    }

    // // Empty folders aren't included in _materialPathMap, so we add them by looking at the file
    // // system
    // if (!filter || options.includeEmptyFolders()) {
    //     if (isLocal()) {
    //         auto& materialLibrary =
    //             *(reinterpret_cast<const Materials::MaterialLibraryLocal*>(this));
    //         auto folderList = MaterialLoader::getMaterialFolders(materialLibrary);
    //         for (auto& folder : *folderList) {
    //             std::vector<std::string> list = folder.split("/");

    //             // Start at the root
    //             auto node = materialTree;
    //             for (auto& itp : list) {
    //                 // Add the folder only if it's not already there
    //                 if (!node->contains(itp)) {
    //                     std::shared_ptr<std::map<std::string, std::shared_ptr<MaterialTreeNode>>>
    //                         mapPtr = std::make_shared<
    //                             std::map<std::string, std::shared_ptr<MaterialTreeNode>>>();
    //                     std::shared_ptr<MaterialTreeNode> child =
    //                         std::make_shared<MaterialTreeNode>();
    //                     child->setFolder(mapPtr);
    //                     (*node)[itp] = child;
    //                     node = mapPtr;
    //                 }
    //                 else {
    //                     node = (*node)[itp]->getFolder();
    //                 }
    //             }
    //         }
    //     }
    // }

    return materialTree;
}

/* TRANSLATOR Material::Materials */

TYPESYSTEM_SOURCE(Materials::MaterialLibraryLocal, Materials::MaterialLibrary)

MaterialLibraryLocal::MaterialLibraryLocal(const std::string& libraryName,
                                           const fs::path& dir,
                                           const std::string& icon,
                                           bool readOnly)
    : MaterialLibrary(libraryName, dir, icon, readOnly)
    , _materialPathMap(std::make_unique<std::map<std::string, std::shared_ptr<Material>>>())
{
    setLocal(true);
}

void MaterialLibraryLocal::createFolder(const std::string& path)
{
    const fs::path filePath = getLocalPath(path);
    std::error_code ec;
    fs::create_directories(filePath, ec);
    if (ec) {
        Base::Console().error("Unable to create directory path '{}'\n",
                              Materials::pathToPortableUtf8(filePath));
    }
}

void MaterialLibraryLocal::renameFolder(const std::string& oldPath, const std::string& newPath)
{
    const fs::path filePath = getLocalPath(oldPath);
    if (fs::exists(filePath)) {
        std::error_code ec;
        fs::rename(filePath, getLocalPath(newPath), ec);
        if (ec) {
            Base::Console().error("Unable to rename directory path '{}'\n",
                                  Materials::pathToPortableUtf8(filePath));
        }
    }

    updatePaths(oldPath, newPath);
}

void MaterialLibraryLocal::deleteRecursive(const std::string& path)
{
    if (isRoot(path)) {
        return;
    }

    const fs::path filePath = getLocalPath(path);
    auto& manager = MaterialManager::getManager();
    if (fs::is_directory(filePath)) {
        deleteDir(manager, filePath);
    }
    else {
        deleteFile(manager, filePath);
    }
}

void MaterialLibraryLocal::deleteDir(MaterialManager& manager, const fs::path& path)
{
    // Collect the children first so the directory isn't changed while iterating it
    std::vector<fs::path> dirList;
    std::vector<fs::path> fileList;
    std::error_code ec;
    for (const auto& entry : fs::directory_iterator(path, ec)) {
        if (entry.is_directory()) {
            dirList.push_back(entry.path());
        }
        else if (entry.is_regular_file()) {
            fileList.push_back(entry.path());
        }
    }

    // Remove the subdirs first
    for (const auto& dirPath : dirList) {
        deleteDir(manager, dirPath);
    }

    // Remove the files
    for (const auto& filePath : fileList) {
        deleteFile(manager, filePath);
    }

    // Finally, remove ourself
    if (!fs::remove(path, ec) || ec) {
        throw DeleteError(Materials::pathToPortableUtf8(path));
    }
}

void MaterialLibraryLocal::deleteFile(MaterialManager& manager, const fs::path& path)
{
    std::error_code ec;
    if (fs::remove(path, ec) && !ec) {
        // Remove from the map
        const std::string rPath = getRelativePath(path);
        try {
            auto material = getMaterialByPath(rPath);
            manager.remove(material->getUUID());
        }
        catch (const MaterialNotFound&) {
            Base::Console().log("Unable to remove file from materials list\n");
        }
        _materialPathMap->erase(rPath);
    }
    else {
        throw DeleteError("DeleteError: Unable to delete " + Materials::pathToPortableUtf8(path));
    }
}

void MaterialLibraryLocal::updatePaths(const std::string& oldPath, const std::string& newPath)
{
    // Update the path map
    const std::string op = getRelativePath(oldPath);
    const std::string np = getRelativePath(newPath);
    std::unique_ptr<std::map<std::string, std::shared_ptr<Material>>> pathMap =
        std::make_unique<std::map<std::string, std::shared_ptr<Material>>>();
    for (auto& itp : *_materialPathMap) {
        std::string path = itp.first;
        if (path.starts_with(op)) {
            path = np + path.substr(op.size());
        }
        itp.second->setDirectory(path);
        (*pathMap)[path] = itp.second;
    }
    _materialPathMap = std::move(pathMap);
}

std::shared_ptr<Material>
MaterialLibraryLocal::saveMaterial(const std::shared_ptr<Material>& material,
                                   const std::string& path,
                                   bool overwrite,
                                   bool saveAsCopy,
                                   bool saveInherited)
{
    const fs::path filePath = getLocalPath(path);
    std::error_code ec;
    fs::create_directories(filePath.parent_path(), ec);
    if (ec) {
        Base::Console().error("Unable to create directory path '{}'\n",
                              Materials::pathToPortableUtf8(filePath.parent_path()));
    }

    if (fs::exists(filePath) && !overwrite) {
        Base::Console().error("File already exists '{}'\n", Materials::pathToPortableUtf8(filePath));
        throw MaterialExists();
    }

    std::ofstream stream(filePath);
    if (stream) {
        // Write the contents
        material->setName(Materials::pathToPortableUtf8(filePath.stem()));
        material->setLibrary(getptr());
        material->setDirectory(getRelativePath(path));
        material->save(stream, overwrite, saveAsCopy, saveInherited);
    }

    return addMaterial(material, path);
}

bool MaterialLibraryLocal::fileExists(const std::string& path) const
{
    return fs::exists(getLocalPath(path));
}

std::shared_ptr<Material>
MaterialLibraryLocal::addMaterial(const std::shared_ptr<Material>& material, const std::string& path)
{
    return addMaterialRelative(material, getRelativePath(path));
}

std::shared_ptr<Material>
MaterialLibraryLocal::addMaterial(const std::shared_ptr<Material>& material, const fs::path& file)
{
    return addMaterialRelative(material, getRelativePath(file));
}

std::shared_ptr<Material>
MaterialLibraryLocal::addMaterialRelative(const std::shared_ptr<Material>& material,
                                          const std::string& filePath)
{
    const std::string filename = filePath.substr(filePath.rfind('/') + 1);
    std::shared_ptr<Material> newMaterial = std::make_shared<Material>(*material);
    newMaterial->setLibrary(getptr());
    newMaterial->setDirectory(getLibraryPath(filePath, filename));
    newMaterial->setFilename(filename);
    (*_materialPathMap)[filePath] = newMaterial;
    return newMaterial;
}

std::shared_ptr<Material> MaterialLibraryLocal::getMaterialByPath(const std::string& path) const
{
    return findMaterial(getRelativePath(path));
}

std::shared_ptr<Material> MaterialLibraryLocal::getMaterialByPath(const fs::path& file) const
{
    return findMaterial(getRelativePath(file));
}

std::shared_ptr<Material> MaterialLibraryLocal::findMaterial(const std::string& filePath) const
{
    auto search = _materialPathMap->find(filePath);
    if (search != _materialPathMap->end()) {
        return search->second;
    }

    throw MaterialNotFound();
}

std::string MaterialLibraryLocal::getUUIDFromPath(const std::string& path) const
{
    return getMaterialByPath(path)->getUUID();
}
