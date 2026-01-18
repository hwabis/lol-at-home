#include "serialization/GameStateSerializer.h"
#include <flatbuffers/flatbuffers.h>
#include "domain/EcsComponents.h"
#include "game_state_generated.h"

using namespace lah_shared;

namespace lah::shared {

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
    HealthData = HealthFB(health->current, health->max, health->regenPerSec);
    healthPtr = &HealthData;
  }

  ManaFB ManaData{};
  const ManaFB* manaPtr = nullptr;
  if (const auto* mana = registry.try_get<Mana>(entity)) {
    ManaData = ManaFB(mana->mana, mana->maxMana, mana->manaRegenPerSec);
    manaPtr = &ManaData;
  }

  MovementStatsFB MovementStatsData{};
  const MovementStatsFB* movementStatsPtr = nullptr;
  if (const auto* movementStats = registry.try_get<MovementStats>(entity)) {
    MovementStatsData = MovementStatsFB(movementStats->speed);
    movementStatsPtr = &MovementStatsData;
  }

  CharacterStateDataFB CharacterStateData{};
  const CharacterStateDataFB* characterStatePtr = nullptr;
  if (const auto* characterState = registry.try_get<CharacterState>(entity)) {
    CharacterStateFB charState{};
    switch (characterState->state) {
      case CharacterState::State::Idle:
        charState = CharacterStateFB::Idle;
        break;
      case CharacterState::State::Moving:
        charState = CharacterStateFB::Moving;
        break;
      case CharacterState::State::AutoAttackWindup:
        charState = CharacterStateFB::AutoAttackWindup;
        break;
    }

    CharacterStateData = CharacterStateDataFB(charState);
    characterStatePtr = &CharacterStateData;
  }

  MoveTargetFB MoveTargetData{};
  const MoveTargetFB* moveTargetPtr = nullptr;
  if (const auto* moveTarget = registry.try_get<MoveTarget>(entity)) {
    MoveTargetData = MoveTargetFB({moveTarget->targetX, moveTarget->targetY});
    moveTargetPtr = &MoveTargetData;
  }

  TeamFB TeamData{};
  const TeamFB* teamPtr = nullptr;
  if (const auto* team = registry.try_get<Team>(entity)) {
    TeamData = TeamFB(team->color == Team::Color::Blue ? TeamColorFB::Blue
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

  auto entityOffset = CreateEntityFB(
      builder, static_cast<uint32_t>(entity), posPtr, healthPtr, manaPtr,
      movementStatsPtr, characterStatePtr, moveTargetPtr, teamPtr, abilitiesOffset);

  return entityOffset;
}

}  // namespace

auto GameStateSerializer::Serialize(
    flatbuffers::FlatBufferBuilder& builder,
    const entt::registry& registry,
    const std::vector<entt::entity>& dirtyEntities,
    const std::vector<entt::entity>& deletedEntities)
    -> flatbuffers::Offset<lah_shared::GameStateDeltaFB> {
  std::vector<flatbuffers::Offset<lah_shared::EntityFB>> entityOffsets;

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
    const lah_shared::GameStateDeltaFB& gamestate) -> void {
  for (const auto* entityFB : *gamestate.entities()) {
    auto entity = static_cast<entt::entity>(entityFB->id());

    if (!registry.valid(entity)) {
      entity = registry.create(entity);
    }

    deserializePosition(registry, entity, entityFB->position());
    deserializeHealth(registry, entity, entityFB->health());
    deserializeMana(registry, entity, entityFB->mana());
    deserializeMovementStats(registry, entity, entityFB->movement_stats());
    deserializeCharacterState(registry, entity, entityFB->character_state());
    deserializeMoveTarget(registry, entity, entityFB->move_target());
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

void GameStateSerializer::deserializeMovementStats(
    entt::registry& registry,
    entt::entity entity,
    const MovementStatsFB* movementStats) {
  if (movementStats != nullptr) {
    registry.emplace_or_replace<MovementStats>(
        entity, MovementStats{.speed = movementStats->speed()});
  }
}

void GameStateSerializer::deserializeCharacterState(
    entt::registry& registry,
    entt::entity entity,
    const CharacterStateDataFB* characterStateData) {
  if (characterStateData != nullptr) {
    CharacterState::State state{};
    switch (characterStateData->state()) {
      case CharacterStateFB::Idle:
        state = CharacterState::State::Idle;
        break;
      case CharacterStateFB::Moving:
        state = CharacterState::State::Moving;
        break;
      case CharacterStateFB::AutoAttackWindup:
        state = CharacterState::State::AutoAttackWindup;
        break;
    }

    registry.emplace_or_replace<CharacterState>(entity, CharacterState{.state = state});
  }
}

void GameStateSerializer::deserializeMoveTarget(
    entt::registry& registry,
    entt::entity entity,
    const MoveTargetFB* moveTarget) {
  if (moveTarget != nullptr) {
    registry.emplace_or_replace<MoveTarget>(
        entity, MoveTarget{.targetX = moveTarget->target_pos().x(),
                           .targetY = moveTarget->target_pos().y()});
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
    if (abilitiesFB != nullptr && abilitiesFB->abilities() != nullptr) {
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
    }

    registry.emplace_or_replace<Abilities>(entity, abilities);
  }
}

}  // namespace lah::shared
