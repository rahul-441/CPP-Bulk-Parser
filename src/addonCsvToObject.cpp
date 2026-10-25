#include <napi.h>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_set>
#include <algorithm>

// Split one CSV line by commas
static std::vector<std::string> SplitLine(const std::string& line) {
  std::vector<std::string> cells;
  std::stringstream ss(line);
  std::string cell;
  while (std::getline(ss, cell, ',')) cells.push_back(cell);
  return cells;
}

// Returns true if the whole string is a valid integer
static bool IsInteger(const std::string& s) {
  if (s.empty()) return false;
  size_t i = (s[0] == '-') ? 1 : 0;
  if (i == s.size()) return false;
  for (; i < s.size(); i++) {
    if (s[i] < '0' || s[i] > '9') return false;
  }
  return true;
}

Napi::Value ParseCsvObjects(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  if (info.Length() < 1 || !info[0].IsString()) {
    Napi::TypeError::New(env, "Expected file path string").ThrowAsJavaScriptException();
    return env.Null();
  }

  std::string path = info[0].As<Napi::String>().Utf8Value();
  std::ifstream file(path);
  if (!file.is_open()) {
    Napi::Error::New(env, "Could not open file: " + path).ThrowAsJavaScriptException();
    return env.Null();
  }

  // First line = headers
  std::string line;
  if (!std::getline(file, line)) return Napi::Array::New(env);
  if (!line.empty() && line.back() == '\r') line.pop_back();
  std::vector<std::string> headers = SplitLine(line);

  Napi::Array result = Napi::Array::New(env);
  uint32_t r = 0;

  while (std::getline(file, line)) {
    if (!line.empty() && line.back() == '\r') line.pop_back();
    if (line.empty()) continue;

    std::vector<std::string> cells = SplitLine(line);
    Napi::Object obj = Napi::Object::New(env);

    for (size_t c = 0; c < headers.size() && c < cells.size(); c++) {
      if (IsInteger(cells[c])) {
        obj.Set(headers[c], Napi::Number::New(env, std::stod(cells[c])));
      } else {
        obj.Set(headers[c], Napi::String::New(env, cells[c]));
      }
    }
    result.Set(r++, obj);
  }
  return result;
}

Napi::Value SummarizeCsv(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  if (info.Length() < 1 || !info[0].IsString()) {
    Napi::TypeError::New(env, "Expected file path string").ThrowAsJavaScriptException();
    return env.Null();
  }

  std::string path = info[0].As<Napi::String>().Utf8Value();
  std::ifstream file(path, std::ios::binary);
  if (!file.is_open()) {
    Napi::Error::New(env, "Could not open file: " + path).ThrowAsJavaScriptException();
    return env.Null();
  }

  // Read the whole file in one go
  file.seekg(0, std::ios::end);
  size_t size = static_cast<size_t>(file.tellg());
  file.seekg(0, std::ios::beg);
  std::string data(size, '\0');
  file.read(&data[0], size);

  uint32_t rows = 0, invalid = 0, duplicates = 0;
  double totalQty = 0;
  std::unordered_set<std::string> seen;
  const size_t npos = std::string::npos;

  size_t pos = data.find('\n');          // skip header line
  pos = (pos == npos) ? data.size() : pos + 1;

  while (pos < data.size()) {
    size_t end = data.find('\n', pos);
    if (end == npos) end = data.size();
    size_t lineEnd = end;
    if (lineEnd > pos && data[lineEnd - 1] == '\r') lineEnd--;

    if (lineEnd > pos) {                 // skip blank lines
      rows++;
      size_t c1 = data.find(',', pos);
      size_t c2 = (c1 != npos && c1 < lineEnd) ? data.find(',', c1 + 1) : npos;

      if (c1 == npos || c1 >= lineEnd || c2 == npos || c2 >= lineEnd) {
        invalid++;                       // fewer than 3 columns
      } else {
        if (!seen.insert(data.substr(pos, c1 - pos)).second) duplicates++;

        size_t i = c2 + 1;
        bool neg = false, ok = i < lineEnd;
        if (ok && data[i] == '-') { neg = true; i++; ok = i < lineEnd; }
        long long q = 0;
        for (; ok && i < lineEnd; i++) {
          if (data[i] < '0' || data[i] > '9') { ok = false; break; }
          q = q * 10 + (data[i] - '0');
        }
        if (!ok) invalid++;
        else totalQty += neg ? -q : q;
      }
    }
    pos = end + 1;
  }

  Napi::Object result = Napi::Object::New(env);
  result.Set("rows", Napi::Number::New(env, rows));
  result.Set("totalQty", Napi::Number::New(env, totalQty));
  result.Set("invalidRows", Napi::Number::New(env, invalid));
  result.Set("duplicateSkus", Napi::Number::New(env, duplicates));
  return result;
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  exports.Set("parseCsvObjects", Napi::Function::New(env, ParseCsvObjects));
  exports.Set("summarizeCsv", Napi::Function::New(env, SummarizeCsv));
  return exports;
}

NODE_API_MODULE(addonCsvToObject, Init)