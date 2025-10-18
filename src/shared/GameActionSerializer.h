#pragma once

#include <cstddef>
#include <vector>
#include "GameAction.h"

namespace lol_at_home_shared {

class GameActionSerializer {
 public:
  GameActionSerializer() = delete;

  static auto Serialize(const GameActionVariant& action)
      -> std::vector<std::byte>;

  static auto Deserialize(const std::vector<std::byte>& data)
      -> GameActionVariant;
};

}  // namespace lol_at_home_shared
