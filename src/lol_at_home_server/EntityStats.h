#pragma once

#include <memory>
#include <unordered_map>

namespace lol_at_home_server {

class Entity;

using EntityId = int;
using GameStateEntities = std::unordered_map<EntityId, std::unique_ptr<Entity>>;

struct Vector2 {
  double X{};
  double Y{};
};

struct EntityStats {
  EntityId Id{};
  double Health{};
  Vector2 CurrentPosition;
  Vector2 EndPosition;
  double MovementSpeed{};
};

}  // namespace lol_at_home_server
