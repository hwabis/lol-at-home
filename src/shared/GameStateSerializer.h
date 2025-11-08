#pragma once

#include <entt/entt.hpp>
#include <tuple>
#include <vector>
#include "EcsComponents.h"

namespace lol_at_home_shared {

using SerializableComponents =
    std::tuple<Position, Health, Mana, Movable, Moving, Team, Abilities>;

class GameStateSerializer {
 public:
  GameStateSerializer() = delete;

  // todo obliterate
  static auto Serialize(const entt::registry& registry,
                        const std::vector<entt::entity>& entities)
      -> std::vector<std::byte>;
  // todo obliterate
  static void Deserialize(entt::registry& registry,
                          const std::vector<std::byte>& data);
};

}  // namespace lol_at_home_shared
