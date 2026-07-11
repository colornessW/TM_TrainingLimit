cmake_minimum_required(VERSION 3.21)

project(
	TM_TrainingLimit
	LANGUAGES CXX
	VERSION 1.0.0
)

set(ENABLE_SKYRIM_SE ON CACHE BOOL "Enable Skyrim SE support")
set(ENABLE_SKYRIM_AE ON CACHE BOOL "Enable Skyrim AE support")
set(ENABLE_SKYRIM_VR ON CACHE BOOL "Enable Skyrim VR support")
set(BUILD_TESTS OFF CACHE BOOL "Disable CommonLibSSE-NG tests" FORCE)
set(SKSE_SUPPORT_XBYAK OFF CACHE BOOL "Disable Xbyak" FORCE)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

include(FetchContent)
FetchContent_Declare(spdlog
	GIT_REPOSITORY https://github.com/gabime/spdlog.git
	GIT_TAG v1.14.1
	GIT_SHALLOW TRUE
)
set(SPDLOG_FMT_EXTERNAL OFF CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_SHARED OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(spdlog)

if(NOT TARGET fmt::fmt)
	add_library(spdlog_fmt INTERFACE)
	target_include_directories(spdlog_fmt INTERFACE "${spdlog_SOURCE_DIR}/include")
	add_library(fmt::fmt ALIAS spdlog_fmt)
endif()
set(fmt_INCLUDE_DIRS "" CACHE PATH "" FORCE)
set(spdlog_INCLUDE_DIRS "" CACHE PATH "" FORCE)

file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/spdlog-stub")
file(WRITE "${CMAKE_BINARY_DIR}/spdlog-stub/spdlogConfig.cmake"
	"if(NOT TARGET spdlog::spdlog)\n"
	"  message(FATAL_ERROR \"spdlog target not found\")\n"
	"endif()\n"
)
file(WRITE "${CMAKE_BINARY_DIR}/spdlog-stub/spdlogConfigVersion.cmake"
	"set(PACKAGE_VERSION \"1.14.1\")\n"
	"if(PACKAGE_VERSION VERSION_LESS PACKAGE_FIND_VERSION)\n"
	"  set(PACKAGE_VERSION_COMPATIBLE FALSE)\n"
	"else()\n"
	"  set(PACKAGE_VERSION_COMPATIBLE TRUE)\n"
	"endif()\n"
)
set(spdlog_DIR "${CMAKE_BINARY_DIR}/spdlog-stub" CACHE PATH "" FORCE)

file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/fmt-stub")
file(WRITE "${CMAKE_BINARY_DIR}/fmt-stub/fmtConfig.cmake"
	"if(NOT TARGET fmt::fmt)\n"
	"  message(FATAL_ERROR \"fmt target not found\")\n"
	"endif()\n"
)
file(WRITE "${CMAKE_BINARY_DIR}/fmt-stub/fmtConfigVersion.cmake"
	"set(PACKAGE_VERSION \"10.2.1\")\n"
	"if(PACKAGE_VERSION VERSION_LESS PACKAGE_FIND_VERSION)\n"
	"  set(PACKAGE_VERSION_COMPATIBLE FALSE)\n"
	"else()\n"
	"  set(PACKAGE_VERSION_COMPATIBLE TRUE)\n"
	"endif()\n"
)
set(fmt_DIR "${CMAKE_BINARY_DIR}/fmt-stub" CACHE PATH "" FORCE)

set(COMMONLIBSSE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CommonLibSSE-NG" CACHE PATH "Path to CommonLibSSE-NG source tree")

add_subdirectory(
	"${COMMONLIBSSE_PATH}"
	"${CMAKE_CURRENT_BINARY_DIR}/CommonLibSSE-NG"
)

add_library(${PROJECT_NAME} SHARED
	src/main.cpp
	src/Limits.cpp
	src/Counter.cpp
	src/Hooks.cpp
	src/UI.cpp
)

target_link_libraries(${PROJECT_NAME} PUBLIC
	CommonLibSSE
)

target_compile_features(${PROJECT_NAME} PUBLIC
	cxx_std_23
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
	WINVER=0x0601
	_WIN32_WINNT=0x0601
)

if(MSVC)
	target_compile_options(${PROJECT_NAME} PRIVATE
		/we4715
		/wd4005
		/wd4061
		/wd4068
		/wd4200
		/wd4201
		/wd4265
		/wd4266
		/wd4371
		/wd4514
		/wd4582
		/wd4583
		/wd4623
		/wd4625
		/wd4626
		/wd4710
		/wd4711
		/wd4820
		/wd4996
		/wd5026
		/wd5027
		/wd5045
		/wd5053
		/wd5204
		/wd5220
	)
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES
	PREFIX ""
	OUTPUT_NAME "z_TM_TrainingLimit"
)
