#pragma once

namespace lol_at_home_server {

using EntityId = int;

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
