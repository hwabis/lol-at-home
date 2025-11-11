#pragma once

#include <entt/entt.hpp>
#include <tuple>
#include <vector>
#include "domain/EcsComponents.h"
#include "game_state_generated.h"

namespace lol_at_home_shared {

using SerializableComponents =
    std::tuple<Position, Health, Mana, Movable, Moving, Team, Abilities>;

class GameStateSerializer {
 public:
  GameStateSerializer() = delete;

  // Called on server
  static auto Serialize(flatbuffers::FlatBufferBuilder& builder,
                        const entt::registry& registry,
                        const std::vector<entt::entity>& dirtyEntities,
                        const std::vector<entt::entity>& deletedEntities)
      -> flatbuffers::Offset<lol_at_home_shared::GameStateSnapshotFB>;

  // todo deserialize
};

}  // namespace lol_at_home_shared
