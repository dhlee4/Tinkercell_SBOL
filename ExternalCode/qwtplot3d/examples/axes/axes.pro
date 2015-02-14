<<<<<<< HEAD
include( ../common.pro )

SOURCES =  src/main.cpp \
		       src/axesmainwindow.cpp 

HEADERS =  src/axesmainwindow.h \
           src/axes.h

!isEmpty( ISQT4 ) {
  FORMS =  src/axesmainwindowbase4.ui
}
isEmpty( ISQT4 ) {
  FORMS =  src/axesmainwindowbase.ui
}

=======
include( ../common.pro )

SOURCES =  src/main.cpp \
		       src/axesmainwindow.cpp 

HEADERS =  src/axesmainwindow.h \
           src/axes.h

!isEmpty( ISQT4 ) {
  FORMS =  src/axesmainwindowbase4.ui
}
isEmpty( ISQT4 ) {
  FORMS =  src/axesmainwindowbase.ui
}

>>>>>>> 1905cfc9e294ef1fd9bc7c874b4a4e2af0fff3ea
