SET(LIBSBOL_DIR ${TINKERCELL_SOURCE_DIR}/ldh_work/test_1/libSBOLc)
SET( LIBSBOL_INCLUDE_DIR ${LIBSBOL_DIR}/headers)

#ADD_SUBDIRECTORY(libSBOLc)

INCLUDE_DIRECTORIES( ${LIBSBOL_INCLUDE_DIR})

LINK_DIRECTORIES(${LIBSBOL_DIR})

FIND_LIBRARY(sbol
    NAME libsbol.dll.a
    PATHS ${LIBSBOL_DIR}
    )

Message(sbol)

#do for test_1
ADD_EXECUTABLE( test_1 test_1.cpp )

TARGET_LINK_LIBRARIES( test_1
#                TinkerCellCore
                sbol
                )

