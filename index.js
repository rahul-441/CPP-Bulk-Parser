const path = require('path')
const addon = require('./build/Release/addon');
const addonCsvToObject = require('./build/Release/addonCsvToObject')


// "HEllo from C++!"
console.log("Reply::Message : ", addon.hello("Say, Hi to C++"));

// Parsed CSV 
console.log("Csv Parsing :: ");
const rows = addon.parseCsv(path.join(__dirname, 'data.csv'));
console.log(rows);


// Parsing CSV to objects
console.log("Csv Parsing to object :: ");
const rowsOfObject = addonCsvToObject.parseCsvObjects(path.join(__dirname, 'data.csv'));
console.log(rowsOfObject);