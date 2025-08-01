/***************************************************************************
 *   Copyright (c) 2005 Werner Mayer <wmayer[at]users.sourceforge.net>     *
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

#include "PreCompiled.h"

#ifndef _PreComp_
# include <QApplication>
# include <QDir>
# include <QPrinter>
# include <QFileInfo>
# include <Inventor/SoInput.h>
# include <Inventor/actions/SoGetPrimitiveCountAction.h>
# include <Inventor/nodes/SoSeparator.h>
# include <xercesc/util/TranscodingException.hpp>
# include <xercesc/util/XMLString.hpp>
#endif

#include <boost/regex.hpp>

#include <App/DocumentObjectPy.h>
#include <App/DocumentPy.h>
#include <App/PropertyFile.h>
#include <Base/Interpreter.h>
#include <Base/Console.h>
#include <Base/PyWrapParseTupleAndKeywords.h>
#include <CXX/Objects.hxx>

#include <Gui/PreferencePages/DlgSettingsPDF.h>

#include "Application.h"
#include "ApplicationPy.h"
#include "BitmapFactory.h"
#include "Command.h"
#include "Dialogs/DlgPreferencesImp.h"
#include "Document.h"
#include "DocumentObserverPython.h"
#include "DownloadManager.h"
#include "EditorView.h"
#include "FileHandler.h"
#include "Macro.h"
#include "MainWindow.h"
#include "MainWindowPy.h"
#include "PythonEditor.h"
#include "PythonWrapper.h"
#include "SoFCDB.h"
#include "SplitView3DInventor.h"
#include "View3DInventor.h"
#include "ViewProvider.h"
#include "WaitCursor.h"
#include "WidgetFactory.h"
#include "Workbench.h"
#include "WorkbenchManager.h"
#include "WorkbenchManipulatorPython.h"
#include "Inventor/MarkerBitmaps.h"
#include "Language/Translator.h"


using namespace Gui;

// Application methods structure
PyMethodDef ApplicationPy::Methods[] = {
  {"activateWorkbench",(PyCFunction) ApplicationPy::sActivateWorkbenchHandler, METH_VARARGS,
   "activateWorkbench(name) -> bool\n"
    "\n"
   "Activate workbench by its name. Return False if the workbench is\n"
   "already active.\n"
    "\n"
    "name : str\n    Name of the workbench to activate."},
  {"addWorkbench",     (PyCFunction) ApplicationPy::sAddWorkbenchHandler, METH_VARARGS,
   "addWorkbench(workbench) -> None\n"
    "\n"
   "Add a workbench.\n"
    "\n"
    "workbench : Workbench, Workbench type\n"
    "    Instance of a Workbench subclass or subclass of the\n"
    "    Workbench class."},
  {"removeWorkbench",  (PyCFunction) ApplicationPy::sRemoveWorkbenchHandler, METH_VARARGS,
   "removeWorkbench(name) -> None\n"
   "\n"
   "Remove a workbench.\n"
   "\n"
   "name : str\n    Name of the workbench to remove."},
  {"getWorkbench",     (PyCFunction) ApplicationPy::sGetWorkbenchHandler, METH_VARARGS,
   "getWorkbench(name) -> Workbench\n"
   "\n"
   "Get the workbench by its name.\n"
   "\n"
   "name : str\n    Name of the workbench to return."},
  {"listWorkbenches",   (PyCFunction) ApplicationPy::sListWorkbenchHandlers, METH_VARARGS,
   "listWorkbenches() -> dict\n"
   "\n"
   "Get a dictionary with all workbenches."},
  {"activeWorkbench", (PyCFunction) ApplicationPy::sActiveWorkbenchHandler, METH_VARARGS,
   "activeWorkbench() -> Workbench\n"
   "\n"
   "Return the active workbench object."},
  {"addResourcePath",             (PyCFunction) ApplicationPy::sAddResPath, METH_VARARGS,
   "addResourcePath(path) -> None\n"
   "\n"
   "Add a new path to the system where to find resource files\n"
   "like icons or localization files.\n"
   "\n"
   "path : str, bytes, bytearray\n    Path to resource files."},
  {"addLanguagePath",             (PyCFunction) ApplicationPy::sAddLangPath, METH_VARARGS,
   "addLanguagePath(path) -> None\n"
   "\n"
   "Add a new path to the system where to find language files.\n"
   "\n"
   "path : str, bytes, bytearray\n    Path to language files."},
  {"addIconPath",             (PyCFunction) ApplicationPy::sAddIconPath, METH_VARARGS,
   "addIconPath(path) -> None\n"
   "\n"
   "Add a new path to the system where to find icon files.\n"
   "\n"
   "path : str, bytes, bytearray\n    Path to icon files."},
  {"addIcon",                 (PyCFunction) ApplicationPy::sAddIcon, METH_VARARGS,
   "addIcon(name, content, format='XPM') -> None\n"
   "\n"
   "Add an icon to the system.\n"
   "\n"
   "name : str\n    Name of the icon.\n"
   "content : str, bytes-like\n    Content of the icon.\n"
   "format : str\n    Format of the icon."},
  {"getIcon",                 (PyCFunction) ApplicationPy::sGetIcon, METH_VARARGS,
   "getIcon(name) -> QIcon or None\n"
   "\n"
   "Get an icon in the system. If the pixmap is null, return None.\n"
   "\n"
   "name : str\n    Name of the icon."},
  {"isIconCached",           (PyCFunction) ApplicationPy::sIsIconCached, METH_VARARGS,
   "isIconCached(name) -> Bool\n"
   "\n"
   "Check if an icon with the given name is cached.\n"
   "\n"
   "name : str\n    Name of the icon."},
  {"getMainWindow",           (PyCFunction) ApplicationPy::sGetMainWindow, METH_VARARGS,
   "getMainWindow() -> QMainWindow\n"
   "\n"
   "Return the main window instance."},
  {"updateGui",               (PyCFunction) ApplicationPy::sUpdateGui, METH_VARARGS,
   "updateGui() -> None\n"
   "\n"
   "Update the main window and all its windows."},
  {"updateLocale",            (PyCFunction) ApplicationPy::sUpdateLocale, METH_VARARGS,
   "updateLocale() -> None\n"
   "\n"
   "Update the localization."},
  {"getLocale",            (PyCFunction) ApplicationPy::sGetLocale, METH_VARARGS,
   "getLocale() -> str\n"
   "\n"
   "Returns the locale currently used by FreeCAD."},
  {"setLocale",            (PyCFunction) ApplicationPy::sSetLocale, METH_VARARGS,
   "setLocale(name) -> None\n"
   "\n"
   "Sets the locale used by FreeCAD. Can be set by top-level\n"
   "domain (e.g. \"de\") or the language name (e.g. \"German\").\n"
   "\n"
   "name : str\n    Locale name."},
  {"supportedLocales", (PyCFunction) ApplicationPy::sSupportedLocales, METH_VARARGS,
   "supportedLocales() -> dict\n"
   "\n"
   "Returns a dict of all supported locales. The keys are the language\n"
   "names and the values the top-level domains."},
  {"createDialog",            (PyCFunction) ApplicationPy::sCreateDialog, METH_VARARGS,
   "createDialog(path) -> PyResource\n"
   "\n"
   "Open a UI file.\n"
   "\n"
   "path : str\n    UI file path."},
  {"addPreferencePage",       (PyCFunction) ApplicationPy::sAddPreferencePage, METH_VARARGS,
   "addPreferencePage(path, group) -> None\n"
   "addPreferencePage(dialog, group) -> None\n"
   "\n"
   "Add a UI form to the preferences dialog in the specified group.\n"
   "\n"
   "path : str\n    UI file path.\n"
   "group : str\n    Group name.\n"
   "dialog : type\n    Preference page."},
  {"addCommand",              (PyCFunction) ApplicationPy::sAddCommand, METH_VARARGS,
   "addCommand(name, cmd, activation) -> None\n"
   "\n"
   "Add a command object.\n"
   "\n"
   "name : str\n    Name of the command.\n"
   "cmd : object\n    Command instance.\n"
   "activation : str\n    Activation sequence. Optional."},
  {"runCommand",              (PyCFunction) ApplicationPy::sRunCommand, METH_VARARGS,
   "runCommand(name, index=0) -> None\n"
   "\n"
   "Run command by its name.\n"
   "\n"
   "name : str\n    Name of the command.\n"
   "index : int\n    Index of the child command."},
  {"SendMsgToActiveView",     (PyCFunction) ApplicationPy::sSendActiveView, METH_VARARGS,
   "SendMsgToActiveView(name, suppress=False) -> str or None\n"
   "\n"
   "Send message to the active view. Deprecated, use class View.\n"
   "\n"
   "name : str\n    Name of the view command.\n"
   "suppress : bool\n    If the sent message fail, suppress warning message."},
  {"sendMsgToFocusView",     (PyCFunction) ApplicationPy::sSendFocusView, METH_VARARGS,
   "sendMsgToFocusView(name, suppress=False) -> str or None\n"
   "\n"
   "Send message to the focused view.\n"
   "\n"
   "name : str\n    Name of the view command.\n"
   "suppress : bool\n    If send message fail, suppress warning message."},
  {"hide",                    (PyCFunction) ApplicationPy::sHide, METH_VARARGS,
   "hide(name) -> None\n"
   "\n"
   "Hide the given feature. Deprecated.\n"
   "\n"
   "name : str\n    Feature name."},
  {"show",                    (PyCFunction) ApplicationPy::sShow, METH_VARARGS,
   "show(name) -> None\n"
   "\n"
   "Show the given feature. Deprecated.\n"
   "\n"
   "name : str\n    Feature name."},
  {"hideObject",              (PyCFunction) ApplicationPy::sHideObject, METH_VARARGS,
   "hideObject(obj) -> None\n"
   "\n"
   "Hide the view provider of the given object.\n"
   "\n"
   "obj : App.DocumentObject"},
  {"showObject",              (PyCFunction) ApplicationPy::sShowObject, METH_VARARGS,
   "showObject(obj) -> None\n"
   "\n"
   "Show the view provider of the given object.\n"
   "\n"
   "obj : App.DocumentObject"},
  {"open",                    (PyCFunction) ApplicationPy::sOpen, METH_VARARGS,
   "open(fileName) -> None\n"
   "\n"
   "Open a macro, Inventor or VRML file.\n"
   "\n"
   "fileName : str, bytes, bytearray\n    File name."},
  {"insert",                  (PyCFunction) ApplicationPy::sInsert, METH_VARARGS,
   "insert(fileName, docName) -> None\n"
   "\n"
   "Insert a macro, Inventor or VRML file. If no document name\n"
   "is given the active document is used.\n"
   "\n"
   "fileName : str, bytes, bytearray\n    File name.\n"
   "docName : str\n    Document name."},
  {"export",                  (PyCFunction) ApplicationPy::sExport, METH_VARARGS,
   "export(objs, fileName) -> None\n"
   "\n"
   "Save scene to Inventor or VRML file.\n"
   "\n"
   "objs : sequence of App.DocumentObject\n    Sequence of objects to save.\n"
   "fileName : str, bytes, bytearray\n    File name."},
  {"activeDocument",          (PyCFunction) ApplicationPy::sActiveDocument, METH_VARARGS,
   "activeDocument() -> Gui.Document or None\n"
   "\n"
   "Return the active document. If no one exists, return None."},
  {"setActiveDocument",       (PyCFunction) ApplicationPy::sSetActiveDocument, METH_VARARGS,
   "setActiveDocument(doc) -> None\n"
   "\n"
   "Activate the specified document.\n"
   "\n"
   "doc : str, App.Document\n    Document to activate."},
  {"activeView", (PyCFunction)ApplicationPy::sActiveView, METH_VARARGS,
   "activeView(typeName) -> object or None\n"
   "\n"
   "Return the active view of the active document. If no one\n"
   "exists, return None.\n"
   "\n"
   "typeName : str\n    Type name."},
  {"activateView", (PyCFunction)ApplicationPy::sActivateView, METH_VARARGS,
   "activateView(typeName, create=False) -> None\n"
   "\n"
   "Activate a view of the given type in the active document.\n"
   "If a view of this type doesn't exist and create is True, a\n"
   "new view of this type is created.\n"
   "\n"
   "type : str\n    Type name.\n"
   "create : bool"},
  {"editDocument", (PyCFunction)ApplicationPy::sEditDocument, METH_VARARGS,
   "editDocument() -> Gui.Document or None\n"
   "\n"
   "Return the current editing document. If no one exists,\n"
   "return None."},
  {"getDocument",             (PyCFunction) ApplicationPy::sGetDocument, METH_VARARGS,
   "getDocument(doc) -> Gui.Document\n"
   "\n"
   "Get a document.\n"
   "\n"
   "doc : str, App.Document\n    `App.Document` name or `App.Document` object."},
  {"doCommand",               (PyCFunction) ApplicationPy::sDoCommand, METH_VARARGS,
   "doCommand(cmd) -> None\n"
   "\n"
   "Prints the given string in the python console and runs it.\n"
   "\n"
   "cmd : str"},
  {"doCommandGui",               (PyCFunction) ApplicationPy::sDoCommandGui, METH_VARARGS,
   "doCommandGui(cmd) -> None\n"
   "\n"
   "Prints the given string in the python console and runs it\n"
   "but doesn't record it in macros.\n"
   "\n"
   "cmd : str"},
  {"doCommandEval",               (PyCFunction) ApplicationPy::sDoCommandEval, METH_VARARGS,
          "doCommandEval(cmd) -> PyObject\n"
          "\n"
          "Runs the given string without showing in the python console or recording in\n"
          "macros, and returns the result.\n"
          "\n"
          "cmd : str"},
  {"doCommandSkip",               (PyCFunction) ApplicationPy::sDoCommandSkip, METH_VARARGS,
          "doCommandSkip(cmd) -> None\n"
          "\n"
          "Record the given string in the Macro but comment it out in the console\n"
          "\n"
          "cmd : str"},
  {"addModule",               (PyCFunction) ApplicationPy::sAddModule, METH_VARARGS,
   "addModule(mod) -> None\n"
   "\n"
   "Prints the given module import only once in the macro recording.\n"
   "\n"
   "mod : str"},
  {"showDownloads",               (PyCFunction) ApplicationPy::sShowDownloads, METH_VARARGS,
   "showDownloads() -> None\n\n"
   "Show the downloads manager window."},
  {"showPreferences",               (PyCFunction) ApplicationPy::sShowPreferences, METH_VARARGS,
   "showPreferences(grp, index=0) -> None\n"
   "\n"
   "Show the preferences window.\n"
   "\n"
   "grp: str\n    Group to show.\n"
   "index : int\n    Page index."},
  {"showPreferencesByName",       (PyCFunction) ApplicationPy::sShowPreferencesByName, METH_VARARGS,
   "showPreferencesByName(grp, pagename) -> None\n"
   "\n"
   "Show the preferences window.\n"
   "\n"
   "grp: str\n    Group to show.\n"
   "pagename : str\n    Page to show."},
  {"createViewer",               (PyCFunction) ApplicationPy::sCreateViewer, METH_VARARGS,
   "createViewer(views=1, name) -> View3DInventorPy or AbstractSplitViewPy\n"
   "\n"
   "Show and returns a viewer.\n"
   "\n"
   "views : int\n    If > 1 a `AbstractSplitViewPy` object is returned.\n"
   "name : str\n    Viewer title."},
  {"getMarkerIndex", (PyCFunction) ApplicationPy::sGetMarkerIndex, METH_VARARGS,
   "getMarkerIndex(marker, size=9) -> int\n"
   "\n"
   "Get marker index according to marker name and size.\n"
   "\n"
   "marker : str\n    Marker style name.\n"
   "size : int\n    Marker size."},
  {"addDocumentObserver",  (PyCFunction) ApplicationPy::sAddDocObserver, METH_VARARGS,
   "addDocumentObserver(obj) -> None\n"
   "\n"
   "Add an observer to get notifications about changes on documents.\n"
   "\n"
   "obj : object"},
  {"removeDocumentObserver",  (PyCFunction) ApplicationPy::sRemoveDocObserver, METH_VARARGS,
   "removeDocumentObserver(obj) -> None\n"
   "\n"
   "Remove an added document observer.\n"
   "\n"
   "obj : object"},
  {"addWorkbenchManipulator",  (PyCFunction) ApplicationPy::sAddWbManipulator, METH_VARARGS,
   "addWorkbenchManipulator(obj) -> None\n"
   "\n"
   "Add a workbench manipulator to modify a workbench when it is activated.\n"
   "\n"
   "obj : object"},
  {"removeWorkbenchManipulator",  (PyCFunction) ApplicationPy::sRemoveWbManipulator, METH_VARARGS,
   "removeWorkbenchManipulator(obj) -> None\n"
   "\n"
   "Remove an added workbench manipulator.\n"
   "\n"
   "obj : object"},
  {"listUserEditModes", (PyCFunction) ApplicationPy::sListUserEditModes, METH_VARARGS,
   "listUserEditModes() -> list\n"
   "\n"
   "List available user edit modes."},
  {"getUserEditMode", (PyCFunction) ApplicationPy::sGetUserEditMode, METH_VARARGS,
   "getUserEditMode() -> str\n"
   "\n"
   "Get current user edit mode."},
  {"setUserEditMode", (PyCFunction) ApplicationPy::sSetUserEditMode, METH_VARARGS,
   "setUserEditMode(mode) -> bool\n"
   "\n"
   "Set user edit mode. Returns True if exists, False otherwise.\n"
   "\n"
   "mode : str"},
  {"reload",                    (PyCFunction) ApplicationPy::sReload, METH_VARARGS,
   "reload(name) -> App.Document or None\n"
   "\n"
   "Reload a partial opened document. If the document is not open,\n"
   "return None.\n"
   "\n"
   "name : str\n    `App.Document` name."},
  {"loadFile",       (PyCFunction) ApplicationPy::sLoadFile, METH_VARARGS,
   "loadFile(fileName, module) -> None\n"
   "\n"
   "Loads an arbitrary file by delegating to the given Python module.\n"
   "If no module is given it will be determined by the file extension.\n"
   "If more than one module can load a file the first one will be taken.\n"
   "If no module exists to load the file an exception will be raised.\n"
   "\n"
   "fileName : str\n"
   "module : str"},
  {"coinRemoveAllChildren",     (PyCFunction) ApplicationPy::sCoinRemoveAllChildren, METH_VARARGS,
   "coinRemoveAllChildren(node) -> None\n"
   "\n"
   "Remove all children from a group node.\n"
   "\n"
   "node : object"},
  {"suspendWaitCursor", (PyCFunction) ApplicationPy::sSuspendWaitCursor, METH_VARARGS,
   "suspendWaitCursor() -> None\n\n"
   "Temporarily suspends the application's wait cursor and event filter."},
  {"resumeWaitCursor",  (PyCFunction) ApplicationPy::sResumeWaitCursor, METH_VARARGS,
   "resumeWaitCursor() -> None\n\n"
   "Resumes the application's wait cursor and event filter."},
  {nullptr, nullptr, 0, nullptr}    /* Sentinel */
};

