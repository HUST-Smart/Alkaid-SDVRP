# AlkaidSD: An Efficient Iterated Local Search Heuristic for the Split Delivery Vehicle Routing Problem

DIMACS-12 competition: http://dimacs.rutgers.edu/programs/challenge/vrp/vrpsd/

## Build

### Linux

```bash
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

Then the executable file is under `./build/`

### Windows (MSVC)

```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

Then the executable file is under `.\build\Release\`

## Run

```
Usage: ./alkaid-sd-main [OPTIONS] config instance output

Positionals:
  config TEXT REQUIRED        Config path
  instance TEXT REQUIRED      Instance name
  output TEXT REQUIRED        Output

Options:
  -h,--help                   Print this help message and exit
  -s INT                      Random Seed
```

## Examples

```bash
# Solve instance `p05_3070` and output solution to `out_p05_3070.txt`
$ ./alkaid-sd-main ./config/AlkaidSD.json p05_3070 out_p05_3070.txt

# time, value
0.0729653,6207
0.0941946,6112
0.112931,6018
0.190977,5806
0.242988,5671
0.258149,5668
0.328725,5643
0.376124,5579
...

$ cat out_p05_3070.txt

Route 1: 0 - 97 ( 121 ) - 187 ( 79 ) - 0
...
Route 102: 0 - 152 ( 0 ) - 173 ( 109 ) - 171 ( 91 ) - 0
5320
Intel Core i7-8700 @ 3.20GHz
1347.01
```

## Config
```json5
{
  "problem_dir": "data/",
  "random_seed": 42,
  
  // Processor type as listed in PassMark. See rule http://dimacs.rutgers.edu/files/3816/3754/6419/SDVRP-Rules.pdf
  "processor": "Intel Core i7-8700 @ 3.20GHz",
  "time_limit": 1347,

  // Possible time limit types:
  // - real
  // - cpu
  "time_limit_type": "real",

  // Possible acceptance rules:
  //
  // - type: HC (Hill Climbing)
  //
  // - type: HCWE (Hill Climbing With Equal)
  //
  // - type: LAHC (Late Acceptance Hill Climbing)
  //   length: 83
  //
  // - type: SA (Simulated Annealing)
  //   initial_temperature: 1000
  //   decay: 0.99
  //
  "acceptance_rule": {
    "type": "LAHC",
    "length": 83
  },

  // Possible ruin methods:
  //
  // - type: SISRs
  //   average_customers: 36
  //   max_length: 8
  //   split_rate: 0.740
  //   preserved_probability: 0.096
  //
  // - type: Random
  //   num_perturb_customers: [5, 6, 7]
  //
  "ruin_method": {
    "type": "SISRs",
    "average_customers": 36,
    "max_length": 8,
    "split_rate": 0.740,
    "preserved_probability": 0.096
  },

  "sorter": {
    "random": 0.078,
    "demand": 0.225,
    "far": 0.942,
    "close": 0.120
  },
  "blink_rate": 0.021,

  // Operators for HRVND (Hierarchical random variable neighborhood descent)
  // Possible inter-route operators:
  // - Swap<2, 0>
  // - Swap<2, 1>
  // - Swap<2, 2>
  // - Relocate
  // - SwapStar
  // - Cross
  // - SdSwapStar
  // - SdSwapOneOne
  // - SdSwapTwoOne
  "inter_operators": [
    [
      "Relocate",
      "Swap<2, 0>",
      "Swap<2, 1>",
      "Swap<2, 2>",
      "Cross",
      "SwapStar",
      "SdSwapStar"
    ]
  ],
  // Possible intra-route operators:
  // - Exchange
  // - OrOpt<1>
  // - OrOpt<2>
  // - OrOpt<3>
  "intra_operators": [
    "Exchange",
    "OrOpt<1>"
  ]
}
```
