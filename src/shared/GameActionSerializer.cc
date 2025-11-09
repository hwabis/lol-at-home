#include "GameActionSerializer.h"
#include <flatbuffers/flatbuffers.h>
#include "game_actions_generated.h"

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
    PositionDataFB pos(ops.Target.x, ops.Target.y);
    return CreateOnePointSkillshotFB(builder, &pos).Union();
  }

  if (std::holds_alternative<TwoPointSkillshot>(target)) {
    auto& tps = std::get<TwoPointSkillshot>(target);
    PositionDataFB pos1(tps.Target1.x, tps.Target1.y);
    PositionDataFB pos2(tps.Target2.x, tps.Target2.y);
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

auto GameActionSerializer::Deserialize(
    const lol_at_home_shared::GameActionFB* action)
    -> std::optional<GameActionVariant> {
  auto source = static_cast<entt::entity>(action->source());

  switch (action->action_type()) {
    case GameActionDataFB::MoveActionFB: {
      const auto* moveData = action->action_as_MoveActionFB();
      return MoveAction{
          .source = source,
          .targetPosition = {.x = moveData->target_position()->x(),
                             .y = moveData->target_position()->y()}};
    }

    case GameActionDataFB::AbilityActionFB: {
      const auto* AbilityDataFB = action->action_as_AbilityActionFB();

      AbilityTargetVariant target;
      switch (AbilityDataFB->target_type()) {
        case AbilityTargetDataFB::NoTargetFB:
          target = NoTarget{};
          break;

        case AbilityTargetDataFB::EntityTargetFB: {
          const auto* et = AbilityDataFB->target_as_EntityTargetFB();
          target = EntityTarget{static_cast<entt::entity>(et->target())};
          break;
        }

        case AbilityTargetDataFB::OnePointSkillshotFB: {
          const auto* ops = AbilityDataFB->target_as_OnePointSkillshotFB();
          target = OnePointSkillshot{
              {.x = ops->target()->x(), .y = ops->target()->y()}};
          break;
        }

        case AbilityTargetDataFB::TwoPointSkillshotFB: {
          const auto* tps = AbilityDataFB->target_as_TwoPointSkillshotFB();
          target = TwoPointSkillshot{
              .Target1 = {.x = tps->target1()->x(), .y = tps->target1()->y()},
              .Target2 = {.x = tps->target2()->x(), .y = tps->target2()->y()}};
          break;
        }

        default:
          throw std::runtime_error("Unknown ability target type");
      }

      return AbilityAction{
          .source = source,
          .slot = static_cast<AbilitySlot>(AbilityDataFB->slot()),
          .target = target};
    }

    case GameActionDataFB::AutoAttackActionFB: {
      const auto* attackData = action->action_as_AutoAttackActionFB();
      return AutoAttackAction{
          .source = source,
          .target = static_cast<entt::entity>(attackData->target())};
    }

    case GameActionDataFB::StopActionFB:
      return StopGameAction{.source = source};

    case GameActionDataFB::NONE:
      return std::nullopt;
  }
}

}  // namespace lol_at_home_shared
