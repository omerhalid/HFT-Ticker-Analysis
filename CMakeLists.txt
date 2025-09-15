cmake_minimum_required(VERSION 3.16)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
project(CoinbaseTickerAnalyzer VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compiler flags for performance and debugging
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O2")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -DDEBUG")

# Find required packages
find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBCURL REQUIRED libcurl)
pkg_check_modules(LIBWEBSOCKETS REQUIRED libwebsockets)
find_package(OpenSSL REQUIRED)

# Ensure LIBWEBSOCKETS_LIBRARIES is set correctly
if(NOT LIBWEBSOCKETS_LIBRARIES)
    set(LIBWEBSOCKETS_LIBRARIES websockets)
endif()

# Include directories
include_directories(include)
include_directories(third_party)
include_directories(${LIBCURL_INCLUDE_DIRS})
include_directories(${LIBWEBSOCKETS_INCLUDE_DIRS})
include_directories(${OPENSSL_INCLUDE_DIR})

# Add subdirectories for third-party libraries
add_subdirectory(third_party)

# Source files
set(SOURCES
    src/main.cpp
    src/CoinbaseTickerAnalyzer.cpp
    src/EMACalculator.cpp
    src/AsyncCSVLogger.cpp
    src/WebSocketClient.cpp
    src/JSONParser.cpp
    src/TickerData.cpp
    src/ThreadUtils.cpp
)

# Header files
set(HEADERS
    include/CoinbaseTickerAnalyzer.h
    include/EMACalculator.h
    include/AsyncCSVLogger.h
    include/WebSocketClient.h
    include/JSONParser.h
    include/TickerData.h
    include/LockFreeRingBuffer.h
    include/ThreadUtils.h
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Link libraries
target_link_libraries(${PROJECT_NAME} 
    PRIVATE 
    Threads::Threads
    ${LIBCURL_LIBRARIES}
    ${LIBWEBSOCKETS_LIBRARIES}
    OpenSSL::SSL
    OpenSSL::Crypto
    nlohmann_json::nlohmann_json
    ${CMAKE_THREAD_LIBS_INIT}
)

# Add include directories and link directories for pkg-config
target_include_directories(${PROJECT_NAME} PRIVATE ${LIBCURL_INCLUDE_DIRS} ${LIBWEBSOCKETS_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME} PRIVATE ${LIBCURL_LIBRARY_DIRS} ${LIBWEBSOCKETS_LIBRARY_DIRS})

# Enable testing
enable_testing()
add_subdirectory(tests)

# Doxygen documentation
find_package(Doxygen)
if(DOXYGEN_FOUND)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
    add_custom_target(docs
        ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen" VERBATIM
    )
endif()
