#include "GameStateSerializer.h"
#include <flatbuffers/flatbuffers.h>
#include "game_state_generated.h"

namespace lol_at_home_shared {

namespace {
auto serializeEntity(flatbuffers::FlatBufferBuilder& builder,
                     const entt::registry& registry,
                     entt::entity entity)
    -> std::optional<flatbuffers::Offset<Entity>> {
  PositionData posData{};
  const PositionData* posPtr = nullptr;
  if (const auto* pos = registry.try_get<Position>(entity)) {
    posData = PositionData(pos->X, pos->Y);
    posPtr = &posData;
  }

  HealthData healthData{};
  const HealthData* healthPtr = nullptr;
  if (const auto* health = registry.try_get<Health>(entity)) {
    healthData = HealthData(health->CurrentHealth, health->MaxHealth,
                            health->HealthRegenPerSec);
    healthPtr = &healthData;
  }

  ManaData manaData{};
  const ManaData* manaPtr = nullptr;
  if (const auto* mana = registry.try_get<Mana>(entity)) {
    manaData = ManaData(mana->Mana, mana->MaxMana, mana->ManaRegenPerSec);
    manaPtr = &manaData;
  }

  MovableData movableData{};
  const MovableData* movablePtr = nullptr;
  if (const auto* movable = registry.try_get<Movable>(entity)) {
    movableData = MovableData(movable->Speed);
    movablePtr = &movableData;
  }

  MovingData movingData{};
  const MovingData* movingPtr = nullptr;
  if (const auto* moving = registry.try_get<Moving>(entity)) {
    movingData = MovingData(moving->TargetPosition.X, moving->TargetPosition.Y);
    movingPtr = &movingData;
  }

  TeamData teamData{};
  const TeamData* teamPtr = nullptr;
  if (const auto* team = registry.try_get<Team>(entity)) {
    teamData = TeamData(team->TeamColor == Team::Color::Blue ? TeamColor::Blue
                                                             : TeamColor::Red);
    teamPtr = &teamData;
  }

  flatbuffers::Offset<AbilitiesData> abilitiesOffset = 0;
  if (const auto* abilities = registry.try_get<Abilities>(entity)) {
    std::vector<flatbuffers::Offset<AbilityEntry>> abilityEntries;

    for (const auto& [slot, ability] : abilities->Abilities) {
      auto tagData = static_cast<AbilityTagData>(ability.Tag);
      auto cooldown = ability.CooldownRemaining;
      auto rank = ability.Rank;
      auto charges = ability.CurrentCharges;
      auto maxCharges = ability.MaxCharges;

      AbilityData abilityData(tagData, cooldown, rank, charges, maxCharges);

      auto entry = CreateAbilityEntry(
          builder, static_cast<AbilitySlotData>(slot), &abilityData);
      abilityEntries.push_back(entry);
    }

    auto entriesVector = builder.CreateVector(abilityEntries);
    abilitiesOffset = CreateAbilitiesData(builder, entriesVector);
  }

  auto entityOffset =
      CreateEntity(builder, static_cast<uint32_t>(entity), posPtr, healthPtr,
                   manaPtr, movablePtr, movingPtr, teamPtr, abilitiesOffset);

  return entityOffset;
}
}  // namespace

auto GameStateSerializer::Serialize(const entt::registry& registry,
                                    const std::vector<entt::entity>& entities)
    -> std::vector<std::byte> {
  constexpr size_t bufferSize = 1024;
  flatbuffers::FlatBufferBuilder builder(bufferSize);

  std::vector<flatbuffers::Offset<Entity>> entityOffsets;

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
  auto snapshot =
      CreateGameStateSnapshot(builder, entitiesVector, entities.empty());

  builder.Finish(snapshot);

  auto* buf = builder.GetBufferPointer();
  auto size = builder.GetSize();
  return {
      // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
      reinterpret_cast<const std::byte*>(buf),
      // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast,cppcoreguidelines-pro-bounds-pointer-arithmetic)
      reinterpret_cast<const std::byte*>(buf) + size};
}

void GameStateSerializer::Deserialize(entt::registry& registry,
                                      const std::vector<std::byte>& data) {
  const auto* snapshot = GetGameStateSnapshot(data.data());

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
      Team::Color color = entityData->team()->color() == TeamColor::Blue
                              ? Team::Color::Blue
                              : Team::Color::Red;
      registry.emplace_or_replace<Team>(entity, color);
    }

    if (entityData->abilities() != nullptr) {
      Abilities abilities;

      for (const auto* abilityEntry : *entityData->abilities()->abilities()) {
        auto slot =
            static_cast<lol_at_home_shared::AbilitySlot>(abilityEntry->slot());
        const auto* abilityData = abilityEntry->ability();

        Abilities::Ability ability{
            .Tag =
                static_cast<lol_at_home_shared::AbilityTag>(abilityData->id()),
            .CooldownRemaining = abilityData->cooldown_remaining(),
            .Rank = abilityData->rank(),
            .CurrentCharges = abilityData->current_charges(),
            .MaxCharges = abilityData->max_charges()};

        abilities.Abilities[slot] = ability;
      }

      registry.emplace_or_replace<Abilities>(entity, std::move(abilities));
    }
  }
}

}  // namespace lol_at_home_shared
