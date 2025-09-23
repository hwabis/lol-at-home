#pragma once

#include <chrono>

namespace lol_at_home_server {

struct GameStateThreadConfig {
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
  std::chrono::milliseconds UpdateInterval = std::chrono::milliseconds{33};
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
  std::chrono::seconds FullStateBroadcastInterval = std::chrono::seconds{5};
};

}  // namespace lol_at_home_server
