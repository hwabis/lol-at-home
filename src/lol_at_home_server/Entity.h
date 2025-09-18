#pragma once

#include "Vector2.h"

namespace lol_at_home_server {

using EntityId = int;

class Entity {
 public:
  explicit Entity(Vector2 position);
  virtual ~Entity() = default;

  Entity(const Entity&) = delete;
  auto operator=(const Entity&) -> Entity& = delete;
  Entity(Entity&&) = delete;
  auto operator=(Entity&&) -> Entity& = delete;

  virtual void Update(double deltaTimeMs) = 0;
  [[nodiscard]] auto GetPosition() const -> Vector2 { return position_; }
  [[nodiscard]] auto GetId() const -> EntityId { return id_; }

 private:
  Vector2 position_;
  EntityId id_;
};

}  // namespace lol_at_home_server
