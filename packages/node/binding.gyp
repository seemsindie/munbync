{
  "targets": [
    {
      "target_name": "munbync",
      "sources": [
        "src/addon.cc",
        "core/munbyn_printer.c"
      ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")",
        "core"
      ],
      "defines": [
        "NAPI_VERSION=8",
        "NAPI_DISABLE_CPP_EXCEPTIONS"
      ],
      "conditions": [
        ["OS=='win'", {
          "libraries": ["-lws2_32"]
        }],
        ["OS=='linux'", {
          "libraries": ["-lm"],
          "cflags": ["-fPIC"],
          "cflags_c": ["-std=c99"]
        }],
        ["OS=='mac'", {
          "cflags_c": ["-std=c99"],
          "xcode_settings": {
            "OTHER_CFLAGS": ["-std=c99"]
          }
        }]
      ]
    }
  ]
}
