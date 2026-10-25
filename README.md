# CPP-Bulk-Parser

A Node.js native addon (C++ / N-API) for fast bulk CSV validation and aggregation.

## What it does
- `summarizeCsv(path)`: validates every row, detects duplicate SKUs, totals qty, and returns a small summary object
- `parseCsvObjects(path)`: parses a CSV into JS objects (kept for comparison)

## Benchmark (500K rows, median of 7 runs)
| Task | Node.js | C++ addon | Result |
|---|---|---|---|
| Validate + aggregate | XX ms | XX ms | X.Xx faster |
| Parse to JS objects | XX ms | XX ms | C++ slower |

## Key finding
C++ wins when it does heavy work and returns a small result. When it has to build
hundreds of thousands of JS objects, N-API boundary cost makes it slower than pure JS.

## Run it
```bash
npm install
npm run build
npm run generate
npm run bench
```
Requires Node 18+, Python 3, and a C++ toolchain (Visual Studio Build Tools on Windows).