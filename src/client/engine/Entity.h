#pragma once

#include <entt/entt.hpp>

namespace lol_at_home_engine {

class Entity {
 public:
  Entity(entt::entity handle, entt::registry* registry)
      : handle_(handle), registry_(registry) {}

  template <typename T, typename... Args>
  auto Add(Args&&... args) -> T& {
    return registry_->emplace<T>(handle_, std::forward<Args>(args)...);
  }

  template <typename T>
  auto Get() -> T& {
    return registry_->get<T>(handle_);
  }

  template <typename T>
  [[nodiscard]] auto Has() const -> bool {
    return registry_->all_of<T>(handle_);
  }

  auto Destroy() -> void { registry_->destroy(handle_); }

 private:
  entt::entity handle_;
  entt::registry* registry_;
};

}  // namespace lol_at_home_engine
