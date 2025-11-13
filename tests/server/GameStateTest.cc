// NOLINTBEGIN

#include <gtest/gtest.h>
#include "core/GameState.h"

namespace lol_at_home_server {

class GameStateTest : public ::testing::Test {
 protected:
  GameState gameState_{nullptr, nullptr};
};  // namespace lol_at_home_server

TEST_F(GameStateTest, s) {
  ASSERT_TRUE(true);
}

}  // namespace lol_at_home_server