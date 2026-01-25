#include "serialization/S2CMessageSerializer.h"
#include <flatbuffers/flatbuffers.h>
#include "domain/EcsComponents.h"
#include "s2c_message_generated.h"

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

  RadiusFB radiusData{};
  const RadiusFB* radiusPtr = nullptr;
  if (const auto* radius = registry.try_get<Radius>(entity)) {
    radiusData = RadiusFB(radius->radius);
    radiusPtr = &radiusData;
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
      case CharacterState::State::AutoAttackMoving:
        charState = CharacterStateFB::AutoAttackMoving;
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

  auto entityOffset =
      CreateEntityFB(builder, static_cast<uint32_t>(entity), posPtr, radiusPtr,
                     healthPtr, manaPtr, movementStatsPtr, characterStatePtr,
                     moveTargetPtr, teamPtr, abilitiesOffset);

  return entityOffset;
}

}  // namespace

auto S2CMessageSerializer::SerializeGameStateDelta(
    const entt::registry& registry,
    const std::vector<entt::entity>& dirtyEntities,
    const std::vector<entt::entity>& deletedEntities)
    -> std::vector<std::byte> {
  flatbuffers::FlatBufferBuilder builder;

  std::vector<flatbuffers::Offset<EntityFB>> entityOffsets;

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

  auto gameStateDelta =
      CreateGameStateDeltaFB(builder, entitiesVector, deletedVector);

  auto s2cMessage = CreateS2CMessageFB(builder, S2CDataFB::GameStateDeltaFB,
                                       gameStateDelta.Union());

  builder.Finish(s2cMessage);

  return {reinterpret_cast<const std::byte*>(builder.GetBufferPointer()),
          reinterpret_cast<const std::byte*>(builder.GetBufferPointer()) +
              builder.GetSize()};
}

auto S2CMessageSerializer::DeserializeGameStateDelta(
    std::span<const std::byte> data) -> std::optional<GameStateDeltaData> {
  const auto* s2cMessage = GetS2CMessageFB(data.data());

  if (s2cMessage->message_type() != S2CDataFB::GameStateDeltaFB) {
    return std::nullopt;
  }

  const auto* gamestate = s2cMessage->message_as_GameStateDeltaFB();
  GameStateDeltaData result;

  if (gamestate->entities() != nullptr) {
    for (const auto* entityFB : *gamestate->entities()) {
      EntitySnapshot snapshot;
      snapshot.entityId = entityFB->id();

      if (entityFB->position() != nullptr) {
        snapshot.position = Position{
            .x = entityFB->position()->x(),
            .y = entityFB->position()->y(),
        };
      }

      if (entityFB->radius() != nullptr) {
        snapshot.radius = Radius{
            .radius = entityFB->radius()->radius(),
        };
      }

      if (entityFB->health() != nullptr) {
        snapshot.health = Health{
            .current = entityFB->health()->current_health(),
            .max = entityFB->health()->max_health(),
            .regenPerSec = entityFB->health()->health_regen_per_sec(),
        };
      }

      if (entityFB->mana() != nullptr) {
        snapshot.mana = Mana{
            .mana = entityFB->mana()->mana(),
            .maxMana = entityFB->mana()->max_mana(),
            .manaRegenPerSec = entityFB->mana()->mana_regen_per_sec(),
        };
      }

      if (entityFB->movement_stats() != nullptr) {
        snapshot.movementStats = MovementStats{
            .speed = entityFB->movement_stats()->speed(),
        };
      }

      if (entityFB->character_state() != nullptr) {
        CharacterState::State state{};
        switch (entityFB->character_state()->state()) {
          case CharacterStateFB::Idle:
            state = CharacterState::State::Idle;
            break;
          case CharacterStateFB::Moving:
            state = CharacterState::State::Moving;
            break;
          case CharacterStateFB::AutoAttackMoving:
            state = CharacterState::State::AutoAttackMoving;
            break;
          case CharacterStateFB::AutoAttackWindup:
            state = CharacterState::State::AutoAttackWindup;
            break;
        }
        snapshot.characterState = CharacterState{.state = state};
      }

      if (entityFB->move_target() != nullptr) {
        snapshot.moveTarget = MoveTarget{
            .targetX = entityFB->move_target()->target_pos().x(),
            .targetY = entityFB->move_target()->target_pos().y(),
        };
      }

      if (entityFB->team() != nullptr) {
        snapshot.team = Team{
            .color = entityFB->team()->color() == TeamColorFB::Blue
                         ? Team::Color::Blue
                         : Team::Color::Red,
        };
      }

      if (entityFB->abilities() != nullptr &&
          entityFB->abilities()->abilities() != nullptr) {
        Abilities abilities;
        for (const auto* abilityEntryFB : *entityFB->abilities()->abilities()) {
          auto slot = static_cast<AbilitySlot>(abilityEntryFB->slot());
          const auto* abilityFB = abilityEntryFB->ability();

          Abilities::Ability ability{
              .tag = static_cast<AbilityTag>(abilityFB->id()),
              .cooldownRemaining = abilityFB->cooldown_remaining(),
              .rank = abilityFB->rank(),
              .currentCharges = abilityFB->current_charges(),
              .maxCharges = abilityFB->max_charges(),
          };

          abilities.abilities[slot] = ability;
        }
        snapshot.abilities = abilities;
      }

      result.entities.push_back(snapshot);
    }
  }

  if (gamestate->deleted_entity_ids() != nullptr) {
    for (uint32_t deletedId : *gamestate->deleted_entity_ids()) {
      result.deletedEntityIds.push_back(deletedId);
    }
  }

  return result;
}

