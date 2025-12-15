import std;

#include "pthread.h"
#include "sched.h"

using ll = long long;

void bench_function();

// TODO: remove this so consumers can provide their own function
void bench_function() {
  // Function to benchmark
  volatile int x = 0;
  for (int i = 0; i < 1000; ++i) {
    x += i;
  }
}

// pin thread to a CPU core
void pin_thread_to_core(int core_id) {
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(core_id, &cpuset);
  pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
  std::print("Thread pinned to core {}\n", core_id);
  if (sched_getcpu() != core_id) {
    std::print("Error: Thread not running on the expected core. expected {} != "
               "actual {}\n",
               core_id, sched_getcpu());
    std::exit(1);
  }
}

// Get the p-th percentile data from a sorted vector<ll>
ll percentile(const std::vector<ll> &times_ns, double p) {
  if (times_ns.empty())
    return 0;
  size_t idx = static_cast<size_t>((p / 100.0) * times_ns.size());
  if (idx >= times_ns.size())
    idx = times_ns.size() - 1;
  else if (idx < 0)
    idx = 0;
  return static_cast<double>(times_ns[idx]);
}

// flush 256MB of cache to simulate cold caches
void flush_caches() {
  constexpr size_t flush_size =
      1 * 1024 * 1024; // 1 MB, maybe too low, but this is costly
  static std::vector<int> flush(flush_size, 1);
  volatile int sink = 0;
  for (auto &x : flush)
    sink += x;
}

int main(int argc, char **argv) {
  int iterations = 1000000;
  const int warmup = 10000;
  bool cold_path = false;
  std::vector<ll> timings;
  timings.reserve(iterations);

  std::print("Running on core: {}\n", sched_getcpu());

  bool pinnedCore = false;

  for (int i = 0; i < argc; i++) {
    if (std::string(argv[i]) == "--cold") {
      cold_path = true;
    } else if (std::string(argv[i]) == "--core" && i + 1 < argc) {
      int core_id = std::stoi(argv[i + 1]);
      pin_thread_to_core(core_id);
      pinnedCore = true;
    }
  }

  if (!pinnedCore)
    pin_thread_to_core(0);

  for (int i = 0; i < warmup; ++i) {
    bench_function();
  }

  for (int i = 0; i < iterations; ++i) {
    if (cold_path) {
      flush_caches();
      iterations = std::min(iterations, 100);
    }

    auto start = std::chrono::high_resolution_clock::now();

    bench_function();

    auto end = std::chrono::high_resolution_clock::now();
    ll ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start)
                .count();
    timings.push_back(ns);
  }

  std::sort(timings.begin(), timings.end());
  ll total = std::accumulate(timings.begin(), timings.end(), 0LL);
  double mean = double(total) / timings.size();
  double p50 = double(percentile(timings, 50.0));
  double p99 = double(percentile(timings, 99.0));
  double p999 = double(percentile(timings, 99.9));

  std::cout << "Iterations: " << iterations << "\n";
  std::cout << "Mean time: " << mean << " ns\n";
  std::cout << "p50 time: " << p50 << " ns\n";
  std::cout << "p99 time: " << p99 << " ns\n";
  std::cout << "p99.9 time: " << p999 << " ns\n";

  std::print("Finished on core: {}\n", sched_getcpu());

  return 0;
}