PyObject* Gui::ApplicationPy::sEditDocument(PyObject * /*self*/, PyObject *args)
{
    if (!PyArg_ParseTuple(args, "")) {
        return nullptr;
    }

    Document *pcDoc = Application::Instance->editDocument();
    if (pcDoc) {
        return pcDoc->getPyObject();
    }

    Py_Return;
}

PyObject* Gui::ApplicationPy::sActiveDocument(PyObject * /*self*/, PyObject *args)
{
    if (!PyArg_ParseTuple(args, "")) {
        return nullptr;
    }

    Document *pcDoc = Application::Instance->activeDocument();
    if (pcDoc) {
        return pcDoc->getPyObject();
    }

    Py_Return;
}

PyObject* Gui::ApplicationPy::sActiveView(PyObject * /*self*/, PyObject *args)
{
    const char *typeName = nullptr;
    if (!PyArg_ParseTuple(args, "|s", &typeName)) {
        return nullptr;
    }

    PY_TRY {
        Base::Type type;
        if (typeName) {
            type = Base::Type::fromName(typeName);
            if (type.isBad()) {
                PyErr_Format(PyExc_TypeError, "Invalid type '%s'", typeName);
                return nullptr;
            }
        }

        Gui::MDIView* mdiView = Application::Instance->activeView();
        if (mdiView && (type.isBad() || mdiView->isDerivedFrom(type))) {
            auto res = Py::asObject(mdiView->getPyObject());
            if(!res.isNone() || !type.isBad()) {
                return Py::new_reference_to(res);
            }
        }

        if (type.isBad()) {
            type = Gui::View3DInventor::getClassTypeId();
        }
        Application::Instance->activateView(type, true);
        mdiView = Application::Instance->activeView();
        if (mdiView) {
            return mdiView->getPyObject();
        }

        Py_Return;

    }
    PY_CATCH
}

