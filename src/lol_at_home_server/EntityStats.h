#pragma once

#include <memory>
#include <optional>
#include <unordered_map>
#include <variant>

namespace lol_at_home_server {

class Entity;

using EntityId = int;
using GameStateEntities = std::unordered_map<EntityId, std::unique_ptr<Entity>>;

struct Vector2 {
  double X{};
  double Y{};
};

// Warning: all stats implementations are duck typed. Fields with the same
// semantic meaning must have the exact same name and type
struct EntityStats {
  EntityId Id{};
};

struct ChampionStats : EntityStats {
  double Health{};
  Vector2 Position;
  std::optional<Vector2> EndPosition;  // nullopt == not moving
  double Speed{};
};

// todo minion ?????

struct ProjectileStats : EntityStats {
  Vector2 Position;
  std::optional<Vector2> EndPosition;  // nullopt == not moving
  double Speed{};
};

struct TowerStats : EntityStats {
  double Health{};
  Vector2 Position;
};

using EntityStatsVariant =
    std::variant<ChampionStats, ProjectileStats, TowerStats>;

}  // namespace lol_at_home_server
