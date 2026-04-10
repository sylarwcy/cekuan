lessThan(QT_MAJOR_VERSION, 5): QT += script

HEADERS += \
    $$PWD/dbcleanthread.h \
    $$PWD/dbconnthread.h \
    $$PWD/dbdelegate.h \
    $$PWD/dbhead.h \
    $$PWD/dbhelper.h \
    $$PWD/dbhttpthread.h \
    $$PWD/dbpage.h \
    $$PWD/dbpagemodel.h \
    $$PWD/navpage.h

SOURCES += \
    $$PWD/dbcleanthread.cpp \
    $$PWD/dbconnthread.cpp \
    $$PWD/dbdelegate.cpp \
    $$PWD/dbhelper.cpp \
    $$PWD/dbhttpthread.cpp \
    $$PWD/dbpage.cpp \
    $$PWD/dbpagemodel.cpp \
    $$PWD/navpage.cpp
