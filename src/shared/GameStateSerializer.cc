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
    posData = PositionDataFB(pos->X, pos->Y);
    posPtr = &posData;
  }

  HealthDataFB HealthData{};
  const HealthDataFB* healthPtr = nullptr;
  if (const auto* health = registry.try_get<Health>(entity)) {
    HealthData = HealthDataFB(health->CurrentHealth, health->MaxHealth,
                              health->HealthRegenPerSec);
    healthPtr = &HealthData;
  }

  ManaDataFB ManaData{};
  const ManaDataFB* manaPtr = nullptr;
  if (const auto* mana = registry.try_get<Mana>(entity)) {
    ManaData = ManaDataFB(mana->Mana, mana->MaxMana, mana->ManaRegenPerSec);
    manaPtr = &ManaData;
  }

  MovableDataFB MovableData{};
  const MovableDataFB* movablePtr = nullptr;
  if (const auto* movable = registry.try_get<Movable>(entity)) {
    MovableData = MovableDataFB(movable->Speed);
    movablePtr = &MovableData;
  }

  MovingDataFB MovingData{};
  const MovingDataFB* movingPtr = nullptr;
  if (const auto* moving = registry.try_get<Moving>(entity)) {
    MovingData =
        MovingDataFB(moving->TargetPosition.X, moving->TargetPosition.Y);
    movingPtr = &MovingData;
  }

  TeamDataFB TeamData{};
  const TeamDataFB* teamPtr = nullptr;
  if (const auto* team = registry.try_get<Team>(entity)) {
    TeamData =
        TeamDataFB(team->TeamColorFB == Team::Color::Blue ? TeamColorFB::Blue
                                                          : TeamColorFB::Red);
    teamPtr = &TeamData;
  }

  flatbuffers::Offset<AbilitiesDataFB> abilitiesOffset = 0;
  if (const auto* abilities = registry.try_get<Abilities>(entity)) {
    std::vector<flatbuffers::Offset<AbilityEntryFB>> abilityEntries;

    for (const auto& [slot, ability] : abilities->Abilities) {
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
    const std::vector<entt::entity>& dirtyEntities)
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
        // todo it was deleted i guess ??? then how did it get into dirty
        // entities in the first place? and how do we communicate this to client
        continue;
      }

      auto offset = serializeEntity(builder, registry, entity);
      if (offset.has_value()) {
        entityOffsets.push_back(*offset);
      }
    }
  }

  auto entitiesVector = builder.CreateVector(entityOffsets);
  auto snapshot = CreateGameStateSnapshotFB(builder, entitiesVector);
  return snapshot;
}

auto GameStateSerializer::Serialize(const entt::registry& registry,
                                    const std::vector<entt::entity>& entities)
    -> std::vector<std::byte> {
  constexpr size_t bufferSize = 1024;
  flatbuffers::FlatBufferBuilder builder(bufferSize);

  std::vector<flatbuffers::Offset<EntityFB>> entityOffsets;

  if (entities.empty()) {
    for (auto entity : registry.view<entt::entity>()) {
      auto offset = serializeEntity(builder, registry, entity);
      if (offset.has_value()) {
        entityOffsets.push_back(*offset);
      }
    }
  } else {
    for (auto entity : entities) {
      if (!registry.valid(entity)) {
        continue;
      }

      auto offset = serializeEntity(builder, registry, entity);
      if (offset.has_value()) {
        entityOffsets.push_back(*offset);
      }
    }
  }

  auto entitiesVector = builder.CreateVector(entityOffsets);
  auto snapshot = CreateGameStateSnapshotFB(builder, entitiesVector);

  builder.Finish(snapshot);

  auto* buf = builder.GetBufferPointer();
  auto size = builder.GetSize();
  return {reinterpret_cast<const std::byte*>(buf),
          reinterpret_cast<const std::byte*>(buf) + size};
}

void GameStateSerializer::Deserialize(entt::registry& registry,
                                      const std::vector<std::byte>& data) {
  const auto* snapshot = GetGameStateSnapshotFB(data.data());

  if (snapshot == nullptr || snapshot->entities() == nullptr) {
    return;
  }

  for (const auto* entityData : *snapshot->entities()) {
    auto entity = static_cast<entt::entity>(entityData->id());

    if (!registry.valid(entity)) {
      entity = registry.create(entity);
    }

    if (entityData->position() != nullptr) {
      registry.emplace_or_replace<Position>(entity, entityData->position()->x(),
                                            entityData->position()->y());
    }

    if (entityData->health() != nullptr) {
      registry.emplace_or_replace<Health>(
          entity, entityData->health()->current_health(),
          entityData->health()->max_health(),
          entityData->health()->health_regen_per_sec());
    }

    if (entityData->mana() != nullptr) {
      registry.emplace_or_replace<Mana>(
          entity, entityData->mana()->mana(), entityData->mana()->max_mana(),
          entityData->mana()->mana_regen_per_sec());
    }

    if (entityData->movable() != nullptr) {
      registry.emplace_or_replace<Movable>(entity,
                                           entityData->movable()->speed());
    }

    if (entityData->moving() != nullptr) {
      registry.emplace_or_replace<Moving>(
          entity, Position{.X = entityData->moving()->target_x(),
                           .Y = entityData->moving()->target_y()});
    }

    if (entityData->team() != nullptr) {
      Team::Color color = entityData->team()->color() == TeamColorFB::Blue
                              ? Team::Color::Blue
                              : Team::Color::Red;
      registry.emplace_or_replace<Team>(entity, color);
    }

    if (entityData->abilities() != nullptr) {
      Abilities abilities;

      for (const auto* AbilityEntryFB : *entityData->abilities()->abilities()) {
        auto slot = static_cast<lol_at_home_shared::AbilitySlot>(
            AbilityEntryFB->slot());
        const auto* AbilityDataFB = AbilityEntryFB->ability();

        Abilities::Ability ability{
            .Tag = static_cast<lol_at_home_shared::AbilityTag>(
                AbilityDataFB->id()),
            .CooldownRemaining = AbilityDataFB->cooldown_remaining(),
            .Rank = AbilityDataFB->rank(),
            .CurrentCharges = AbilityDataFB->current_charges(),
            .MaxCharges = AbilityDataFB->max_charges()};

        abilities.Abilities[slot] = ability;
      }

      registry.emplace_or_replace<Abilities>(entity, std::move(abilities));
    }
  }
}

}  // namespace lol_at_home_shared
