#include "serialization/GameActionSerializer.h"
#include <flatbuffers/flatbuffers.h>

namespace lol_at_home_shared {

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
    PositionDataFB pos(target.Target.x, target.Target.y);
    return {AbilityTargetDataFB::OnePointSkillshotFB,
            CreateOnePointSkillshotFB(*builder, &pos).Union()};
  }

  auto operator()(const TwoPointSkillshot& target) const
      -> std::pair<AbilityTargetDataFB, flatbuffers::Offset<void>> {
    PositionDataFB pos1(target.Target1.x, target.Target1.y);
    PositionDataFB pos2(target.Target2.x, target.Target2.y);
    return {AbilityTargetDataFB::TwoPointSkillshotFB,
            CreateTwoPointSkillshotFB(*builder, &pos1, &pos2).Union()};
  }
};

struct GameActionSerializeVisitor {
  // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
  flatbuffers::FlatBufferBuilder* builder;

  auto operator()(const MoveAction& action) const
      -> flatbuffers::Offset<GameActionFB> {
    PositionDataFB pos(action.targetPosition.x, action.targetPosition.y);
    auto moveOffset = CreateMoveActionFB(*builder, &pos);
    return CreateGameActionFB(*builder, static_cast<uint32_t>(action.source),
                              GameActionDataFB::MoveActionFB,
                              moveOffset.Union());
  }

  auto operator()(const AbilityAction& action)
      -> flatbuffers::Offset<GameActionFB> {
    auto [targetType, targetOffset] =
        std::visit(AbilityTargetSerializer{builder}, action.target);
    auto abilityOffset = CreateAbilityActionFB(
        *builder, static_cast<AbilitySlotDataFB>(action.slot), targetType,
        targetOffset);
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

}  // namespace

auto GameActionSerializer::Serialize(flatbuffers::FlatBufferBuilder& builder,
                                     const GameActionVariant& action)
    -> flatbuffers::Offset<lol_at_home_shared::GameActionFB> {
  return std::visit(GameActionSerializeVisitor{&builder}, action);
}

auto GameActionSerializer::Deserialize(
    const lol_at_home_shared::GameActionFB& action)
    -> std::optional<GameActionVariant> {
  auto source = static_cast<entt::entity>(action.source());

  switch (action.action_type()) {
    case GameActionDataFB::MoveActionFB: {
      const auto* moveData = action.action_as_MoveActionFB();
      return MoveAction{
          .source = source,
          .targetPosition = {.x = moveData->target_position()->x(),
                             .y = moveData->target_position()->y()}};
    }

    case GameActionDataFB::AbilityActionFB: {
      const auto* AbilityDataFB = action.action_as_AbilityActionFB();

      AbilityTargetVariant target;
      switch (AbilityDataFB->target_type()) {
        case AbilityTargetDataFB::NoTargetFB:
          target = NoTarget{};
          break;

        case AbilityTargetDataFB::EntityTargetFB: {
          const auto* etarget = AbilityDataFB->target_as_EntityTargetFB();
          target = EntityTarget{static_cast<entt::entity>(etarget->target())};
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

        case AbilityTargetDataFB::NONE: {
          return std::nullopt;
        }
      }

      return AbilityAction{
          .source = source,
          .slot = static_cast<AbilitySlot>(AbilityDataFB->slot()),
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
}

}  // namespace lol_at_home_shared
