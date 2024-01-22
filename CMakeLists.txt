cmake_minimum_required(VERSION 3.0.0)
project(cpptree
	VERSION 1.0.1
	DESCRIPTION "A bidirectional tree structure for C++17"
	HOMEPAGE_URL "https://github.com/CCColda/cpptree#readme"
	LANGUAGES CXX
)

set(CPPTREE_CUSTOM_ALLOCATOR CACHE STRING "")
option(CPPTREE_BUILD_TEST "Build tests" OFF)

# directory layout

set(CPPTREE_RESOURCE_DIR data)
set(CPPTREE_SRC_DIR src)
set(CPPTREE_INCLUDE_DIR include)
set(CPPTREE_TEST_DIR test)

set(CPPTREE_SOURCES
	${CPPTREE_SRC_DIR}/cppTreeNode.cpp
)

set(CPPTREE_TEST_SOURCES
	${CPPTREE_TEST_DIR}/main.cpp
)

set(CPPTREE_HEADERS
	${CPPTREE_INCLUDE_DIR}/cppTreeNode.h
)

# compilation

add_library(cpptree STATIC)
add_library(colda::cpptree ALIAS cpptree)

target_compile_features(cpptree PRIVATE cxx_std_17)
target_include_directories(cpptree PUBLIC ${CPPTREE_INCLUDE_DIR})
target_sources(cpptree PRIVATE ${CPPTREE_SOURCES} ${CPPTREE_HEADERS})

if (NOT CPPTREE_CUSTOM_ALLOCATOR STREQUAL "")
	target_compile_definitions(CPPTREE PUBLIC CPPTREE_CUSTOM_ALLOCATOR=${CPPTREE_CUSTOM_ALLOCATOR})
endif()

# tests

if (CPPTREE_BUILD_TEST)
	set(CMAKE_BUILD_TYPE DEBUG)

	include(FetchContent)

	FetchContent_Declare(
		catch2
		GIT_REPOSITORY https://github.com/catchorg/Catch2.git
		GIT_TAG        v3.5.2
		GIT_SHALLOW    TRUE
		GIT_PROGRESS   TRUE
	)

	FetchContent_MakeAvailable(catch2)

	add_executable(cpptree_test)
	target_sources(cpptree_test PRIVATE ${CPPTREE_TEST_SOURCES})
	target_compile_features(cpptree_test PRIVATE cxx_std_17)
	target_link_libraries(cpptree_test PRIVATE colda::cpptree Catch2::Catch2WithMain)

	list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
	include(CTest)
	include(Catch)

	catch_discover_tests(cpptree_test)
endif()
