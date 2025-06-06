# -*- coding: utf-8 -*-
# ***************************************************************************
# *   Copyright (c) 2017 sliptonic <shopinthewoods@gmail.com>               *
# *                                                                         *
# *   This program is free software; you can redistribute it and/or modify  *
# *   it under the terms of the GNU Lesser General Public License (LGPL)    *
# *   as published by the Free Software Foundation; either version 2 of     *
# *   the License, or (at your option) any later version.                   *
# *   for detail see the LICENCE text file.                                 *
# *                                                                         *
# *   This program is distributed in the hope that it will be useful,       *
# *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
# *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
# *   GNU Library General Public License for more details.                  *
# *                                                                         *
# *   You should have received a copy of the GNU Library General Public     *
# *   License along with this program; if not, write to the Free Software   *
# *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  *
# *   USA                                                                   *
# *                                                                         *
# ***************************************************************************

import FreeCAD
import FreeCADGui
import Path
import Path.Base.Gui.GetPoint as PathGetPoint
import Path.Base.Gui.Util as PathGuiUtil
import Path.Base.SetupSheet as PathSetupSheet
import Path.Base.Util as PathUtil
import Path.Main.Job as PathJob
import Path.Op.Base as PathOp
import Path.Op.Gui.Selection as PathSelection
import PathGui
import PathScripts.PathUtils as PathUtils
import importlib
from PySide.QtCore import QT_TRANSLATE_NOOP

from PySide import QtCore, QtGui

__title__ = "CAM Operation UI base classes"
__author__ = "sliptonic (Brad Collette)"
__url__ = "https://www.freecad.org"
__doc__ = "Base classes and framework for CAM operation's UI"

translate = FreeCAD.Qt.translate

if False:
    Path.Log.setLevel(Path.Log.Level.DEBUG, Path.Log.thisModule())
    Path.Log.trackModule(Path.Log.thisModule())
else:
    Path.Log.setLevel(Path.Log.Level.INFO, Path.Log.thisModule())


class ViewProvider(object):
    """
    Generic view provider for path objects.
    Deducts the icon name from operation name, brings up the TaskPanel
    with pages corresponding to the operation's opFeatures() and forwards
    property change notifications to the page controllers.
    """

    def __init__(self, vobj, resources):
        Path.Log.track()
        self.deleteOnReject = True
        self.OpIcon = ":/icons/%s.svg" % resources.pixmap
        self.OpName = resources.name
        self.OpPageModule = resources.opPageClass.__module__
        self.OpPageClass = resources.opPageClass.__name__

        # initialized later
        self.vobj = vobj
        self.Object = None
        self.panel = None

    def attach(self, vobj):
        Path.Log.track()
        self.vobj = vobj
        self.Object = vobj.Object
        self.panel = None
        return

    def deleteObjectsOnReject(self):
        """
        deleteObjectsOnReject() ... return true if all objects should
        be created if the user hits cancel. This is used during the initial
        edit session, if the user does not press OK, it is assumed they've
        changed their mind about creating the operation.
        """
        Path.Log.track()
        return hasattr(self, "deleteOnReject") and self.deleteOnReject

    def setDeleteObjectsOnReject(self, state=False):
        Path.Log.track()
        self.deleteOnReject = state
        return self.deleteOnReject

    def setEdit(self, vobj=None, mode=0):
        """setEdit(vobj, mode=0) ... initiate editing of receivers model."""
        Path.Log.track()
        if 0 == mode:
            if vobj is None:
                vobj = self.vobj
            page = self.getTaskPanelOpPage(vobj.Object)
            page.setTitle(self.OpName)
            page.setIcon(self.OpIcon)
            selection = self.getSelectionFactory()
            self.setupTaskPanel(
                TaskPanel(vobj.Object, self.deleteObjectsOnReject(), page, selection)
            )
            self.deleteOnReject = False
            return True
        # no other editing possible
        return False

    def setupTaskPanel(self, panel):
        """setupTaskPanel(panel) ... internal function to start the editor."""
        self.panel = panel
        FreeCADGui.Control.closeDialog()
        FreeCADGui.Control.showDialog(panel)
        panel.setupUi()
        job = self.Object.Proxy.getJob(self.Object)
        if job:
            job.ViewObject.Proxy.setupEditVisibility(job)
        else:
            Path.Log.info("did not find no job")

    def clearTaskPanel(self):
        """clearTaskPanel() ... internal callback function when editing has finished."""
        self.panel = None
        job = self.Object.Proxy.getJob(self.Object)
        if job:
            job.ViewObject.Proxy.resetEditVisibility(job)

    def unsetEdit(self, arg1, arg2):
        if self.panel:
            self.panel.reject(False)

    def dumps(self):
        """dumps() ... callback before receiver is saved to a file.
        Returns a dictionary with the receiver's resources as strings."""
        Path.Log.track()
        state = {}
        state["OpName"] = self.OpName
        state["OpIcon"] = self.OpIcon
        state["OpPageModule"] = self.OpPageModule
        state["OpPageClass"] = self.OpPageClass
        return state

    def loads(self, state):
        """loads(state) ... callback on restoring a saved instance, pendant to dumps()
        state is the dictionary returned by dumps()."""
        self.OpName = state["OpName"]
        self.OpIcon = state["OpIcon"]
        self.OpPageModule = state["OpPageModule"]
        self.OpPageClass = state["OpPageClass"]

    def getIcon(self):
        """getIcon() ... the icon used in the object tree"""
        if self.Object.Active:
            return self.OpIcon
        else:
            return ":/icons/CAM_OpActive.svg"

    def getTaskPanelOpPage(self, obj):
        """getTaskPanelOpPage(obj) ... use the stored information to instantiate the receiver op's page controller."""
        mod = importlib.import_module(self.OpPageModule)
        cls = getattr(mod, self.OpPageClass)
        return cls(obj, 0)

    def getSelectionFactory(self):
        """getSelectionFactory() ... return a factory function that can be used to create the selection observer."""
        return PathSelection.select(self.OpName)

    def updateData(self, obj, prop):
        """updateData(obj, prop) ... callback whenever a property of the receiver's model is assigned.
        The callback is forwarded to the task panel - in case an editing session is ongoing."""
        # Path.Log.track(obj.Label, prop) # Creates a lot of noise
        if self.panel:
            self.panel.updateData(obj, prop)

    def onDelete(self, vobj, arg2=None):
        PathUtil.clearExpressionEngine(vobj.Object)
        return True

    def setupContextMenu(self, vobj, menu):
        Path.Log.track()
        for action in menu.actions():
            menu.removeAction(action)
        action = QtGui.QAction(translate("PathOp", "Edit"), menu)
        action.triggered.connect(self._editInContextMenuTriggered)
        menu.addAction(action)

    def _editInContextMenuTriggered(self, checked):
        self.setEdit()


