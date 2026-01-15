#include <thread>
#include "Components.h"
#include "Game.h"
#include "InboundEvent.h"
#include "NetworkClient.h"
#include "OutboundEvent.h"
#include "ThreadSafeQueue.h"
#include "systems/InputSystem.h"
#include "systems/NetworkSyncSystem.h"
#include "systems/RenderSystem.h"

namespace {

auto getScene(const std::shared_ptr<lah::game::ThreadSafeQueue<
                  lah::game::InboundEvent>>& inboundEvents,
              const std::shared_ptr<lah::game::ThreadSafeQueue<
                  lah::game::OutboundEvent>>& outboundEvents)
    -> lah::engine::Scene {
  lah::engine::Scene scene;

  scene.AddSystem(
      std::make_unique<lah::game::NetworkSyncSystem>(inboundEvents));
  scene.AddSystem(std::make_unique<lah::game::RenderSystem>());
  scene.AddSystem(
      std::make_unique<lah::game::InputSystem>(outboundEvents));

  return scene;
}

auto runNetworkClient(const std::shared_ptr<lah::game::ThreadSafeQueue<
                          lah::game::InboundEvent>>& inboundEvents,
                      const std::shared_ptr<lah::game::ThreadSafeQueue<
                          lah::game::OutboundEvent>>& outboundEvents)
    -> std::jthread {
  return std::jthread([inboundEvents, outboundEvents](std::stop_token stoken) {
    lah::game::NetworkClient client(inboundEvents, outboundEvents);
    client.Connect("127.0.0.1", 1111);
    while (!stoken.stop_requested()) {
      client.Poll();
      std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
  });
}

}  // namespace

auto main() -> int {
  lah::engine::Game game({
      .windowTitle = "LoL at Home Client",
      .windowType = lah::engine::GameConfig::WindowType::Windowed,
      .windowSize = {.x = 1280.0F, .y = 720.0F},
      .targetFPS = 60,
  });

  std::shared_ptr<
      lah::game::ThreadSafeQueue<lah::game::InboundEvent>>
      inboundEvents = std::make_shared<
          lah::game::ThreadSafeQueue<lah::game::InboundEvent>>();
  std::shared_ptr<
      lah::game::ThreadSafeQueue<lah::game::OutboundEvent>>
      outboundEvents = std::make_shared<
          lah::game::ThreadSafeQueue<lah::game::OutboundEvent>>();

  auto networkThread = runNetworkClient(inboundEvents, outboundEvents);
  game.Run(getScene(inboundEvents, outboundEvents));
  return 0;
}
