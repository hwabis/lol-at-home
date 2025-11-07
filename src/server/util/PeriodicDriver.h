#pragma once

#include <thread>
#include "IPeriodic.h"

namespace lol_at_home_server {

class PeriodicDriver {
 public:
  PeriodicDriver(std::unique_ptr<IPeriodic> periodic,
                 std::chrono::milliseconds period)
      : periodic_(std::move(periodic)), period_(period) {}

  virtual ~PeriodicDriver() { stop(); }

  PeriodicDriver(const PeriodicDriver&) = delete;
  auto operator=(const PeriodicDriver&) -> PeriodicDriver& = delete;
  PeriodicDriver(const PeriodicDriver&&) = delete;
  auto operator=(const PeriodicDriver&&) -> PeriodicDriver& = delete;

  void StartAsync() {
    driverThread_ = std::jthread([this](const std::stop_token& stoken) {
      auto lastTime = std::chrono::steady_clock::now();

      while (!stoken.stop_requested()) {
        auto start = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            start - lastTime);
        lastTime = start;

        if (periodic_) {
          periodic_->Cycle(elapsed);
        }

        auto workDuration = std::chrono::steady_clock::now() - start;
        auto sleepTime = period_ - workDuration;

        if (sleepTime > std::chrono::milliseconds(0)) {
          std::this_thread::sleep_for(sleepTime);
        }
      }
    });
  }

 private:
  void stop() {
    if (driverThread_.joinable()) {
      driverThread_.request_stop();
      driverThread_.join();
    }
  }

  std::unique_ptr<IPeriodic> periodic_;
  std::chrono::milliseconds period_;
  std::jthread driverThread_;
};

}  // namespace lol_at_home_server
