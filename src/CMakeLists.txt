if (CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|ARM64|arm64")
	set(NX_APP_TITLE "DoomIIRPG")
	set(NX_APP_AUTHOR "PG Team, Erick194")
	set(NX_APP_VERSION "0.2.2")
	set(PROJECT_TID "0x16092009efdd2000")
endif()
file(GLOB_RECURSE HEADER_FILES
	"*.hpp"
	"*.h")

file(GLOB_RECURSE SOURCE_FILES
	"*.cpp"
	"*.c")

file(GLOB_RECURSE RESOURCES_FILES
	"*.rc"
	"*.ico")

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX "Header Files" FILES ${HEADER_FILES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX "Source Files" FILES ${SOURCE_FILES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX "Resources Files" FILES ${RESOURCES_FILES})

add_executable(${PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES} ${RESOURCES_FILES})

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

target_link_libraries(${PROJECT_NAME}
		SDL2main
		SDL2
		picaGL
		ZLIB::ZLIB
		OpenAL::OpenAL
		ctru
		citro3d
	${HASH_LIBRARY_TGT_NAME})
add_custom_command(
		TARGET DoomIIRPG POST_BUILD
		COMMAND 3dsxtool
		"${CMAKE_BINARY_DIR}/src/${PROJECT_NAME}.elf"
		"${CMAKE_BINARY_DIR}/src/${PROJECT_NAME}.3dsx"
		#COMMAND nxlink -a 172.20.10.2 -p /switch/DoomIIRPG.nro -s /Users/five/CLionProjects/DoomIIRPG-RE-NSLite/build-nx/src/DoomIIRPG.nro
)
if (CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|ARM64|arm64")
	add_custom_command(
			TARGET DoomIIRPG POST_BUILD
			COMMAND nacptool --create "${NX_APP_TITLE}" "${NX_APP_AUTHOR}" "${NX_APP_VERSION}" "${PROJECT_NAME}.nacp" --titleid=${PROJECT_TID}
			COMMAND elf2nro
			"${CMAKE_BINARY_DIR}/src/${PROJECT_NAME}.elf"
			"${CMAKE_BINARY_DIR}/src/${PROJECT_NAME}.nro"
			--icon "${CMAKE_CURRENT_SOURCE_DIR}/icon.jpg"
			--nacp=${PROJECT_NAME}.nacp
			#COMMAND nxlink -a 172.20.10.2 -p /switch/DoomIIRPG.nro -s /Users/five/CLionProjects/DoomIIRPG-RE-NSLite/build-nx/src/DoomIIRPG.nro
	)
endif()