PyObject* Gui::ApplicationPy::sActivateView(PyObject * /*self*/, PyObject *args)
{
    char* typeStr = nullptr;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
    PyObject *create = Py_False;
    if (!PyArg_ParseTuple(args, "sO!", &typeStr, &PyBool_Type, &create)) {
        return nullptr;
    }

    Base::Type type = Base::Type::fromName(typeStr);
    Application::Instance->activateView(type, Base::asBoolean(create));

    Py_Return;
}

PyObject* Gui::ApplicationPy::sSetActiveDocument(PyObject * /*self*/, PyObject *args)
{
    Document *pcDoc = nullptr;

    do {
        char *pstr = nullptr;
        if (PyArg_ParseTuple(args, "s", &pstr)) {
            pcDoc = Application::Instance->getDocument(pstr);
            if (!pcDoc) {
                PyErr_Format(PyExc_NameError, "Unknown document '%s'", pstr);
                return nullptr;
            }
            break;
        }

        PyErr_Clear();
        PyObject* doc = nullptr;
        if (PyArg_ParseTuple(args, "O!", &(App::DocumentPy::Type), &doc)) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            pcDoc = Application::Instance->getDocument(static_cast<App::DocumentPy*>(doc)->getDocumentPtr());
            if (!pcDoc) {
                PyErr_Format(PyExc_KeyError, "Unknown document instance");
                return nullptr;
            }
            break;
        }
    }
    while(false);

    if (!pcDoc) {
        PyErr_SetString(PyExc_TypeError, "Either string or App.Document expected");
        return nullptr;
    }

    if (Application::Instance->activeDocument() != pcDoc) {
        Gui::MDIView* view = pcDoc->getActiveView();
        getMainWindow()->setActiveWindow(view);
    }

    Py_Return;
}

PyObject* ApplicationPy::sGetDocument(PyObject * /*self*/, PyObject *args)
{
    char *pstr = nullptr;
    if (PyArg_ParseTuple(args, "s", &pstr)) {
        Document *pcDoc = Application::Instance->getDocument(pstr);
        if (!pcDoc) {
            PyErr_Format(PyExc_NameError, "Unknown document '%s'", pstr);
            return nullptr;
        }
        return pcDoc->getPyObject();
    }

    PyErr_Clear();
    PyObject* doc = nullptr;
    if (PyArg_ParseTuple(args, "O!", &(App::DocumentPy::Type), &doc)) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
        Document *pcDoc = Application::Instance->getDocument(static_cast<App::DocumentPy*>(doc)->getDocumentPtr());
        if (!pcDoc) {
            PyErr_Format(PyExc_KeyError, "Unknown document instance");
            return nullptr;
        }
        return pcDoc->getPyObject();
    }

    PyErr_SetString(PyExc_TypeError, "Either string or App.Document expected");
    return nullptr;
}

