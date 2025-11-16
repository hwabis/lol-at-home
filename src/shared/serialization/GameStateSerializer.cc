#include "serialization/GameStateSerializer.h"
#include <flatbuffers/flatbuffers.h>
#include "game_state_generated.h"

namespace lol_at_home_shared {

namespace {
auto serializeEntity(flatbuffers::FlatBufferBuilder& builder,
                     const entt::registry& registry,
                     entt::entity entity)
    -> std::optional<flatbuffers::Offset<EntityFB>> {
  PositionFB posData{};
  const PositionFB* posPtr = nullptr;
  if (const auto* pos = registry.try_get<Position>(entity)) {
    posData = PositionFB(pos->x, pos->y);
    posPtr = &posData;
  }

  HealthFB HealthData{};
  const HealthFB* healthPtr = nullptr;
  if (const auto* health = registry.try_get<Health>(entity)) {
    HealthData = HealthFB(health->currentHealth, health->maxHealth,
                              health->healthRegenPerSec);
    healthPtr = &HealthData;
  }

  ManaFB ManaData{};
  const ManaFB* manaPtr = nullptr;
  if (const auto* mana = registry.try_get<Mana>(entity)) {
    ManaData = ManaFB(mana->mana, mana->maxMana, mana->manaRegenPerSec);
    manaPtr = &ManaData;
  }

  MovableFB MovableData{};
  const MovableFB* movablePtr = nullptr;
  if (const auto* movable = registry.try_get<Movable>(entity)) {
    MovableData = MovableFB(movable->speed);
    movablePtr = &MovableData;
  }

  MovingDataFB MovingData{};
  const MovingDataFB* movingPtr = nullptr;
  if (const auto* moving = registry.try_get<Moving>(entity)) {
    MovingData =
        MovingDataFB(moving->targetPosition.x, moving->targetPosition.y);
    movingPtr = &MovingData;
  }

  TeamFB TeamData{};
  const TeamFB* teamPtr = nullptr;
  if (const auto* team = registry.try_get<Team>(entity)) {
    TeamData =
        TeamFB(team->teamColor == Team::Color::Blue ? TeamColorFB::Blue
                                                        : TeamColorFB::Red);
    teamPtr = &TeamData;
  }

  flatbuffers::Offset<AbilitiesFB> abilitiesOffset = 0;
  if (const auto* abilities = registry.try_get<Abilities>(entity)) {
    std::vector<flatbuffers::Offset<AbilityEntryFB>> abilityEntries;

    for (const auto& [slot, ability] : abilities->abilities) {
      auto tagData = static_cast<AbilityTagFB>(ability.tag);
      auto cooldown = ability.cooldownRemaining;
      auto rank = ability.rank;
      auto charges = ability.currentCharges;
      auto maxCharges = ability.maxCharges;

      AbilityFB abilityData(tagData, cooldown, rank, charges, maxCharges);

      auto entry = CreateAbilityEntryFB(
          builder, static_cast<AbilitySlotFB>(slot), &abilityData);
      abilityEntries.push_back(entry);
    }

    auto entriesVector = builder.CreateVector(abilityEntries);
    abilitiesOffset = CreateAbilitiesFB(builder, entriesVector);
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
    -> flatbuffers::Offset<lol_at_home_shared::GameStateDeltaFB> {
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
      CreateGameStateDeltaFB(builder, entitiesVector, deletedVector);
  return snapshot;
}

auto GameStateSerializer::Deserialize(
    const lol_at_home_shared::GameStateDeltaFB& gamestate,
    entt::registry& registry) -> void {
  // NOTE: This does not handle component removal. Once an entity has a
  // component, it keeps it forever. This matches LoL's design (turrets always
  // have health, etc.)

  for (const auto* entityFB : *gamestate.entities()) {
    auto entity = static_cast<entt::entity>(entityFB->id());

    if (!registry.valid(entity)) {
      entity = registry.create(entity);
    }

    if (entityFB->position() != nullptr) {
      registry.emplace_or_replace<Position>(entity, entityFB->position()->x(),
                                            entityFB->position()->y());
    }

    if (entityFB->health() != nullptr) {
      registry.emplace_or_replace<Health>(
          entity, entityFB->health()->current_health(),
          entityFB->health()->max_health(),
          entityFB->health()->health_regen_per_sec());
    }

    if (entityFB->mana() != nullptr) {
      registry.emplace_or_replace<Mana>(entity, entityFB->mana()->mana(),
                                        entityFB->mana()->max_mana(),
                                        entityFB->mana()->mana_regen_per_sec());
    }

    if (entityFB->movable() != nullptr) {
      registry.emplace_or_replace<Movable>(entity,
                                           entityFB->movable()->speed());
    }

    if (entityFB->moving() != nullptr) {
      registry.emplace_or_replace<Moving>(
          entity, Position{.x = entityFB->moving()->target_x(),
                           .y = entityFB->moving()->target_y()});
    }

    if (entityFB->team() != nullptr) {
      registry.emplace_or_replace<Team>(
          entity, entityFB->team()->color() == TeamColorFB::Blue
                      ? Team::Color::Blue
                      : Team::Color::Red);
    }

    if (entityFB->abilities() != nullptr) {
      Abilities abilities;
      for (const auto* abilityEntryFB : *entityFB->abilities()->abilities()) {
        auto slot = static_cast<AbilitySlot>(abilityEntryFB->slot());
        const auto* AbilityFB = abilityEntryFB->ability();

        Abilities::Ability ability{
            .tag = static_cast<AbilityTag>(AbilityFB->id()),
            .cooldownRemaining = AbilityFB->cooldown_remaining(),
            .rank = AbilityFB->rank(),
            .currentCharges = AbilityFB->current_charges(),
            .maxCharges = AbilityFB->max_charges()};

        abilities.abilities[slot] = ability;
      }
      registry.emplace_or_replace<Abilities>(entity, abilities);
    }
  }

  for (uint32_t deletedId : *gamestate.deleted_entity_ids()) {
    auto entity = static_cast<entt::entity>(deletedId);
    if (registry.valid(entity)) {
      registry.destroy(entity);
    }
  }
}

}  // namespace lol_at_home_shared
