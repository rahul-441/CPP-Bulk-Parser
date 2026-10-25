# CPP-Bulk-Parser

A Node.js native addon (C++ / N-API) for fast bulk CSV validation and aggregation, with benchmarks against a pure JavaScript implementation.

## What it does

| Function | Description |
|---|---|
| `summarizeCsv(path)` | Validates every row, detects duplicate SKUs, totals `qty`, and returns a small summary object |
| `parseCsvObjects(path)` | Parses a CSV into an array of JS objects with numeric conversion (kept as a comparison case) |
| `parseCsv(path)` | Parses a CSV into an array of string arrays |
| `hello()` | Smoke test that confirms the addon loads |

Example output of `summarizeCsv`:
```js
{ rows: 500000, totalQty: 25197944, invalidRows: 1072, duplicateSkus: 499 }
```

## Benchmark

Median of 7 runs after a warm-up run, Node v21.7.1, Windows x64.
Both implementations return identical results (verified in the benchmark script).

| Task | Rows | Node.js | C++ addon | Result |
|---|---|---|---|---|
| Validate + aggregate (`summarizeCsv`) | 500K | 547.8 ms | 177.2 ms | **3.09x faster** (67.7% less time) |
| Validate + aggregate (`summarizeCsv`) | 50K | 38.0 ms | 12.2 ms | **3.11x faster** (67.9% less time) |
| Parse to JS objects (`parseCsvObjects`) | 50K | 45.8 ms | 166.4 ms | C++ ~3.6x slower |

The speedup is consistent as the file grows 10x, so it is not a small-file or warm-up artifact.
The "Parse to JS objects" row was measured in an earlier test and is not part of the current `npm run bench`.

## Key finding

C++ wins when it does heavy work and returns a **small result**. When it has to build
tens of thousands of JS objects, the cost of crossing the N-API boundary (every
`obj.Set(...)` call) outweighs the faster parsing, and pure JS is faster.

**Why `summarizeCsv` is fast**
- Reads the whole file in one call
- Scans by index with `find()` instead of `stringstream` and per-cell string copies
- Uses `unordered_set` for duplicate detection
- Returns only 4 numbers to JS

**Why `parseCsvObjects` is slow**
- About 150K N-API calls to build objects (50K rows x 3 fields)
- Header key strings are recreated as JS strings for every cell
- `stringstream` and `std::stod` add overhead

## Project structure

```
src/addon.cpp               hello + parseCsv
src/addonCsvToObject.cpp    parseCsvObjects + summarizeCsv
binding.gyp                 build targets (addon, addonCsvToObject)
generate.js                 creates a test CSV with invalid rows and duplicates
bench.js                    correctness check + benchmark (C++ vs JS)
```

## Run it

```bash
npm install
npm run build
npm run generate
npm run bench
```

Requirements: Node 18+, Python 3, and a C++ toolchain
(Visual Studio Build Tools with "Desktop development with C++" on Windows,
`build-essential` on Linux, Xcode CLI tools on macOS).

## Notes and limitations
- Uses `node-gyp@10` (newer versions require Node 22+).
- CSV parsing is simplified: no quoted fields (`"Smith, John"`) and no decimal values.
- The JS version decodes the file as UTF-8 while C++ reads raw bytes.
- Results vary by machine; run `npm run bench` to reproduce on yours.