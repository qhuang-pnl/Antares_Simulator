
OMESSAGE("antares-study-finder")




# Le main
set(SRCS 	main.cpp)

if(WIN32)
	FILE(REMOVE "${CMAKE_CURRENT_SOURCE_DIR}/win32/finder.o")
	CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/win32/finder.rc.cmake"
		"${CMAKE_CURRENT_SOURCE_DIR}/win32/finder.rc")
	SET(SRCS ${SRCS} "win32/finder.rc")
endif(WIN32)


set(execname "antares-${ANTARES_PRG_VERSION}-study-finder")
add_executable(${execname}  ${SRCS})

set(FINDER_LIBS
	libantares-core
	yuni-static-core
	${CMAKE_THREADS_LIBS_INIT}
)

if (WIN32 OR WIN64)
	target_link_libraries(${execname} ${FINDER_LIBS})
else()
	target_link_libraries(${execname} -Wl,--start-group ${FINDER_LIBS} -Wl,--end-group)
endif()


import_std_libs(${execname})
executable_strip(${execname})

