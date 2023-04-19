debug_and_release {
    CONFIG -= debug_and_release
    CONFIG += debug_and_release
}

# ensure one "debug" or "release" in CONFIG so they can be used as
# conditionals instead of writing "CONFIG(debug, debug|release)"...
CONFIG(debug, debug|release) {
    CONFIG -= debug release
    CONFIG += debug
    }
CONFIG(release, debug|release) {
        CONFIG -= debug release
        CONFIG += release
}

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

DESTDIR_RELEASE= ./../../../build_osgeo4w/release
DESTDIR_DEBUG= ./../../../build_osgeo4w/debug
#QUAZIPLIB_PATH= ./../../../depends/libQuaZip-1.2

INCLUDEPATH += ../../libs/libParameters
INCLUDEPATH += ../../libs/libProcessTools
#INCLUDEPATH += C:\Qt\Qt5.15.2\5.15.2\msvc2019_64\include\QtZlib
#INCLUDEPATH += . $$QUAZIPLIB_PATH/include

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    PAFyCToolsDialog.cpp \
    main.cpp

HEADERS += \
    PAFyCToolsDialog.h

FORMS += \
    PAFyCToolsDialog.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc
