#pragma once

#include <chrono>

namespace lol_at_home_server {

struct GameStateThreadConfig {
 public:
  std::chrono::milliseconds UpdateInterval =
      std::chrono::milliseconds{defaultUpdateInterval_};
  std::chrono::seconds FullStateBroadcastInterval =
      std::chrono::seconds{defaultFullStateBroadcastInterval_};

 private:
  static constexpr int defaultUpdateInterval_ = 33;
  static constexpr int defaultFullStateBroadcastInterval_ = 33;
};

}  // namespace lol_at_home_server
