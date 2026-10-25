const fs = require('fs');
const path = require('path');
const cpp = require('./build/Release/addonCsvToObject');

const file = path.join(__dirname, 'big.csv');

function summarizeJS(filePath) {
  const lines = fs.readFileSync(filePath, 'utf8').split('\n');
  const seen = new Set();
  let rows = 0, invalidRows = 0, duplicateSkus = 0, totalQty = 0;
  for (let i = 1; i < lines.length; i++) {
    let line = lines[i];
    if (line.endsWith('\r')) line = line.slice(0, -1);
    if (!line) continue;
    rows++;
    const cells = line.split(',');
    if (cells.length < 3) { invalidRows++; continue; }
    if (seen.has(cells[0])) duplicateSkus++; else seen.add(cells[0]);
    if (/^-?\d+$/.test(cells[2])) totalQty += Number(cells[2]);
    else invalidRows++;
  }
  return { rows, totalQty, invalidRows, duplicateSkus };
}

function time(fn) {
  const t = process.hrtime.bigint();
  const out = fn();
  return { ms: Number(process.hrtime.bigint() - t) / 1e6, out };
}
const median = (a) => [...a].sort((x, y) => x - y)[Math.floor(a.length / 2)];

// Correctness first
const a = summarizeJS(file);
const b = cpp.summarizeCsv(file);
console.log('JS :', a);
console.log('C++:', b);
console.log('Match:', JSON.stringify(a) === JSON.stringify(b));

// Warm-up, then timed runs
summarizeJS(file); cpp.summarizeCsv(file);
const RUNS = 7, jsT = [], cppT = [];
for (let i = 0; i < RUNS; i++) {
  jsT.push(time(() => summarizeJS(file)).ms);
  cppT.push(time(() => cpp.summarizeCsv(file)).ms);
}
const js = median(jsT), cp = median(cppT);
console.log(`\nJS   median: ${js.toFixed(1)} ms`);
console.log(`C++  median: ${cp.toFixed(1)} ms`);
console.log(`Speedup: ${(js / cp).toFixed(2)}x  (${(((js - cp) / js) * 100).toFixed(1)}% less time)`);