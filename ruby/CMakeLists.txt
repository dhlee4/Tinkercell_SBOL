SET(LIBRARY_OUTPUT_PATH ${EXECUTABLE_OUTPUT_PATH}/ruby)
ADD_DEFINITIONS(-DTC_EXPORTS)

IF (NOT DEFINED RUBY_INCLUDE_PATH OR NOT DEFINED RUBY_LIBRARY)
	FIND_PACKAGE( Ruby )
ENDIF (NOT DEFINED RUBY_INCLUDE_PATH OR NOT DEFINED RUBY_LIBRARY)

IF (DEFINED RUBY_LIB)
	SET(RUBY_LIBRARY ${RUBY_LIB})
ENDIF (DEFINED RUBY_LIB)

IF (DEFINED RUBY_INCLUDE_PATH)
   SET(RUBY_INCLUDE_DIR ${RUBY_INCLUDE_PATH})
ENDIF (DEFINED RUBY_INCLUDE_PATH)

IF (DEFINED RUBY_INCLUDE_DIRS)
   SET(RUBY_INCLUDE_DIR ${RUBY_INCLUDE_DIRS})
ENDIF (DEFINED RUBY_INCLUDE_DIRS)

IF (NOT DEFINED RUBY_INCLUDE_PATH OR NOT DEFINED RUBY_LIBRARY)
	IF (NOT IS_DIRECTORY ${RUBY_INCLUDE_DIR})
		 MESSAGE(WARNING ": path RUBY_INCLUDE_PATH not found. Use Add Entry to add it")
	ENDIF (NOT IS_DIRECTORY ${RUBY_INCLUDE_DIR})
	IF (NOT DEFINED RUBY_LIBRARY)
		 MESSAGE(WARNING ": filepath RUBY_LIB not found. Use Add Entry to add it")
	ENDIF (NOT DEFINED RUBY_LIBRARY)
ELSE (NOT DEFINED RUBY_INCLUDE_PATH OR NOT DEFINED RUBY_LIBRARY)

MESSAGE(STATUS "Using Ruby library: ${RUBY_LIBRARY}")

#DBL_ definitions are missing in new MinGW -- why did they do that?
IF (WIN32 AND MINGW)
	ADD_DEFINITIONS(-DDBL_EPSILON=2.22045e-16)
	ADD_DEFINITIONS(-DDBL_MIN=1e-999)
	ADD_DEFINITIONS(-DDBL_MIN_EXP=-999)
	ADD_DEFINITIONS(-DDBL_MIN_10_EXP=-307)
	ADD_DEFINITIONS(-DDBL_MAX_10_EXP=+307)
	ADD_DEFINITIONS(-DDBL_MAX=1.79769e+308)
ENDIF (WIN32 AND MINGW)

IF ( WIN32 )
  ADD_DEFINITIONS(-D_WIN32)
  IF (MINGW)
	 ADD_DEFINITIONS(-D__MINGW32__)
  ENDIF(MINGW)
ENDIF( WIN32 )

INCLUDE_DIRECTORIES( BEFORE ${RUBY_INCLUDE_DIR} )
ADD_DEFINITIONS(-DLIB_EXPORTS=0)

ADD_LIBRARY( tcruby
  SHARED
  runruby.c
  tinkercell_wrap.c
)

TARGET_LINK_LIBRARIES( tcruby
tcruby
tinkercellapi
${RUBY_LIBRARY}
)

IF ( WIN32 )
 IF (MINGW)
	ADD_CUSTOM_COMMAND ( 
		TARGET tcruby POST_BUILD
		COMMAND move /Y ..\\bin\\ruby\\libtcruby.dll ..\\bin\\ruby\\tinkercell.dll
	)
 ELSE (MINGW)
	ADD_CUSTOM_COMMAND ( 
		TARGET tcruby POST_BUILD
		COMMAND move /Y ..\\bin\\ruby\\tcruby.dll ..\\bin\\ruby\\tinkercell.dll
	)
 ENDIF (MINGW)
ELSE ( WIN32 )
 IF( APPLE )
	ADD_CUSTOM_COMMAND ( 
		  TARGET tcruby POST_BUILD
		  COMMAND cp ${TINKERCELL_BINARY_DIR}/bin/ruby/libtcruby.dylib ${TINKERCELL_BINARY_DIR}/bin/ruby/tinkercell.so
		  COMMAND mv ${TINKERCELL_BINARY_DIR}/bin/ruby/libtcruby.dylib ${TINKERCELL_BINARY_DIR}/bin/ruby/tinkercell.dylib
	)
ELSE( APPLE )
	ADD_CUSTOM_COMMAND ( 
		  TARGET tcruby POST_BUILD 
		  COMMAND mv ${TINKERCELL_BINARY_DIR}/bin/ruby/libtcruby.so ${TINKERCELL_BINARY_DIR}/bin/ruby/tinkercell.so
	)
ENDIF( APPLE )
ENDIF( WIN32 )

FILE(GLOB RUBY_LOADER ${TINKERCELL_BINARY_DIR}/bin/ruby/tinkercell.*)
FILE(GLOB RUBY_SCRIPTS *.rb *.ruby)
INSTALL(FILES ${RUBY_SCRIPTS} DESTINATION ${TINKERCELL_PACKAGE_ROOT_DIR}/ruby )
INSTALL(FILES ${RUBY_LOADER} DESTINATION ${TINKERCELL_PACKAGE_ROOT_DIR}/ruby )

ENDIF (NOT DEFINED RUBY_INCLUDE_PATH OR NOT DEFINED RUBY_LIBRARY)

