#pragma once

#include <entt/entt.hpp>
#include <shared_mutex>

namespace lol_at_home_server {

class ThreadSafeRegistry {
 public:
  // todo is there a way to have someone who's writing/reading to acquire the
  // appropriate lock without manually calling

  auto GetWriteLock() -> std::unique_lock<std::shared_mutex> {
    return std::unique_lock<std::shared_mutex>(mutex_);
  }

  auto GetReadLock() const -> std::shared_lock<std::shared_mutex> {
    return std::shared_lock<std::shared_mutex>(mutex_);
  }

  auto GetRegistry() -> entt::registry& { return registry_; }
  auto GetRegistry() const -> const entt::registry& { return registry_; }

 private:
  mutable std::shared_mutex mutex_;
  entt::registry registry_;
};

}  // namespace lol_at_home_server
