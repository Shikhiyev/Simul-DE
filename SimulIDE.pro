 ###########################################################################
 #   Copyright (C) 2012   by Santiago González                             #
 #   santigoro@gmail.com                                                   #
 #                                                                         #
 #   This program is free software; you can redistribute it and/or modify  #
 #   it under the terms of the GNU General Public License as published by  #
 #   the Free Software Foundation; either version 3 of the License, or     #
 #   (at your option) any later version.                                   #
 #                                                                         #
 #   This program is distributed in the hope that it will be useful,       #
 #   but WITHOUT ANY WARRANTY; without even the implied warranty of        #
 #   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         #
 #   GNU General Public License for more details.                          #
 #                                                                         #
 #   You should have received a copy of the GNU General Public License     #
 #   along with this program; if not, see <http://www.gnu.org/licenses/>.  #
 #                                                                         #
 ###########################################################################
 
VERSION     = "0.4.13"
RELEASE = "-SR5-CE"

TEMPLATE = app

TARGET = simulide

QT += core gui multimedia widgets serialport concurrent xml svg

win32 {
    OS = Windows
    LIBS += -lws2_32
    
    # Path for local libelf if user followed setup script
    exists($$PWD/3rdparty/libelf): {
        INCLUDEPATH += $$PWD/3rdparty/libelf/include
        LIBS += -L$$PWD/3rdparty/libelf/lib -lelf
    }

    # Check for MSVC vs GCC
    win32-msvc* {
        QMAKE_CXXFLAGS += /std:c++14
        QMAKE_CFLAGS += /std:c11
        DEFINES += _CRT_SECURE_NO_WARNINGS
    }
}

linux {
    OS = Linux
    QMAKE_LFLAGS += -no-pie
}

macx {
    OS = MacOS
    INCLUDEPATH += \
        /usr/local/Cellar/libelf/0.8.13_1/include \
        /usr/local/Cellar/libelf/0.8.13_1/include/libelf
    
    LIBS += /usr/local/lib/libelf.a
    QMAKE_LFLAGS += -no-pie
}

include(./SimulIDE.pri)

!win32-msvc* {
    QMAKE_CXXFLAGS += -include cstdint
    QMAKE_CXXFLAGS += -Wno-unused-parameter
    QMAKE_CXXFLAGS += -Wno-missing-field-initializers
    QMAKE_CXXFLAGS += -Wno-implicit-fallthrough
    QMAKE_CXXFLAGS -= -fPIC
    QMAKE_CXXFLAGS += -fno-pic

    QMAKE_CFLAGS += --std=gnu11
    QMAKE_CFLAGS += -Wno-unused-result
    QMAKE_CFLAGS += -Wno-unused-parameter
    QMAKE_CFLAGS += -Wno-missing-field-initializers
    QMAKE_CFLAGS += -Wno-implicit-function-declaration
    QMAKE_CFLAGS += -Wno-implicit-fallthrough
    QMAKE_CFLAGS += -Wno-int-conversion
    QMAKE_CFLAGS += -Wno-sign-compare
    QMAKE_CFLAGS += -O2
    QMAKE_CFLAGS -= -fPIC
    QMAKE_CFLAGS += -fno-pic
}

QMAKE_LIBS += -lelf

CONFIG += qt 
CONFIG += warn_on
CONFIG += no_qml_debug
CONFIG *= c++11

DEFINES += MAINMODULE_EXPORT=
DEFINES += APP_VERSION=\\\"$$VERSION$$RELEASE\\\"

TARGET_NAME = SimulIDE_$$VERSION$$RELEASE$$

CONFIG( release, debug|release ) {
        TARGET_PREFIX = $$OUT_PWD/release/$$TARGET_NAME
        _OBJECTS_DIR  = $$OUT_PWD/release
}

CONFIG( debug, debug|release ) {
        TARGET_PREFIX = $$OUT_PWD/debug/$$TARGET_NAME
        _OBJECTS_DIR  = $$OUT_PWD/debug
}

OBJECTS_DIR *= $$_OBJECTS_DIR
MOC_DIR     *= $$OUT_PWD/build
INCLUDEPATH += $$MOC_DIR

# Translations
win32: LRELEASE_EXE = $$[QT_INSTALL_BINS]\lrelease.exe
else:  LRELEASE_EXE = lrelease

