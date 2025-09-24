#pragma once

#include <chrono>

namespace lol_at_home_server {

struct GameStateThreadConfig {
 public:
  std::chrono::milliseconds UpdateInterval =
      std::chrono::milliseconds{updateInterval};
  std::chrono::seconds FullStateBroadcastInterval =
      std::chrono::seconds{fullStateBroadcastInterval};

 private:
  static constexpr int updateInterval = 33;
  static constexpr int fullStateBroadcastInterval = 33;
};

}  // namespace lol_at_home_server
