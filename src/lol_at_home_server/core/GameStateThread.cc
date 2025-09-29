#include "core/GameStateThread.h"
#include <spdlog/spdlog.h>
#include <utility>

namespace lol_at_home_server {

GameStateThread::GameStateThread(GameState startGameState,
                                 GameStateThreadConfig config)
    : gameState_(std::move(startGameState)), config_(config) {}

void GameStateThread::Start(
    std::function<void(const SerializedGameState&)> broadcastFn) {
  broadcastFn_ = std::move(broadcastFn);
  isRunning_ = true;
  gameThread_ = std::jthread([this] { runGameLoop(); });
}

void GameStateThread::Stop() {
  isRunning_ = false;
}

void GameStateThread::HandleInput(GameActionVariant input) {
  if (!isRunning_) {
    spdlog::warn("what are you doing bruh");
    return;
  }

  std::lock_guard<std::mutex> lock(actionQueueMutex_);
  actionQueue_.push(input);
}

void GameStateThread::runGameLoop() {
  auto lastFullStateBroadcast = std::chrono::steady_clock::now();
  auto lastFrameTime =
      std::chrono::steady_clock::now() - config_.UpdateInterval;

  while (isRunning_) {
    auto frameStart = std::chrono::steady_clock::now();
    auto deltaTimeMs =
        std::chrono::duration<double, std::milli>(frameStart - lastFrameTime)
            .count();
    lastFrameTime = frameStart;

    auto actions = getAndClearQueuedActions();
    auto delta = gameState_.ProcessActionsAndUpdate(actions, deltaTimeMs);
    broadcastDeltaGameState(delta);

    if (frameStart - lastFullStateBroadcast >=
        config_.FullStateBroadcastInterval) {
      broadcastFullGameState(gameState_.Registry);
      lastFullStateBroadcast = frameStart;
    }

    auto frameEnd = std::chrono::steady_clock::now();
    auto elapsed = frameEnd - frameStart;
    if (elapsed < config_.UpdateInterval) {
      std::this_thread::sleep_for(config_.UpdateInterval - elapsed);
    }
  }
}

auto GameStateThread::getAndClearQueuedActions()
    -> std::vector<GameActionVariant> {
  std::vector<GameActionVariant> inputs;

  {
    std::lock_guard<std::mutex> lock(actionQueueMutex_);
    while (!actionQueue_.empty()) {
      inputs.push_back(actionQueue_.front());
      actionQueue_.pop();
    }
  }

  return inputs;
}

void GameStateThread::broadcastDeltaGameState(const GameStateDelta&) {
  // todo probably only broadcast dirty entities
  broadcastFn_({});
}

void GameStateThread::broadcastFullGameState(const entt::registry&) {
  // todo probably only broadcast entities with position component
  // NO idea yet lmao
  broadcastFn_({});
}

};  // namespace lol_at_home_server