class TaskPanelPage(object):
    """Base class for all task panel pages."""

    # task panel interaction framework
    def __init__(self, obj, features):
        """__init__(obj, features) ... framework initialisation.
        Do not overwrite, implement initPage(obj) instead."""
        self.obj = obj
        self.job = PathUtils.findParentJob(obj)
        self.form = self.getForm()
        self.signalDirtyChanged = None
        self.setClean()
        self.setTitle("-")
        self.setIcon(None)
        self.features = features
        self.isdirty = False
        self.parent = None
        self.panelTitle = "Operation"

        if self._installTCUpdate():
            PathJob.Notification.updateTC.connect(self.resetToolController)

    def show_error_message(self, title, message):
        msg_box = QtGui.QMessageBox()
        msg_box.setIcon(QtGui.QMessageBox.Critical)
        msg_box.setWindowTitle(title)
        msg_box.setText(message)
        msg_box.setStandardButtons(QtGui.QMessageBox.Ok)
        msg_box.exec_()

    def _installTCUpdate(self):
        return hasattr(self.form, "toolController")

    def setParent(self, parent):
        """setParent() ... used to transfer parent object link to child class.
        Do not overwrite."""
        self.parent = parent

    def onDirtyChanged(self, callback):
        """onDirtyChanged(callback) ... set callback when dirty state changes."""
        self.signalDirtyChanged = callback

    def setDirty(self):
        """setDirty() ... mark receiver as dirty, causing the model to be recalculated if OK or Apply is pressed."""
        self.isdirty = True
        if self.signalDirtyChanged:
            self.signalDirtyChanged(self)

    def setClean(self):
        """setClean() ... mark receiver as clean, indicating there is no need to recalculate the model even if the user presses OK or Apply."""
        self.isdirty = False
        if self.signalDirtyChanged:
            self.signalDirtyChanged(self)

    def pageGetFields(self):
        """pageGetFields() ... internal callback.
        Do not overwrite, implement getFields(obj) instead."""
        self.getFields(self.obj)
        self.setDirty()

    def pageSetFields(self):
        """pageSetFields() ... internal callback.
        Do not overwrite, implement setFields(obj) instead."""
        self.setFields(self.obj)

    def pageCleanup(self):
        """pageCleanup() ... internal callback.
        Do not overwrite, implement cleanupPage(obj) instead."""
        if self._installTCUpdate():
            PathJob.Notification.updateTC.disconnect(self.resetToolController)
        self.cleanupPage(self.obj)

    def pageRegisterSignalHandlers(self):
        """pageRegisterSignalHandlers() .. internal callback.
        Registers a callback for all signals returned by getSignalsForUpdate(obj).
        Do not overwrite, implement getSignalsForUpdate(obj) and/or registerSignalHandlers(obj) instead.
        """
        for signal in self.getSignalsForUpdate(self.obj):
            signal.connect(self.pageGetFields)
        self.registerSignalHandlers(self.obj)

    def pageUpdateData(self, obj, prop):
        """pageUpdateData(obj, prop) ... internal callback.
        Do not overwrite, implement updateData(obj) instead."""
        self.updateData(obj, prop)

    def setTitle(self, title):
        """setTitle(title) ... sets a title for the page."""
        self.title = title

    def getTitle(self, obj):
        """getTitle(obj) ... return title to be used for the receiver page.
        The default implementation returns what was previously set with setTitle(title).
        Can safely be overwritten by subclasses."""
        return self.title

    def setIcon(self, icon):
        """setIcon(icon) ... sets the icon for the page."""
        self.icon = icon

    def getIcon(self, obj):
        """getIcon(obj) ... return icon for page or None.
        Can safely be overwritten by subclasses."""
        return self.icon

    # subclass interface
    def initPage(self, obj):
        """initPage(obj) ... overwrite to customize UI for specific model.
        Note that this function is invoked after all page controllers have been created.
        Should be overwritten by subclasses."""
        pass

    def cleanupPage(self, obj):
        """cleanupPage(obj) ... overwrite to perform any cleanup tasks before page is destroyed.
        Can safely be overwritten by subclasses."""
        pass

    def modifyStandardButtons(self, buttonBox):
        """modifyStandardButtons(buttonBox) ... overwrite if the task panel standard buttons need to be modified.
        Can safely be overwritten by subclasses."""
        pass

    def getForm(self):
        """getForm() ... return UI form for this page.
        Must be overwritten by subclasses."""
        pass

    def getFields(self, obj):
        """getFields(obj) ... overwrite to transfer values from UI to obj's properties.
        Can safely be overwritten by subclasses."""
        pass

    def setFields(self, obj):
        """setFields(obj) ... overwrite to transfer obj's property values to UI.
        Can safely be overwritten by subclasses."""
        pass

    def getSignalsForUpdate(self, obj):
        """getSignalsForUpdate(obj) ... return signals which, when triggered, cause the receiver to update the model.
        See also registerSignalHandlers(obj)
        Can safely be overwritten by subclasses."""
        return []

    def registerSignalHandlers(self, obj):
        """registerSignalHandlers(obj) ... overwrite to register custom signal handlers.
        In case an update of a model is not the desired operation of a signal invocation
        (see getSignalsForUpdate(obj)) this function can be used to register signal handlers
        manually.
        Can safely be overwritten by subclasses."""
        pass

    def updateData(self, obj, prop):
        """updateData(obj, prop) ... overwrite if the receiver needs to react to property changes that might not have been caused by the receiver itself.
        Sometimes a model will recalculate properties based on a change of another property. In order to keep the UI up to date with such changes this
        function can be used.
        Please note that the callback is synchronous with the property assignment operation. Also note that the notification is invoked regardless of the
        actual value of the property assignment. In other words it also fires if a property gets assigned the same value it already has.
        Taking above observations into account the implementation has to take care that it doesn't overwrite modified UI values by invoking setFields(obj).
        This can happen if a subclass unconditionally transfers all values in getFields(obj) to the model and just calls setFields(obj) in this callback.
        In such a scenario the first property assignment will cause all changes in the UI of the other fields to be overwritten by setFields(obj).
        You have been warned."""
        pass

    def updateSelection(self, obj, sel):
        """updateSelection(obj, sel) ...
        overwrite to customize UI depending on current selection.
        Can safely be overwritten by subclasses."""
        pass

    def selectInComboBox(self, name, combo):
        """selectInComboBox(name, combo) ...
        helper function to select a specific value in a combo box."""
        blocker = QtCore.QSignalBlocker(combo)
        index = combo.currentIndex()  # Save initial index

        # Search using currentData and return if found
        newindex = combo.findData(name)
        if newindex >= 0:
            combo.setCurrentIndex(newindex)
            return

        # if not found, search using current text
        newindex = combo.findText(name, QtCore.Qt.MatchFixedString)
        if newindex >= 0:
            combo.setCurrentIndex(newindex)
            return

        # not found, return unchanged
        combo.setCurrentIndex(index)
        return

    def populateCombobox(self, form, enumTups, comboBoxesPropertyMap):
        """populateCombobox(form, enumTups, comboBoxesPropertyMap) ... proxy for PathGuiUtil.populateCombobox()"""
        PathGuiUtil.populateCombobox(form, enumTups, comboBoxesPropertyMap)

    def resetToolController(self, job, tc):
        if self.obj is not None:
            self.obj.ToolController = tc
            combo = self.form.toolController
            self.setupToolController(self.obj, combo)

    def setupToolController(self, obj, combo):
        """setupToolController(obj, combo) ...
        helper function to setup obj's ToolController
        in the given combo box."""
        controllers = PathUtils.getToolControllers(self.obj)
        labels = [c.Label for c in controllers]
        combo.blockSignals(True)
        combo.clear()
        combo.addItems(labels)
        combo.blockSignals(False)

        if obj.ToolController is None:
            obj.ToolController = PathUtils.findToolController(obj, obj.Proxy)
        if obj.ToolController is not None:
            self.selectInComboBox(obj.ToolController.Label, combo)

    def updateToolController(self, obj, combo):
        """updateToolController(obj, combo) ...
        helper function to update obj's ToolController property if a different
        one has been selected in the combo box."""
        tc = PathUtils.findToolController(obj, obj.Proxy, combo.currentText())
        if obj.ToolController != tc:
            obj.ToolController = tc

    def setupCoolant(self, obj, combo):
        """setupCoolant(obj, combo) ...
        helper function to setup obj's Coolant option."""
        job = PathUtils.findParentJob(obj)
        options = job.SetupSheet.CoolantModes
        combo.blockSignals(True)
        combo.clear()
        combo.addItems(options)
        combo.blockSignals(False)

        if hasattr(obj, "CoolantMode"):
            self.selectInComboBox(obj.CoolantMode, combo)

    def updateCoolant(self, obj, combo):
        """updateCoolant(obj, combo) ...
        helper function to update obj's Coolant property if a different
        one has been selected in the combo box."""
        option = combo.currentText()
        if hasattr(obj, "CoolantMode"):
            if obj.CoolantMode != option:
                obj.CoolantMode = option

    def updatePanelVisibility(self, panelTitle, obj):
        """updatePanelVisibility(panelTitle, obj) ...
        Function to call the `updateVisibility()` GUI method of the
        page whose panel title is as indicated."""
        if hasattr(self, "parent"):
            parent = getattr(self, "parent")
            if parent and hasattr(parent, "featurePages"):
                for page in parent.featurePages:
                    if hasattr(page, "panelTitle"):
                        if page.panelTitle == panelTitle and hasattr(page, "updateVisibility"):
                            page.updateVisibility()
                            break


