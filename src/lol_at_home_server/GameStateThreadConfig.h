#pragma once

#include <chrono>

namespace lol_at_home_server {

class GameStateThreadConfig {
 public:
  GameStateThreadConfig(std::chrono::milliseconds updateInterval,
                        std::chrono::seconds fullStateBroadcastInterval)
      : UpdateInterval(updateInterval),
        FullStateBroadcastInterval(fullStateBroadcastInterval) {}
  std::chrono::milliseconds UpdateInterval;
  std::chrono::seconds FullStateBroadcastInterval;
};

}  // namespace lol_at_home_server
