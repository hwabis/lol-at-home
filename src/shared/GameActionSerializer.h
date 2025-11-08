#pragma once

#include <optional>
#include <vector>
#include "GameAction.h"
#include "game_actions_generated.h"

namespace lol_at_home_shared {

class GameActionSerializer {
 public:
  GameActionSerializer() = delete;

  // todo obliterate this method
  static auto Serialize(const GameActionVariant& action)
      -> std::vector<std::byte>;

  // todo obliterate this method
  static auto Deserialize(const std::vector<std::byte>& data)
      -> std::optional<GameActionVariant>;

  static auto UnpackGameAction(const lol_at_home_shared::GameActionFB* action)
      -> std::optional<GameActionVariant>;
};

}  // namespace lol_at_home_shared
