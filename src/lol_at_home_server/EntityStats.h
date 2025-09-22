#pragma once

#include "Vector2.h"

namespace lol_at_home_server {

using EntityId = int;

// A mishmosh of every possible stat for an entity. Not every entity has all of
// these stats, e.g. a projectile has no health
struct EntityStats {
  EntityId Id;  // Required
  double Health;
  Vector2 CurrentPosition;
  Vector2 EndPosition;
  double MovementSpeed;
};

}  // namespace lol_at_home_server
