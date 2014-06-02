CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(epoxy)
INCLUDE(cmake/Modules/FindLibcouchbase.cmake)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

ADD_LIBRARY(cliopts STATIC contrib/cliopts/cliopts.c)
INCLUDE_DIRECTORIES(${LCB_INCLUDE_DIR})
INCLUDE_DIRECTORIES(contrib/cliopts)
INCLUDE_DIRECTORIES(contrib/memcached/include)

FILE(GLOB EPOXY_CORE_SRC src/*.cpp)
ADD_EXECUTABLE(epoxy ${EPOXY_CORE_SRC} src/logging/epoxy_yolog.c)

TARGET_LINK_LIBRARIES(epoxy ${LCB_DSO} cliopts ev)
