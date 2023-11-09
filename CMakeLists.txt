cmake_minimum_required(VERSION 3.8)

project(inf5171-tp2 LANGUAGES C CXX VERSION 23.1)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

enable_testing()
include(CTest)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(TBB REQUIRED)

add_subdirectory(src)
add_subdirectory(test)

# Definir les matricules de l'equipe
set(MATRICULES "BOSG64080108_BELY22119707")

if(MATRICULES)
  set(CPACK_SOURCE_PACKAGE_FILE_NAME ${CMAKE_PROJECT_NAME}-${CMAKE_PROJECT_VERSION}-${MATRICULES})
endif()

set(CPACK_SOURCE_IGNORE_FILES
	"/build/;/.git/;~$;CMakeLists.txt.user;/venv;${CPACK_SOURCE_IGNORE_FILES}")
include(CPack)
add_custom_target(dist COMMAND ${CMAKE_MAKE_PROGRAM} package_source)
