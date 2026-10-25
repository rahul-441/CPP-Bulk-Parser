{
  "targets": [
    {
      "target_name": "addon",
      "sources": ["src/addon.cpp"],
      "include_dirs": ["<!@(node -p \"require('node-addon-api').include\")"],
      "defines": ["NAPI_DISABLE_CPP_EXCEPTIONS"]
    },

    {
        "target_name": "addonCsvToObject",
        "sources": ["src/addonCsvToObject.cpp"],
        "include_dirs": ["<!@(node -p \"require('node-addon-api').include\")"],
        "defines": ["NAPI_DISABLE_CPP_EXCEPTIONS"]
    }
  ]
}