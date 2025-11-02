#pragma once

#include <cstddef>
#include <vector>
#include "PlayerAssignment.h"

namespace lol_at_home_shared {

class PlayerAssignmentSerializer {
 public:
  PlayerAssignmentSerializer() = delete;

  static auto Serialize(const PlayerAssignment& assignment)
      -> std::vector<std::byte>;

  static auto Deserialize(const std::vector<std::byte>& data)
      -> PlayerAssignment;
};

}  // namespace lol_at_home_shared
