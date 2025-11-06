#pragma once

#include <chrono>

namespace lol_at_home_server {

class IPeriodic {
 public:
  virtual ~IPeriodic() = default;
  IPeriodic(const IPeriodic&) = delete;
  auto operator=(const IPeriodic&) -> IPeriodic& = delete;
  IPeriodic(const IPeriodic&&) = delete;
  auto operator=(const IPeriodic&&) -> IPeriodic& = delete;

  virtual void Cycle(std::chrono::milliseconds timeElapsed) = 0;
};

}  // namespace lol_at_home_server