class TaskPanelBaseGeometryPage(TaskPanelPage):
    """Page controller for the base geometry."""

    DataObject = QtCore.Qt.ItemDataRole.UserRole
    DataObjectSub = QtCore.Qt.ItemDataRole.UserRole + 1

    def __init__(self, obj, features):
        super(TaskPanelBaseGeometryPage, self).__init__(obj, features)

        self.panelTitle = "Base Geometry"
        self.OpIcon = ":/icons/CAM_BaseGeometry.svg"
        self.setIcon(self.OpIcon)

    def getForm(self):
        panel = FreeCADGui.PySideUic.loadUi(":/panels/PageBaseGeometryEdit.ui")
        self.modifyPanel(panel)
        return panel

    def modifyPanel(self, panel):
        """modifyPanel(self, panel) ...
        Helper method to modify the current form immediately after
        it is loaded."""
        # Determine if Job operations are available with Base Geometry
        availableOps = list()
        ops = self.job.Operations.Group
        for op in ops:
            if hasattr(op, "Base") and isinstance(op.Base, list):
                if len(op.Base) > 0:
                    availableOps.append(op.Label)

        # Load available operations into combobox
        if len(availableOps) > 0:
            # Populate the operations list
            panel.geometryImportList.blockSignals(True)
            panel.geometryImportList.clear()
            availableOps.sort()
            for opLbl in availableOps:
                panel.geometryImportList.addItem(opLbl)
            panel.geometryImportList.blockSignals(False)
        else:
            panel.geometryImportList.hide()
            panel.geometryImportButton.hide()

    def getTitle(self, obj):
        return translate("PathOp", "Base Geometry")

    def getFields(self, obj):
        pass

    def setFields(self, obj):
        self.form.baseList.blockSignals(True)
        self.form.baseList.clear()
        for base in self.obj.Base:
            for sub in base[1]:
                item = QtGui.QListWidgetItem("%s.%s" % (base[0].Label, sub))
                item.setData(self.DataObject, base[0])
                item.setData(self.DataObjectSub, sub)
                self.form.baseList.addItem(item)
        self.form.baseList.blockSignals(False)
        self.resizeBaseList()

    def itemActivated(self):
        FreeCADGui.Selection.clearSelection()
        for item in self.form.baseList.selectedItems():
            obj = item.data(self.DataObject)
            sub = item.data(self.DataObjectSub)
            if sub:
                FreeCADGui.Selection.addSelection(obj, sub)
            else:
                FreeCADGui.Selection.addSelection(obj)
        # FreeCADGui.updateGui()

    def supportsVertexes(self):
        return self.features & PathOp.FeatureBaseVertexes

    def supportsEdges(self):
        return self.features & PathOp.FeatureBaseEdges

    def supportsFaces(self):
        return self.features & PathOp.FeatureBaseFaces

    def supportsPanels(self):
        return self.features & PathOp.FeatureBasePanels

    def featureName(self):
        if self.supportsEdges() and self.supportsFaces():
            return "features"
        if self.supportsFaces():
            return "faces"
        if self.supportsEdges():
            return "edges"
        return "nothing"

    def selectionSupportedAsBaseGeometry(self, selection, ignoreErrors):
        if len(selection) != 1:
            if not ignoreErrors:
                msg = translate(
                    "PathOp",
                    "Please select %s from a single solid" % self.featureName(),
                )
                Path.Log.debug(msg)
            return False
        sel = selection[0]
        if sel.HasSubObjects:
            if not self.supportsVertexes() and selection[0].SubObjects[0].ShapeType == "Vertex":
                return False
            if not self.supportsEdges() and selection[0].SubObjects[0].ShapeType == "Edge":
                return False
            if not self.supportsFaces() and selection[0].SubObjects[0].ShapeType == "Face":
                return False
        else:
            if not self.supportsPanels() or "Panel" not in sel.Object.Name:
                return False
        return True

    def addBaseGeometry(self, selection):
        Path.Log.track(selection)
        if self.selectionSupportedAsBaseGeometry(selection, False):
            sel = selection[0]
            for sub in sel.SubElementNames:
                self.obj.Proxy.addBase(self.obj, sel.Object, sub)
            return True
        return False

    def addBase(self):
        Path.Log.track()
        if self.addBaseGeometry(FreeCADGui.Selection.getSelectionEx()):
            # self.obj.Proxy.execute(self.obj)
            self.setFields(self.obj)
            self.setDirty()
            self.updatePanelVisibility("Operation", self.obj)

    def deleteBase(self):
        Path.Log.track()
        selected = self.form.baseList.selectedItems()
        for item in selected:
            self.form.baseList.takeItem(self.form.baseList.row(item))
            self.setDirty()
        self.updateBase()
        self.updatePanelVisibility("Operation", self.obj)
        self.resizeBaseList()

    def updateBase(self):
        newlist = []
        for i in range(self.form.baseList.count()):
            item = self.form.baseList.item(i)
            obj = item.data(self.DataObject)
            sub = item.data(self.DataObjectSub)
            if sub:
                base = (obj, str(sub))
                newlist.append(base)
        Path.Log.debug("Setting new base: %s -> %s" % (self.obj.Base, newlist))
        self.obj.Base = newlist

    def clearBase(self):
        self.obj.Base = []
        self.setDirty()
        self.updatePanelVisibility("Operation", self.obj)
        self.resizeBaseList()

    def importBaseGeometry(self):
        opLabel = str(self.form.geometryImportList.currentText())
        ops = FreeCAD.ActiveDocument.getObjectsByLabel(opLabel)
        if len(ops) > 1:
            msg = translate("PathOp", "Multiple operations are labeled as")
            msg += " {}\n".format(opLabel)
            FreeCAD.Console.PrintWarning(msg)
        for base, subList in ops[0].Base:
            FreeCADGui.Selection.clearSelection()
            FreeCADGui.Selection.addSelection(base, subList)
            self.addBase()

    def registerSignalHandlers(self, obj):
        self.form.baseList.itemSelectionChanged.connect(self.itemActivated)
        self.form.addBase.clicked.connect(self.addBase)
        self.form.deleteBase.clicked.connect(self.deleteBase)
        self.form.clearBase.clicked.connect(self.clearBase)
        self.form.geometryImportButton.clicked.connect(self.importBaseGeometry)

    def pageUpdateData(self, obj, prop):
        if prop in ["Base"]:
            self.setFields(obj)

    def updateSelection(self, obj, sel):
        if self.selectionSupportedAsBaseGeometry(sel, True):
            self.form.addBase.setEnabled(True)
        else:
            self.form.addBase.setEnabled(False)

    def resizeBaseList(self):
        # Set base geometry list window to resize based on contents
        # Code reference:
        # https://stackoverflow.com/questions/6337589/qlistwidget-adjust-size-to-content
        # ml: disabling this logic because I can't get it to work on HPD monitor.
        #     On my systems the values returned by the list object are also incorrect on
        #     creation, leading to a list object of size 15. count() always returns 0 until
        #     the list is actually displayed. The same is true for sizeHintForRow(0), which
        #     returns -1 until the widget is rendered. The widget claims to have a size of
        #     (100, 30), once it becomes visible the size is (535, 192).
        #     Leaving the framework here in case somebody figures out how to set this up
        #     properly.
        qList = self.form.baseList
        row = (qList.count() + qList.frameWidth()) * 15
        # qList.setMinimumHeight(row)
        Path.Log.debug(
            "baseList({}, {}) {} * {}".format(
                qList.size(), row, qList.count(), qList.sizeHintForRow(0)
            )
        )


