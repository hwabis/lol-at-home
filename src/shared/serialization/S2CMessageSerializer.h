#pragma once

#include <cstddef>
#include <entt/entt.hpp>
#include <optional>
#include <span>
#include <string>
#include <vector>
#include "domain/EcsComponents.h"
#include "game_state_generated.h"

namespace lah::shared {

struct PlayerAssignmentData {
  uint32_t assignedEntityId;
};

struct ChatBroadcastData {
  uint32_t senderEntityId;
  std::string message;
};

struct EntitySnapshot {
  uint32_t entityId{};
  std::optional<Position> position;
  std::optional<Radius> radius;
  std::optional<Health> health;
  std::optional<Mana> mana;
  std::optional<MovementStats> movementStats;
  std::optional<CharacterState> characterState;
  std::optional<MoveTarget> moveTarget;
  std::optional<Team> team;
  std::optional<Abilities> abilities;
  std::optional<ChampionType> championType;
};

struct GameStateDeltaData {
  std::vector<EntitySnapshot> entities;
  std::vector<uint32_t> deletedEntityIds;
};

enum class S2CMessageType : uint8_t {
  GameStateDelta,
  PlayerAssignment,
  ChatBroadcast,
  Unknown,
};

class S2CMessageSerializer {
 public:
  S2CMessageSerializer() = delete;

  static auto GetMessageType(std::span<const std::byte> data) -> S2CMessageType;

  static auto SerializeGameStateDelta(
      const entt::registry& registry,
      const std::vector<entt::entity>& dirtyEntities,
      const std::vector<entt::entity>& deletedEntities)
      -> std::vector<std::byte>;

  static auto SerializePlayerAssignment(uint32_t entityId)
      -> std::vector<std::byte>;

  static auto SerializeChatBroadcast(uint32_t senderEntityId,
                                     const std::string& message)
      -> std::vector<std::byte>;

  static auto DeserializeGameStateDelta(std::span<const std::byte> data)
      -> std::optional<GameStateDeltaData>;

  static auto DeserializePlayerAssignment(std::span<const std::byte> data)
      -> std::optional<PlayerAssignmentData>;

  static auto DeserializeChatBroadcast(std::span<const std::byte> data)
      -> std::optional<ChatBroadcastData>;

 private:
  static void deserializePosition(entt::registry& registry,
                                  entt::entity entity,
                                  const lah_shared::PositionFB* pos);
  static void deserializeRadius(entt::registry& registry,
                                entt::entity entity,
                                const lah_shared::RadiusFB* radius);
  static void deserializeHealth(entt::registry& registry,
                                entt::entity entity,
                                const lah_shared::HealthFB* health);
  static void deserializeMana(entt::registry& registry,
                              entt::entity entity,
                              const lah_shared::ManaFB* mana);
  static void deserializeMovementStats(
      entt::registry& registry,
      entt::entity entity,
      const lah_shared::MovementStatsFB* movementStats);
  static void deserializeMoveTarget(entt::registry& registry,
                                    entt::entity entity,
                                    const lah_shared::MoveTargetFB* moveTarget);
  static void deserializeTeam(entt::registry& registry,
                              entt::entity entity,
                              const lah_shared::TeamFB* team);
  static void deserializeAbilities(entt::registry& registry,
                                   entt::entity entity,
                                   const lah_shared::AbilitiesFB* abilitiesFB);
};

}  // namespace lah::shared
