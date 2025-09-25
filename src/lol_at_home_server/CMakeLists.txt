include(${CMAKE_SOURCE_DIR}/cmake/SetTargetDefaults.cmake)

add_executable(lol-at-home-server
  core/GameState.cc
  core/GameStateThread.cc
  Main.cc
)

set_target_defaults(lol-at-home-server)

target_include_directories(lol-at-home-server PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(lol-at-home-server PRIVATE
  spdlog::spdlog
  unofficial::enet::enet
  EnTT::EnTT
)

# For enet
if(WIN32)
  target_link_libraries(lol-at-home-server PRIVATE
    winmm
    ws2_32
  )
endif()