PyObject* ApplicationPy::sHide(PyObject * /*self*/, PyObject *args)
{
    char *psFeatStr = nullptr;
    if (!PyArg_ParseTuple(args, "s;Name of the object to hide has to be given!",&psFeatStr)) {
        return nullptr;
    }

    Document *pcDoc = Application::Instance->activeDocument();

    if (pcDoc) {
        pcDoc->setHide(psFeatStr);
    }

    Py_Return;
}

PyObject* ApplicationPy::sShow(PyObject * /*self*/, PyObject *args)
{
    char *psFeatStr = nullptr;
    if (!PyArg_ParseTuple(args, "s;Name of the object to show has to be given!",&psFeatStr)) {
        return nullptr;
    }

    Document *pcDoc = Application::Instance->activeDocument();

    if (pcDoc) {
        pcDoc->setShow(psFeatStr);
    }

    Py_Return;
}

PyObject* ApplicationPy::sHideObject(PyObject * /*self*/, PyObject *args)
{
    PyObject *object = nullptr;
    if (!PyArg_ParseTuple(args, "O!",&(App::DocumentObjectPy::Type),&object)) {
        return nullptr;
    }

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
    App::DocumentObject* obj = static_cast<App::DocumentObjectPy*>(object)->getDocumentObjectPtr();
    Application::Instance->hideViewProvider(obj);

    Py_Return;
}

PyObject* ApplicationPy::sShowObject(PyObject * /*self*/, PyObject *args)
{
    PyObject *object = nullptr;
    if (!PyArg_ParseTuple(args, "O!",&(App::DocumentObjectPy::Type),&object)) {
        return nullptr;
    }

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
    App::DocumentObject* obj = static_cast<App::DocumentObjectPy*>(object)->getDocumentObjectPtr();
    Application::Instance->showViewProvider(obj);

    Py_Return;
}

PyObject* ApplicationPy::sOpen(PyObject * /*self*/, PyObject *args)
{
    // only used to open Python files
    char* Name = nullptr;
    if (!PyArg_ParseTuple(args, "et","utf-8",&Name)) {
        return nullptr;
    }

    std::string Utf8Name = std::string(Name);
    PyMem_Free(Name);
    PY_TRY {
        QString fileName = QString::fromUtf8(Utf8Name.c_str());
        FileHandler handler(fileName);
        if (!handler.openFile()) {
            QString ext = handler.extension();
            Base::Console().error("File type '%s' not supported\n", ext.toLatin1().constData());
        }
    }
    PY_CATCH;

    Py_Return;
}

PyObject* ApplicationPy::sInsert(PyObject * /*self*/, PyObject *args)
{
    char* Name = nullptr;
    char* DocName = nullptr;
    if (!PyArg_ParseTuple(args, "et|s","utf-8",&Name,&DocName)) {
        return nullptr;
    }

    std::string Utf8Name = std::string(Name);
    PyMem_Free(Name);

    PY_TRY {
        QString fileName = QString::fromUtf8(Utf8Name.c_str());
        FileHandler handler(fileName);
        if (!handler.importFile(std::string(DocName ? DocName : ""))) {
            QString ext = handler.extension();
            Base::Console().error("File type '%s' not supported\n", ext.toLatin1().constData());
        }
    } PY_CATCH;

    Py_Return;
}

PyObject* ApplicationPy::sExport(PyObject * /*self*/, PyObject *args)
{
    PyObject* object = nullptr;
    char* Name = nullptr;
    if (!PyArg_ParseTuple(args, "Oet",&object,"utf-8",&Name)) {
        return nullptr;
    }

    std::string Utf8Name = std::string(Name);
    PyMem_Free(Name);

    PY_TRY {
        App::Document* doc = nullptr;
        Py::Sequence list(object);
        for (Py::Sequence::iterator it = list.begin(); it != list.end(); ++it) {
            PyObject* item = (*it).ptr();
            if (PyObject_TypeCheck(item, &(App::DocumentObjectPy::Type))) {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
                App::DocumentObject* obj = static_cast<App::DocumentObjectPy*>(item)->getDocumentObjectPtr();
                doc = obj->getDocument();
                break;
            }
        }

        QString fileName = QString::fromUtf8(Utf8Name.c_str());
        QFileInfo fi;
        fi.setFile(fileName);
        QString ext = fi.suffix().toLower();
        if (ext == QLatin1String("iv") ||
            ext == QLatin1String("wrl") ||
            ext == QLatin1String("vrml") ||
            ext == QLatin1String("wrz") ||
            ext == QLatin1String("x3d") ||
            ext == QLatin1String("x3dz") ||
            ext == QLatin1String("xhtml")) {

            // build up the graph
            auto sep = new SoSeparator();
            sep->ref();

            for (Py::Sequence::iterator it = list.begin(); it != list.end(); ++it) {
                PyObject* item = (*it).ptr();
                if (PyObject_TypeCheck(item, &(App::DocumentObjectPy::Type))) {
                    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
                    App::DocumentObject* obj = static_cast<App::DocumentObjectPy*>(item)->getDocumentObjectPtr();

                    Gui::ViewProvider* vp = Gui::Application::Instance->getViewProvider(obj);
                    if (vp) {
                        sep->addChild(vp->getRoot());
                    }
                }
            }


            SoGetPrimitiveCountAction action;
            action.setCanApproximate(true);
            action.apply(sep);

            constexpr const int triangleLimit = 100000;
            constexpr const int pointLimit = 30000;
            constexpr const int lineLimit = 10000;

            bool binary = false;
            if (action.getTriangleCount() > triangleLimit ||
                action.getPointCount() > pointLimit ||
                action.getLineCount() > lineLimit) {
                binary = true;
            }

            SoFCDB::writeToFile(sep, Utf8Name.c_str(), binary);
            sep->unref();
        }
        else if (ext == QLatin1String("pdf")) {
            // get the view that belongs to the found document
            Gui::Document* gui_doc = Application::Instance->getDocument(doc);
            if (gui_doc) {
                Gui::MDIView* view = gui_doc->getActiveView();
                if (view) {
                    auto view3d = qobject_cast<View3DInventor*>(view);
                    if (view3d) {
                        view3d->viewAll();
                    }
                    QPrinter printer(QPrinter::ScreenResolution);
                    // setPdfVersion sets the printed PDF Version to what is chosen in
                    // Preferences/Import-Export/PDF more details under:
                    // https://www.kdab.com/creating-pdfa-documents-qt/
                    printer.setPdfVersion(Gui::Dialog::DlgSettingsPDF::evaluatePDFVersion());
                    printer.setOutputFormat(QPrinter::PdfFormat);
                    printer.setOutputFileName(fileName);
                    printer.setCreator(QString::fromStdString(App::Application::getNameWithVersion()));
                    view->print(&printer);
                }
            }
        }
        else {
            Base::Console().error("File type '%s' not supported\n", ext.toLatin1().constData());
        }
    } PY_CATCH;

    Py_Return;
}

PyObject* ApplicationPy::sSendActiveView(PyObject * /*self*/, PyObject *args)
{
    char *psCommandStr = nullptr;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
    PyObject *suppress = Py_False;
    if (!PyArg_ParseTuple(args, "s|O!",&psCommandStr,&PyBool_Type,&suppress)) {
        return nullptr;
    }

    const char* ppReturn = nullptr;
    if (!Application::Instance->sendMsgToActiveView(psCommandStr,&ppReturn)) {
        if (!Base::asBoolean(suppress)) {
            Base::Console().warning("Unknown view command: %s\n",psCommandStr);
        }
    }

    // Print the return value to the output
    if (ppReturn) {
        return Py_BuildValue("s",ppReturn);
    }

    Py_Return;
}

