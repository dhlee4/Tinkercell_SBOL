<<<<<<< HEAD
include( ../common.pro )

SOURCES = src/main.cpp \
		src/mesh2mainwindow.cpp \
		src/colormapreader.cpp \
		src/lightingdlg.cpp \
		src/mesh.cpp

HEADERS =  src/mesh2mainwindow.h \
		src/functions.h \
		src/colormapreader.h \
		src/lightingdlg.h \
		src/femreader.h

!isEmpty( ISQT4 ) {
SOURCES += src/designerworkaround.cpp
HEADERS += src/designerworkaround.h 
FORMS =  src/mesh2mainwindowbase4.ui \
         src/lightingdlgbase4.ui
}
isEmpty( ISQT4 ) {
FORMS =  src/mesh2mainwindowbase.ui \
         src/lightingdlgbase.ui
}

=======
include( ../common.pro )

SOURCES = src/main.cpp \
		src/mesh2mainwindow.cpp \
		src/colormapreader.cpp \
		src/lightingdlg.cpp \
		src/mesh.cpp

HEADERS =  src/mesh2mainwindow.h \
		src/functions.h \
		src/colormapreader.h \
		src/lightingdlg.h \
		src/femreader.h

!isEmpty( ISQT4 ) {
SOURCES += src/designerworkaround.cpp
HEADERS += src/designerworkaround.h 
FORMS =  src/mesh2mainwindowbase4.ui \
         src/lightingdlgbase4.ui
}
isEmpty( ISQT4 ) {
FORMS =  src/mesh2mainwindowbase.ui \
         src/lightingdlgbase.ui
}

>>>>>>> 1905cfc9e294ef1fd9bc7c874b4a4e2af0fff3ea
