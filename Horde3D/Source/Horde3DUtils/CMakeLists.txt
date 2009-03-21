
include_directories(../Shared)
include_directories(../../Bindings/C++)

if(${CMAKE_BUILD_TYPE} MATCHES "Profile")
	add_library(Horde3DUtils STATIC
		main.cpp
		)
else(${CMAKE_BUILD_TYPE} MATCHES "Profile")
	add_library(Horde3DUtils SHARED
		main.cpp
		)
endif(${CMAKE_BUILD_TYPE} MATCHES "Profile")



target_link_libraries(Horde3DUtils Horde3D)

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
endif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	install(TARGETS Horde3DUtils
		RUNTIME DESTINATION bin
		LIBRARY DESTINATION lib
		ARCHIVE DESTINATION lib
		)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
endif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
