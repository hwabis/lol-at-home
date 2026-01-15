#include "Server.h"
#include <iostream>
#include "core/EnetInterface.h"
#include "core/GameState.h"
#include "util/PeriodicDriver.h"

namespace lah::server {

auto Server::Run() -> void {
  constexpr int simulationHz = 60;

  PeriodicDriver gameStateDriver{
      std::make_unique<GameState>(incoming_, outgoing_, simulationHz),
      simulationHz};
  gameStateDriver.StartAsync();

  constexpr uint16_t port = 1111;
  PeriodicDriver networkDriver{
      std::make_unique<EnetInterface>(incoming_, outgoing_, port),
      simulationHz};
  networkDriver.StartAsync();

  // Block
  std::cin.get();
}

}  // namespace lah::server
