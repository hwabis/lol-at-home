#pragma once

#include <chrono>

namespace lah::server {

class IPeriodic {
 public:
  IPeriodic() = default;
  virtual ~IPeriodic() = default;

  IPeriodic(const IPeriodic&) = delete;
  auto operator=(const IPeriodic&) -> IPeriodic& = delete;
  IPeriodic(IPeriodic&&) = delete;
  auto operator=(IPeriodic&&) -> IPeriodic& = delete;

  virtual void Cycle(std::chrono::milliseconds timeElapsed) = 0;
};

}  // namespace lah::server
