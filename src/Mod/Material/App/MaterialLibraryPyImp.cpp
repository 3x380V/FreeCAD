// SPDX-License-Identifier: LGPL-2.1-or-later

/***************************************************************************
 *   Copyright (c) 2023-2024 David Carter <dcarter@david.carter.ca>        *
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


#include "PathUtils.h"
#include <Base/Quantity.h>
#include <Base/QuantityPy.h>
#include <CXX/Objects.hxx>

#include <span>
#include <string>

#include "MaterialLibrary.h"
#include "PyVariants.h"

#include "MaterialLibraryPy.h"

#include "MaterialLibraryPy.cpp"

using namespace Materials;

// Forward declaration
// static PyObject* pyObjectFromValue(const Value& value);
// static Py::List getList(const ValueList& value);

// returns a string which represents the object e.g. when printed in python
std::string MaterialLibraryPy::representation() const
{
    std::stringstream str;
    str << "<MaterialLibrary object at " << getMaterialLibraryPtr() << ">";

    return str.str();
}

PyObject* MaterialLibraryPy::PyMake(struct _typeobject*, PyObject*, PyObject*)  // Python wrapper
{
    // never create such objects with the constructor
    return new MaterialLibraryPy(new MaterialLibrary());
}

// constructor method
int MaterialLibraryPy::PyInit(PyObject* /*args*/, PyObject* /*kwd*/)
{
    return 0;
}

Py::String MaterialLibraryPy::getName() const
{
    auto filterName = getMaterialLibraryPtr()->getName();
    return {filterName};
}

void MaterialLibraryPy::setName(const Py::String value)
{
    getMaterialLibraryPtr()->setName(value);
}

Py::Object MaterialLibraryPy::getIcon() const
{
    return bytesFromIconData(getMaterialLibraryPtr()->loadIconData());
}

void MaterialLibraryPy::setIcon(const Py::Object value)
{
    if (value.isNone()) {
        getMaterialLibraryPtr()->setIconData({});
    }
    else {
        const std::string bytes = Py::Bytes(value).as_std_string();
        const auto view = std::as_bytes(std::span(bytes));
        getMaterialLibraryPtr()->setIconData(Materials::IconData(view.begin(), view.end()));
    }
}

Py::String MaterialLibraryPy::getIconPath() const
{
    return {getMaterialLibraryPtr()->getIconPath()};
}

void MaterialLibraryPy::setIconPath(const Py::String value)
{
    getMaterialLibraryPtr()->setIconPath(value.as_std_string());
}

Py::String MaterialLibraryPy::getDirectory() const
{
    return {Materials::pathToPortableUtf8(getMaterialLibraryPtr()->getDirectory())};
}

void MaterialLibraryPy::setDirectory(const Py::String value)
{
    getMaterialLibraryPtr()->setDirectory(Materials::pathFromUtf8(value.as_std_string()));
}

Py::Boolean MaterialLibraryPy::getReadOnly() const
{
    return getMaterialLibraryPtr()->isReadOnly();
}

void MaterialLibraryPy::setReadOnly(Py::Boolean value)
{
    getMaterialLibraryPtr()->setReadOnly(value);
}

Py::Boolean MaterialLibraryPy::getLocal() const
{
    return getMaterialLibraryPtr()->isLocal();
}

void MaterialLibraryPy::setLocal(Py::Boolean value)
{
    getMaterialLibraryPtr()->setLocal(value);
}

PyObject* MaterialLibraryPy::getCustomAttributes(const char* /*attr*/) const
{
    return nullptr;
}

int MaterialLibraryPy::setCustomAttributes(const char* /*attr*/, PyObject* /*obj*/)
{
    return 0;
}