class TaskPanelBaseLocationPage(TaskPanelPage):
    """Page controller for base locations. Uses PathGetPoint."""

    DataLocation = QtCore.Qt.ItemDataRole.UserRole

    def __init__(self, obj, features):
        super(TaskPanelBaseLocationPage, self).__init__(obj, features)

        # members initialized later
        self.editRow = None
        self.panelTitle = "Base Location"

    def getForm(self):
        self.formLoc = FreeCADGui.PySideUic.loadUi(":/panels/PageBaseLocationEdit.ui")
        if QtCore.qVersion()[0] == "4":
            self.formLoc.baseList.horizontalHeader().setResizeMode(QtGui.QHeaderView.Stretch)
        else:
            self.formLoc.baseList.horizontalHeader().setSectionResizeMode(QtGui.QHeaderView.Stretch)
        self.getPoint = PathGetPoint.TaskPanel(self.formLoc.addRemoveEdit)
        return self.formLoc

    def modifyStandardButtons(self, buttonBox):
        self.getPoint.buttonBox = buttonBox

    def getTitle(self, obj):
        return translate("PathOp", "Base Location")

    def getFields(self, obj):
        pass

    def setFields(self, obj):
        self.formLoc.baseList.blockSignals(True)
        self.formLoc.baseList.clearContents()
        self.formLoc.baseList.setRowCount(0)
        for location in self.obj.Locations:
            self.formLoc.baseList.insertRow(self.formLoc.baseList.rowCount())

            item = QtGui.QTableWidgetItem("%.2f" % location.x)
            item.setData(self.DataLocation, location.x)
            self.formLoc.baseList.setItem(self.formLoc.baseList.rowCount() - 1, 0, item)

            item = QtGui.QTableWidgetItem("%.2f" % location.y)
            item.setData(self.DataLocation, location.y)
            self.formLoc.baseList.setItem(self.formLoc.baseList.rowCount() - 1, 1, item)
        self.formLoc.baseList.resizeColumnToContents(0)
        self.formLoc.baseList.blockSignals(False)
        self.itemActivated()

    def removeLocation(self):
        deletedRows = []
        selected = self.formLoc.baseList.selectedItems()
        for item in selected:
            row = self.formLoc.baseList.row(item)
            if row not in deletedRows:
                deletedRows.append(row)
                self.formLoc.baseList.removeRow(row)
        self.updateLocations()
        FreeCAD.ActiveDocument.recompute()

    def updateLocations(self):
        Path.Log.track()
        locations = []
        for i in range(self.formLoc.baseList.rowCount()):
            x = self.formLoc.baseList.item(i, 0).data(self.DataLocation)
            y = self.formLoc.baseList.item(i, 1).data(self.DataLocation)
            location = FreeCAD.Vector(x, y, 0)
            locations.append(location)
        self.obj.Locations = locations

    def addLocation(self):
        self.getPoint.getPoint(self.addLocationAt)

    def addLocationAt(self, point, obj):
        if point:
            locations = self.obj.Locations
            locations.append(point)
            self.obj.Locations = locations
            FreeCAD.ActiveDocument.recompute()

    def editLocation(self):
        selected = self.formLoc.baseList.selectedItems()
        if selected:
            row = self.formLoc.baseList.row(selected[0])
            self.editRow = row
            x = self.formLoc.baseList.item(row, 0).data(self.DataLocation)
            y = self.formLoc.baseList.item(row, 1).data(self.DataLocation)
            start = FreeCAD.Vector(x, y, 0)
            self.getPoint.getPoint(self.editLocationAt, start)

    def editLocationAt(self, point, obj):
        if point:
            self.formLoc.baseList.item(self.editRow, 0).setData(self.DataLocation, point.x)
            self.formLoc.baseList.item(self.editRow, 1).setData(self.DataLocation, point.y)
            self.updateLocations()
            FreeCAD.ActiveDocument.recompute()

    def itemActivated(self):
        if self.formLoc.baseList.selectedItems():
            self.form.removeLocation.setEnabled(True)
            self.form.editLocation.setEnabled(True)
        else:
            self.form.removeLocation.setEnabled(False)
            self.form.editLocation.setEnabled(False)

    def registerSignalHandlers(self, obj):
        self.form.baseList.itemSelectionChanged.connect(self.itemActivated)
        self.formLoc.addLocation.clicked.connect(self.addLocation)
        self.formLoc.removeLocation.clicked.connect(self.removeLocation)
        self.formLoc.editLocation.clicked.connect(self.editLocation)

    def pageUpdateData(self, obj, prop):
        if prop in ["Locations"]:
            self.setFields(obj)


