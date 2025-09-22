#include <enet/enet.h>
#include <spdlog/spdlog.h>
#include "GameState.h"
#include "GameStateThread.h"

auto main() -> int {
  // todo when creating entities we need like a static unique ID pool or smth
  lol_at_home_server::GameStateThread trololol{
      lol_at_home_server::GameState{{}}};
  trololol.Start();
  spdlog::info("i show spd");
  if (enet_initialize() != 0) {
    spdlog::error("Failed to initialize ENet");
    return 1;
  }
}
