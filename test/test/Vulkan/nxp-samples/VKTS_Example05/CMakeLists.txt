#
# VKTS Example CMake file.
#

cmake_minimum_required(VERSION 3.2)

set (VKTS_Example "VKTS_Example05")

project (${VKTS_Example})

set(VKTS_WSI "" CACHE STRING "Enter VKTS_DISPLAY_VISUAL for alternative WSI.")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../VKTS_External/include ${CMAKE_CURRENT_SOURCE_DIR}/../VKTS/include)


if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm")

	set(VKTS_ARCHITECTURE "arm")
	
else ()

	set(VKTS_ARCHITECTURE "intel")
	
endif ()

if (CMAKE_SIZEOF_VOID_P MATCHES 8)

	set(VKTS_BITS "64")
	
else ()

	set(VKTS_BITS "32")
	
endif ()


if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")

	set(VKTS_OS "Windows")
	
	set(VKTS_WINDOW "Win32")

    if (${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
		
		set(VKTS_COMPILER "MSVC")

		set(VKTS_LIB ${VKTS_COMPILER}/lib)

        add_definitions(-D_CRT_SECURE_NO_WARNINGS)
		
	else ()
        
		set(VKTS_COMPILER "GNU")
		
		set(VKTS_LIB "lib")
		
    endif ()        

	set(VKTS_ADDITIONAL_LIBS vulkan-1 WinMM Xinput9_1_0 Pdh Psapi)

    find_path(Vulkan_INCLUDE_DIR NAMES vulkan/vulkan.h PATHS "$ENV{VULKAN_SDK}/Include")
    include_directories(AFTER ${Vulkan_INCLUDE_DIR})
	
    if (${VKTS_BITS} MATCHES "64")
    
        find_path(Vulkan_LIBRARY_DIR NAMES vulkan-1.lib PATHS "$ENV{VULKAN_SDK}/Bin")
       
    else ()
    
        find_path(Vulkan_LIBRARY_DIR NAMES vulkan-1.lib PATHS "$ENV{VULKAN_SDK}/Bin32")
            
    endif ()
    
    link_directories(${Vulkan_LIBRARY_DIR})
    
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

	set(VKTS_OS "Linux")
	
	set(VKTS_COMPILER "GNU")

	set(VKTS_LIB "lib")

	if (${VKTS_WSI} MATCHES "VKTS_DISPLAY_VISUAL")
		add_definitions(-DVKTS_DISPLAY_VISUAL)

		set(VKTS_WINDOW "Display")

		set(VKTS_ADDITIONAL_LIBS vulkan pthread)
	else ()
		set(VKTS_WINDOW "Xlib")
		
		set(VKTS_ADDITIONAL_LIBS vulkan pthread xcb X11-xcb Xrandr X11)
	endif ()

endif ()

link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../VKTS/${VKTS_LIB})

file(GLOB_RECURSE CPP_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_executable(${VKTS_Example} ${CPP_FILES})

set_property(TARGET ${VKTS_Example} PROPERTY RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../VKTS_Binaries)
set_property(TARGET ${VKTS_Example} PROPERTY RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/../VKTS_Binaries)
set_property(TARGET ${VKTS_Example} PROPERTY RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/../VKTS_Binaries)
set_property(TARGET ${VKTS_Example} PROPERTY RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_CURRENT_SOURCE_DIR}/../VKTS_Binaries)
set_property(TARGET ${VKTS_Example} PROPERTY RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_CURRENT_SOURCE_DIR}/../VKTS_Binaries)

set_property(TARGET ${VKTS_Example} PROPERTY CXX_STANDARD 11)
set_property(TARGET ${VKTS_Example} PROPERTY CXX_STANDARD_REQUIRED ON)

target_link_libraries(${VKTS_Example} VKTS ${VKTS_ADDITIONAL_LIBS})
