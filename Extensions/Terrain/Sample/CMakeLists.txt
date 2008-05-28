
include_directories(../Bindings/C++ ../../../Horde3D/Bindings/C++ ../../../Horde3D/Dependencies/glfw/include/GL)

IF(WIN32)
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ENTRY:mainCRTStartup ")
ENDIF(WIN32)

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	FIND_LIBRARY(CARBON_LIBRARY Carbon)
	FIND_LIBRARY(APPLICATION_SERVICES_LIBRARY ApplicationServices)
	FIND_LIBRARY(AGL_LIBRARY AGL)
	target_link_libraries(Terrain ${CARBON_LIBRARY} ${APPLICATION_SERVICES_LIBRARY} ${AGL_LIBRARY})
endif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

add_executable(Terrain WIN32
	app.h
	app.cpp
	main.cpp
	)


target_link_libraries(Terrain Horde3D Horde3DUtils glfw)

