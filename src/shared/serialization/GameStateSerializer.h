#pragma once

#include <entt/entt.hpp>
#include <vector>
#include "game_state_generated.h"

namespace lol_at_home_shared {

class GameStateSerializer {
 public:
  GameStateSerializer() = delete;

  // Called on server
  static auto Serialize(flatbuffers::FlatBufferBuilder& builder,
                        const entt::registry& registry,
                        const std::vector<entt::entity>& dirtyEntities,
                        const std::vector<entt::entity>& deletedEntities)
      -> flatbuffers::Offset<lol_at_home_shared::GameStateDeltaFB>;

  // Called on client
  static auto Deserialize(entt::registry& registry,
                          const lol_at_home_shared::GameStateDeltaFB& gamestate)
      -> void;

 private:
  static void deserializePosition(entt::registry& registry,
                                  entt::entity entity,
                                  const PositionFB* pos);
  static void deserializeHealth(entt::registry& registry,
                                entt::entity entity,
                                const HealthFB* health);
  static void deserializeMana(entt::registry& registry,
                              entt::entity entity,
                              const ManaFB* mana);
  static void deserializeMovable(entt::registry& registry,
                                 entt::entity entity,
                                 const MovableFB* movable);
  static void deserializeTeam(entt::registry& registry,
                              entt::entity entity,
                              const TeamFB* team);
  static void deserializeAbilities(entt::registry& registry,
                                   entt::entity entity,
                                   const AbilitiesFB* abilitiesFB);
};

}  // namespace lol_at_home_shared
