cmake_minimum_required(VERSION 3.16)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
project(CoinbaseTickerAnalyzer VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Aggressive optimization flags for HFT (Linux-specific)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    # Release mode: Maximum optimization for HFT
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -march=native -mtune=native")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -flto -ffast-math -funroll-loops")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fno-strict-aliasing -fomit-frame-pointer")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -finline-functions -finline-limit=1000")
    
    # Link-time optimization
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -flto")
    
    # Default flags (used if not in Release mode)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O3 -march=native")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffast-math -funroll-loops")
    
    # Debug mode
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -DDEBUG -O0")
else()
    # Non-Linux: Standard optimization
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O2")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -DDEBUG")
endif()

# Find required packages
find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBCURL REQUIRED libcurl)
pkg_check_modules(LIBWEBSOCKETS REQUIRED libwebsockets)
find_package(OpenSSL REQUIRED)

# Find NUMA library (Linux-specific)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    find_library(NUMA_LIBRARY numa)
    if(NUMA_LIBRARY)
        message(STATUS "Found NUMA library: ${NUMA_LIBRARY}")
    else()
        message(WARNING "NUMA library not found. NUMA optimizations will be disabled.")
    endif()
endif()

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
    src/HighResTimer.cpp
    src/NUMAUtils.cpp
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
    include/HighResTimer.h
    include/NUMAUtils.h
    include/BranchPrediction.h
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

# Link NUMA library if available (Linux-specific)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NUMA_LIBRARY)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${NUMA_LIBRARY})
    target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_NUMA)
endif()

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
