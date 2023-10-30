add_library(kmeanslib
  point.cpp
  point.h

  kmeans.cpp
  kmeans.h

  kmeans_parallel.cpp
  kmeans_parallel.h

  kmeanproblem.cpp
  kmeanproblem.h

  modules.cpp
  modules.h
)
target_link_libraries(kmeanslib PUBLIC Threads::Threads TBB::tbb)
target_include_directories(kmeanslib PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

add_executable(kmeans main.cpp)
target_link_libraries(kmeans kmeanslib)

add_executable(kmeans_benchmark kmeans_benchmark.cpp)
target_link_libraries(kmeans_benchmark kmeanslib)
