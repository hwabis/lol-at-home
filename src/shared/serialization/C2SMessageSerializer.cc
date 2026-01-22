#include "serialization/C2SMessageSerializer.h"
#include <flatbuffers/flatbuffers.h>
#include "c2s_message_generated.h"
#include "game_actions_generated.h"

using namespace lah_shared;

namespace lah::shared {

namespace {

struct AbilityTargetSerializer {
  // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
  flatbuffers::FlatBufferBuilder* builder;

  auto operator()(const NoTarget& /*target*/) const
      -> std::pair<AbilityTargetDataFB, flatbuffers::Offset<void>> {
    return {AbilityTargetDataFB::NoTargetFB,
            CreateNoTargetFB(*builder).Union()};
  }

  auto operator()(const EntityTarget& target) const
      -> std::pair<AbilityTargetDataFB, flatbuffers::Offset<void>> {
    return {AbilityTargetDataFB::EntityTargetFB,
            CreateEntityTargetFB(*builder, static_cast<uint32_t>(target.Target))
                .Union()};
  }

  auto operator()(const OnePointSkillshot& target) const
      -> std::pair<AbilityTargetDataFB, flatbuffers::Offset<void>> {
    PositionFB pos(target.targetX, target.targetY);
    return {AbilityTargetDataFB::OnePointSkillshotFB,
            CreateOnePointSkillshotFB(*builder, &pos).Union()};
  }

  auto operator()(const TwoPointSkillshot& target) const
      -> std::pair<AbilityTargetDataFB, flatbuffers::Offset<void>> {
    PositionFB pos1(target.target1X, target.target1Y);
    PositionFB pos2(target.target2X, target.target2Y);
    return {AbilityTargetDataFB::TwoPointSkillshotFB,
            CreateTwoPointSkillshotFB(*builder, &pos1, &pos2).Union()};
  }
};

struct GameActionSerializeVisitor {
  // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
  flatbuffers::FlatBufferBuilder* builder;

  auto operator()(const MoveAction& action) const
      -> flatbuffers::Offset<GameActionFB> {
    PositionFB pos(action.targetX, action.targetY);
    auto moveOffset = CreateMoveActionFB(*builder, &pos);
    return CreateGameActionFB(*builder, static_cast<uint32_t>(action.source),
                              GameActionDataFB::MoveActionFB,
                              moveOffset.Union());
  }

  auto operator()(const AbilityAction& action)
      -> flatbuffers::Offset<GameActionFB> {
    auto [targetType, targetOffset] =
        std::visit(AbilityTargetSerializer{builder}, action.target);
    auto abilityOffset =
        CreateAbilityActionFB(*builder, static_cast<AbilitySlotFB>(action.slot),
                              targetType, targetOffset);
    return CreateGameActionFB(*builder, static_cast<uint32_t>(action.source),
                              GameActionDataFB::AbilityActionFB,
                              abilityOffset.Union());
  }

  auto operator()(const AutoAttackAction& action) const
      -> flatbuffers::Offset<GameActionFB> {
    auto attackOffset = CreateAutoAttackActionFB(
        *builder, static_cast<uint32_t>(action.target));
    return CreateGameActionFB(*builder, static_cast<uint32_t>(action.source),
                              GameActionDataFB::AutoAttackActionFB,
                              attackOffset.Union());
  }

