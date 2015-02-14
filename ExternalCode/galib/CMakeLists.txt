<<<<<<< HEAD
#---------------------------------------------------------------------------
# GAlib from lancet.mit.edu/ga/
#---------------------------------------------------------------------------

INCLUDE_DIRECTORIES( 
	BEFORE
	${TINKERCELL_SOURCE_DIR}/ExternalCode/galib/
	${TINKERCELL_SOURCE_DIR}/ExternalCode/galib/ga
)

FILE(GLOB GALIB_FILES ga/*.cpp)

ADD_LIBRARY( galib
  STATIC
  ${GALIB_FILES}
)
=======
#---------------------------------------------------------------------------
# GAlib from lancet.mit.edu/ga/
#---------------------------------------------------------------------------

INCLUDE_DIRECTORIES( 
	BEFORE
	${TINKERCELL_SOURCE_DIR}/ExternalCode/galib/
	${TINKERCELL_SOURCE_DIR}/ExternalCode/galib/ga
)

FILE(GLOB GALIB_FILES ga/*.cpp)

ADD_LIBRARY( galib
  STATIC
  ${GALIB_FILES}
)
>>>>>>> 1905cfc9e294ef1fd9bc7c874b4a4e2af0fff3ea
