#pragma once

namespace lol_at_home_server {

class Vector2 {
 public:
  // NOLINTNEXTLINE
  Vector2(double x, double y) : x_(x), y_(y) {}
  [[nodiscard]] auto GetX() const -> double { return x_; }
  [[nodiscard]] auto GetY() const -> double { return y_; }

 private:
  double x_;
  double y_;
};

}  // namespace lol_at_home_server
