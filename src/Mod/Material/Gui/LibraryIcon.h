// SPDX-License-Identifier: LGPL-2.1-or-later
// SPDX-FileCopyrightText: 2026 Ladislav Michl
// SPDX-FileNotice: Part of the FreeCAD project.

#pragma once

#include <QIcon>

#include <Mod/Material/App/Library.h>
#include <Mod/Material/MaterialGlobal.h>

namespace MatGui
{

MatGuiExport QIcon libraryIcon(const Materials::Library& library);
MatGuiExport Materials::IconData libraryIconData(const Materials::Library& library);

}  // namespace MatGui
