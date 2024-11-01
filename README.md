# BenchmarkDB

## Description

this is a simple benchmarking tool for databases in cpp

1. for mapping names to integers:
   1. `std::unordered_map`
   2. `std::map`
   3. B+ tree
   4. `std::unordered_map` with alternative hash functions

2. for db insertion, deletion, and range queries:
   1. `std::unordered_map`
   2. `std::map`
   3. B+ tree
   4. `std::unordered_map` with alternative hash functions

### Scale of the data

The whole data has 10,000,000 rows and the experiments are done on 500, 20,000, 500,000 and 10,000,000 rows respectively 

### Results

| Container     | DataSize | InsertTime(ms) | DeleteTime(ms) | AccessTime(ms) |
|---------------|----------|----------------|----------------|----------------|
| unordered_map | 500      | 0.06851        | 0.043523       | 0.036541       |
| map           | 500      | 0.144066       | 0.117032       | 0.076609       |
| unordered_map | 20000    | 2.76169        | 2.0779         | 1.41781        |
| map           | 20000    | 7.16381        | 6.81802        | 4.84598        |
| unordered_map | 500000   | 118.135        | 92.5282        | 80.9376        |
| map           | 500000   | 295.526        | 320.349        | 257.784        |
| unordered_map | 10000000 | 4140.89        | 3366.52        | 2544.57        |
| map           | 10000000 | 14265.2        | 15274.4        | 14761.7        |


