const fs = require('fs');

const N = Number(process.argv[2]) || 500000;
const lines = ['sku,name,qty'];
for (let i = 1; i <= N; i++) {
  if (i % 1000 === 0) lines.push(`A${i - 1},Item${i},5`);        // duplicate SKU
  else if (i % 777 === 0) lines.push(`A${i},Item${i},abc`);      // invalid qty
  else if (i % 999 === 0) lines.push(`A${i},Item${i}`);          // missing column
  else lines.push(`A${i},Item${i},${(i % 100) + 1}`);
}
fs.writeFileSync('big.csv', lines.join('\n'));
console.log(`Wrote big.csv with ${N} rows`);