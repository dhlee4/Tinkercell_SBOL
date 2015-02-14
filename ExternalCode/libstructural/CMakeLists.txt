<<<<<<< HEAD
INCLUDE_DIRECTORIES( BEFORE
	${TINKERCELL_SOURCE_DIR}/ExternalCode/libstructural
	${TINKERCELL_SOURCE_DIR}/ExternalCode/libstructural/include
	${TINKERCELL_SOURCE_DIR}/ExternalCode/copasi
)

FILE(GLOB LIBSTRUCT_SRC
    LibLA/*.cpp
    LibStructural/libstructural.cpp
)
  
ADD_DEFINITIONS(-DNO_SBML)
IF (WIN32)
	ADD_DEFINITIONS(-DWIN32)
	ADD_DEFINITIONS(-DLIB_EXPORTS)
ENDIF(WIN32)
#ADD_DEFINITIONS(-D__cplusplus)
#ADD_DEFINITIONS(-DNO_BLAS_WRAP)

ADD_LIBRARY ( structural
#   SHARED
  STATIC
  ${LIBSTRUCT_SRC}
)

TARGET_LINK_LIBRARIES( structural
  structural
  lapack
  blas
  f2c
)

=======
INCLUDE_DIRECTORIES( BEFORE
	${TINKERCELL_SOURCE_DIR}/ExternalCode/libstructural
	${TINKERCELL_SOURCE_DIR}/ExternalCode/libstructural/include
	${TINKERCELL_SOURCE_DIR}/ExternalCode/copasi
)

FILE(GLOB LIBSTRUCT_SRC
    LibLA/*.cpp
    LibStructural/libstructural.cpp
)
  
ADD_DEFINITIONS(-DNO_SBML)
IF (WIN32)
	ADD_DEFINITIONS(-DWIN32)
	ADD_DEFINITIONS(-DLIB_EXPORTS)
ENDIF(WIN32)
#ADD_DEFINITIONS(-D__cplusplus)
#ADD_DEFINITIONS(-DNO_BLAS_WRAP)

ADD_LIBRARY ( structural
#   SHARED
  STATIC
  ${LIBSTRUCT_SRC}
)

TARGET_LINK_LIBRARIES( structural
  structural
  lapack
  blas
  f2c
)

>>>>>>> 1905cfc9e294ef1fd9bc7c874b4a4e2af0fff3ea
