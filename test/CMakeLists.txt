add_executable(test_kmeans test_kmeans.cpp)
target_link_libraries(test_kmeans PRIVATE kmeanslib)

add_test(NAME test_kmeans COMMAND test_kmeans)
