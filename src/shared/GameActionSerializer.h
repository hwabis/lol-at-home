#pragma once

#include <optional>
#include "GameAction.h"
#include "game_actions_generated.h"

namespace lol_at_home_shared {

class GameActionSerializer {
 public:
  GameActionSerializer() = delete;

  static auto Deserialize(const lol_at_home_shared::GameActionFB* action)
      -> std::optional<GameActionVariant>;
};

}  // namespace lol_at_home_shared
