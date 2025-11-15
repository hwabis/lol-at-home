#pragma once

#include <mutex>
#include <queue>

namespace lol_at_home_server {

template <typename T>
class ThreadSafeQueue {
 public:
  ThreadSafeQueue() = default;
  virtual ~ThreadSafeQueue() = default;

  ThreadSafeQueue(const ThreadSafeQueue&) = delete;
  auto operator=(const ThreadSafeQueue&) -> ThreadSafeQueue& = delete;
  ThreadSafeQueue(ThreadSafeQueue&&) = delete;
  auto operator=(ThreadSafeQueue&&) -> ThreadSafeQueue& = delete;

  void Push(const T& value) {
    std::lock_guard<std::mutex> lock(mutex_);
    queue_.push(value);
  }

  auto PopAll() -> std::queue<T> {
    std::lock_guard<std::mutex> lock(mutex_);

    std::queue<T> result;

    while (!queue_.empty()) {
      result.push(std::move(queue_.front()));
      queue_.pop();
    }

    return result;
  }

  [[nodiscard]] auto Empty() const -> bool {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.empty();
  }

  [[nodiscard]] auto Size() const -> size_t {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.size();
  }

 private:
  std::queue<T> queue_;
  mutable std::mutex mutex_;
};

}  // namespace lol_at_home_server
