cmake_minimum_required(VERSION 3.15)

project(
    P2P-Server
    VERSION 0.1
    LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(JsonCpp MODULE)
if(JsonCpp_FOUND)
    set(JSONCPP_INCLUDE_DIRS ${JsonCpp_INCLUDE_DIRS})
    set(JSONCPP_LIBRARIES ${JsonCpp_LIBRARIES})
else()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(JSONCPP REQUIRED jsoncpp)
endif()

find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)

find_path(MYSQL_DIR
    mysql/mysql.h
    /usr/include
    /usr/include/mysql
)

find_library(MYSQL_CLIENT_LIB
    mysqlclient
    /usr/lib
    /usr/lib64
    /usr/lib/x86_64-linux-gnu
)

find_path(HIREDIS_DIR
    hiredis/hiredis.h
    /usr/include
)
find_library(HIREDIS_LIB
    hiredis
    /usr/lib
    /usr/lib64
    /usr/lib/x86_64-linux-gnu
)

find_path(REDIS_PLUS_PLUS_DIR
    sw/redis++/redis++.h
)
find_library(REDIS_PLUS_PLUS_LIB redis++)

find_path(HTTPLIB_DIR httplib.h)

find_path(JWTCPP_DIR jwt-cpp/jwt.h)


file(GLOB SRC_FILES
    "${PROJECT_SOURCE_DIR}/*/*.h"
    "${PROJECT_SOURCE_DIR}/*/*.cpp"
    "${PROJECT_SOURCE_DIR}/main.cpp"
)

add_executable(${CMAKE_PROJECT_NAME} ${SRC_FILES})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${JSONCPP_INCLUDE_DIRS}
    ${MYSQL_DIR}
    ${HIREDIS_DIR}
    ${REDIS_PLUS_PLUS_DIR}
    ${HTTPLIB_DIR}
    ${JWTCPP_DIR}
)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE ${MYSQL_CLIENT_LIB})
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE ${HIREDIS_LIB})
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE ${REDIS_PLUS_PLUS_LIB})

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE ${JSONCPP_LIBRARIES})

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE OpenSSL::SSL)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE OpenSSL::Crypto)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE Threads::Threads)

