CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(epoxy)
INCLUDE(cmake/Modules/FindLibcouchbase.cmake)


ADD_LIBRARY(cliopts STATIC contrib/cliopts/cliopts.c)
ADD_LIBRARY(jsoncpp STATIC contrib/json-cpp/jsoncpp.cpp)
INCLUDE_DIRECTORIES(${LCB_INCLUDE_DIR})
INCLUDE_DIRECTORIES(contrib/json-cpp)
INCLUDE_DIRECTORIES(contrib/cliopts)
INCLUDE_DIRECTORIES(contrib/memcached/include)

FILE(GLOB EPOXY_CORE_SRC src/*.cpp)
ADD_EXECUTABLE(epoxy ${EPOXY_CORE_SRC})

TARGET_LINK_LIBRARIES(epoxy ${LCB_DSO} cliopts jsoncpp ev)
