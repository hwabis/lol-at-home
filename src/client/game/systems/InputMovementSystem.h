#pragma once

#include <spdlog/spdlog.h>
#include "Components.h"
#include "IEcsSystem.h"
#include "OutboundEvent.h"
#include "ThreadSafeQueue.h"
#include "domain/EcsComponents.h"
#include "domain/GameAction.h"
#include "serialization/C2SMessageSerializer.h"

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
    auto view = registry.view<lah::shared::Position, lah::shared::Radius,
                              lah::shared::Team, ServerEntityId>();

    for (auto entity : view) {
      if (entity == excludeEntity) {
        continue;
      }

      auto& team = view.get<lah::shared::Team>(entity);
      if (team.color == myTeam) {
        continue;
      }

      auto& position = view.get<lah::shared::Position>(entity);
      auto& radius = view.get<lah::shared::Radius>(entity);
      float dx = position.x - x;
      float dy = position.y - y;
      float distSq = dx * dx + dy * dy;
      float radiusSq = radius.radius * radius.radius;

      if (distSq <= radiusSq) {
        auto& serverId = view.get<ServerEntityId>(entity);
        return serverId.id;
      }
    }

    return std::nullopt;
  }

  void sendMoveAction(uint32_t sourceEntityId, float targetX, float targetY) {
    lah::shared::MoveAction action{
        .source = static_cast<entt::entity>(sourceEntityId),
        .targetX = targetX,
        .targetY = targetY,
    };

    OutboundEvent event;
    event.c2sMessage =
        lah::shared::C2SMessageSerializer::SerializeGameAction(action);
    outboundEvents_->Push(event);
  }

  void sendAutoAttackAction(uint32_t sourceEntityId, uint32_t targetEntityId) {
    lah::shared::AutoAttackAction action{
        .source = static_cast<entt::entity>(sourceEntityId),
        .target = static_cast<entt::entity>(targetEntityId),
    };

    OutboundEvent event;
    event.c2sMessage =
        lah::shared::C2SMessageSerializer::SerializeGameAction(action);
    outboundEvents_->Push(event);
  }

  std::shared_ptr<ThreadSafeQueue<OutboundEvent>> outboundEvents_;
};

}  // namespace lah::game