auto S2CMessageSerializer::GetMessageType(std::span<const std::byte> data)
    -> lah::shared::S2CMessageType {
  const auto* s2cMessage = GetS2CMessageFB(data.data());

  switch (s2cMessage->message_type()) {
    case S2CDataFB::GameStateDeltaFB:
      return lah::shared::S2CMessageType::GameStateDelta;
    case S2CDataFB::PlayerAssignmentFB:
      return lah::shared::S2CMessageType::PlayerAssignment;
    case S2CDataFB::ChatBroadcastFB:
      return lah::shared::S2CMessageType::ChatBroadcast;
    case S2CDataFB::NONE:
      return lah::shared::S2CMessageType::Unknown;
  }

  return lah::shared::S2CMessageType::Unknown;
}

auto S2CMessageSerializer::SerializePlayerAssignment(uint32_t entityId)
    -> std::vector<std::byte> {
  flatbuffers::FlatBufferBuilder builder;

  auto paOffset = CreatePlayerAssignmentFB(builder, entityId);
  auto s2cMessage = CreateS2CMessageFB(builder, S2CDataFB::PlayerAssignmentFB,
                                       paOffset.Union());
  builder.Finish(s2cMessage);

  return {reinterpret_cast<const std::byte*>(builder.GetBufferPointer()),
          reinterpret_cast<const std::byte*>(builder.GetBufferPointer()) +
              builder.GetSize()};
}

auto S2CMessageSerializer::SerializeChatBroadcast(uint32_t senderEntityId,
                                                  const std::string& message)
    -> std::vector<std::byte> {
  flatbuffers::FlatBufferBuilder builder;

  auto textOffset = builder.CreateString(message);
  auto chatBroadcast =
      CreateChatBroadcastFB(builder, senderEntityId, textOffset);
  auto s2cMessage = CreateS2CMessageFB(builder, S2CDataFB::ChatBroadcastFB,
                                       chatBroadcast.Union());
  builder.Finish(s2cMessage);

  return {reinterpret_cast<const std::byte*>(builder.GetBufferPointer()),
          reinterpret_cast<const std::byte*>(builder.GetBufferPointer()) +
              builder.GetSize()};
}

auto S2CMessageSerializer::DeserializePlayerAssignment(
    std::span<const std::byte> data) -> std::optional<PlayerAssignmentData> {
  const auto* s2cMessage = GetS2CMessageFB(data.data());

  if (s2cMessage->message_type() != S2CDataFB::PlayerAssignmentFB) {
    return std::nullopt;
  }

  const auto* playerAssignment = s2cMessage->message_as_PlayerAssignmentFB();
  return PlayerAssignmentData{.assignedEntityId =
                                  playerAssignment->assigned_entity()};
}

auto S2CMessageSerializer::DeserializeChatBroadcast(
    std::span<const std::byte> data) -> std::optional<ChatBroadcastData> {
  const auto* s2cMessage = GetS2CMessageFB(data.data());

  if (s2cMessage->message_type() != S2CDataFB::ChatBroadcastFB) {
    return std::nullopt;
  }

  const auto* chatBroadcast = s2cMessage->message_as_ChatBroadcastFB();
  return ChatBroadcastData{.senderEntityId = chatBroadcast->sender_entity(),
                           .message = chatBroadcast->text()->str()};
}

}  // namespace lah::shared
