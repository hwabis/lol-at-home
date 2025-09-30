#include <enet/enet.h>
#include <spdlog/spdlog.h>
#include <iostream>
#include "core/GameState.h"
#include "core/GameStateThread.h"
#include "core/GameStateThreadConfig.h"
#include "networking/EnetNetworkManager.h"

auto main() -> int {
  lol_at_home_server::GameStateThread game{
      lol_at_home_server::GameState{},
      lol_at_home_server::GameStateThreadConfig{}};
  lol_at_home_server::EnetNetworkManager net;

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
