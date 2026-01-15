#include "serialization/GameActionSerializer.h"
#include <flatbuffers/flatbuffers.h>

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
      -> flatbuffers::Offset<lah_shared::GameActionFB> {
    lah_shared::PositionFB pos(action.targetX, action.targetY);
    auto moveOffset = CreateMoveActionFB(*builder, &pos);
    return CreateGameActionFB(*builder, static_cast<uint32_t>(action.source),
                              lah_shared::GameActionDataFB::MoveActionFB,
                              moveOffset.Union());
  }

  auto operator()(const AbilityAction& action)
      -> flatbuffers::Offset<lah_shared::GameActionFB> {
    auto [targetType, targetOffset] =
        std::visit(AbilityTargetSerializer{builder}, action.target);
    auto abilityOffset = CreateAbilityActionFB(
        *builder, static_cast<lah_shared::AbilitySlotFB>(action.slot),
        targetType, targetOffset);
    return CreateGameActionFB(*builder, static_cast<uint32_t>(action.source),
                              lah_shared::GameActionDataFB::AbilityActionFB,
                              abilityOffset.Union());
  }

  auto operator()(const AutoAttackAction& action) const
      -> flatbuffers::Offset<lah_shared::GameActionFB> {
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
    -> flatbuffers::Offset<GameActionFB> {
  return std::visit(GameActionSerializeVisitor{&builder}, action);
}

auto GameActionSerializer::Deserialize(const GameActionFB& action)
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
      const auto* AbilityFB = action.action_as_AbilityActionFB();

      AbilityTargetVariant target;
      switch (AbilityFB->target_type()) {
        case AbilityTargetDataFB::NoTargetFB:
          target = NoTarget{};
          break;

        case AbilityTargetDataFB::EntityTargetFB: {
          const auto* etarget = AbilityFB->target_as_EntityTargetFB();
          target = EntityTarget{static_cast<entt::entity>(etarget->target())};
          break;
        }

        case AbilityTargetDataFB::OnePointSkillshotFB: {
          const auto* ops = AbilityFB->target_as_OnePointSkillshotFB();
          target = OnePointSkillshot{.targetX = ops->target()->x(),
                                     .targetY = ops->target()->y()};
          break;
        }

        case AbilityTargetDataFB::TwoPointSkillshotFB: {
          const auto* tps = AbilityFB->target_as_TwoPointSkillshotFB();
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
                           .slot = static_cast<AbilitySlot>(AbilityFB->slot()),
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

}  // namespace lah::shared
