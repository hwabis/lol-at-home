include(${CMAKE_SOURCE_DIR}/cmake/SetTargetDefaults.cmake)

add_executable(lol-at-home-game
  Main.cc
  NetworkClient.cc
)

set_target_defaults(lol-at-home-game)

target_include_directories(lol-at-home-game PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(lol-at-home-game PRIVATE
  lol-at-home-engine
  lol-at-home-shared
  spdlog::spdlog
  unofficial::enet::enet
  EnTT::EnTT
)

# For enet
if(WIN32)
  target_link_libraries(lol-at-home-game PRIVATE
    winmm
    ws2_32
  )
endif()
