cmake_minimum_required(VERSION 3.18)

project(inf5171-233-tp3 LANGUAGES CXX)

find_package(MFEM REQUIRED)

add_executable(heatsim heatsim.cpp)
target_link_libraries(heatsim mfem)

configure_file(env.sh.in env.sh)

# Package
string(TIMESTAMP TS "%Y%m%d%H%M%S")
set(CPACK_SOURCE_PACKAGE_FILE_NAME ${CMAKE_PROJECT_NAME}-${CMAKE_PROJECT_VERSION}-${TS})
set(CPACK_SOURCE_GENERATOR "ZIP")
set(CPACK_SOURCE_IGNORE_FILES
        "/build/;/.git/;~$;CMakeLists.txt.user;/data/.*\\\\.png;/results/;${CPACK_SOURCE_IGNORE_FILES}")
include(CPack)
add_custom_target(dist COMMAND ${CMAKE_MAKE_PROGRAM} package_source)

