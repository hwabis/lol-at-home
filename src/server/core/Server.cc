#include "Server.h"
#include <chrono>
#include <iostream>
#include "util/PeriodicDriver.h"

namespace lol_at_home_server {

auto Server::Run() -> void {
  GameState gameState{incoming_, outgoing_};
  constexpr std::chrono::milliseconds interval{17};
  PeriodicDriver gameStateDriver{gameState, interval};
  gameStateDriver.StartAsync();

  PeriodicDriver networkDriver{interval};
  networkDriver.StartAsync();

  // Block
  std::cin.get();
}

}  // namespace lol_at_home_server
