// SPDX-License-Identifier: LGPL-2.1-or-later
// SPDX-FileCopyrightText: 2026 Ladislav Michl
// SPDX-FileNotice: Part of the FreeCAD project.

#include <QByteArrayView>
#include <QFile>
#include <QImage>
#include <QPixmap>
#include <QString>

#include <Base/Console.h>

#include "LibraryIcon.h"

using namespace MatGui;

QIcon MatGui::libraryIcon(const Materials::Library& library)
{
    if (library.hasIconData()) {
        QImage image;
        if (!image.loadFromData(QByteArrayView(library.getIconData()))) {
            Base::Console().log("Unable to load icon image for library '{}'\n", library.getName());
            return {};
        }
        return QIcon(QPixmap::fromImage(image));
    }
    if (!library.getIconPath().empty()) {
        return QIcon(QString::fromStdString(library.getIconPath()));
    }
    return {};
}

Materials::IconData MatGui::libraryIconData(const Materials::Library& library)
{
    if (library.hasIconData() || library.getIconPath().empty()) {
        return library.getIconData();
    }

    // QFile reads Qt resources as well as files
    QFile file(QString::fromStdString(library.getIconPath()));
    if (!file.open(QIODevice::ReadOnly)) {
        Base::Console().log("Failed to open icon file '{}'\n", library.getIconPath());
        return {};
    }

    Materials::IconData data(static_cast<std::size_t>(file.size()));
    const auto read = file.read(reinterpret_cast<char*>(data.data()), static_cast<qint64>(data.size()));
    if (read < 0) {
        return {};
    }
    data.resize(static_cast<std::size_t>(read));
    return data;
}
