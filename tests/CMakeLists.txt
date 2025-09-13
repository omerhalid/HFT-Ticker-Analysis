# Test configuration - use FetchContent for Google Test
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)
FetchContent_MakeAvailable(googletest)

# Test executable
add_executable(tests
    test_main.cpp
    test_EMACalculator.cpp
    test_JSONParser.cpp
    test_CSVLogger.cpp
    test_TickerData.cpp
    test_CoinbaseTickerAnalyzer.cpp
    # Source files for linking
    ${CMAKE_SOURCE_DIR}/src/EMACalculator.cpp
    ${CMAKE_SOURCE_DIR}/src/JSONParser.cpp
    ${CMAKE_SOURCE_DIR}/src/CSVLogger.cpp
    ${CMAKE_SOURCE_DIR}/src/AsyncCSVLogger.cpp
    ${CMAKE_SOURCE_DIR}/src/TickerData.cpp
    ${CMAKE_SOURCE_DIR}/src/CoinbaseTickerAnalyzer.cpp
    ${CMAKE_SOURCE_DIR}/src/WebSocketClient.cpp
    ${CMAKE_SOURCE_DIR}/src/ThreadUtils.cpp
)

# Include directories
target_include_directories(tests PRIVATE 
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

# Link libraries
target_link_libraries(tests
    PRIVATE
    gtest
    gtest_main
    ${LIBCURL_LIBRARIES}
    /opt/homebrew/Cellar/libwebsockets/4.4.1/lib/libwebsockets.dylib
    OpenSSL::SSL
    OpenSSL::Crypto
    nlohmann_json::nlohmann_json
    ${CMAKE_THREAD_LIBS_INIT}
)

# Add test to CTest
include(GoogleTest)
gtest_discover_tests(tests)
