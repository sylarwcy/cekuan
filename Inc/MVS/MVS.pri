HEADERS += \
    $$PWD/includes/MvCameraControl.h \
    $$PWD/includes/MvCamera.h \
    $$PWD/includes/myCameraGigE.h

SOURCES += $$PWD/includes/MvCamera.cpp \
    $$PWD/includes/myCameraGigE.cpp \
    $$PWD/includes/myCameraProcess.cpp

LIBS += -L$$PWD/Libraries -lMvCameraControl
