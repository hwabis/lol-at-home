#pragma once

#include <flatbuffers/flatbuffers.h>
#include <spdlog/spdlog.h>
#include "Components.h"
#include "IEcsSystem.h"
#include "OutboundEvent.h"
#include "ThreadSafeQueue.h"
#include "c2s_message_generated.h"

namespace lol_at_home_game {

class InputSystem : public lol_at_home_engine::IEcsSystem {
 public:
  explicit InputSystem(
      std::shared_ptr<ThreadSafeQueue<OutboundEvent>> outboundEvents)
      : outboundEvents_(std::move(outboundEvents)) {}

  void Cycle(entt::registry& registry,
             lol_at_home_engine::SceneInfo& info,
             std::chrono::duration<double, std::milli> /*deltaTime*/) override {
    if (info.input.IsMouseButtonPressed(SDL_BUTTON_RIGHT)) {
      auto mousePos = info.input.GetMousePosition();
      auto worldPos = info.camera.ScreenToWorld(mousePos);

      auto view = registry.view<LocalPlayer>();
      if (view.empty()) {
        spdlog::warn("No LocalPlayer entity found for InputSystem");
        return;
      }

      auto& localPlayer = view.get<LocalPlayer>(view.front());

      flatbuffers::FlatBufferBuilder builder;

      auto targetPos = lol_at_home_shared::PositionFB(worldPos.x, worldPos.y);
      auto moveAction =
          lol_at_home_shared::CreateMoveActionFB(builder, &targetPos);

      auto gameAction = lol_at_home_shared::CreateGameActionFB(
          builder, localPlayer.serverEntityId,
          lol_at_home_shared::GameActionDataFB::MoveActionFB,
          moveAction.Union());

      auto c2sMessage = lol_at_home_shared::CreateC2SMessageFB(
          builder, lol_at_home_shared::C2SDataFB::GameActionFB,
          gameAction.Union());

      builder.Finish(c2sMessage);

      OutboundEvent event;
      event.c2sMessage = std::vector<std::byte>(
          reinterpret_cast<const std::byte*>(builder.GetBufferPointer()),
          reinterpret_cast<const std::byte*>(builder.GetBufferPointer()) +
              builder.GetSize());
      outboundEvents_->Push(event);
    }
  }

 private:
  std::shared_ptr<ThreadSafeQueue<OutboundEvent>> outboundEvents_;
};

}  // namespace lol_at_home_game
