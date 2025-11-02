include(${CMAKE_SOURCE_DIR}/cmake/SetTargetDefaults.cmake)

set(FBS_FILES 
  ${CMAKE_CURRENT_SOURCE_DIR}/game_state.fbs
  ${CMAKE_CURRENT_SOURCE_DIR}/game_actions.fbs
)
find_program(FLATC flatc REQUIRED)
set(GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${GENERATED_DIR})

add_custom_command(
  OUTPUT 
    ${GENERATED_DIR}/game_state_generated.h
    ${GENERATED_DIR}/game_actions_generated.h
  COMMAND ${FLATC} --cpp --scoped-enums -o ${GENERATED_DIR} ${FBS_FILES}
  DEPENDS ${FBS_FILES}
  COMMENT "Generating FlatBuffers C++ code"
)

add_library(lol-at-home-shared
  ${GENERATED_DIR}/game_actions_generated.h
  ${GENERATED_DIR}/game_state_generated.h
  GameActionSerializer.cc
  GameStateSerializer.cc
  PlayerAssignmentSerializer.cc
)

set_target_defaults(lol-at-home-shared)

target_include_directories(lol-at-home-shared PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${GENERATED_DIR}
)

target_link_libraries(lol-at-home-shared PUBLIC
  flatbuffers::flatbuffers
  EnTT::EnTT
)