  auto operator()(const StopGameAction& action) const
      -> flatbuffers::Offset<GameActionFB> {
    auto stopOffset = CreateStopActionFB(*builder);
    return CreateGameActionFB(*builder, static_cast<uint32_t>(action.source),
                              GameActionDataFB::StopActionFB,
                              stopOffset.Union());
  }
};

auto deserializeGameActionFromFB(const GameActionFB& action)
    -> std::optional<GameActionVariant> {
  auto source = static_cast<entt::entity>(action.source());

  switch (action.action_type()) {
    case GameActionDataFB::MoveActionFB: {
      const auto* moveData = action.action_as_MoveActionFB();
      return MoveAction{.source = source,
                        .targetX = moveData->target_position()->x(),
                        .targetY = moveData->target_position()->y()};
    }

    case GameActionDataFB::AbilityActionFB: {
      const auto* abilityFB = action.action_as_AbilityActionFB();

      AbilityTargetVariant target;
      switch (abilityFB->target_type()) {
        case AbilityTargetDataFB::NoTargetFB:
          target = NoTarget{};
          break;

        case AbilityTargetDataFB::EntityTargetFB: {
          const auto* etarget = abilityFB->target_as_EntityTargetFB();
          target = EntityTarget{static_cast<entt::entity>(etarget->target())};
          break;
        }

        case AbilityTargetDataFB::OnePointSkillshotFB: {
          const auto* ops = abilityFB->target_as_OnePointSkillshotFB();
          target = OnePointSkillshot{.targetX = ops->target()->x(),
                                     .targetY = ops->target()->y()};
          break;
        }

        case AbilityTargetDataFB::TwoPointSkillshotFB: {
          const auto* tps = abilityFB->target_as_TwoPointSkillshotFB();
          target = TwoPointSkillshot{.target1X = tps->target1()->x(),
                                     .target1Y = tps->target1()->y(),
                                     .target2X = tps->target2()->x(),
                                     .target2Y = tps->target2()->y()};
          break;
        }

        case AbilityTargetDataFB::NONE: {
          return std::nullopt;
        }
      }

      return AbilityAction{.source = source,
                           .slot = static_cast<AbilitySlot>(abilityFB->slot()),
                           .target = target};
    }

    case GameActionDataFB::AutoAttackActionFB: {
      const auto* attackData = action.action_as_AutoAttackActionFB();
      return AutoAttackAction{
          .source = source,
          .target = static_cast<entt::entity>(attackData->target())};
    }

    case GameActionDataFB::StopActionFB:
      return StopGameAction{.source = source};

    case GameActionDataFB::NONE:
      return std::nullopt;
  }

  return std::nullopt;
}

}  // namespace

auto C2SMessageSerializer::GetMessageType(std::span<const std::byte> data)
    -> lah::shared::C2SMessageType {
  const auto* c2sMessage = GetC2SMessageFB(data.data());

  switch (c2sMessage->message_type()) {
    case C2SDataFB::GameActionFB:
      return lah::shared::C2SMessageType::GameAction;
    case C2SDataFB::ChampionSelectFB:
      return lah::shared::C2SMessageType::ChampionSelect;
    case C2SDataFB::ChatMessageFB:
      return lah::shared::C2SMessageType::ChatMessage;
    case C2SDataFB::NONE:
      return lah::shared::C2SMessageType::Unknown;
  }

  return lah::shared::C2SMessageType::Unknown;
}

auto C2SMessageSerializer::SerializeGameAction(const GameActionVariant& action)
    -> std::vector<std::byte> {
  flatbuffers::FlatBufferBuilder builder;

  auto gameActionOffset =
      std::visit(GameActionSerializeVisitor{&builder}, action);

  auto c2sMessage = CreateC2SMessageFB(builder, C2SDataFB::GameActionFB,
                                       gameActionOffset.Union());

  builder.Finish(c2sMessage);

  return std::vector<std::byte>(
      reinterpret_cast<const std::byte*>(builder.GetBufferPointer()),
      reinterpret_cast<const std::byte*>(builder.GetBufferPointer()) +
          builder.GetSize());
}

auto C2SMessageSerializer::SerializeChampionSelect(ChampionId champion,
                                                   Team::Color team)
    -> std::vector<std::byte> {
  flatbuffers::FlatBufferBuilder builder;

  auto championFB = static_cast<ChampionIdFB>(champion);
  auto teamFB =
      team == Team::Color::Blue ? TeamColorFB::Blue : TeamColorFB::Red;

  auto championSelect = CreateChampionSelectFB(builder, championFB, teamFB);

  auto c2sMessage = CreateC2SMessageFB(builder, C2SDataFB::ChampionSelectFB,
                                       championSelect.Union());

  builder.Finish(c2sMessage);

  return std::vector<std::byte>(
      reinterpret_cast<const std::byte*>(builder.GetBufferPointer()),
      reinterpret_cast<const std::byte*>(builder.GetBufferPointer()) +
          builder.GetSize());
}

auto C2SMessageSerializer::DeserializeGameAction(
    std::span<const std::byte> data) -> std::optional<GameActionVariant> {
  const auto* c2sMessage = GetC2SMessageFB(data.data());

  if (c2sMessage->message_type() != C2SDataFB::GameActionFB) {
    return std::nullopt;
  }

  const auto* action = c2sMessage->message_as_GameActionFB();
  return deserializeGameActionFromFB(*action);
}

auto C2SMessageSerializer::DeserializeChampionSelect(
    std::span<const std::byte> data) -> std::optional<ChampionSelectData> {
  const auto* c2sMessage = GetC2SMessageFB(data.data());

  if (c2sMessage->message_type() != C2SDataFB::ChampionSelectFB) {
    return std::nullopt;
  }

  const auto* champSelect = c2sMessage->message_as_ChampionSelectFB();

  return ChampionSelectData{
      .championId = static_cast<ChampionId>(champSelect->champion_id()),
      .teamColor = champSelect->team_color() == TeamColorFB::Blue
                       ? Team::Color::Blue
                       : Team::Color::Red};
}

auto C2SMessageSerializer::DeserializeChatMessage(
    std::span<const std::byte> data) -> std::optional<ChatMessageData> {
  const auto* c2sMessage = GetC2SMessageFB(data.data());

  if (c2sMessage->message_type() != C2SDataFB::ChatMessageFB) {
    return std::nullopt;
  }

  const auto* chatMessage = c2sMessage->message_as_ChatMessageFB();
  return ChatMessageData{.message = chatMessage->text()->str()};
}

}  // namespace lah::shared
