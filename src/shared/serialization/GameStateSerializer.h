#pragma once

#include <entt/entt.hpp>
#include <vector>
#include "game_state_generated.h"

namespace lah::shared {

class GameStateSerializer {
 public:
  GameStateSerializer() = delete;

  // Called on server
  static auto Serialize(flatbuffers::FlatBufferBuilder& builder,
                        const entt::registry& registry,
                        const std::vector<entt::entity>& dirtyEntities,
                        const std::vector<entt::entity>& deletedEntities)
      -> flatbuffers::Offset<lah_shared::GameStateDeltaFB>;

  // Called on client
  static auto Deserialize(entt::registry& registry,
                          const lah_shared::GameStateDeltaFB& gamestate)
      -> void;

 private:
  static void deserializePosition(entt::registry& registry,
                                  entt::entity entity,
                                  const lah_shared::PositionFB* pos);
  static void deserializeHealth(entt::registry& registry,
                                entt::entity entity,
                                const lah_shared::HealthFB* health);
  static void deserializeMana(entt::registry& registry,
                              entt::entity entity,
                              const lah_shared::ManaFB* mana);
  static void deserializeMovable(entt::registry& registry,
                                 entt::entity entity,
                                 const lah_shared::MovableFB* movable);
  static void deserializeTeam(entt::registry& registry,
                              entt::entity entity,
                              const lah_shared::TeamFB* team);
  static void deserializeAbilities(entt::registry& registry,
                                   entt::entity entity,
                                   const lah_shared::AbilitiesFB* abilitiesFB);
};

}  // namespace lah::shared
