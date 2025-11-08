#include "Server.h"
#include <chrono>
#include <iostream>
#include "core/EnetInterface.h"
#include "core/GameState.h"
#include "util/PeriodicDriver.h"

namespace lol_at_home_server {

auto Server::Run() -> void {
  constexpr std::chrono::milliseconds interval{17};

  PeriodicDriver gameStateDriver{
      std::make_unique<GameState>(registry_, incoming_, outgoing_), interval};
  gameStateDriver.StartAsync();

  PeriodicDriver networkDriver{
      std::make_unique<EnetInterface>(registry_, incoming_, outgoing_), interval};
  networkDriver.StartAsync();

  // Block
  std::cin.get();
}

}  // namespace lol_at_home_server