class TaskPanelHeightsPage(TaskPanelPage):
    """Page controller for heights."""

    def __init__(self, obj, features):
        super(TaskPanelHeightsPage, self).__init__(obj, features)

        # members initialized later
        self.clearanceHeight = None
        self.safeHeight = None
        self.panelTitle = "Heights"
        self.OpIcon = ":/icons/CAM_Heights.svg"
        self.setIcon(self.OpIcon)

    def getForm(self):
        return FreeCADGui.PySideUic.loadUi(":/panels/PageHeightsEdit.ui")

    def initPage(self, obj):
        self.safeHeight = PathGuiUtil.QuantitySpinBox(self.form.safeHeight, obj, "SafeHeight")
        self.clearanceHeight = PathGuiUtil.QuantitySpinBox(
            self.form.clearanceHeight, obj, "ClearanceHeight"
        )

    def getTitle(self, obj):
        return translate("PathOp", "Heights")

    def getFields(self, obj):
        self.safeHeight.updateProperty()
        self.clearanceHeight.updateProperty()

    def setFields(self, obj):
        self.safeHeight.updateWidget()
        self.clearanceHeight.updateWidget()

    def getSignalsForUpdate(self, obj):
        signals = []
        signals.append(self.form.safeHeight.editingFinished)
        signals.append(self.form.clearanceHeight.editingFinished)
        return signals

    def pageUpdateData(self, obj, prop):
        if prop in ["SafeHeight", "ClearanceHeight"]:
            self.setFields(obj)


