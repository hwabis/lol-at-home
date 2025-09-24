#pragma once

#include <cstdint>

namespace lol_at_home_server {

struct Position {
  double X{};
  double Y{};
};

struct Health {
  double CurrentHealth{};
  double MaxHealth{};
  double HealthRegenPerSec{};
};

struct Mana {
  double Mana{};
};

struct LinearMovement {
  Position TargetPosition;
  double Speed{};
};

struct Team {
  enum class Color : uint8_t { Blue, Red };
  Color Color;
};

struct DamagingArea {
  Team OwningTeam;
};

struct Dirty {};

}  // namespace lol_at_home_server
