#pragma once

#include <entt/entt.hpp>
#include <tuple>
#include <vector>
#include "EcsComponents.h"
#include "game_state_generated.h"

namespace lol_at_home_shared {

using SerializableComponents =
    std::tuple<Position, Health, Mana, Movable, Moving, Team, Abilities>;

class GameStateSerializer {
 public:
  GameStateSerializer() = delete;

  static auto Serialize(flatbuffers::FlatBufferBuilder& builder,
                        const entt::registry& registry,
                        const std::vector<entt::entity>& dirtyEntities,
                        const std::vector<entt::entity>& deletedEntities)
      -> flatbuffers::Offset<lol_at_home_shared::GameStateSnapshotFB>;

  // todo obliterate
  static auto Serialize(const entt::registry& registry,
                        const std::vector<entt::entity>& entities)
      -> std::vector<std::byte>;
  // todo obliterate
  static void Deserialize(entt::registry& registry,
                          const std::vector<std::byte>& data);
};

}  // namespace lol_at_home_shared
