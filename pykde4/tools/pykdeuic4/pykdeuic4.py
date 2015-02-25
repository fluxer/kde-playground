#!/usr/bin/env python
#
# Copyright (C) 2007-9 Simon Edwards <simon@simonzone.com>
# Copyright (C) 2011 Luca Beltrame <einar@heavensinferno.net>
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Library General Public
# License as published by the Free Software Foundation; either
# version 2 of the License or (at your option) version 2.1 or
# version 3 or, at the discretion of KDE e.V. (which shall act as
# a proxy as in section 14 of the GPLv3), any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.
#
# You should have received a copy of the GNU Library General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston MA  02110-1301 USA

import sys
import time
import optparse

from PyQt4 import QtCore
from PyQt4.uic.Compiler import indenter, compiler
from PyQt4.uic.Compiler import qtproxies
from PyQt4.uic.objcreator import MATCH,NO_MATCH


HEADER = """#!/usr/bin/env python
# coding=UTF-8
#
# Generated by pykdeuic4 from %s on %s
#
# WARNING! All changes to this file will be lost.
from PyKDE4 import kdecore
from PyKDE4 import kdeui
"""

DISPLAY_CODE = """
if __name__ == '__main__':
    import sys
    global app
    class MainWin(kdeui.KMainWindow, %s):
        def __init__ (self, *args):
            kdeui.KMainWindow.__init__ (self)
            rootWidget = QtGui.QWidget(self)
            self.setupUi(rootWidget)
            self.resize(640, 480)
            self.setCentralWidget(rootWidget)

    appName     = "default"
    catalog     = ""
    programName = kdecore.ki18n("default")
    version     = "1.0"
    description = kdecore.ki18n("Default Example")
    license     = kdecore.KAboutData.License_GPL
    copyright   = kdecore.ki18n("unknown")
    text        = kdecore.ki18n("none")
    homePage    = ""
    bugEmail    = "email"

    aboutData   = kdecore.KAboutData(appName, catalog, programName, version, description,
                              license, copyright, text, homePage, bugEmail)
    kdecore.KCmdLineArgs.init(sys.argv, aboutData)

    app = kdeui.KApplication()
    mainWindow = MainWin(None, "main window")
    mainWindow.show()
    app.lastWindowClosed.connect(app.quit)
    app.exec_ ()
"""

# Override how messages are translated.
original_i18n_string = qtproxies.i18n_string

class kde_i18n_string(qtproxies.i18n_string):

    "Wrapper around qtproxies.i18n_string to add support for kdecore's i18n."

    def __init__(self, string, disambig=None):
        original_i18n_string.__init__(self,string, disambig)

    def __str__(self):
        return "kdecore.i18n(%s)" % (qtproxies.as_string(self.string),)

qtproxies.i18n_string = kde_i18n_string


def processUI(uifile, output_filename=None, exe=False, indent=4):

    """Compile and process the UI file."""

    if output_filename is not None:
        output = open(output_filename,'w')
    else:
        output = sys.stdout

    # Write out the header.
    output.write(HEADER % (uifile, time.ctime()))
    indenter.indentwidth = indent
    comp = compiler.UICompiler()
    pyqt_version_tuple = tuple(map(int, QtCore.PYQT_VERSION_STR.split(".")))
    # the method signature for compileUI changed in 4.10.0
    if pyqt_version_tuple < (4,10,0):
        winfo = comp.compileUi(uifile, output, None)
    else:
        winfo = comp.compileUi(uifile, output, None, "")

    if exe:
        output.write(DISPLAY_CODE % winfo["uiclass"])

    if output_filename is not None:
        output.close()


def main():

    usage = "pykdeuic4 [-h] [-e] [-o output_file] ui_file"

    parser = optparse.OptionParser(usage=usage)
    parser.add_option("-e", action="store_true", dest="exe",
                      help="Generate extra code to display the UI",
                      default=False)
    parser.add_option("-o", dest="output_filename",
                      metavar="file",
                      help="Write the output to file instead of stdout",
                      default=None)

    options, arguments = parser.parse_args()

    if len(arguments) != 1:
        parser.error("Wrong number of arguments.")

    source_ui = arguments[0]
    exe = options.exe
    output_filename = options.output_filename

    processUI(source_ui, output_filename, exe)


if __name__ == '__main__':
    main()
