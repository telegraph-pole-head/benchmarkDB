#include <iostream>
#include <fstream>
#include <unordered_map>
#include <map>
#include <vector>
#include <chrono>
#include <string>
#include <sstream>
#include <algorithm>

using namespace std;
using namespace std::chrono;

struct BenchmarkResult {
    string container;
    size_t dataSize;
    double insertTime;
    double deleteTime;
    double accessTime;
};

vector<pair<string, int>> readCSV(const string& filename) {
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
BenchmarkResult benchmark(const vector<pair<string, int>>& data, size_t dataSize, const string& containerName) {
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
    for (size_t i = 0; i < dataSize; ++i) {
        map.erase(data[i].first);
    }
    end = high_resolution_clock::now();
    double deleteTime = duration_cast<nanoseconds>(end - start).count() / 1e6;

    return {containerName, dataSize, insertTime, deleteTime, accessTime};
}

void printResults(const vector<BenchmarkResult>& results) {
    cout << "Container,DataSize,InsertTime(ms),DeleteTime(ms),AccessTime(ms)" << endl;
    for (const auto& result : results) {
        cout << result.container << "," << result.dataSize << "," << result.insertTime << "," << result.deleteTime << "," << result.accessTime << endl;
    }
}

int main() {
    cout<<"Reading data from file"<<endl;
    vector<pair<string, int>> data = readCSV("../data/data.csv");
    cout<<"Data read successfully"<<endl;
    vector<size_t> scales = {500, 20000, 500000, 10000000};
    vector<BenchmarkResult> results;

    for (size_t scale : scales) {
        if (scale > data.size()) {
            cerr << "Scale " << scale << " is larger than the available data size." << endl;
            continue;
        }
        cout << "Benchmarking with scale " << scale  << "..." << endl;
        results.push_back(benchmark<unordered_map<string, int>>(data, scale, "unordered_map"));
        results.push_back(benchmark<map<string, int>>(data, scale, "map"));
    }

    printResults(results);
    return 0;
}

