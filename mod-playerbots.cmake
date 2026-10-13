# Included by the core's modules/CMakeLists.txt after the `modules` target exists.
#
# The core links the MySQL client library privately into its `database` target,
# so its include directories do not reach module code. PlayerbotsDatabase.cpp
# needs the complete MySQLPreparedStatement type (which pulls in mysql.h) and
# Playerbots.cpp uses ER_BAD_DB_ERROR from mysqld_error.h. Linking the imported
# `mysql` target here propagates those include directories to the module build.
target_link_libraries(modules
  PRIVATE
    mysql)

# Unit tests for the module's pure logic, built as their own executable (playerbots_tests) and run as
# build/modules/playerbots_tests. The core's unit_tests target does not build with mod-playerbots present, so
# they are not registered there. Tests live outside src/ so the module's recursive source collection skips them.
option(PLAYERBOTS_TESTS "Build mod-playerbots' unit tests (playerbots_tests)" OFF)

if(PLAYERBOTS_TESTS)
  # With BUILD_TESTING on, the core fetches googletest itself later in the configure; targets resolve at generate time.
  if(NOT BUILD_TESTING AND NOT TARGET gtest_main)
    include("${CMAKE_SOURCE_DIR}/src/cmake/googletest.cmake")
    fetch_googletest("${CMAKE_SOURCE_DIR}/src/cmake" "${CMAKE_BINARY_DIR}/googletest")
  endif()

  file(GLOB PLAYERBOTS_TEST_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/tests/*.cpp")
  add_executable(playerbots_tests
    ${PLAYERBOTS_TEST_SOURCES}
    "${CMAKE_CURRENT_LIST_DIR}/src/Mgr/PvpLoadout/PvpLoadoutRules.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/Mgr/PvpLoadout/PvpLoadoutEp.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/Util/Helpers.cpp")
  target_include_directories(playerbots_tests PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/src/Mgr/PvpLoadout"
    "${CMAKE_CURRENT_LIST_DIR}/src/Util"
    "${CMAKE_SOURCE_DIR}/src/common")
  target_link_libraries(playerbots_tests PRIVATE gtest_main)
  set_target_properties(playerbots_tests PROPERTIES
    FOLDER "modules"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/modules")
endif()
