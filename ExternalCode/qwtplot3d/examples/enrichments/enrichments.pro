<<<<<<< HEAD
include( ../common.pro )

SOURCES =  src/main.cpp \
		       src/enrichmentmainwindow.cpp \
		       src/enrichments.cpp 

HEADERS =  src/enrichmentmainwindow.h \
					 src/enrichments.h 
	 
!isEmpty( ISQT4 ) {
  FORMS =  src/enrichmentmainwindowbase4.ui
}
isEmpty( ISQT4 ) {
  FORMS =  src/enrichmentmainwindowbase.ui
}

=======
include( ../common.pro )

SOURCES =  src/main.cpp \
		       src/enrichmentmainwindow.cpp \
		       src/enrichments.cpp 

HEADERS =  src/enrichmentmainwindow.h \
					 src/enrichments.h 
	 
!isEmpty( ISQT4 ) {
  FORMS =  src/enrichmentmainwindowbase4.ui
}
isEmpty( ISQT4 ) {
  FORMS =  src/enrichmentmainwindowbase.ui
}

>>>>>>> 1905cfc9e294ef1fd9bc7c874b4a4e2af0fff3ea