class TaskPanelDepthsPage(TaskPanelPage):
    """Page controller for depths."""

    def __init__(self, obj, features):
        super(TaskPanelDepthsPage, self).__init__(obj, features)

        # members initialized later
        self.startDepth = None
        self.finalDepth = None
        self.finishDepth = None
        self.stepDown = None
        self.panelTitle = "Depths"
        self.OpIcon = ":/icons/CAM_Depths.svg"
        self.setIcon(self.OpIcon)

    def getForm(self):
        return FreeCADGui.PySideUic.loadUi(":/panels/PageDepthsEdit.ui")

    def haveStartDepth(self):
        return PathOp.FeatureDepths & self.features

    def haveFinalDepth(self):
        return (
            PathOp.FeatureDepths & self.features and not PathOp.FeatureNoFinalDepth & self.features
        )

    def haveFinishDepth(self):
        return PathOp.FeatureDepths & self.features and PathOp.FeatureFinishDepth & self.features

    def haveStepDown(self):
        return PathOp.FeatureStepDown & self.features

    def initPage(self, obj):

        if self.haveStartDepth():
            self.startDepth = PathGuiUtil.QuantitySpinBox(self.form.startDepth, obj, "StartDepth")
        else:
            self.form.startDepth.hide()
            self.form.startDepthLabel.hide()
            self.form.startDepthSet.hide()

        if self.haveFinalDepth():
            self.finalDepth = PathGuiUtil.QuantitySpinBox(self.form.finalDepth, obj, "FinalDepth")
        else:
            if self.haveStartDepth():
                self.form.finalDepth.setEnabled(False)
                self.form.finalDepth.setToolTip(
                    translate(
                        "PathOp",
                        "FinalDepth cannot be modified for this operation.\nIf it is necessary to set the FinalDepth manually please select a different operation.",
                    )
                )
            else:
                self.form.finalDepth.hide()
                self.form.finalDepthLabel.hide()
            self.form.finalDepthSet.hide()

        if self.haveStepDown():
            self.stepDown = PathGuiUtil.QuantitySpinBox(self.form.stepDown, obj, "StepDown")
        else:
            self.form.stepDown.hide()
            self.form.stepDownLabel.hide()

        if self.haveFinishDepth():
            self.finishDepth = PathGuiUtil.QuantitySpinBox(
                self.form.finishDepth, obj, "FinishDepth"
            )
        else:
            self.form.finishDepth.hide()
            self.form.finishDepthLabel.hide()

    def getTitle(self, obj):
        return translate("PathOp", "Depths")

    def getFields(self, obj):
        if self.haveStartDepth():
            self.startDepth.updateProperty()
        if self.haveFinalDepth():
            self.finalDepth.updateProperty()
        if self.haveStepDown():
            self.stepDown.updateProperty()
        if self.haveFinishDepth():
            self.finishDepth.updateProperty()

    def setFields(self, obj):
        if self.haveStartDepth():
            self.startDepth.updateWidget()
        if self.haveFinalDepth():
            self.finalDepth.updateWidget()
        if self.haveStepDown():
            self.stepDown.updateWidget()
        if self.haveFinishDepth():
            self.finishDepth.updateWidget()
        self.updateSelection(obj, FreeCADGui.Selection.getSelectionEx())

    def getSignalsForUpdate(self, obj):
        signals = []
        if self.haveStartDepth():
            signals.append(self.form.startDepth.editingFinished)
        if self.haveFinalDepth():
            signals.append(self.form.finalDepth.editingFinished)
        if self.haveStepDown():
            signals.append(self.form.stepDown.editingFinished)
        if self.haveFinishDepth():
            signals.append(self.form.finishDepth.editingFinished)
        return signals

    def registerSignalHandlers(self, obj):
        if self.haveStartDepth():
            self.form.startDepthSet.clicked.connect(
                lambda: self.depthSet(obj, self.startDepth, "StartDepth")
            )
        if self.haveFinalDepth():
            self.form.finalDepthSet.clicked.connect(
                lambda: self.depthSet(obj, self.finalDepth, "FinalDepth")
            )

    def pageUpdateData(self, obj, prop):
        if prop in ["StartDepth", "FinalDepth", "StepDown", "FinishDepth"]:
            self.setFields(obj)

    def depthSet(self, obj, spinbox, prop):
        z = self.selectionZLevel(FreeCADGui.Selection.getSelectionEx())
        if z is not None:
            Path.Log.debug("depthSet(%s, %s, %.2f)" % (obj.Label, prop, z))
            if spinbox.expression():
                obj.setExpression(prop, None)
                self.setDirty()
            spinbox.updateWidget(FreeCAD.Units.Quantity(z, FreeCAD.Units.Length))
            if spinbox.updateProperty():
                self.setDirty()
        else:
            Path.Log.info("depthSet(-)")

    def selectionZLevel(self, sel):
        if len(sel) == 1 and len(sel[0].SubObjects) == 1:
            sub = sel[0].SubObjects[0]
            if "Vertex" == sub.ShapeType:
                return sub.Z
            if Path.Geom.isHorizontal(sub):
                if "Edge" == sub.ShapeType:
                    return sub.Vertexes[0].Z
                if "Face" == sub.ShapeType:
                    return sub.BoundBox.ZMax
        return None

    def updateSelection(self, obj, sel):
        if self.selectionZLevel(sel) is not None:
            self.form.startDepthSet.setEnabled(True)
            self.form.finalDepthSet.setEnabled(True)
        else:
            self.form.startDepthSet.setEnabled(False)
            self.form.finalDepthSet.setEnabled(False)


class TaskPanelDiametersPage(TaskPanelPage):
    """Page controller for diameters."""

    def __init__(self, obj, features):
        super(TaskPanelDiametersPage, self).__init__(obj, features)

        # members initialized later
        self.clearanceHeight = None
        self.safeHeight = None

    def getForm(self):
        return FreeCADGui.PySideUic.loadUi(":/panels/PageDiametersEdit.ui")

    def initPage(self, obj):
        self.minDiameter = PathGuiUtil.QuantitySpinBox(self.form.minDiameter, obj, "MinDiameter")
        self.maxDiameter = PathGuiUtil.QuantitySpinBox(self.form.maxDiameter, obj, "MaxDiameter")

    def getTitle(self, obj):
        return translate("PathOp", "Diameters")

    def getFields(self, obj):
        self.minDiameter.updateProperty()
        self.maxDiameter.updateProperty()

    def setFields(self, obj):
        self.minDiameter.updateWidget()
        self.maxDiameter.updateWidget()

    def getSignalsForUpdate(self, obj):
        signals = []
        signals.append(self.form.minDiameter.editingFinished)
        signals.append(self.form.maxDiameter.editingFinished)
        return signals

    def pageUpdateData(self, obj, prop):
        if prop in ["MinDiameter", "MaxDiameter"]:
            self.setFields(obj)


