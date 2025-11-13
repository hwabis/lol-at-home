#include "Server.h"
#include <iostream>
#include "core/EnetInterface.h"
#include "core/GameState.h"
#include "util/PeriodicDriver.h"

namespace lol_at_home_server {

auto Server::Run() -> void {
  constexpr int freqHz = 60;

  PeriodicDriver gameStateDriver{
      std::make_unique<GameState>(incoming_, outgoing_), freqHz};
  gameStateDriver.StartAsync();

  PeriodicDriver networkDriver{
      std::make_unique<EnetInterface>(incoming_, outgoing_), freqHz};
  networkDriver.StartAsync();

  // Block
  std::cin.get();
}

}  // namespace lol_at_home_server
