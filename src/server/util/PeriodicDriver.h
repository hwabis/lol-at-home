#pragma once

#include <chrono>
#include <memory>
#include <thread>
#include "IPeriodic.h"

namespace lah::server {

class PeriodicDriver {
 public:
  explicit PeriodicDriver(std::unique_ptr<IPeriodic> periodic, int frequencyHz)
      : periodic_(std::move(periodic)),
        period_(std::chrono::duration<double>(1.0 / frequencyHz)) {}

  virtual ~PeriodicDriver() { stop(); }

  PeriodicDriver(const PeriodicDriver&) = delete;
  auto operator=(const PeriodicDriver&) -> PeriodicDriver& = delete;
  PeriodicDriver(PeriodicDriver&&) = delete;
  auto operator=(PeriodicDriver&&) -> PeriodicDriver& = delete;

  void StartAsync() {
    driverThread_ = std::jthread([this](const std::stop_token& stoken) {
      while (!stoken.stop_requested()) {
        auto start = std::chrono::steady_clock::now();

        if (periodic_) {
          // todo this is actually being rounded
          periodic_->Cycle(
              std::chrono::duration_cast<std::chrono::milliseconds>(period_));
        }

        auto workDuration = std::chrono::steady_clock::now() - start;
        auto sleepTime = period_ - workDuration;

        if (sleepTime > std::chrono::duration<double>(0)) {
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
  std::chrono::duration<double> period_;
  std::jthread driverThread_;
};

}  // namespace lah::server
