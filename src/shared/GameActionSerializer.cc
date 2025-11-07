#include "GameActionSerializer.h"
#include <flatbuffers/flatbuffers.h>
#include "game_actions_generated.h"

// todo what the bofa is happening in here
namespace lol_at_home_shared {

namespace {

auto serializeAbilityTarget(flatbuffers::FlatBufferBuilder& builder,
                            const AbilityTargetVariant& target)
    -> flatbuffers::Offset<void> {
  if (std::holds_alternative<NoTarget>(target)) {
    return CreateNoTargetFB(builder).Union();
  }

  if (std::holds_alternative<EntityTarget>(target)) {
    auto& et = std::get<EntityTarget>(target);
    return CreateEntityTargetFB(builder, static_cast<uint32_t>(et.Target))
        .Union();
  }

  if (std::holds_alternative<OnePointSkillshot>(target)) {
    auto& ops = std::get<OnePointSkillshot>(target);
    PositionData pos(ops.Target.X, ops.Target.Y);
    return CreateOnePointSkillshotFB(builder, &pos).Union();
  }

  if (std::holds_alternative<TwoPointSkillshot>(target)) {
    auto& tps = std::get<TwoPointSkillshot>(target);
    PositionData pos1(tps.Target1.X, tps.Target1.Y);
    PositionData pos2(tps.Target2.X, tps.Target2.Y);
    return CreateTwoPointSkillshotFB(builder, &pos1, &pos2).Union();
  }

  throw std::runtime_error("Unknown ability target type");
}

auto getAbilityTargetType(const AbilityTargetVariant& target)
    -> AbilityTargetDataFB {
  if (std::holds_alternative<NoTarget>(target)) {
    return AbilityTargetDataFB::NoTargetFB;
  }
  if (std::holds_alternative<EntityTarget>(target)) {
    return AbilityTargetDataFB::EntityTargetFB;
  }
  if (std::holds_alternative<OnePointSkillshot>(target)) {
    return AbilityTargetDataFB::OnePointSkillshotFB;
  }
  if (std::holds_alternative<TwoPointSkillshot>(target)) {
    return AbilityTargetDataFB::TwoPointSkillshotFB;
  }
  throw std::runtime_error("Unknown ability target type");
}

}  // namespace

auto GameActionSerializer::Serialize(const GameActionVariant& actionVariant)
    -> std::vector<std::byte> {
  flatbuffers::FlatBufferBuilder builder(256);

  entt::entity source{};
  flatbuffers::Offset<void> actionOffset;
  GameActionDataFB actionType;

  if (std::holds_alternative<MoveAction>(actionVariant)) {
    auto& move = std::get<MoveAction>(actionVariant);
    source = move.Source;
    PositionData pos(move.TargetPosition.X, move.TargetPosition.Y);
    actionOffset = CreateMoveActionFB(builder, &pos).Union();
    actionType = GameActionDataFB::MoveActionFB;
  } else if (std::holds_alternative<AbilityAction>(actionVariant)) {
    auto& ability = std::get<AbilityAction>(actionVariant);
    source = ability.Source;

    auto targetType = getAbilityTargetType(ability.Target);
    auto targetOffset = serializeAbilityTarget(builder, ability.Target);

    actionOffset = CreateAbilityActionFB(
                       builder, static_cast<AbilitySlotData>(ability.Slot),
                       targetType, targetOffset)
                       .Union();
    actionType = GameActionDataFB::AbilityActionFB;
  } else if (std::holds_alternative<AutoAttackAction>(actionVariant)) {
    auto& attack = std::get<AutoAttackAction>(actionVariant);
    source = attack.Source;
    actionOffset =
        CreateAutoAttackActionFB(builder, static_cast<uint32_t>(attack.Target))
            .Union();
    actionType = GameActionDataFB::AutoAttackActionFB;
  } else if (std::holds_alternative<StopGameAction>(actionVariant)) {
    auto& stop = std::get<StopGameAction>(actionVariant);
    source = stop.Source;
    actionOffset = CreateStopActionFB(builder).Union();
    actionType = GameActionDataFB::StopActionFB;
  } else {
    throw std::runtime_error("Unknown action type");
  }

  auto action = CreateGameActionFB(builder, static_cast<uint32_t>(source),
                                   actionType, actionOffset);

  builder.Finish(action);

  auto* buf = builder.GetBufferPointer();
  auto size = builder.GetSize();
  return {reinterpret_cast<const std::byte*>(buf),
          reinterpret_cast<const std::byte*>(buf) + size};
}

auto GameActionSerializer::Deserialize(const std::vector<std::byte>& data)
    -> std::optional<GameActionVariant> {
  const lol_at_home_shared::GameActionFB* action = GetGameActionFB(data.data());
  auto source = static_cast<entt::entity>(action->source());

  switch (action->action_type()) {
    case GameActionDataFB::MoveActionFB: {
      const auto* moveData = action->action_as_MoveActionFB();
      return MoveAction{
          .Source = source,
          .TargetPosition = {.X = moveData->target_position()->x(),
                             .Y = moveData->target_position()->y()}};
    }

    case GameActionDataFB::AbilityActionFB: {
      const auto* abilityData = action->action_as_AbilityActionFB();

      AbilityTargetVariant target;
      switch (abilityData->target_type()) {
        case AbilityTargetDataFB::NoTargetFB:
          target = NoTarget{};
          break;

        case AbilityTargetDataFB::EntityTargetFB: {
          const auto* et = abilityData->target_as_EntityTargetFB();
          target = EntityTarget{static_cast<entt::entity>(et->target())};
          break;
        }

        case AbilityTargetDataFB::OnePointSkillshotFB: {
          const auto* ops = abilityData->target_as_OnePointSkillshotFB();
          target = OnePointSkillshot{{ops->target()->x(), ops->target()->y()}};
          break;
        }

        case AbilityTargetDataFB::TwoPointSkillshotFB: {
          const auto* tps = abilityData->target_as_TwoPointSkillshotFB();
          target = TwoPointSkillshot{
              .Target1 = {.X = tps->target1()->x(), .Y = tps->target1()->y()},
              .Target2 = {.X = tps->target2()->x(), .Y = tps->target2()->y()}};
          break;
        }

        default:
          throw std::runtime_error("Unknown ability target type");
      }

      return AbilityAction{
          .Source = source,
          .Slot = static_cast<AbilitySlot>(abilityData->slot()),
          .Target = target};
    }

    case GameActionDataFB::AutoAttackActionFB: {
      auto* attackData = action->action_as_AutoAttackActionFB();
      return AutoAttackAction{
          .Source = source,
          .Target = static_cast<entt::entity>(attackData->target())};
    }

    case GameActionDataFB::StopActionFB:
      return StopGameAction{.Source = source};

    case GameActionDataFB::NONE:
      return std::nullopt;
  }
}

}  // namespace lol_at_home_shared
