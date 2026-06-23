# FREECAD_QT_VERSION selects between Qt6 and no Qt support at all.

# If it is set to 6 or to Auto (the value older trees have in their cache),
# FreeCAD is built against the Qt6 installation found on the system.  If it is
# None, Qt support is disabled and only the parts of FreeCAD which do not need
# Qt are built.

# The output variables are FREECAD_HAS_QT, FREECAD_QT_MAJOR_VERSION and
# FREECAD_QT_BASE_COMPONENTS.

macro(ChooseQtVersion)
  set(freecad_supported_qt_versions "Auto" 6 "None")
  set(FREECAD_QT_BASE_COMPONENTS Core Concurrent Network Xml LinguistTools)
  set(FREECAD_HAS_QT OFF)

  if (NOT DEFINED FREECAD_QT_VERSION)
    set(FREECAD_QT_VERSION 6 CACHE
      STRING "Expected Qt major version. Valid values are Auto, 6 and None.")
    set_property(CACHE FREECAD_QT_VERSION PROPERTY STRINGS "${freecad_supported_qt_versions}")
  endif()

  if (NOT FREECAD_QT_VERSION IN_LIST freecad_supported_qt_versions)
    message(FATAL_ERROR
      "Supported Qt versions are \"${freecad_supported_qt_versions}\". But "
      "FREECAD_QT_VERSION is set to ${FREECAD_QT_VERSION}.")
  endif()

  if (BUILD_GUI AND FREECAD_QT_VERSION STREQUAL "None")
    message(FATAL_ERROR "The GUI needs Qt, so FREECAD_QT_VERSION must not be None.")
  endif()

  if (FREECAD_QT_VERSION STREQUAL "None")
    unset(FREECAD_QT_MAJOR_VERSION CACHE)
    message(STATUS "Qt support is disabled")
  else()
    find_package(Qt6 REQUIRED COMPONENTS Core)
    set(FREECAD_HAS_QT ON)
    set(FREECAD_QT_MAJOR_VERSION 6 CACHE INTERNAL
      "Major version number for the Qt installation used.")
    message(STATUS "Compiling with Qt ${FREECAD_QT_MAJOR_VERSION}")
  endif()
endmacro()
