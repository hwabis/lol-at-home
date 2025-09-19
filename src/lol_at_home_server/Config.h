#pragma once

#include <chrono>

namespace lol_at_home_server {

class Config {
 public:
  Config() = delete;
  static constexpr auto UpdateInterval = std::chrono::milliseconds(1000 / 30);
  static constexpr auto FullStateBroadcastInterval = std::chrono::seconds(5);
};

}  // namespace lol_at_home_server
