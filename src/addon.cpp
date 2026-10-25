#include <iostream>
#include <napi.h>
#include <fstream>
#include <sstream>
#include <string>

using namespace std;

Napi::Value ParseCsv(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  cout << "path: " << info[0].As<Napi::String>().Utf8Value() << endl;

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

  Napi::Array rows = Napi::Array::New(env);
  std::string line;
  uint32_t r = 0;

  while (std::getline(file, line)) {
    if (!line.empty() && line.back() == '\r') line.pop_back(); // Windows line endings
    if (line.empty()) continue;

    Napi::Array row = Napi::Array::New(env);
    std::stringstream ss(line);
    std::string cell;
    uint32_t c = 0;
    while (std::getline(ss, cell, ',')) {
      row.Set(c++, Napi::String::New(env, cell));
    }
    rows.Set(r++, row);
  }
  return rows;
}

Napi::String Hello(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  cout << "Message From Node : " << info[0].As<Napi::String>().Utf8Value() << endl;

  return Napi::String::New(info.Env(), "Hello from C++!");
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  exports.Set("hello", Napi::Function::New(env, Hello));
  exports.Set("parseCsv", Napi::Function::New(env, ParseCsv));
  return exports;
}

NODE_API_MODULE(addon, Init)