class TaskPanel(object):
    """
    Generic TaskPanel implementation handling the standard Path operation layout.
    This class only implements the framework and takes care of bringing all pages up and down in a controller fashion.
    It implements the standard editor behaviour for OK, Cancel and Apply and tracks if the model is still in sync with
    the UI.
    However, all display and processing of fields is handled by the page controllers which are managed in a list. All
    event callbacks and framework actions are forwarded to the page controllers in turn and each page controller is
    expected to process all events concerning the data it manages.
    """

    def __init__(self, obj, deleteOnReject, opPage, selectionFactory):
        Path.Log.track(obj.Label, deleteOnReject, opPage, selectionFactory)
        FreeCAD.ActiveDocument.openTransaction(translate("PathOp", "AreaOp Operation"))
        self.obj = obj
        self.deleteOnReject = deleteOnReject
        self.featurePages = []
        self.parent = None

        # members initialized later
        self.clearanceHeight = None
        self.safeHeight = None
        self.startDepth = None
        self.finishDepth = None
        self.finalDepth = None
        self.stepDown = None
        self.buttonBox = None
        self.minDiameter = None
        self.maxDiameter = None

        features = obj.Proxy.opFeatures(obj)
        opPage.features = features

        if PathOp.FeatureBaseGeometry & features:
            if hasattr(opPage, "taskPanelBaseGeometryPage"):
                self.featurePages.append(opPage.taskPanelBaseGeometryPage(obj, features))
            else:
                self.featurePages.append(TaskPanelBaseGeometryPage(obj, features))

        if PathOp.FeatureLocations & features:
            if hasattr(opPage, "taskPanelBaseLocationPage"):
                self.featurePages.append(opPage.taskPanelBaseLocationPage(obj, features))
            else:
                self.featurePages.append(TaskPanelBaseLocationPage(obj, features))

        if PathOp.FeatureDepths & features or PathOp.FeatureStepDown & features:
            if hasattr(opPage, "taskPanelDepthsPage"):
                self.featurePages.append(opPage.taskPanelDepthsPage(obj, features))
            else:
                self.featurePages.append(TaskPanelDepthsPage(obj, features))

        if PathOp.FeatureHeights & features:
            if hasattr(opPage, "taskPanelHeightsPage"):
                self.featurePages.append(opPage.taskPanelHeightsPage(obj, features))
            else:
                self.featurePages.append(TaskPanelHeightsPage(obj, features))

        if PathOp.FeatureDiameters & features:
            if hasattr(opPage, "taskPanelDiametersPage"):
                self.featurePages.append(opPage.taskPanelDiametersPage(obj, features))
            else:
                self.featurePages.append(TaskPanelDiametersPage(obj, features))

        self.featurePages.append(opPage)

        for page in self.featurePages:
            page.parent = self  # save pointer to this current class as "parent"
            page.initPage(obj)
            page.onDirtyChanged(self.pageDirtyChanged)

        taskPanelLayout = Path.Preferences.defaultTaskPanelLayout()

        if taskPanelLayout < 2:
            opTitle = opPage.getTitle(obj)
            opPage.setTitle(translate("PathOp", "Operation"))
            toolbox = QtGui.QToolBox()
            if taskPanelLayout == 0:
                for page in self.featurePages:
                    toolbox.addItem(page.form, page.getTitle(obj))
                    itemIdx = toolbox.count() - 1
                    if page.icon:
                        toolbox.setItemIcon(itemIdx, QtGui.QIcon(page.icon))
                toolbox.setCurrentIndex(len(self.featurePages) - 1)
            else:
                for page in reversed(self.featurePages):
                    toolbox.addItem(page.form, page.getTitle(obj))
                    itemIdx = toolbox.count() - 1
                    if page.icon:
                        toolbox.setItemIcon(itemIdx, QtGui.QIcon(page.icon))
            toolbox.setWindowTitle(opTitle)
            if opPage.getIcon(obj):
                toolbox.setWindowIcon(QtGui.QIcon(opPage.getIcon(obj)))

            self.form = toolbox
        elif taskPanelLayout == 2:
            forms = []
            for page in self.featurePages:
                page.form.setWindowTitle(page.getTitle(obj))
                forms.append(page.form)
            self.form = forms
        elif taskPanelLayout == 3:
            forms = []
            for page in reversed(self.featurePages):
                page.form.setWindowTitle(page.getTitle(obj))
                forms.append(page.form)
            self.form = forms

        self.selectionFactory = selectionFactory
        self.obj = obj
        self.isdirty = deleteOnReject
        self.visibility = obj.ViewObject.Visibility
        obj.ViewObject.Visibility = True

    def isDirty(self):
        """isDirty() ... returns true if the model is not in sync with the UI anymore."""
        for page in self.featurePages:
            if page.isdirty:
                return True
        return self.isdirty

    def setClean(self):
        """setClean() ... set the receiver and all its pages clean."""
        self.isdirty = False
        for page in self.featurePages:
            page.setClean()

    def accept(self, resetEdit=True):
        """accept() ... callback invoked when user presses the task panel OK button."""
        self.preCleanup()
        if self.isDirty():
            self.panelGetFields()
        FreeCAD.ActiveDocument.commitTransaction()
        self.cleanup(resetEdit)

    def reject(self, resetEdit=True):
        """reject() ... callback invoked when user presses the task panel Cancel button."""
        self.preCleanup()
        FreeCAD.ActiveDocument.abortTransaction()
        if self.deleteOnReject:
            FreeCAD.ActiveDocument.openTransaction(translate("PathOp", "Uncreate AreaOp Operation"))
            try:
                PathUtil.clearExpressionEngine(self.obj)
                FreeCAD.ActiveDocument.removeObject(self.obj.Name)
            except Exception as ee:
                Path.Log.debug("{}\n".format(ee))
            FreeCAD.ActiveDocument.commitTransaction()
        self.cleanup(resetEdit)
        return True

    def preCleanup(self):
        for page in self.featurePages:
            page.onDirtyChanged(None)
        PathSelection.clear()
        FreeCADGui.Selection.removeObserver(self)
        self.obj.ViewObject.Proxy.clearTaskPanel()
        self.obj.ViewObject.Visibility = self.visibility

    def cleanup(self, resetEdit):
        """cleanup() ... implements common cleanup tasks."""
        self.panelCleanup()
        FreeCADGui.Control.closeDialog()
        if resetEdit:
            FreeCADGui.ActiveDocument.resetEdit()
        FreeCAD.ActiveDocument.recompute()

    def pageDirtyChanged(self, page):
        """pageDirtyChanged(page) ... internal callback"""
        self.buttonBox.button(QtGui.QDialogButtonBox.Apply).setEnabled(self.isDirty())

    def clicked(self, button):
        """clicked(button) ... callback invoked when the user presses any of the task panel buttons."""
        if button == QtGui.QDialogButtonBox.Apply:
            self.panelGetFields()
            self.setClean()
            FreeCAD.ActiveDocument.recompute()

    def modifyStandardButtons(self, buttonBox):
        """modifyStandarButtons(buttonBox) ... callback in case the task panel buttons need to be modified."""
        self.buttonBox = buttonBox
        for page in self.featurePages:
            page.modifyStandardButtons(buttonBox)
        self.pageDirtyChanged(None)

    def panelGetFields(self):
        """panelGetFields() ... invoked to trigger a complete transfer of UI data to the model."""
        Path.Log.track()
        for page in self.featurePages:
            page.pageGetFields()

    def panelSetFields(self):
        """panelSetFields() ... invoked to trigger a complete transfer of the model's properties to the UI."""
        Path.Log.track()
        self.obj.Proxy.sanitizeBase(self.obj)
        for page in self.featurePages:
            page.pageSetFields()

    def panelCleanup(self):
        """panelCleanup() ... invoked before the receiver is destroyed."""
        Path.Log.track()
        for page in self.featurePages:
            page.pageCleanup()

    def open(self):
        """open() ... callback invoked when the task panel is opened."""
        self.selectionFactory()
        FreeCADGui.Selection.addObserver(self)

    def getStandardButtons(self):
        """getStandardButtons() ... returns the Buttons for the task panel."""
        return (
            QtGui.QDialogButtonBox.Ok | QtGui.QDialogButtonBox.Apply | QtGui.QDialogButtonBox.Cancel
        )

    def setupUi(self):
        """setupUi() ... internal function to initialise all pages."""
        Path.Log.track(self.deleteOnReject)

        if self.deleteOnReject and PathOp.FeatureBaseGeometry & self.obj.Proxy.opFeatures(self.obj):
            sel = FreeCADGui.Selection.getSelectionEx()
            for page in self.featurePages:
                if getattr(page, "InitBase", True) and hasattr(page, "addBase"):
                    page.clearBase()
                    page.addBaseGeometry(sel)

        # Update properties based upon expressions in case expression value has changed
        for prp, expr in self.obj.ExpressionEngine:
            val = FreeCAD.Units.Quantity(self.obj.evalExpression(expr))
            value = val.Value if hasattr(val, "Value") else val
            prop = getattr(self.obj, prp)
            if hasattr(prop, "Value"):
                prop.Value = value
            else:
                prop = value

        self.panelSetFields()

        for page in self.featurePages:
            page.pageRegisterSignalHandlers()

    def updateData(self, obj, prop):
        """updateDate(obj, prop) ... callback invoked whenever a model's property is assigned a value."""
        # Path.Log.track(obj.Label, prop) # creates a lot of noise
        for page in self.featurePages:
            page.pageUpdateData(obj, prop)

    def needsFullSpace(self):
        return True

    def updateSelection(self):
        sel = FreeCADGui.Selection.getSelectionEx()
        for page in self.featurePages:
            page.updateSelection(self.obj, sel)

    # SelectionObserver interface
    def addSelection(self, doc, obj, sub, pnt):
        self.updateSelection()

    def removeSelection(self, doc, obj, sub):
        self.updateSelection()

    def setSelection(self, doc):
        self.updateSelection()

    def clearSelection(self, doc):
        self.updateSelection()


