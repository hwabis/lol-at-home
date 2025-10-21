include(${CMAKE_SOURCE_DIR}/cmake/SetTargetDefaults.cmake)

add_executable(lol-at-home-cli-client
  CliClient.cc
)

set_target_defaults(lol-at-home-cli-client)

target_include_directories(lol-at-home-cli-client PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(lol-at-home-cli-client PRIVATE
  lol-at-home-shared
  spdlog::spdlog
  unofficial::enet::enet
  EnTT::EnTT
)

# For enet
if(WIN32)
  target_link_libraries(lol-at-home-cli-client PRIVATE
    winmm
    ws2_32
  )
endif()