PyObject* ApplicationPy::sSendFocusView(PyObject * /*self*/, PyObject *args)
{
    char *psCommandStr = nullptr;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
    PyObject *suppress = Py_False;
    if (!PyArg_ParseTuple(args, "s|O!",&psCommandStr,&PyBool_Type,&suppress)) {
        return nullptr;
    }

    const char* ppReturn = nullptr;
    if (!Application::Instance->sendMsgToFocusView(psCommandStr,&ppReturn)) {
        if (!Base::asBoolean(suppress)) {
            Base::Console().warning("Unknown view command: %s\n",psCommandStr);
        }
    }

    // Print the return value to the output
    if (ppReturn) {
        return Py_BuildValue("s",ppReturn);
    }

    Py_Return;
}

PyObject* ApplicationPy::sGetMainWindow(PyObject * /*self*/, PyObject *args)
{
    if (!PyArg_ParseTuple(args, "")) {
        return nullptr;
    }

    try {
        return Py::new_reference_to(MainWindowPy::createWrapper(Gui::getMainWindow()));
    }
    catch (const Py::Exception&) {
        return nullptr;
    }
}

PyObject* ApplicationPy::sUpdateGui(PyObject * /*self*/, PyObject *args)
{
    if (!PyArg_ParseTuple(args, "")) {
        return nullptr;
    }

    qApp->processEvents();

    Py_Return;
}

PyObject* ApplicationPy::sUpdateLocale(PyObject * /*self*/, PyObject *args)
{
    if (!PyArg_ParseTuple(args, "")) {
        return nullptr;
    }

    Translator::instance()->refresh();

    Py_Return;
}

PyObject* ApplicationPy::sGetLocale(PyObject * /*self*/, PyObject *args)
{
    if (!PyArg_ParseTuple(args, "")) {
        return nullptr;
    }

    std::string locale = Translator::instance()->activeLanguage();
    return PyUnicode_FromString(locale.c_str());
}

PyObject* ApplicationPy::sSetLocale(PyObject * /*self*/, PyObject *args)
{
    char* name = nullptr;
    if (!PyArg_ParseTuple(args, "s", &name)) {
        return nullptr;
    }

    std::string cname(name);
    TStringMap map = Translator::instance()->supportedLocales();
    map["English"] = "en";
    for (const auto& it : map) {
        if (it.first == cname || it.second == cname) {
            Translator::instance()->activateLanguage(it.first.c_str());
            break;
        }
    }

    Py_Return;
}

PyObject* ApplicationPy::sSupportedLocales(PyObject * /*self*/, PyObject *args)
{
    if (!PyArg_ParseTuple(args, "")) {
        return nullptr;
    }

    TStringMap map = Translator::instance()->supportedLocales();
    Py::Dict dict;
    dict.setItem(Py::String("English"), Py::String("en"));
    for (const auto& it : map) {
        Py::String key(it.first);
        Py::String val(it.second);
        dict.setItem(key, val);
    }
    return Py::new_reference_to(dict);
}

PyObject* ApplicationPy::sCreateDialog(PyObject * /*self*/, PyObject *args)
{
    char* fn = nullptr;
    if (!PyArg_ParseTuple(args, "s", &fn)) {
        return nullptr;
    }

    PyObject* pPyResource = nullptr;
    try{
        pPyResource = new PyResource();
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
        static_cast<PyResource*>(pPyResource)->load(fn);
    }
    catch (const Base::Exception& e) {
        PyErr_SetString(PyExc_AssertionError, e.what());
        return nullptr;
    }

    return pPyResource;
}

PyObject* ApplicationPy::sAddPreferencePage(PyObject * /*self*/, PyObject *args)
{
    char* fn = nullptr;
    char* grp = nullptr;
    if (PyArg_ParseTuple(args, "ss", &fn,&grp)) {
        QFileInfo fi(QString::fromUtf8(fn));
        if (!fi.exists()) {
            PyErr_SetString(PyExc_RuntimeError, "UI file does not exist");
            return nullptr;
        }

        // add to the preferences dialog
        new PrefPageUiProducer(fn, grp);
        Py_Return;
    }

    PyErr_Clear();
    PyObject* dlg = nullptr;
    // new style classes
    if (PyArg_ParseTuple(args, "O!s", &PyType_Type, &dlg, &grp)) {
        // add to the preferences dialog
        new PrefPagePyProducer(Py::Object(dlg), grp);
        Py_Return;
    }

    return nullptr;
}

PyObject* ApplicationPy::sActivateWorkbenchHandler(PyObject * /*self*/, PyObject *args)
{
    char* psKey = nullptr;
    if (!PyArg_ParseTuple(args, "s", &psKey)) {
        return nullptr;
    }

    // search for workbench handler from the dictionary
    PyObject* pcWorkbench = PyDict_GetItemString(Application::Instance->_pcWorkbenchDictionary, psKey);
    if (!pcWorkbench) {
        PyErr_Format(PyExc_KeyError, "No such workbench '%s'", psKey);
        return nullptr;
    }

    try {
        bool ok = Application::Instance->activateWorkbench(psKey);
        return Py::new_reference_to(Py::Boolean(ok));
    }
    catch (const Base::Exception& e) {
        std::stringstream err;
        err << psKey << ": " << e.what();
        PyErr_SetString(e.getPyExceptionType(), err.str().c_str());
        return nullptr;
    }
    catch (const XERCES_CPP_NAMESPACE_QUALIFIER TranscodingException& e) {
        std::stringstream err;
        char *pMsg = XERCES_CPP_NAMESPACE_QUALIFIER XMLString::transcode(e.getMessage());
        err << "Transcoding exception in Xerces-c:\n\n"
            << "Transcoding exception raised in activateWorkbench.\n"
            << "Check if your user configuration file is valid.\n"
            << "  Exception message:"
            << pMsg;
        XERCES_CPP_NAMESPACE_QUALIFIER XMLString::release(&pMsg);
        PyErr_SetString(PyExc_RuntimeError, err.str().c_str());
        return nullptr;
    }
    catch (...) {
        std::stringstream err;
        err << "Unknown C++ exception raised in activateWorkbench('" << psKey << "')";
        PyErr_SetString(Base::PyExc_FC_GeneralError, err.str().c_str());
        return nullptr;
    }
}

PyObject* ApplicationPy::sAddWorkbenchHandler(PyObject * /*self*/, PyObject *args)
{
    PyObject* pcObject = nullptr;
    if (!PyArg_ParseTuple(args, "O", &pcObject)) {
        return nullptr;
    }

    try {
        // get the class object 'Workbench' from the main module that is expected
        // to be base class for all workbench classes
        Py::Module module("__main__");
        Py::Object baseclass(module.getAttr(std::string("Workbench")));

        // check whether it is an instance or class object
        Py::Object object(pcObject);
        Py::String name;

        if (PyObject_IsSubclass(object.ptr(), baseclass.ptr()) == 1) {
            // create an instance of this class
            name = object.getAttr(std::string("__name__"));
            Py::Tuple arg;
            Py::Callable creation(object);
            object = creation.apply(arg);
        }
        else if (PyObject_IsInstance(object.ptr(), baseclass.ptr()) == 1) {
            // extract the class name of the instance
            PyErr_Clear(); // PyObject_IsSubclass set an exception
            Py::Object classobj = object.getAttr(std::string("__class__"));
            name = classobj.getAttr(std::string("__name__"));
        }
        else {
            PyErr_SetString(PyExc_TypeError, "arg must be a subclass or an instance of "
                                             "a subclass of 'Workbench'");
            return nullptr;
        }

        // NOLINTBEGIN(bugprone-unused-raii)
        // Search for some methods and members without invoking them
        Py::Callable(object.getAttr(std::string("Initialize")));
        Py::Callable(object.getAttr(std::string("GetClassName")));
        // NOLINTEND(bugprone-unused-raii)

        std::string item = name.as_std_string("ascii");
        PyObject* wb = PyDict_GetItemString(Application::Instance->_pcWorkbenchDictionary,item.c_str());
        if (wb) {
            PyErr_Format(PyExc_KeyError, "'%s' already exists.", item.c_str());
            return nullptr;
        }

        PyDict_SetItemString(Application::Instance->_pcWorkbenchDictionary,item.c_str(),object.ptr());
        Application::Instance->signalRefreshWorkbenches();
    }
    catch (const Py::Exception&) {
        return nullptr;
    }

    Py_Return;
}