class CommandSetStartPoint:
    """Command to set the start point for an operation."""

    def GetResources(self):
        return {
            "Pixmap": "CAM_StartPoint",
            "MenuText": QT_TRANSLATE_NOOP("PathOp", "Pick Start Point"),
            "ToolTip": QT_TRANSLATE_NOOP("PathOp", "Pick Start Point"),
        }

    def IsActive(self):
        if FreeCAD.ActiveDocument is None:
            return False
        sel = FreeCADGui.Selection.getSelection()
        if not sel:
            return False
        obj = sel[0]
        return obj and hasattr(obj, "StartPoint")

    def setpoint(self, point, o):
        FreeCADGui.Snapper.grid.off()
        obj = self.obj
        obj.StartPoint.x = point.x
        obj.StartPoint.y = point.y
        obj.StartPoint.z = obj.ClearanceHeight.Value
        obj.UseStartPoint = True
        obj.recompute()
        textPoint = f"{obj.StartPoint.x:.2f}, {obj.StartPoint.y:.2f}, {obj.StartPoint.z:.2f}"
        print(f"Set start point for operation {obj.Label} >>> {textPoint}")

    def Activated(self):
        self.obj = FreeCADGui.Selection.getSelection()[0]
        if not hasattr(FreeCADGui, "Snapper"):
            import DraftTools
        FreeCADGui.Snapper.getPoint(callback=self.setpoint)


def Create(res):
    """Create(res) ... generic implementation of a create function.
    res is an instance of CommandResources. It is not expected that the user invokes
    this function directly, but calls the Activated() function of the Command object
    that is created in each operations Gui implementation."""
    FreeCAD.ActiveDocument.openTransaction("Create %s" % res.name)
    try:
        obj = res.objFactory(res.name, obj=None, parentJob=res.job)
        if obj.Proxy:
            obj.ViewObject.Proxy = ViewProvider(obj.ViewObject, res)
            obj.ViewObject.Visibility = True
            FreeCAD.ActiveDocument.commitTransaction()

            obj.ViewObject.Document.setEdit(obj.ViewObject, 0)
            return obj
    except PathUtils.PathNoTCExistsException:
        msg = translate("PathOp", "No suitable tool controller found.\nAborting op creation")
        diag = QtGui.QMessageBox(QtGui.QMessageBox.Warning, "Error", msg)
        diag.setWindowModality(QtCore.Qt.ApplicationModal)
        diag.exec_()
    except PathOp.PathNoTCException:
        Path.Log.warning(translate("PathOp", "No tool controller, aborting op creation"))

    FreeCAD.ActiveDocument.abortTransaction()
    FreeCAD.ActiveDocument.recompute()
    return None


class CommandPathOp:
    """Generic, data driven implementation of a Path operation creation command.
    Instances of this class are stored in all Path operation Gui modules and can
    be used to create said operations with view providers and all."""

    def __init__(self, resources):
        self.res = resources

    def GetResources(self):
        ress = {
            "Pixmap": self.res.pixmap,
            "MenuText": self.res.menuText,
            "ToolTip": self.res.toolTip,
        }
        if self.res.accelKey:
            ress["Accel"] = self.res.accelKey
        return ress

    def IsActive(self):
        if FreeCAD.ActiveDocument is not None:
            for o in FreeCAD.ActiveDocument.Objects:
                if o.Name[:3] == "Job":
                    return True
        return False

    def Activated(self):
        return Create(self.res)


class CommandResources:
    """POD class to hold command specific resources."""

    def __init__(self, name, objFactory, opPageClass, pixmap, menuText, accelKey, toolTip):
        self.name = name
        self.objFactory = objFactory
        self.opPageClass = opPageClass
        self.pixmap = pixmap
        self.menuText = menuText
        self.accelKey = accelKey
        self.toolTip = toolTip
        self.job = None


def SetupOperation(name, objFactory, opPageClass, pixmap, menuText, toolTip, setupProperties=None):
    """SetupOperation(name, objFactory, opPageClass, pixmap, menuText, toolTip, setupProperties=None)
    Creates an instance of CommandPathOp with the given parameters and registers the command with FreeCAD.
    When activated it creates a model with proxy (by invoking objFactory), assigns a view provider to it
    (see ViewProvider in this module) and starts the editor specifically for this operation (driven by opPageClass).
    This is an internal function that is automatically called by the initialisation code for each operation.
    It is not expected to be called manually.
    """

    res = CommandResources(name, objFactory, opPageClass, pixmap, menuText, None, toolTip)

    command = CommandPathOp(res)
    FreeCADGui.addCommand("CAM_%s" % name.replace(" ", "_"), command)

    if setupProperties is not None:
        PathSetupSheet.RegisterOperation(name, objFactory, setupProperties)

    return command


FreeCADGui.addCommand("CAM_SetStartPoint", CommandSetStartPoint())

FreeCAD.Console.PrintLog("Loading PathOpGui... done\n")
