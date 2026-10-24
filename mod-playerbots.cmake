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

#
# Optional integration-test build for mod-playerbots.
#
option(PLAYERBOTS_INTEGRATION_TESTS "Compile mod-playerbots integration test scenarios" OFF)

if(PLAYERBOTS_INTEGRATION_TESTS)
  target_compile_definitions(modules PRIVATE PLAYERBOTS_INTEGRATION_TESTS)
  message(STATUS "mod-playerbots: integration tests ENABLED")

  # Copy the whole integration-tests/ helper folder next to the worldserver binary
  # (copy scheme from the config-merger tool in src/server/apps/CMakeLists.txt).
  if(WIN32 AND "${CMAKE_MAKE_PROGRAM}" MATCHES "MSBuild")
    foreach(cfg IN ITEMS Debug Release RelWithDebInfo MinSizeRel)
      file(COPY "${CMAKE_CURRENT_LIST_DIR}/apps/integration-tests" DESTINATION "${CMAKE_BINARY_DIR}/bin/${cfg}")
    endforeach()
  else()
    file(COPY "${CMAKE_CURRENT_LIST_DIR}/apps/integration-tests" DESTINATION "${CMAKE_BINARY_DIR}/bin")
  endif()
endif()
