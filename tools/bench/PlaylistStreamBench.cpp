// Synthetic microbenchmark for the M3U stream constructor in PlaylistLoader.cpp.
// Build: c++ -std=c++20 -O2 -Wall -Wextra -Werror tools/bench/PlaylistStreamBench.cpp -o /tmp/playlist-bench
// No external IPTV URLs or credentials are used.
#include <algorithm>
#include <chrono>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

static volatile size_t consumed = 0;

static long long Measure(const std::string& source, bool moveContent)
{
  std::string content(source);
  auto started = std::chrono::steady_clock::now();
  if (moveContent)
  {
    std::istringstream stream(std::move(content));
    std::string line;
    size_t lines = 0;
    while (std::getline(stream, line))
      ++lines;
    consumed = lines;
  }
  else
  {
    std::stringstream stream(content);
    std::string line;
    size_t lines = 0;
    while (std::getline(stream, line))
      ++lines;
    consumed = lines;
  }
  return std::chrono::duration_cast<std::chrono::microseconds>(
             std::chrono::steady_clock::now() - started)
      .count();
}

int main()
{
  std::string playlist;
  const std::string record =
      "#EXTINF:-1 tvg-id=\"dede\",DEDE TV\nhttps://example.invalid/stream?token=fake\n";
  for (int i = 0; i < 200000; ++i)
    playlist += record;

  std::vector<long long> copyTimes, moveTimes;
  for (int trial = 0; trial < 10; ++trial)
  {
    copyTimes.push_back(Measure(playlist, false));
    moveTimes.push_back(Measure(playlist, true));
  }
  std::sort(copyTimes.begin(), copyTimes.end());
  std::sort(moveTimes.begin(), moveTimes.end());
  std::cout << "payload_bytes=" << playlist.size()
            << " copy_and_parse_median_us=" << copyTimes[5]
            << " move_and_parse_median_us=" << moveTimes[5]
            << " rows=" << consumed << '\n';
}
