QT      += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

QT += multimedia multimediawidgets

CONFIG += c++11 no-reorder

QMAKE_CXXFLAGS += -Wno-reorder -Wno-switch -Wno-unused-parameter -Wno-unused-but-set-variable -Wno-unknown-pragmas

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    BKView.cpp \
    BKVKBDView.cpp \
    DisasmView.cpp \
    RegDumpViewCPU.cpp \
    MainWindow.cpp \
    MainWindowMenu.cpp \
    main.cpp

SOURCES += $$files(devemu/*.cpp, true)  # with recursion
SOURCES += $$files(helpers/*.cpp, true)
SOURCES += $$files(emutools/*.cpp, true)
SOURCES += $$files(shared/*.cpp, true)

HEADERS += \
    BKView.h \
    BKVKBDView.h \
    DisasmView.h \
    RegDumpViewCPU.h \
    MainWindow.h \

HEADERS += $$files(devemu/*.h, true)
HEADERS += $$files(helpers/*.h, true)
HEADERS += $$files(emutools/*.h, true)
HEADERS += $$files(shared/*.h, true)

INCLUDEPATH += devemu helpers shared emutools

FORMS += \
    mainwindow.ui

TRANSLATIONS += \
    bkemu-qt_en_US.ts

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc
