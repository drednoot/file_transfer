QT+=core gui network
greaterThan(QT_MAJOR_VERSION, 4): QT+=widgets

CONFIG+=c++17

SOURCES+=                                   \
				client/main.cc                      \
				client/client.cc                    \

HEADERS+=                                   \
				client/client.h                     \

DESTDIR     = build/client
MOC_DIR		  = build/client/moc
OBJECTS_DIR = build/client/obj


qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS+=target
