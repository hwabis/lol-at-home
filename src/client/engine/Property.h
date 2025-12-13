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
    auto operator=(Subscription&&) -> Subscription& = delete;

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

  auto Subscribe(std::function<void(const T&)> callback) -> Subscription {
    int newId = ++lastId;
    observers.emplace_back(newId, std::move(callback));
    return Subscription(this, newId);
  }

  auto Get() const -> const T& { return value; }

  // todo this shouldnt be seen by non-owners
  void Set(const T& newValue) {
    if (value == newValue) {
      return;
    }

    value = newValue;
    notify();
  }

 private:
  void notify() {
    for (auto& [_, callback] : observers) {
      callback(value);
    }
  }

  void unsubscribe(int subId) {
    observers.erase(
        std::remove_if(observers.begin(), observers.end(),
                       [&](auto& obs) { return obs.first == subId; }),
        observers.end());
  }

  T value{};
  int lastId = 0;
  std::vector<std::pair<int, Callback>> observers;
};

}  // namespace lol_at_home_engine
