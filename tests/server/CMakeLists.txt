include(${CMAKE_SOURCE_DIR}/cmake/AddTestExecutable.cmake)

add_test_executable(lol-at-home-server-tests lol-at-home-server
  GameStateTest.cc
)
