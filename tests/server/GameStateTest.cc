// NOLINTBEGIN

#include <gtest/gtest.h>
#include "core/GameState.h"

namespace lol_at_home_server {

class GameStateTest : public ::testing::Test {
 protected:
  GameState gameState_{nullptr, nullptr, 1};
};  // namespace lol_at_home_server

TEST_F(GameStateTest, s) {
  ASSERT_TRUE(true);
}

}  // namespace lol_at_home_server

// todo need end to end unit tests for each ecs component
// fbs serialization -> deserialization -> registry updated? -> cycle game state
// -> corresponding ecs system did its job?
