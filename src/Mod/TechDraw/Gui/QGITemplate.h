/***************************************************************************
 *   Copyright (c) 2012-2014 Luke Parry <l.parry@warwick.ac.uk>            *
 *                                                                         *
 *   This file is part of the FreeCAD CAx development system.              *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Library General Public           *
 *   License as published by the Free Software Foundation; either          *
 *   version 2 of the License, or (at your option) any later version.      *
 *                                                                         *
 *   This library  is distributed in the hope that it will be useful,      *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU Library General Public License for more details.                  *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this library; see the file COPYING.LIB. If not,    *
 *   write to the Free Software Foundation, Inc., 59 Temple Place,         *
 *   Suite 330, Boston, MA  02111-1307, USA                                *
 *                                                                         *
 ***************************************************************************/

#ifndef DRAWINGGUI_QGRAPHICSITEMTEMPLATE_H
#define DRAWINGGUI_QGRAPHICSITEMTEMPLATE_H

#include <Mod/TechDraw/TechDrawGlobal.h>

#include <QGraphicsItemGroup>
#include <QObject>

#include "QGIUserTypes.h"

QT_BEGIN_NAMESPACE
class QGraphicsScene;
QT_END_NAMESPACE

namespace TechDraw {
class DrawTemplate;
}

namespace TechDrawGui
{
class TemplateTextField;
class QGSPage;

class TechDrawGuiExport QGITemplate : public QObject, public QGraphicsItemGroup
{
    Q_OBJECT

public:
    QGITemplate(QGSPage *);
    ~QGITemplate() override;

    enum {Type = UserType::QGITemplate};
    int type() const override { return Type;}

    void clearContents();

    void setTemplate(TechDraw::DrawTemplate *obj);
    TechDraw::DrawTemplate * getTemplate() { return pageTemplate; }

    inline qreal getY() { return y() * -1; }

    virtual void updateView(bool update = false);
    virtual std::vector<TemplateTextField *> getTextFields() { return textFields; };

    virtual void draw() = 0;

protected:
    TechDraw::DrawTemplate *pageTemplate;

    std::vector<TemplateTextField *> textFields;
};

} // namespace

#endif // DRAWINGGUI_QGRAPHICSITEMTEMPLATE_H