runLrelease.commands = $$LRELEASE_EXE $$PWD/resources/translations/simulide.ts \
    $$PWD/resources/translations/simulide_en.ts \
    $$PWD/resources/translations/simulide_es.ts \
    $$PWD/resources/translations/simulide_fr.ts \
    $$PWD/resources/translations/simulide_ru.ts \
    $$PWD/resources/translations/simulide_de.ts \
    $$PWD/resources/translations/simulide_pt_BR.ts

QMAKE_EXTRA_TARGETS += runLrelease
POST_TARGETDEPS     += runLrelease

RESOURCES = ./src/application.qrc

win32 | linux {
    DESTDIR = $$TARGET_PREFIX/bin
    
    # OS Specific commands
    win32 {
        SEP = &
        MKDIR_CMD = -mkdir
        P_WD = $$replace(PWD, /, \\)
        T_PREFIX = $$replace(TARGET_PREFIX, /, \\)
        
        copy2dest.commands = \
            $$MKDIR_CMD \"$$T_PREFIX\\share\\simulide\\data\" $$SEP \
            $$MKDIR_CMD \"$$T_PREFIX\\share\\simulide\\examples\" $$SEP \
            $$MKDIR_CMD \"$$T_PREFIX\\share\\simulide\\translations\" $$SEP \
            xcopy /S /Q /Y /I \"$$P_WD\\resources\\data\" \"$$T_PREFIX\\share\\simulide\\data\" $$SEP \
            xcopy /S /Q /Y /I \"$$P_WD\\resources\\examples\" \"$$T_PREFIX\\share\\simulide\\examples\" $$SEP \
            xcopy /S /Q /Y /I \"$$P_WD\\resources\\icons\" \"$$T_PREFIX\\share\\icons\" $$SEP \
            move /Y \"$$P_WD\\resources\\translations\\*.qm\" \"$$T_PREFIX\\share\\simulide\\translations\"
    } linux {
        SEP = ;
        MKDIR_CMD = $(MKDIR) -p
        copy2dest.commands = \
            $$MKDIR_CMD $$TARGET_PREFIX/share/simulide/data ; \
            $$MKDIR_CMD $$TARGET_PREFIX/share/simulide/examples ; \
            $$MKDIR_CMD $$TARGET_PREFIX/share/simulide/translations ; \
            $(COPY_DIR) $$PWD/resources/data              $$TARGET_PREFIX/share/simulide ; \
            $(COPY_DIR) $$PWD/resources/examples          $$TARGET_PREFIX/share/simulide ; \
            $(COPY_DIR) $$PWD/resources/icons             $$TARGET_PREFIX/share ; \
            $(MOVE)     $$PWD/resources/translations/*.qm $$TARGET_PREFIX/share/simulide/translations/ ;
    }
}


macx {
    DESTDIR = $$TARGET_PREFIX
    mkpath( $$TARGET_PREFIX/simulide.app )
    copy2dest.commands = \
        $(MKDIR)    $$TARGET_PREFIX/simulide.app/Contents/share/simulide/data ; \
        $(MKDIR)    $$TARGET_PREFIX/simulide.app/Contents/share/simulide/examples ; \
        $(MKDIR)    $$TARGET_PREFIX/simulide.app/Contents/share/simulide/translations ; \
        $(COPY_DIR) $$PWD/resources/data              $$TARGET_PREFIX/simulide.app/Contents/share/simulide ; \
        $(COPY_DIR) $$PWD/resources/examples          $$TARGET_PREFIX/simulide.app/Contents/share/simulide ; \
        $(COPY_DIR) $$PWD/resources/icons             $$TARGET_PREFIX/simulide.app/Contents/share ; \
        $(MOVE)     $$PWD/resources/translations/*.qm $$TARGET_PREFIX/simulide.app/Contents/share/simulide/translations ;
}

QMAKE_EXTRA_TARGETS += copy2dest
POST_TARGETDEPS     += copy2dest

message( "-----------------------------------")
message( "    "                               )
message( "    "$$TARGET_NAME for $$OS         )
message( "    "                               )
message( "      Qt version: "$$QT_VERSION     )
message( "    "                               )
message( "    Destination Folder:"            )
message( $$TARGET_PREFIX                      )
message( "-----------------------------------")
