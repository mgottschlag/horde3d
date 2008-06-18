
include_directories(../../Bindings/C++)
include_directories(../../Dependencies/Include)

IF(WIN32)
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ENTRY:mainCRTStartup ")
ENDIF(WIN32)

add_executable(Knight WIN32
	app.h
	app.cpp
	main.cpp
	)


target_link_libraries(Knight Horde3D Horde3DUtils glfw)


if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
       FIND_LIBRARY(CARBON_LIBRARY Carbon)
       FIND_LIBRARY(APPLICATION_SERVICES_LIBRARY ApplicationServices)
       FIND_LIBRARY(AGL_LIBRARY AGL)
       target_link_libraries(Knight ${CARBON_LIBRARY} ${APPLICATION_SERVICES_LIBRARY} ${AGL_LIBRARY})
endif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
