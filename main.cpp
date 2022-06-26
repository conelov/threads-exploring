#include "range/v3/view/iota.hpp"
#include <atomic>
#include <chrono>
#include <functional>
#include <iostream>
#include <mutex>
#include <random>
#include <thread>
#include <vector>


namespace {

std::random_device rd;
std::mt19937       g(rd());
}// namespace

int main() {
  enum State : std::uint8_t {
    Wait,
    Do,
    Stop
  };

  std::atomic<State> state = Wait;
  std::mutex         mutex;
  int                global_i{};

  std::vector<std::thread> executors;
  {
    const auto handler = [&](auto&& f) {
      while (state != Stop) {
        switch (state) {
          case Do:
            std::invoke(std::forward<decltype(f)>(f));
        }
      }
    };
    using namespace ranges;
    for ([[maybe_unused]] auto i : views::iota(0, 200)) {
      executors.emplace_back(std::bind_front(handler, [&] {
        std::lock_guard const _{mutex};
        ++global_i;
      }));
      executors.emplace_back(std::bind_front(handler, [&] {
        std::lock_guard const _{mutex};
        --global_i;
      }));
    }
  }

  std::shuffle(executors.begin(), executors.end(), g);

  state = Do;
  std::this_thread::sleep_for(std::chrono::seconds{1});

  state = Stop;

  for (auto& thread : executors) {
    thread.join();
  }

  std::cout << global_i;

  return EXIT_SUCCESS;
}