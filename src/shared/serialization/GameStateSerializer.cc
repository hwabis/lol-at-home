#include "serialization/GameStateSerializer.h"
#include <flatbuffers/flatbuffers.h>
#include "domain/EcsComponents.h"
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
    MovementStateFB moveState{};
    switch (movable->state) {
      case MovementState::Idle:
        moveState = MovementStateFB::Idle;
        break;
      case MovementState::Moving:
        moveState = MovementStateFB::Moving;
        break;
    }

    MovableData =
        MovableFB(movable->speed, moveState,
                  {movable->targetPosition.x, movable->targetPosition.y});
    movablePtr = &MovableData;
  }

  TeamFB TeamData{};
  const TeamFB* teamPtr = nullptr;
  if (const auto* team = registry.try_get<Team>(entity)) {
    TeamData = TeamFB(team->teamColor == Team::Color::Blue ? TeamColorFB::Blue
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
                     manaPtr, movablePtr, teamPtr, abilitiesOffset);

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
    entt::registry& registry,
    const lol_at_home_shared::GameStateDeltaFB& gamestate) -> void {
  for (const auto* entityFB : *gamestate.entities()) {
    auto entity = static_cast<entt::entity>(entityFB->id());

    if (!registry.valid(entity)) {
      entity = registry.create(entity);
    }

    deserializePosition(registry, entity, entityFB->position());
    deserializeHealth(registry, entity, entityFB->health());
    deserializeMana(registry, entity, entityFB->mana());
    deserializeMovable(registry, entity, entityFB->movable());
    deserializeTeam(registry, entity, entityFB->team());
    deserializeAbilities(registry, entity, entityFB->abilities());
  }

  for (uint32_t deletedId : *gamestate.deleted_entity_ids()) {
    auto entity = static_cast<entt::entity>(deletedId);
    if (registry.valid(entity)) {
      registry.destroy(entity);
    }
  }
}

void GameStateSerializer::deserializePosition(entt::registry& registry,
                                              entt::entity entity,
                                              const PositionFB* pos) {
  if (pos != nullptr) {
    registry.emplace_or_replace<Position>(entity, pos->x(), pos->y());
  }
}

void GameStateSerializer::deserializeHealth(entt::registry& registry,
                                            entt::entity entity,
                                            const HealthFB* health) {
  if (health != nullptr) {
    registry.emplace_or_replace<Health>(entity, health->current_health(),
                                        health->max_health(),
                                        health->health_regen_per_sec());
  }
}

void GameStateSerializer::deserializeMana(entt::registry& registry,
                                          entt::entity entity,
                                          const ManaFB* mana) {
  if (mana != nullptr) {
    registry.emplace_or_replace<Mana>(entity, mana->mana(), mana->max_mana(),
                                      mana->mana_regen_per_sec());
  }
}

void GameStateSerializer::deserializeMovable(entt::registry& registry,
                                             entt::entity entity,
                                             const MovableFB* movable) {
  if (movable != nullptr) {
    MovementState moveState{};
    switch (movable->state()) {
      case MovementStateFB::Idle:
        moveState = MovementState::Idle;
        break;
      case MovementStateFB::Moving:
        moveState = MovementState::Moving;
        break;
    }

    registry.emplace_or_replace<Movable>(
        entity, Movable{.speed = movable->speed(),
                        .state = moveState,
                        .targetPosition = {.x = movable->target_pos().x(),
                                           .y = movable->target_pos().y()}});
  }
}

void GameStateSerializer::deserializeTeam(entt::registry& registry,
                                          entt::entity entity,
                                          const TeamFB* team) {
  if (team != nullptr) {
    registry.emplace_or_replace<Team>(entity, team->color() == TeamColorFB::Blue
                                                  ? Team::Color::Blue
                                                  : Team::Color::Red);
  }
}

void GameStateSerializer::deserializeAbilities(entt::registry& registry,
                                               entt::entity entity,
                                               const AbilitiesFB* abilitiesFB) {
  if (abilitiesFB != nullptr) {
    Abilities abilities;
    for (const auto* abilityEntryFB : *abilitiesFB->abilities()) {
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

}  // namespace lol_at_home_shared
