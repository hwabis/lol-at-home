include(${CMAKE_SOURCE_DIR}/cmake/SetTargetDefaults.cmake)

set(FBS_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/fbs/c2s_message.fbs
  ${CMAKE_CURRENT_SOURCE_DIR}/fbs/common_types.fbs
  ${CMAKE_CURRENT_SOURCE_DIR}/fbs/game_actions.fbs
  ${CMAKE_CURRENT_SOURCE_DIR}/fbs/game_state.fbs
  ${CMAKE_CURRENT_SOURCE_DIR}/fbs/s2c_message.fbs
)

find_program(FLATC flatc REQUIRED)
set(GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${GENERATED_DIR})

set(GENERATED_HEADERS
  ${GENERATED_DIR}/c2s_message_generated.h
  ${GENERATED_DIR}/common_types_generated.h
  ${GENERATED_DIR}/game_actions_generated.h
  ${GENERATED_DIR}/game_state_generated.h
  ${GENERATED_DIR}/s2c_message_generated.h
)

add_custom_command(
  OUTPUT ${GENERATED_HEADERS}
  COMMAND ${FLATC} --cpp --scoped-enums -o ${GENERATED_DIR} ${FBS_FILES}
  DEPENDS ${FBS_FILES}
  COMMENT "Generating FlatBuffers C++ code for lol-at-home-shared"
)

add_library(lol-at-home-shared
  ${GENERATED_HEADERS}
  serialization/GameActionSerializer.cc
  serialization/GameStateSerializer.cc
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
