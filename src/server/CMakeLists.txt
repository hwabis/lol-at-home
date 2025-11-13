include(${CMAKE_SOURCE_DIR}/cmake/SetTargetDefaults.cmake)

add_library(lol-at-home-server
  core/EnetInterface.cc
  core/GameState.cc
  core/InboundEventVisitor.cc
  core/Server.cc
)

set_target_defaults(lol-at-home-server)

target_include_directories(lol-at-home-server PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(lol-at-home-server PUBLIC
  lol-at-home-shared
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

add_executable(lol-at-home-server-app
  Main.cc
)

set_target_defaults(lol-at-home-server-app)

target_link_libraries(lol-at-home-server-app PRIVATE
  lol-at-home-server
)
