#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "BpTree.h"

using namespace std;
using namespace std::chrono;

struct BenchmarkResult {
  string container;
  size_t dataSize;
  double insertTime;
  double deleteTime;
  double accessTime;
};

vector<pair<string, int>> readCSV(const string &filename) {
  vector<pair<string, int>> data;
  ifstream file(filename);
  string line, name;
  int id;

  while (getline(file, line)) {
    stringstream ss(line);
    getline(ss, name, ',');
    ss >> id;
    data.emplace_back(name, id);
  }

  return data;
}

template <typename MapType>
BenchmarkResult benchmark(const vector<pair<string, int>> &data,
                          size_t dataSize, const string &containerName) {
  MapType map;
  auto start = high_resolution_clock::now();

  // Insert
  for (size_t i = 0; i < dataSize; ++i) {
    map[data[i].first] = data[i].second;
  }
  auto end = high_resolution_clock::now();
  double insertTime = duration_cast<nanoseconds>(end - start).count() / 1e6;

  // Access
  start = high_resolution_clock::now();
  for (size_t i = 0; i < dataSize; ++i) {
    volatile int value = map[data[i].first];
  }
  end = high_resolution_clock::now();
  double accessTime = duration_cast<nanoseconds>(end - start).count() / 1e6;

  // Delete
  start = high_resolution_clock::now();
  for (size_t i = 0; i < dataSize; i++) {
    map.erase(data[i].first);
  }
  end = high_resolution_clock::now();
  double deleteTime = duration_cast<nanoseconds>(end - start).count() / 1e6;

  return {containerName, dataSize, insertTime, deleteTime, accessTime};
}

void printResults(const vector<BenchmarkResult> &results) {
  cout << "Container,DataSize,InsertTime(ms),DeleteTime(ms),AccessTime(ms)"
       << endl;
  for (const auto &result : results) {
    cout << result.container << "," << result.dataSize << ","
         << result.insertTime << "," << result.deleteTime << ","
         << result.accessTime << endl;
  }
}

void saveResultsToCSV(const vector<BenchmarkResult> &results,
                      const string &filename) {
  ofstream file(filename);
  file << "Container,DataSize,InsertTime(ms),DeleteTime(ms),AccessTime(ms)\n";
  for (const auto &result : results) {
    file << result.container << "," << result.dataSize << ","
         << result.insertTime << "," << result.deleteTime << ","
         << result.accessTime << "\n";
  }
}

unsigned naive_mod_hash(const string &key, size_t tableSize) {
  unsigned hash = 0;
  for (char c : key) {
    hash = (hash * 256 + c) % tableSize;
  }
  return hash;
}

struct CustomHashMod {
  size_t operator()(const string &key) const {
    return naive_mod_hash(key, 10000000);
  }
};

unsigned fnv_hash_1a_32(void *key, int len) {
  unsigned char *p = static_cast<unsigned char *>(key);
  unsigned h = 0x811c9dc5;
  for (int i = 0; i < len; i++) {
    h = (h ^ p[i]) * 0x01000193;
  }
  return h;
}

struct CustomHashFNV1A {
  size_t operator()(const string &key) const {
    return fnv_hash_1a_32((void *)key.data(), key.size());
  }
};

int main() {
  cout << "Reading data from file" << endl;
  vector<pair<string, int>> data = readCSV("../data/data.csv");
  cout << "Data read successfully" << endl;
  // vector<size_t> scales = {500, 20000, 500000, 10000000};
  vector<size_t> scales(50);
  generate(scales.begin(), scales.end(),
           [n = 0]() mutable { return n += 10000 * (1 + n / 200000); });
  vector<BenchmarkResult> results;

  for (size_t scale : scales) {
    if (scale > data.size()) {
      cerr << "Scale " << scale << " is larger than the available data size."
           << endl;
      continue;
    }
    cout << "Benchmarking with scale " << scale << "..." << endl;
    results.push_back(benchmark<map<string, int>>(data, scale, "map"));
    results.push_back(
        benchmark<unordered_map<string, int>>(data, scale, "unordered_map"));
    results.push_back(benchmark<unordered_map<string, int, CustomHashFNV1A>>(
        data, scale, "unordered_map_fnv1a"));
    results.push_back(benchmark<unordered_map<string, int, CustomHashMod>>(
        data, scale, "unordered_map_mod"));
    results.push_back(benchmark<BpTree<string, int>>(data, scale, "B+Tree"));
  }

  saveResultsToCSV(results, "../data/results/benchmark1_results.csv");
  printResults(results);
  return 0;
}
