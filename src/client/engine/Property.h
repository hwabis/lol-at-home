#pragma once

#include <functional>
#include <vector>

namespace lol_at_home_engine {

template <typename T>
class Property {
 public:
  class Subscription {
   public:
    Subscription(Property* prop, int subId) : prop_(prop), subId_(subId) {}

    Subscription(const Subscription&) = delete;
    auto operator=(const Subscription&) -> Subscription& = delete;

    Subscription(Subscription&& other) noexcept
        : prop_(other.prop_), subId_(other.subId_) {
      other.prop_ = nullptr;
    }

    auto operator=(Subscription&& other) noexcept -> Subscription& {
      if (this != &other) {
        if (prop_) {
          prop_->unsubscribe(subId_);
        }

        prop_ = other.prop_;
        subId_ = other.subId_;
        other.prop_ = nullptr;
      }

      return *this;
    }

    ~Subscription() {
      if (prop_) {
        prop_->unsubscribe(subId_);
      }
    }

   private:
    Property* prop_;
    int subId_;
  };

  using Callback = std::function<void(const T&)>;

  auto Subscribe(Callback callback) -> Subscription {
    int newId = ++lastId_;
    observers_.emplace_back(newId, std::move(callback));
    observers_.back().second(value_);
    return Subscription(this, newId);
  }

  auto Get() const -> const T& { return value_; }

  void Set(const T& newValue) {
    if (value_ == newValue) {
      return;
    }

    value_ = newValue;
    notify();
  }

 private:
  void notify() {
    for (auto& [_, callback] : observers_) {
      callback(value_);
    }
  }

  void unsubscribe(int subId) {
    observers_.erase(
        std::remove_if(observers_.begin(), observers_.end(),
                       [&](auto& obs) { return obs.first == subId; }),
        observers_.end());
  }

  T value_{};
  int lastId_ = 0;
  std::vector<std::pair<int, Callback>> observers_;
};

}  // namespace lol_at_home_engine
