set(GLFW_SOURCES
	lib/enable.c
	lib/fullscreen.c
	lib/glext.c
	lib/image.c
	lib/init.c
	lib/input.c
	lib/joystick.c
	lib/stream.c
	lib/tga.c
	lib/thread.c
	lib/time.c
	lib/window.c
	)


if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	include_directories(lib lib/win32)
	set(GLFW_SOURCES
		${GLFW_SOURCES}
		lib/win32/win32_dllmain.c
		lib/win32/win32_enable.c
		lib/win32/win32_fullscreen.c
		lib/win32/win32_glext.c
		lib/win32/win32_init.c
		lib/win32/win32_joystick.c
		lib/win32/win32_thread.c
		lib/win32/win32_time.c
		lib/win32/win32_window.c
		)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")


if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	include_directories(lib lib/x11)
	set(GLFW_SOURCES
		${GLFW_SOURCES}
		lib/x11/x11_enable.c
		lib/x11/x11_fullscreen.c
		lib/x11/x11_glext.c
		lib/x11/x11_init.c
		lib/x11/x11_joystick.c
		lib/x11/x11_keysym2unicode.c
		lib/x11/x11_thread.c
		lib/x11/x11_time.c
		lib/x11/x11_window.c
		)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")



if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	include_directories(lib lib/macosx)
	set(GLFW_SOURCES
		${GLFW_SOURCES}
		lib/macosx/macosx_enable.c
		lib/macosx/macosx_fullscreen.c
		lib/macosx/macosx_glext.c
		lib/macosx/macosx_init.c
		lib/macosx/macosx_joystick.c
		lib/macosx/macosx_thread.c
		lib/macosx/macosx_time.c
		lib/macosx/macosx_window.c
		)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")


add_library(glfw STATIC
	${GLFW_SOURCES}
	)

