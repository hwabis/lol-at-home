#pragma once

#include "EntityStats.h"

namespace lol_at_home_server {

// An entity is anything that is used in positional game state calculations.
// E.g. characters, towers, skillshots
class Entity {
 public:
  explicit Entity(EntityStats stats);
  virtual ~Entity() = default;

  Entity(const Entity&) = delete;
  auto operator=(const Entity&) -> Entity& = delete;
  Entity(Entity&&) = delete;
  auto operator=(Entity&&) -> Entity& = delete;

  virtual void Update(double deltaTimeMs) = 0;
  [[nodiscard]] auto GetId() const -> EntityId { return id_; }
  [[nodiscard]] auto GetStatsRef() -> EntityStats& { return stats_; }

 private:
  EntityId id_;
  EntityStats stats_;
};

}  // namespace lol_at_home_server
