QT+=core gui network
greaterThan(QT_MAJOR_VERSION, 4): QT+=widgets

CONFIG+=c++17

SOURCES+=                \
				server/main.cc   \
				server/server.cc \
				server/dowload_thread.cc \

HEADERS+=                \
				server/server.h  \
				server/download_thread.h \

DESTDIR     = build/server
MOC_DIR		  = build/server/moc
OBJECTS_DIR = build/server/obj


qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS+=target