PyObject* ApplicationPy::sRemoveWorkbenchHandler(PyObject * /*self*/, PyObject *args)
{
    char* psKey = nullptr;
    if (!PyArg_ParseTuple(args, "s", &psKey)) {
        return nullptr;
    }

    PyObject* wb = PyDict_GetItemString(Application::Instance->_pcWorkbenchDictionary,psKey);
    if (!wb) {
        PyErr_Format(PyExc_KeyError, "No such workbench '%s'", psKey);
        return nullptr;
    }

    WorkbenchManager::instance()->removeWorkbench(psKey);
    PyDict_DelItemString(Application::Instance->_pcWorkbenchDictionary,psKey);
    Application::Instance->signalRefreshWorkbenches();

    Py_Return;
}

PyObject* ApplicationPy::sGetWorkbenchHandler(PyObject * /*self*/, PyObject *args)
{
    char* psKey = nullptr;
    if (!PyArg_ParseTuple(args, "s", &psKey)) {
        return nullptr;
    }

    // get the python workbench object from the dictionary
    PyObject* pcWorkbench = PyDict_GetItemString(Application::Instance->_pcWorkbenchDictionary, psKey);
    if (!pcWorkbench) {
        PyErr_Format(PyExc_KeyError, "No such workbench '%s'", psKey);
        return nullptr;
    }

    Py_INCREF(pcWorkbench);
    return pcWorkbench;
}

PyObject* ApplicationPy::sListWorkbenchHandlers(PyObject * /*self*/, PyObject *args)
{
    if (!PyArg_ParseTuple(args, "")) {
        return nullptr;
    }

    Py_INCREF(Application::Instance->_pcWorkbenchDictionary);
    return Application::Instance->_pcWorkbenchDictionary;
}

PyObject* ApplicationPy::sActiveWorkbenchHandler(PyObject * /*self*/, PyObject *args)
{
    if (!PyArg_ParseTuple(args, "")) {
        return nullptr;
    }

    Workbench* actWb = WorkbenchManager::instance()->active();
    if (!actWb) {
        PyErr_SetString(PyExc_AssertionError, "No active workbench\n");
        return nullptr;
    }

    // get the python workbench object from the dictionary
    std::string key = actWb->name();
    PyObject* pcWorkbench = PyDict_GetItemString(Application::Instance->_pcWorkbenchDictionary, key.c_str());
    if (!pcWorkbench) {
        PyErr_Format(PyExc_KeyError, "No such workbench '%s'", key.c_str());
        return nullptr;
    }

    // object get incremented
    Py_INCREF(pcWorkbench);
    return pcWorkbench;
}

PyObject* ApplicationPy::sAddResPath(PyObject * /*self*/, PyObject *args)
{
    char* filePath = nullptr;
    if (!PyArg_ParseTuple(args, "et", "utf-8", &filePath)) {
        return nullptr;
    }

    QString path = QString::fromUtf8(filePath);
    PyMem_Free(filePath);
    if (QDir::isRelativePath(path)) {
        // Home path ends with '/'
        QString home = QString::fromStdString(App::Application::getHomePath());
        path = home + path;
    }

    BitmapFactory().addPath(path);
    Translator::instance()->addPath(path);

    Py_Return;
}

PyObject* ApplicationPy::sAddLangPath(PyObject * /*self*/, PyObject *args)
{
    char* filePath = nullptr;
    if (!PyArg_ParseTuple(args, "et", "utf-8", &filePath)) {
        return nullptr;
    }

    QString path = QString::fromUtf8(filePath);
    PyMem_Free(filePath);
    if (QDir::isRelativePath(path)) {
        // Home path ends with '/'
        QString home = QString::fromStdString(App::Application::getHomePath());
        path = home + path;
    }

    Translator::instance()->addPath(path);

    Py_Return;
}

PyObject* ApplicationPy::sAddIconPath(PyObject * /*self*/, PyObject *args)
{
    char* filePath = nullptr;
    if (!PyArg_ParseTuple(args, "et", "utf-8", &filePath)) {
        return nullptr;
    }

    QString path = QString::fromUtf8(filePath);
    PyMem_Free(filePath);
    if (QDir::isRelativePath(path)) {
        // Home path ends with '/'
        QString home = QString::fromStdString(App::Application::getHomePath());
        path = home + path;
    }

    BitmapFactory().addPath(path);

    Py_Return;
}

PyObject* ApplicationPy::sAddIcon(PyObject * /*self*/, PyObject *args)
{
    const char *iconName = nullptr;
    Py_buffer content;
    const char *format = "XPM";
    if (!PyArg_ParseTuple(args, "ss*|s", &iconName, &content, &format)) {
        return nullptr;
    }

    QPixmap icon;
    if (BitmapFactory().findPixmapInCache(iconName, icon)) {
        PyErr_SetString(PyExc_AssertionError, "Icon with this name already registered");
        PyBuffer_Release(&content);
        return nullptr;
    }

    const char* contentStr = static_cast<const char*>(content.buf);
    QByteArray ary(contentStr, static_cast<int>(content.len));
    icon.loadFromData(ary, format);

    if (icon.isNull()){
        QString file = QString::fromUtf8(contentStr, static_cast<int>(content.len));
        icon.load(file);
    }

    PyBuffer_Release(&content);

    if (icon.isNull()) {
        PyErr_SetString(Base::PyExc_FC_GeneralError, "Invalid icon added to application");
        return nullptr;
    }

    BitmapFactory().addPixmapToCache(iconName, icon);

    Py_Return;
}

PyObject* ApplicationPy::sGetIcon(PyObject * /*self*/, PyObject *args)
{
    char *iconName = nullptr;
    if (!PyArg_ParseTuple(args, "s", &iconName)) {
        return nullptr;
    }

    PythonWrapper wrap;
    wrap.loadGuiModule();
    wrap.loadWidgetsModule();
    auto pixmap = BitmapFactory().pixmap(iconName);
    if(!pixmap.isNull()) {
        return Py::new_reference_to(wrap.fromQIcon(new QIcon(pixmap)));
    }

    Py_Return;
}

PyObject* ApplicationPy::sIsIconCached(PyObject * /*self*/, PyObject *args)
{
    char *iconName = nullptr;
    if (!PyArg_ParseTuple(args, "s", &iconName)) {
        return nullptr;
    }

    QPixmap icon;

    return Py::new_reference_to(Py::Boolean(BitmapFactory().findPixmapInCache(iconName, icon)));
}

