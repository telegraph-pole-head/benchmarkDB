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

| Container           | DataSize | InsertTime(ms) | DeleteTime(ms) | AccessTime(ms) |
|---------------------|----------|----------------|----------------|----------------|
| unordered_map       | 500      | 0.044391       | 0.018053       | 0.012763       |
| unordered_map_fnv1a | 500      | 0.028271       | 0.018884       | 0.011761       |
| unordered_map_mod   | 500      | 0.038039       | 0.028332       | 0.020187       |
| map                 | 500      | 0.085356       | 0.061121       | 0.042107       |
| B+Tree              | 500      | 0.188263       | 0.114379       | 0.050943       |
| unordered_map       | 20000    | 2.1382         | 1.01666        | 0.742895       |
| unordered_map_fnv1a | 20000    | 1.21247        | 1.02535        | 0.626172       |
| unordered_map_mod   | 20000    | 1.6222         | 1.42183        | 1.0158         |
| map                 | 20000    | 4.03412        | 3.87443        | 3.23124        |
| B+Tree              | 20000    | 10.6816        | 6.82638        | 3.90262        |
| unordered_map       | 500000   | 123.25         | 87.0787        | 53.219         |
| unordered_map_fnv1a | 500000   | 97.6468        | 96.9151        | 49.9943        |
| unordered_map_mod   | 500000   | 119.862        | 102.85         | 81.995         |
| map                 | 500000   | 249.011        | 284.356        | 283.442        |
| B+Tree              | 500000   | 473.021        | 461.521        | 307.739        |
| unordered_map       | 10000000 | 5150.35        | 3739.65        | 1799.93        |
| unordered_map_fnv1a | 10000000 | 5136.71        | 3656.7         | 1536.33        |
| unordered_map_mod   | 10000000 | 7297.3         | 5715.3         | 4195.3         |
| map                 | 10000000 | 17898.1        | 16409          | 17490.2        |
| B+Tree              | 10000000 | 19597.6        | 19157.2        | 14931.7        |

![fig](imgs/b1Big.png)
![fig](imgs/b1Small1.png)

