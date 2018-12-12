cmake_minimum_required (VERSION 2.8.6)

project (llh)

if (CMAKE_BUILD_TYPE MATCHES "ASAN")
  set(san_lib asan)
  set(san_opts -fsanitize=address -Wno-address-of-packed-member)
  set(CMAKE_C_COMPILER "/usr/bin/clang")
endif (CMAKE_BUILD_TYPE MATCHES "ASAN")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_definitions(-Wall -g -O2 -D_GNU_SOURCE ${san_opts})

add_executable(llh-simple llh.c test/llh-simple.c)
add_executable(llh-mt llh.c test/llh-mt.c)

target_link_libraries(llh-simple ${san_lib} m)
target_link_libraries(llh-mt ${san_lib} m pthread)
