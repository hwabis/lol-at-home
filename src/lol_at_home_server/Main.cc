#include <enet/enet.h>
#include <spdlog/spdlog.h>
#include "core/GameState.h"
#include "core/GameStateThread.h"
#include "core/GameStateThreadConfig.h"

auto main() -> int {
  lol_at_home_server::GameStateThread trololol{
      lol_at_home_server::GameState{},
      lol_at_home_server::GameStateThreadConfig{
          std::chrono::milliseconds(1000 / 60), std::chrono::seconds(5)},
  };
  trololol.Start();
  spdlog::info("i show spd");
  if (enet_initialize() != 0) {
    spdlog::error("Failed to initialize ENet");
    return 1;
  }
}
