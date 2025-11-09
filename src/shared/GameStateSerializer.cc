#include "GameStateSerializer.h"
#include <flatbuffers/flatbuffers.h>
#include "game_state_generated.h"

namespace lol_at_home_shared {

namespace {
auto serializeEntity(flatbuffers::FlatBufferBuilder& builder,
                     const entt::registry& registry,
                     entt::entity entity)
    -> std::optional<flatbuffers::Offset<EntityFB>> {
  PositionDataFB posData{};
  const PositionDataFB* posPtr = nullptr;
  if (const auto* pos = registry.try_get<Position>(entity)) {
    posData = PositionDataFB(pos->x, pos->y);
    posPtr = &posData;
  }

  HealthDataFB HealthData{};
  const HealthDataFB* healthPtr = nullptr;
  if (const auto* health = registry.try_get<Health>(entity)) {
    HealthData = HealthDataFB(health->currentHealth, health->maxHealth,
                              health->healthRegenPerSec);
    healthPtr = &HealthData;
  }

  ManaDataFB ManaData{};
  const ManaDataFB* manaPtr = nullptr;
  if (const auto* mana = registry.try_get<Mana>(entity)) {
    ManaData = ManaDataFB(mana->mana, mana->maxMana, mana->manaRegenPerSec);
    manaPtr = &ManaData;
  }

  MovableDataFB MovableData{};
  const MovableDataFB* movablePtr = nullptr;
  if (const auto* movable = registry.try_get<Movable>(entity)) {
    MovableData = MovableDataFB(movable->speed);
    movablePtr = &MovableData;
  }

  MovingDataFB MovingData{};
  const MovingDataFB* movingPtr = nullptr;
  if (const auto* moving = registry.try_get<Moving>(entity)) {
    MovingData =
        MovingDataFB(moving->targetPosition.x, moving->targetPosition.y);
    movingPtr = &MovingData;
  }

  TeamDataFB TeamData{};
  const TeamDataFB* teamPtr = nullptr;
  if (const auto* team = registry.try_get<Team>(entity)) {
    TeamData =
        TeamDataFB(team->teamColorFB == Team::Color::Blue ? TeamColorFB::Blue
                                                          : TeamColorFB::Red);
    teamPtr = &TeamData;
  }

  flatbuffers::Offset<AbilitiesDataFB> abilitiesOffset = 0;
  if (const auto* abilities = registry.try_get<Abilities>(entity)) {
    std::vector<flatbuffers::Offset<AbilityEntryFB>> abilityEntries;

    for (const auto& [slot, ability] : abilities->abilities) {
      auto tagData = static_cast<AbilityTagDataFB>(ability.Tag);
      auto cooldown = ability.CooldownRemaining;
      auto rank = ability.Rank;
      auto charges = ability.CurrentCharges;
      auto maxCharges = ability.MaxCharges;

      AbilityDataFB AbilityDataFB(tagData, cooldown, rank, charges, maxCharges);

      auto entry = CreateAbilityEntryFB(
          builder, static_cast<AbilitySlotDataFB>(slot), &AbilityDataFB);
      abilityEntries.push_back(entry);
    }

    auto entriesVector = builder.CreateVector(abilityEntries);
    abilitiesOffset = CreateAbilitiesDataFB(builder, entriesVector);
  }

  auto entityOffset =
      CreateEntityFB(builder, static_cast<uint32_t>(entity), posPtr, healthPtr,
                     manaPtr, movablePtr, movingPtr, teamPtr, abilitiesOffset);

  return entityOffset;
}
}  // namespace

auto GameStateSerializer::Serialize(
    flatbuffers::FlatBufferBuilder& builder,
    const entt::registry& registry,
    const std::vector<entt::entity>& dirtyEntities,
    const std::vector<entt::entity>& deletedEntities)
    -> flatbuffers::Offset<lol_at_home_shared::GameStateSnapshotFB> {
  std::vector<flatbuffers::Offset<lol_at_home_shared::EntityFB>> entityOffsets;

  if (dirtyEntities.empty()) {
    for (auto entity : registry.view<entt::entity>()) {
      auto offset = serializeEntity(builder, registry, entity);
      if (offset.has_value()) {
        entityOffsets.push_back(*offset);
      }
    }
  } else {
    for (auto entity : dirtyEntities) {
      if (!registry.valid(entity)) {
        // It was deleted; it should be in deletedEntities so we'll get to it
        continue;
      }

      auto offset = serializeEntity(builder, registry, entity);
      if (offset.has_value()) {
        entityOffsets.push_back(*offset);
      }
    }
  }

  std::vector<uint32_t> deletedIds;
  deletedIds.reserve(deletedEntities.size());
  for (auto entity : deletedEntities) {
    deletedIds.push_back(static_cast<uint32_t>(entity));
  }

  auto entitiesVector = builder.CreateVector(entityOffsets);
  auto deletedVector = builder.CreateVector(deletedIds);

  auto snapshot =
      CreateGameStateSnapshotFB(builder, entitiesVector, deletedVector);
  return snapshot;
}

}  // namespace lol_at_home_shared
