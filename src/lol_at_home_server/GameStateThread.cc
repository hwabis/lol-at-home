#include "GameStateThread.h"

#include "Config.h"

namespace lol_at_home_server {

GameStateThread::GameStateThread(GameState startGameState)
    : gameState_(std::move(startGameState)) {}

void GameStateThread::Start() {
  isRunning_ = true;
  gameThread_ = std::jthread([this] { runAndBlockGameLoop(); });
}

void GameStateThread::Stop() {
  isRunning_ = false;
}

void GameStateThread::HandleInput(PlayerInput input) {
  std::lock_guard<std::mutex> lock(inputQueueMutex_);
  inputQueue_.push(input);
}

void GameStateThread::runAndBlockGameLoop() {
  auto lastFullStateBroadcast = std::chrono::steady_clock::now();
  auto lastFrameTime =
      std::chrono::steady_clock::now() - Config::UpdateInterval;

  while (isRunning_) {
    auto frameStart = std::chrono::steady_clock::now();
    auto deltaTimeMs =
        std::chrono::duration<double, std::milli>(frameStart - lastFrameTime)
            .count();
    lastFrameTime = frameStart;

    auto inputs = getAndClearQueuedInputs();
    for (const auto& input : inputs) {
      gameState_.Process(input);
    }
    gameState_.Update(deltaTimeMs);
    broadcastDeltaGameState(gameState_.GetDeltaSincePrevUpdate());

    if (frameStart - lastFullStateBroadcast >=
        Config::FullStateBroadcastInterval) {
      broadcastFullGameState(gameState_.GetFullGameState());
      lastFullStateBroadcast = frameStart;
    }

    auto frameEnd = std::chrono::steady_clock::now();
    auto elapsed = frameEnd - frameStart;
    if (elapsed < Config::UpdateInterval) {
      std::this_thread::sleep_for(Config::UpdateInterval - elapsed);
    }
  }
}

auto GameStateThread::getAndClearQueuedInputs() -> std::vector<PlayerInput> {
  std::vector<PlayerInput> inputs;

  {
    std::lock_guard<std::mutex> lock(inputQueueMutex_);
    while (!inputQueue_.empty()) {
      inputs.push_back(inputQueue_.front());
      inputQueue_.pop();
    }
  }

  return inputs;
}

void GameStateThread::broadcastDeltaGameState(
    const std::vector<GameStateDelta>&) {
  // todo
}

void GameStateThread::broadcastFullGameState(
    const std::unordered_map<EntityId, std::unique_ptr<Entity>>& gameState) {
  // todo
}

};  // namespace lol_at_home_server
