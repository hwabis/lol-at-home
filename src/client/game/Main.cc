#include <thread>
#include "Components.h"
#include "Game.h"
#include "InboundEvent.h"
#include "InputSystem.h"
#include "NetworkClient.h"
#include "OutboundEvent.h"
#include "RenderSystem.h"
#include "ThreadSafeQueue.h"

namespace {

auto getScene(const std::shared_ptr<lol_at_home_game::ThreadSafeQueue<
                  lol_at_home_game::OutboundEvent>>& outboundEvents)
    -> lol_at_home_engine::Scene {
  lol_at_home_engine::Scene scene;

  scene.AddSystem(std::make_unique<lol_at_home_game::RenderSystem>());
  scene.AddSystem(
      std::make_unique<lol_at_home_game::InputSystem>(outboundEvents));

  auto joeBillyBob = scene.GetRegistry().create();
  scene.GetRegistry().emplace<lol_at_home_game::Transform>(joeBillyBob);
  scene.GetRegistry().emplace<lol_at_home_game::RenderableCircle>(joeBillyBob,
                                                                  50.0F);

  return scene;
}

auto runNetworkClient(const std::shared_ptr<lol_at_home_game::ThreadSafeQueue<
                          lol_at_home_game::InboundEvent>>& inboundEvents,
                      const std::shared_ptr<lol_at_home_game::ThreadSafeQueue<
                          lol_at_home_game::OutboundEvent>>& outboundEvents)
    -> void {
  std::jthread thread([inboundEvents, outboundEvents]() {
    lol_at_home_game::NetworkClient client(inboundEvents, outboundEvents);
    while (true) {
      client.Poll();
      std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
  });
}

}  // namespace

auto main() -> int {
  lol_at_home_engine::Game game({
      .windowTitle = "LoL at Home Client",
      .windowType = lol_at_home_engine::GameConfig::WindowType::Windowed,
      .windowSize = {.x = 1280.0F, .y = 720.0F},
      .targetFPS = 60,
  });

  std::shared_ptr<
      lol_at_home_game::ThreadSafeQueue<lol_at_home_game::InboundEvent>>
      inboundEvents = std::make_shared<
          lol_at_home_game::ThreadSafeQueue<lol_at_home_game::InboundEvent>>();
  std::shared_ptr<
      lol_at_home_game::ThreadSafeQueue<lol_at_home_game::OutboundEvent>>
      outboundEvents = std::make_shared<
          lol_at_home_game::ThreadSafeQueue<lol_at_home_game::OutboundEvent>>();

  runNetworkClient(inboundEvents, outboundEvents);
  game.Run(getScene(outboundEvents));
  return 0;
}
