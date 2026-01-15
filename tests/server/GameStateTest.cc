// NOLINTBEGIN

#include <gtest/gtest.h>
#include "core/GameState.h"

namespace lah::server {

class GameStateTest : public ::testing::Test {
 protected:
  GameState gameState_{nullptr, nullptr, 1};
};  // namespace lah::server

TEST_F(GameStateTest, s) {
  ASSERT_TRUE(true);
}

}  // namespace lah::server

// todo need end to end unit tests for each ecs component
// fbs serialization -> deserialization -> registry updated? -> cycle game state
// -> corresponding ecs system did its job?
