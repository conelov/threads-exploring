#include "range/v3/algorithm/for_each.hpp"
#include "range/v3/view/concat.hpp"
#include "range/v3/view/iota.hpp"
#include "range/v3/view/repeat.hpp"
#include "range/v3/view/zip.hpp"
#include <atomic>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>


namespace {

constexpr auto cutoff      = 1'000'000;
constexpr auto threadCount = 4;
}// namespace

int main() {
  bool                     ready = false;
  std::atomic<std::size_t> threadsExitCount{};
  std::mutex               mutex;
  std::condition_variable  cv;
  int                      global_i{};
  const auto               awakeCondition = [&] { return threadsExitCount == threadCount; };

  std::vector<std::thread> executors;
  {
    const auto handler = [&](auto&& val) {
      {
        std::unique_lock ul{mutex};
        cv.wait(ul, [&] { return ready; });
      }
      for (std::remove_cvref_t<decltype(cutoff)> i{}; i < cutoff; ++i) {
        global_i += std::forward<decltype(val)>(val);
      }
      ++threadsExitCount;
      if (awakeCondition()) {
        cv.notify_all();
      }
    };
    using namespace ranges;
    for (const auto [i, val] : views::concat(
           views::zip(views::iota(0, threadCount / 2), views::repeat(1)),
           views::zip(views::iota(0, threadCount / 2), views::repeat(-1)))) {
      executors.emplace_back(std::bind_front(handler, val));
    }
  }
  ready = true;
  cv.notify_all();

  {
    std::unique_lock ul{mutex};
    cv.wait(ul, awakeCondition);
  }
  ranges::for_each(executors, std::mem_fn(&std::thread::join));

  std::cout << global_i;

  return EXIT_SUCCESS;
}