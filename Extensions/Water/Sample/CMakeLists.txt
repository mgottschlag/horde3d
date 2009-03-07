
include_directories(../Bindings/C++ ../../../Horde3D/Bindings/C++ ../../../Horde3D/Dependencies/Include)

IF(MSVC)
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ENTRY:mainCRTStartup ")
ENDIF(MSVC)

add_executable(Water WIN32 MACOSX_BUNDLE
	app.h
	app.cpp
	main.cpp
	)

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	FIND_LIBRARY(CARBON_LIBRARY Carbon)
	FIND_LIBRARY(APPLICATION_SERVICES_LIBRARY ApplicationServices)
	FIND_LIBRARY(AGL_LIBRARY AGL)
	target_link_libraries(Water ${CARBON_LIBRARY} ${APPLICATION_SERVICES_LIBRARY} ${AGL_LIBRARY})
endif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")


target_link_libraries(Water Horde3D Horde3DUtils glfw)