PyObject* ApplicationPy::sAddCommand(PyObject * /*self*/, PyObject *args)
{
    char* pName = nullptr;
    char* pSource = nullptr;
    PyObject* pcCmdObj = nullptr;
    if (!PyArg_ParseTuple(args, "sO|s", &pName,&pcCmdObj,&pSource)) {
        return nullptr;
    }

    // get the call stack to find the Python module name
    //
    std::string module;
    std::string group;
    try {
        Base::PyGILStateLocker lock;

        // Get the filename of the running code by using the low-level sys._getframe() method.
        // We use this instead of the `inspect` module (which may actually cause imports to execute
        // and can result in a circular import if sAddCommand is being called as part of an import
        // statement itself), and the `traceback` module (which cannot access the filename of code
        // that is being run through the C interface).

        Py::Module sysMod(PyImport_ImportModule("sys"), /*owned=*/true);
        Py::Callable getFrame(sysMod.getAttr("_getframe"));

        Py::Object callerFrame;
        Py::Tuple getFrameArgs(1);
        getFrameArgs[0] = Py::Long(0);
        callerFrame = getFrame.apply(getFrameArgs);

        Py::Object codeObj (callerFrame.getAttr("f_code"));

        Py::Object filenameObj (codeObj.getAttr("co_filename"));
        std::string filename (Py::String(filenameObj).as_std_string());

        Base::FileInfo fi(filename);
        // convert backslashes to slashes
        filename = fi.filePath();
        module = fi.fileNamePure();
        // for the group name get the directory name after 'Mod'
        boost::regex rx("/Mod/(\\w+)/");
        boost::smatch what;
        if (boost::regex_search(filename, what, rx)) {
            group = what[1];
        }
        else {
            rx = "/Ext/freecad/(\\w+)/";
            if (boost::regex_search(filename, what, rx)) {
                group = what[1];
            } else {
                group = module;
            }
        }
    }
    catch (Py::Exception& e) {
        e.clear();
    }
    try {
        Base::PyGILStateLocker lock;

        Py::Object cmd(pcCmdObj);
        if (cmd.hasAttr("GetCommands")) {
            Command* cmd = new PythonGroupCommand(pName, pcCmdObj);
            if (!module.empty()) {
                cmd->setAppModuleName(module.c_str());
            }
            if (!group.empty()) {
                cmd->setGroupName(group.c_str());
            }
            Application::Instance->commandManager().addCommand(cmd);
        }
        else {
            Command* cmd = new PythonCommand(pName, pcCmdObj, pSource);
            if (!module.empty()) {
                cmd->setAppModuleName(module.c_str());
            }
            if (!group.empty()) {
                cmd->setGroupName(group.c_str());
            }
            Application::Instance->commandManager().addCommand(cmd);
        }
    }
    catch (const Base::Exception& e) {
        e.setPyException();
        return nullptr;
    }
    catch (...) {
        PyErr_SetString(Base::PyExc_FC_GeneralError, "Unknown C++ exception raised in ApplicationPy::sAddCommand()");
        return nullptr;
    }

    Py_Return;
}

PyObject* ApplicationPy::sRunCommand(PyObject * /*self*/, PyObject *args)
{
    char* pName = nullptr;
    int item = 0;
    if (!PyArg_ParseTuple(args, "s|i", &pName, &item)) {
        return nullptr;
    }

    Gui::Command::LogDisabler d1;
    Gui::SelectionLogDisabler d2;

    Command* cmd = Application::Instance->commandManager().getCommandByName(pName);
    if (cmd) {
        cmd->invoke(item);
        Py_Return;
    }

    PyErr_Format(Base::PyExc_FC_GeneralError, "No such command '%s'", pName);
    return nullptr;
}

PyObject* ApplicationPy::sDoCommand(PyObject * /*self*/, PyObject *args)
{
    char *sCmd = nullptr;
    if (!PyArg_ParseTuple(args, "s", &sCmd)) {
        return nullptr;
    }

    Gui::Command::LogDisabler d1;
    Gui::SelectionLogDisabler d2;

    Gui::Command::printPyCaller();
    Gui::Application::Instance->macroManager()->addLine(MacroManager::App, sCmd);

    PyObject* module = nullptr;
    PyObject* dict = nullptr;

    Base::PyGILStateLocker locker;
    module = PyImport_AddModule("__main__");
    if (!module) {
        return nullptr;
    }

    dict = PyModule_GetDict(module);
    if (!dict) {
        return nullptr;
    }

    return PyRun_String(sCmd, Py_file_input, dict, dict);
}

PyObject* ApplicationPy::sDoCommandGui(PyObject * /*self*/, PyObject *args)
{
    char *sCmd = nullptr;
    if (!PyArg_ParseTuple(args, "s", &sCmd)) {
        return nullptr;
    }

    Gui::Command::LogDisabler d1;
    Gui::SelectionLogDisabler d2;

    Gui::Command::printPyCaller();
    Gui::Application::Instance->macroManager()->addLine(MacroManager::Gui, sCmd);

    PyObject* module = nullptr;
    PyObject* dict = nullptr;

    Base::PyGILStateLocker locker;
    module = PyImport_AddModule("__main__");
    if (!module) {
        return nullptr;
    }

    dict = PyModule_GetDict(module);
    if (!dict) {
        return nullptr;
    }

    return PyRun_String(sCmd, Py_file_input, dict, dict);
}

PyObject* ApplicationPy::sDoCommandEval(PyObject * /*self*/, PyObject *args)
{
    char *sCmd = nullptr;
    if (!PyArg_ParseTuple(args, "s", &sCmd)) {
        return nullptr;
    }

    Gui::Command::LogDisabler d1;
    Gui::SelectionLogDisabler d2;

    PyObject* module = nullptr;
    PyObject* dict = nullptr;

    Base::PyGILStateLocker locker;
    module = PyImport_AddModule("__main__");
    if (!module) {
        return nullptr;
}

    dict = PyModule_GetDict(module);
    if (!dict) {
        return nullptr;
    }

    return PyRun_String(sCmd, Py_eval_input, dict, dict);
}

PyObject* ApplicationPy::sDoCommandSkip(PyObject * /*self*/, PyObject *args)
{
    char *sCmd = nullptr;
    if (!PyArg_ParseTuple(args, "s", &sCmd)) {
        return nullptr;
    }

    Gui::Command::LogDisabler d1;
    Gui::SelectionLogDisabler d2;

    Gui::Command::printPyCaller();
    Gui::Application::Instance->macroManager()->addLine(MacroManager::App, sCmd);
    return Py::None().ptr();
}

PyObject* ApplicationPy::sAddModule(PyObject * /*self*/, PyObject *args)
{
    char *pstr = nullptr;
    if (!PyArg_ParseTuple(args, "s", &pstr)) {
        return nullptr;
    }

    try {
        Command::addModule(Command::Doc,pstr);
        Py_Return;
    }
    catch (const Base::Exception& e) {
        PyErr_SetString(PyExc_ImportError, e.what());
        return nullptr;
    }
}

PyObject* ApplicationPy::sShowDownloads(PyObject * /*self*/, PyObject *args)
{
    if (!PyArg_ParseTuple(args, "")) {
        return nullptr;
    }

    Gui::Dialog::DownloadManager::getInstance();

    Py_Return;
}

PyObject* ApplicationPy::sShowPreferences(PyObject* /*self*/, PyObject* args)
{
    char* pstr = nullptr;
    int idx = 0;
    if (!PyArg_ParseTuple(args, "|si", &pstr, &idx)) {
        return nullptr;
    }

    Gui::Dialog::DlgPreferencesImp cDlg(getMainWindow());
    if (pstr) {
        cDlg.activateGroupPage(QString::fromUtf8(pstr), idx);
    }

    WaitCursor wc;
    wc.restoreCursor();
    cDlg.exec();
    wc.setWaitCursor();

    Py_Return;
}

PyObject* ApplicationPy::sShowPreferencesByName(PyObject* /*self*/, PyObject* args)
{
    char* pstr = nullptr;
    const char* prefType = "";
    if (!PyArg_ParseTuple(args, "s|s", &pstr, &prefType)) {
        return nullptr;
    }

    Gui::Dialog::DlgPreferencesImp cDlg(getMainWindow());
    if (pstr && prefType) {
        cDlg.activateGroupPageByPageName(QString::fromUtf8(pstr), QString::fromUtf8(prefType));
    }

    WaitCursor wc;
    wc.restoreCursor();
    cDlg.exec();
    wc.setWaitCursor();

    Py_Return;
}

