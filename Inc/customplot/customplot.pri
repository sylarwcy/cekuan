lessThan(QT_MAJOR_VERSION, 5): QT += script

HEADERS += \
    $$PWD/qcustomplot.h \
    $$PWD/putText.h

SOURCES += \
    $$PWD/qcustomplot.cpp \
    $$PWD/putText.cpp

LIBS += -L$$PWD -lgdi32
