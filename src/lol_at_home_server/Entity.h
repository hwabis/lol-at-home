#pragma once

#include "EntityStats.h"

namespace lol_at_home_server {

// An entity is anything that is used in positional game state calculations.
// E.g. characters, towers, skillshots
class Entity {
 public:
  explicit Entity(EntityStats stats) : stats_(stats) {}
  virtual ~Entity() = default;

  Entity(const Entity&) = delete;
  auto operator=(const Entity&) -> Entity& = delete;
  Entity(Entity&&) = delete;
  auto operator=(Entity&&) -> Entity& = delete;

  // Returns whether the entity's stats changed during this update
  [[nodiscard]] virtual auto Update(double deltaTimeMs) -> bool = 0;
  [[nodiscard]] auto GetStatsRef() -> EntityStats& { return stats_; }

 private:
  EntityStats stats_;
};

}  // namespace lol_at_home_server
