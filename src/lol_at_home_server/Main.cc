#include <enet/enet.h>
#include <spdlog/spdlog.h>
#include "GameStateThread.h"

auto main() -> int {
  lol_at_home_server::GameStateThread trololol;
  spdlog::info("i show spd");
  if (enet_initialize() != 0) {
    spdlog::error("Failed to initialize ENet");
    return 1;
  }
}
