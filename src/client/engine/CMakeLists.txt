include(${CMAKE_SOURCE_DIR}/cmake/SetTargetDefaults.cmake)

add_library(lol-at-home-engine
  Camera.cc
  Game.cc
  InputAccessor.cc
)

set_target_defaults(lol-at-home-engine)

target_include_directories(lol-at-home-engine PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(lol-at-home-engine PRIVATE
  SDL3::SDL3
  EnTT::EnTT
  spdlog::spdlog
)
