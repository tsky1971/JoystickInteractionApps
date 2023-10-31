cmake_minimum_required(VERSION 3.8)

project(JoystickSDL)

#######################################
# 
#######################################
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#######################################
# cmake find
#######################################
if(MSVC)
	#set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${project_SOURCE_DIR}/cmake")
	set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}../../../cmake)
elseif()
	set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "/usr/local/lib/cmake")	
endif()


#######################################
# defines
#######################################
#add_definitions(-DPLATFORM_RASPIAN=1)
#add_definitions(-D_PLATFORM_LINUX=0)
#add_definitions(-DSDL_STATIC=1)


#######################################
# 
#######################################
if(NOT APPLE)
	# check 64 bit
	if( CMAKE_SIZEOF_VOID_P MATCHES "4" )
		set( HAVE_64_BIT 0 )
	else(  CMAKE_SIZEOF_VOID_P MATCHES "4")
		set( HAVE_64_BIT 1 )
	endif(  CMAKE_SIZEOF_VOID_P MATCHES "4")
endif( NOT APPLE )


#######################################
# cmake settings
#######################################

set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}../../../bin)

#set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}../../../bin)

#set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})

#set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

#######################################
# libs
#######################################
set(OPEN_GL_FILES 	
	glu32
	opengl32
	)

#find_package(glad CONFIG REQUIRED)
#find_package(GLM REQUIRED)
#find_package(GLEW REQUIRED)
#find_package(GLUT REQUIRED)
#find_package(glfw3 CONFIG REQUIRED)
#find_package(imgui CONFIG REQUIRED)
#find_package(assimp CONFIG REQUIRED)
find_package(ZeroMQ CONFIG REQUIRED)
find_package(czmq CONFIG REQUIRED)
#find_package(cppzmq)

#find_package(SDL2 CONFIG REQUIRED)
find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3)
#find_package(SDL2_net CONFIG REQUIRED)

include_directories(${SDL2_INCLUDE_DIRS})

add_definitions(-DGLM_ENABLE_EXPERIMENTAL)

#######################################
# 
#######################################

include_directories("${CMAKE_SOURCE_DIR}../../../include")

include_directories("${CMAKE_SOURCE_DIR}../../Common")

set(HEADER_FILES 
	../Common/UDPsocket.h
	../Common/imgui/imconfig.h
	../Common/imgui/imgui.h
	../Common/imgui/imgui_internal.h
	../Common/imgui/imstb_rectpack.h
	../Common/imgui/imstb_textedit.h
	../Common/imgui/imstb_truetype.h
	../Common/imgui/imgui_impl_sdl3.h
	../Common/imgui/imgui_impl_sdlrenderer3.h
	)    
    
	
set(SOURCE_FILES
	src/main.cpp
	../Common/imgui/imgui.cpp
	../Common/imgui/imgui_demo.cpp
	../Common/imgui/imgui_draw.cpp
	../Common/imgui/imgui_widgets.cpp
	../Common/imgui/imgui_tables.cpp
	../Common/imgui/imgui_impl_sdl3.cpp
	../Common/imgui/imgui_impl_sdlrenderer3.cpp
	)

#######################################
# 
#######################################

	
add_executable(
    ${PROJECT_NAME}
    ${HEADER_FILES}
    ${SOURCE_FILES}
    )
	
#######################################
# 
#######################################

if(MSVC)

target_link_libraries(
	${PROJECT_NAME}		
	PRIVATE SDL3::SDL3
	#	$<TARGET_NAME_IF_EXISTS:SDL2::SDL2main> 
	#	$<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>	
	#PRIVATE 
	#	$<IF:$<TARGET_EXISTS:SDL2_net::SDL2_net>,SDL2_net::SDL2_net,SDL2_net::SDL2_net-static>
	#PRIVATE imgui::imgui
	#PRIVATE glad::glad
	#PRIVATE GLEW::GLEW
	#PRIVATE GLUT::GLUT	
    PRIVATE czmq czmq-static
	PRIVATE libzmq libzmq-static
	#${WIN32_OPEN_GL_FILES}
#	cppzmq
	)
	
else()

target_link_libraries(
	${PROJECT_NAME}
        PRIVATE SDL2
        PRIVATE GL
	PRIVATE GLUT::GLUT
	PRIVATE pthread
        PRIVATE libzmq libzmq-static
	)
	
endif()


#######################################
#
#######################################

install(
	TARGETS ${PROJECT_NAME} 
	CONFIGURATIONS Release
	DESTINATION ${CMAKE_BINARY_DIR}
)

install(
	TARGETS ${PROJECT_NAME} 
	CONFIGURATIONS Debug
	DESTINATION ${CMAKE_BINARY_DIR}/Debug
)

####################################### 

#######################################
#
#######################################

function(my_install_dll)
	# Do something.
	message("Copy DLL...")

	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
			"${CMAKE_TOOLCHAIN_FILE}/../../../installed/x64-windows/bin/glew32.dll"
			${CMAKE_BINARY_DIR})
			
	message("Copy DLL END")
endfunction()

function(my_install_dll2)  
  message("This is my function.")

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD						# Adds a post-build event to MyTest
	COMMAND ${CMAKE_COMMAND} -E copy_if_different							# which executes "cmake - E copy_if_different..."
		"../../../vcpkg/installed/x64-windows/bin/glfw3.dll"				# <--this is in-file
		"../../../bin")														# <--this is out-file path

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD						# Adds a post-build event to MyTest
	COMMAND ${CMAKE_COMMAND} -E copy_if_different							# which executes "cmake - E copy_if_different..."
		"../../../vcpkg/installed/x64-windows/bin/assimp-vc142-mt.dll"		# <--this is in-file
		"../../../bin")														# <--this is out-file path

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD						# Adds a post-build event to MyTest
	COMMAND ${CMAKE_COMMAND} -E copy_if_different							# which executes "cmake - E copy_if_different..."
		"../../../vcpkg/installed/x64-windows/bin/zlib1.dll"				# <--this is in-file
		"../../../bin")														# <--this is out-file path

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD						# Adds a post-build event to MyTest
	COMMAND ${CMAKE_COMMAND} -E copy_if_different							# which executes "cmake - E copy_if_different..."
		"../../../vcpkg/installed/x64-windows/bin/jpeg62.dll"				# <--this is in-file
		"../../../bin")														# <--this is out-file path

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD						# Adds a post-build event to MyTest
	COMMAND ${CMAKE_COMMAND} -E copy_if_different							# which executes "cmake - E copy_if_different..."
		"../../../vcpkg/installed/x64-windows/bin/libpng16.dll"				# <--this is in-file
		"../../../bin")														# <--this is out-file path


add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD						# Adds a post-build event to MyTest
	COMMAND ${CMAKE_COMMAND} -E copy_if_different							# which executes "cmake - E copy_if_different..."
		"../../../vcpkg/installed/x64-windows/bin/bz2.dll"					# <--this is in-file
		"../../../bin")														# <--this is out-file path
		

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
		"../../../vcpkg/installed/x64-windows/bin/SDL2*.dll"
		"../../../bin")

endfunction()

# Call the function.
my_install_dll()

## set working directory to bin directory
set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "../../../bin")

## set the startup project to the current project
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

install(TARGETS ${PROJECT_NAME} DESTINATION bin)