PyObject* ApplicationPy::sCreateViewer(PyObject * /*self*/, PyObject *args)
{
    int num_of_views = 1;
    char* title = nullptr;
    // if one argument (int) is given
    if (!PyArg_ParseTuple(args, "|is", &num_of_views, &title)) {
        return nullptr;
    }

    if (num_of_views <= 0) {
        PyErr_Format(PyExc_ValueError, "views must be > 0");
        return nullptr;
    }
    if (num_of_views == 1) {
        auto viewer = new View3DInventor(nullptr, nullptr);
        if (title) {
            viewer->setWindowTitle(QString::fromUtf8(title));
        }
        Gui::getMainWindow()->addWindow(viewer);
        return viewer->getPyObject();
    }
    else {
        auto viewer = new SplitView3DInventor(num_of_views, nullptr, nullptr);
        if (title) {
            viewer->setWindowTitle(QString::fromUtf8(title));
        }
        Gui::getMainWindow()->addWindow(viewer);
        return viewer->getPyObject();
    }
}

PyObject* ApplicationPy::sGetMarkerIndex(PyObject * /*self*/, PyObject *args)
{
    constexpr const int defaultSize = 9;
    char *pstr {};
    int  defSize = defaultSize;
    if (!PyArg_ParseTuple(args, "s|i", &pstr, &defSize)) {
        return nullptr;
    }

    PY_TRY {
        ParameterGrp::handle const hGrp = App::GetApplication().GetParameterGroupByPath("User parameter:BaseApp/Preferences/View");

        //find the appropriate marker style string token
        std::string marker_arg = pstr;

        std::list<std::pair<std::string, std::string> > markerList = {
            {"square", "DIAMOND_FILLED"},
            {"cross", "CROSS"},
            {"hourglass", "HOURGLASS_FILLED"},
            {"plus", "PLUS"},
            {"empty", "SQUARE_LINE"},
            {"quad", "SQUARE_FILLED"},
            {"circle", "CIRCLE_LINE"},
            {"default", "CIRCLE_FILLED"}
        };

        auto findIt = std::find_if(markerList.begin(), markerList.end(), [&marker_arg](const auto& it) {
            return marker_arg == it.first || marker_arg == it.second;
        });

        marker_arg = (findIt != markerList.end() ? findIt->second : "CIRCLE_FILLED");


        //get the marker size
        auto sizeList = Gui::Inventor::MarkerBitmaps::getSupportedSizes(marker_arg);

        if (std::ranges::find(sizeList, defSize) == std::end(sizeList)) {
            defSize = defaultSize;
        }

        return Py_BuildValue("i", Gui::Inventor::MarkerBitmaps::getMarkerIndex(marker_arg, defSize));
    }
    PY_CATCH;
}

PyObject* ApplicationPy::sReload(PyObject * /*self*/, PyObject *args)
{
    const char *name = nullptr;
    if (!PyArg_ParseTuple(args, "s", &name)) {
        return nullptr;
    }

    PY_TRY {
        auto doc = Application::Instance->reopen(App::GetApplication().getDocument(name));
        if(doc) {
            return doc->getPyObject();
        }
        Py_Return;
    }
    PY_CATCH;
}

PyObject* ApplicationPy::sLoadFile(PyObject * /*self*/, PyObject *args)
{
    const char *path = "";
    const char *mod = "";
    if (!PyArg_ParseTuple(args, "s|s", &path, &mod)) {
        return nullptr;
    }

    PY_TRY {
        Base::FileInfo fi(path);
        if (!fi.isFile() || !fi.exists()) {
            PyErr_Format(PyExc_IOError, "File %s doesn't exist.", path);
            return nullptr;
        }

        std::string module = mod;
        if (module.empty()) {
            std::string ext = fi.extension();
            std::vector<std::string> modules = App::GetApplication().getImportModules(ext.c_str());
            if (modules.empty()) {
                PyErr_Format(PyExc_IOError, "Filetype %s is not supported.", ext.c_str());
                return nullptr;
            }

            module = modules.front();
        }

        Application::Instance->open(path,module.c_str());

        Py_Return;
    }
    PY_CATCH
}

PyObject* ApplicationPy::sAddDocObserver(PyObject * /*self*/, PyObject *args)
{
    PyObject* o = nullptr;
    if (!PyArg_ParseTuple(args, "O",&o)) {
        return nullptr;
    }

    PY_TRY {
        DocumentObserverPython::addObserver(Py::Object(o));
        Py_Return;
    }
    PY_CATCH;
}

PyObject* ApplicationPy::sRemoveDocObserver(PyObject * /*self*/, PyObject *args)
{
    PyObject* o = nullptr;
    if (!PyArg_ParseTuple(args, "O",&o)) {
        return nullptr;
    }

    PY_TRY {
        DocumentObserverPython::removeObserver(Py::Object(o));
        Py_Return;
    }
    PY_CATCH;
}

PyObject* ApplicationPy::sAddWbManipulator(PyObject * /*self*/, PyObject *args)
{
    PyObject* o = nullptr;
    if (!PyArg_ParseTuple(args, "O",&o)) {
        return nullptr;
    }

    PY_TRY {
        WorkbenchManipulatorPython::installManipulator(Py::Object(o));
        Py_Return;
    }
    PY_CATCH;
}

PyObject* ApplicationPy::sRemoveWbManipulator(PyObject * /*self*/, PyObject *args)
{
    PyObject* o = nullptr;
    if (!PyArg_ParseTuple(args, "O",&o)) {
        return nullptr;
    }

    PY_TRY {
        WorkbenchManipulatorPython::removeManipulator(Py::Object(o));
        Py_Return;
    }
    PY_CATCH;
}

PyObject* ApplicationPy::sCoinRemoveAllChildren(PyObject * /*self*/, PyObject *args)
{
    PyObject *pynode = nullptr;
    if (!PyArg_ParseTuple(args, "O", &pynode)) {
        return nullptr;
    }

    PY_TRY {
        void* ptr = nullptr;
        Base::Interpreter().convertSWIGPointerObj("pivy.coin","_p_SoGroup", pynode, &ptr, 0);
        if (!ptr) {
            PyErr_SetString(PyExc_RuntimeError, "Conversion of coin.SoGroup failed");
            return nullptr;
        }

        coinRemoveAllChildren(static_cast<SoGroup*>(ptr));
        Py_Return;
    }
    PY_CATCH;
}

PyObject* ApplicationPy::sListUserEditModes(PyObject * /*self*/, PyObject *args)
{
    Py::List ret;
    if (!PyArg_ParseTuple(args, "")) {
        return nullptr;
    }

    for (auto const &uem : Application::Instance->listUserEditModes()) {
        ret.append(Py::String(uem.second.first));
    }

    return Py::new_reference_to(ret);
}

PyObject* ApplicationPy::sGetUserEditMode(PyObject * /*self*/, PyObject *args)
{
    if (!PyArg_ParseTuple(args, "")) {
        return nullptr;
    }

    return Py::new_reference_to(Py::String(Application::Instance->getUserEditModeUIStrings().first));
}

PyObject* ApplicationPy::sSetUserEditMode(PyObject * /*self*/, PyObject *args)
{
    const char *mode = "";
    if (!PyArg_ParseTuple(args, "s", &mode)) {
        return nullptr;
    }

    bool ok = Application::Instance->setUserEditMode(std::string(mode));

    return Py::new_reference_to(Py::Boolean(ok));
}

PyObject* ApplicationPy::sSuspendWaitCursor(PyObject * /*self*/, PyObject *args)
{
    if (!PyArg_ParseTuple(args, "")) {
        return nullptr;
    }

    WaitCursor::suspend();
    Py_RETURN_NONE;
}

PyObject* ApplicationPy::sResumeWaitCursor(PyObject * /*self*/, PyObject *args)
{
    if (!PyArg_ParseTuple(args, "")) {
        return nullptr;
    }

    WaitCursor::resume();
    Py_RETURN_NONE;
}
