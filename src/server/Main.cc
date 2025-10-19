#include <enet/enet.h>
#include <spdlog/spdlog.h>
#include <iostream>
#include "core/GameState.h"
#include "core/GameStateThread.h"
#include "core/GameStateThreadConfig.h"
#include "networking/EnetNetworkManager.h"

auto main() -> int {
  auto state = lol_at_home_server::GameState{};
  auto entity = state.Registry.create();
  state.Registry.emplace<lol_at_home_shared::Position>(entity, 100.0, 200.0);
  state.Registry.emplace<lol_at_home_shared::Health>(entity, 100.0, 100.0, 5.0);
  state.Registry.emplace<lol_at_home_shared::Movable>(entity, 300.0);

  lol_at_home_server::GameStateThread game{
      std::move(state), lol_at_home_server::GameStateThreadConfig{}};
  lol_at_home_server::EnetNetworkManager net;

  // Technically a race condition (game state can run before network starts) but
  // whatever
  game.Start([&net](const auto& registry, const auto& entities) {
    net.Send(registry, entities);
  });
  net.Start([&game](auto action) { game.HandleInput(action); });

  spdlog::info("Server running on port 12345");

  std::string input;
  std::getline(std::cin, input);

  spdlog::info("Shutting down server...");
  game.Stop();
  net.Stop();
  spdlog::info("Shutted down :3");
}
