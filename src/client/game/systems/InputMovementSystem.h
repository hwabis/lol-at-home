#pragma once

#include <flatbuffers/flatbuffers.h>
#include <spdlog/spdlog.h>
#include "Components.h"
#include "IEcsSystem.h"
#include "OutboundEvent.h"
#include "ThreadSafeQueue.h"
#include "c2s_message_generated.h"
#include "domain/EcsComponents.h"

namespace lah::game {

class InputMovementSystem : public lah::engine::IEcsSystem {
 public:
  explicit InputMovementSystem(
      std::shared_ptr<ThreadSafeQueue<OutboundEvent>> outboundEvents)
      : outboundEvents_(std::move(outboundEvents)) {}

  void Cycle(entt::registry& registry,
             lah::engine::SceneInfo& info,
             std::chrono::duration<double, std::milli> /*deltaTime*/) override {
    if (!info.input.IsMouseButtonPressed(SDL_BUTTON_RIGHT)) {
      return;
    }

    auto mousePos = info.input.GetMousePosition();
    auto worldPos = info.camera.ScreenToWorld(mousePos);

    auto playerView = registry.view<LocalPlayer>();
    if (playerView.empty()) {
      spdlog::warn("No LocalPlayer entity found for InputMovementSystem");
      return;
    }

    auto playerEntity = playerView.front();
    auto& localPlayer = playerView.get<LocalPlayer>(playerEntity);

    auto* playerTeam = registry.try_get<lah::shared::Team>(playerEntity);
    if (playerTeam == nullptr) {
      spdlog::warn("No Team entity found for InputMovementSystem");
      return;
    }

    auto enemyTarget = findEnemyAtPosition(registry, worldPos.x, worldPos.y,
                                           playerTeam->color, playerEntity);

    if (enemyTarget.has_value()) {
      sendAutoAttackAction(localPlayer.serverEntityId, *enemyTarget);
    } else {
      sendMoveAction(localPlayer.serverEntityId, worldPos.x, worldPos.y);
    }
  }

 private:
  static auto findEnemyAtPosition(entt::registry& registry,
                                  float x,
                                  float y,
                                  lah::shared::Team::Color myTeam,
                                  entt::entity excludeEntity)
      -> std::optional<uint32_t> {
    auto view =
        registry
            .view<lah::shared::Position, lah::shared::Team, ServerEntityId>();

    for (auto entity : view) {
      if (entity == excludeEntity) {
        continue;
      }

      auto& team = view.get<lah::shared::Team>(entity);
      if (team.color == myTeam) {
        continue;
      }

      auto& position = view.get<lah::shared::Position>(entity);
      float dx = position.x - x;
      float dy = position.y - y;
      float distSq = dx * dx + dy * dy;
      float radiusSq = position.championRadius * position.championRadius;

      if (distSq <= radiusSq) {
        auto& serverId = view.get<ServerEntityId>(entity);
        return serverId.id;
      }
    }

    return std::nullopt;
  }

  // todo really should be in shared?
  void sendMoveAction(uint32_t sourceEntityId, float targetX, float targetY) {
    flatbuffers::FlatBufferBuilder builder;

    auto targetPos = lah_shared::PositionFB(targetX, targetY);
    auto moveAction = lah_shared::CreateMoveActionFB(builder, &targetPos);

    auto gameAction = lah_shared::CreateGameActionFB(
        builder, sourceEntityId, lah_shared::GameActionDataFB::MoveActionFB,
        moveAction.Union());

    auto c2sMessage = lah_shared::CreateC2SMessageFB(
        builder, lah_shared::C2SDataFB::GameActionFB, gameAction.Union());

    builder.Finish(c2sMessage);

    OutboundEvent event;
    event.c2sMessage = std::vector<std::byte>(
        reinterpret_cast<const std::byte*>(builder.GetBufferPointer()),
        reinterpret_cast<const std::byte*>(builder.GetBufferPointer()) +
            builder.GetSize());
    outboundEvents_->Push(event);
  }

  void sendAutoAttackAction(uint32_t sourceEntityId, uint32_t targetEntityId) {
    flatbuffers::FlatBufferBuilder builder;

    auto autoAttackAction =
        lah_shared::CreateAutoAttackActionFB(builder, targetEntityId);

    auto gameAction = lah_shared::CreateGameActionFB(
        builder, sourceEntityId,
        lah_shared::GameActionDataFB::AutoAttackActionFB,
        autoAttackAction.Union());

    auto c2sMessage = lah_shared::CreateC2SMessageFB(
        builder, lah_shared::C2SDataFB::GameActionFB, gameAction.Union());

    builder.Finish(c2sMessage);

    OutboundEvent event;
    event.c2sMessage = std::vector<std::byte>(
        reinterpret_cast<const std::byte*>(builder.GetBufferPointer()),
        reinterpret_cast<const std::byte*>(builder.GetBufferPointer()) +
            builder.GetSize());
    outboundEvents_->Push(event);
  }

  std::shared_ptr<ThreadSafeQueue<OutboundEvent>> outboundEvents_;
};

}  // namespace lah::game
