#=============================================================================
# Copyright 2005-2011 Kitware, Inc.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# * Redistributions of source code must retain the above copyright
#   notice, this list of conditions and the following disclaimer.
#
# * Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the following disclaimer in the
#   documentation and/or other materials provided with the distribution.
#
# * Neither the name of Kitware, Inc. nor the names of its
#   contributors may be used to endorse or promote products derived
#   from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#=============================================================================

# Portability helpers.

set(QT_QTGUI_LIBRARIES
  ${Qt5Gui_LIBRARIES}
  ${Qt5Widgets_LIBRARIES}
  ${Qt5PrintSupport_LIBRARIES}
  ${Qt5Svg_LIBRARIES}
)

set(QT_INCLUDES
    ${Qt5Gui_INCLUDE_DIRS}
    ${Qt5Widgets_INCLUDE_DIRS}
    ${Qt5PrintSupport_INCLUDE_DIRS}
    ${Qt5Svg_INCLUDE_DIRS}
)

set(_qt_modules
  Core
  Declarative
  Widgets
  Script
  ScriptTools
  DBus
  Network
  Test
  Designer
  Concurrent
  Xml
  UiTools
  WebKit
  Sql
  OpenGL
)

foreach(_module ${_qt_modules})
    string(TOUPPER ${_module} _module_upper)
    set(QT_QT${_module_upper}_LIBRARIES ${Qt5${_module}_LIBRARIES})
    set(QT_QT${_module_upper}_LIBRARY ${QT_QT${_module_upper}_LIBRARIES})
    list(APPEND QT_INCLUDES ${Qt5${_module}_INCLUDE_DIRS})
    set(QT_QT${_module_upper}_FOUND ${Qt5${_module}_FOUND})
endforeach()

get_target_property(QT_QMAKE_EXECUTABLE Qt5::qmake LOCATION)
get_target_property(QT_RCC_EXECUTABLE Qt5::rcc LOCATION)
if (TARGET Qt5::uic)
    get_target_property(QT_UIC_EXECUTABLE Qt5::uic LOCATION)
endif()

if (TARGET Qt5::qdbuscpp2xml)
    get_target_property(QT_QDBUSCPP2XML_EXECUTABLE Qt5::qdbuscpp2xml LOCATION)
endif()

if (TARGET Qt5::qdbusxml2cpp)
    get_target_property(QT_QDBUSXML2CPP_EXECUTABLE Qt5::qdbusxml2cpp LOCATION)
endif()

macro(qt4_wrap_ui)
  qt5_wrap_ui(${ARGN})
endmacro()

macro(qt4_wrap_cpp)
  qt5_wrap_cpp(${ARGN})
endmacro()

macro(qt4_generate_moc)
  qt5_generate_moc(${ARGN})
endmacro()

macro(qt4_add_dbus_adaptor)
  qt5_add_dbus_adaptor(${ARGN})
endmacro()

macro(qt4_add_dbus_interfaces)
  qt5_add_dbus_interfaces(${ARGN})
endmacro()

macro(qt4_add_dbus_interface)
  qt5_add_dbus_interface(${ARGN})
endmacro()

macro(qt4_generate_dbus_interface)
  qt5_generate_dbus_interface(${ARGN})
endmacro()

macro(qt4_add_resources)
  qt5_add_resources(${ARGN})
endmacro()
