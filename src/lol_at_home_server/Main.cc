#include <enet/enet.h>
#include <spdlog/spdlog.h>
#include "core/GameState.h"
#include "core/GameStateThread.h"
#include "core/GameStateThreadConfig.h"
#include "networking/EnetNetworkManager.h"

auto main() -> int {
  lol_at_home_server::GameStateThread game{
      lol_at_home_server::GameState{},
      lol_at_home_server::GameStateThreadConfig{}};
  lol_at_home_server::EnetNetworkManager net;

  game.Start([&net](const auto& state) { net.Send(state); });
  net.Start([&game](auto action) { game.HandleInput(action); });

  while (true) {
    // todo have some stop mechanism idk
    std::this_thread::sleep_for(std::chrono::seconds(5));
  }

  game.Stop();
  net.Stop();
}
