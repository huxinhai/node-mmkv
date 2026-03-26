{
  "targets": [
    {
      "target_name": "mmkv",
      "sources": [
        "src/core/node_mmkv_core.cpp",
        "src/helpers/value_utils.cpp",
        "src/helpers/config_parser.cpp",
        "src/bindings/module_init.cpp",
        "src/bindings/static/admin.cpp",
        "src/bindings/static/backup.cpp",
        "src/bindings/instance/numbers.cpp",
        "src/bindings/instance/strings.cpp",
        "src/bindings/instance/buffers.cpp",
        "src/bindings/instance/keys.cpp",
        "src/bindings/instance/maintenance.cpp",
        "MMKV/Core/MMKV.cpp",
        "MMKV/Core/MMKV_IO.cpp",
        "MMKV/Core/MMKV_OSX.cpp",
        "MMKV/Core/MMKVLog.cpp",
        "MMKV/Core/CodedInputData.cpp",
        "MMKV/Core/CodedInputData_OSX.cpp",
        "MMKV/Core/CodedInputDataCrypt.cpp",
        "MMKV/Core/CodedInputDataCrypt_OSX.cpp",
        "MMKV/Core/CodedOutputData.cpp",
        "MMKV/Core/KeyValueHolder.cpp",
        "MMKV/Core/PBUtility.cpp",
        "MMKV/Core/MiniPBCoder.cpp",
        "MMKV/Core/MiniPBCoder_OSX.cpp",
        "MMKV/Core/MMBuffer.cpp",
        "MMKV/Core/MemoryFile.cpp",
        "MMKV/Core/MemoryFile_Linux.cpp",
        "MMKV/Core/MemoryFile_Win32.cpp",
        "MMKV/Core/MemoryFile_OSX.cpp",
        "MMKV/Core/ThreadLock.cpp",
        "MMKV/Core/ThreadLock_Win32.cpp",
        "MMKV/Core/InterProcessLock.cpp",
        "MMKV/Core/InterProcessLock_Win32.cpp",
        "MMKV/Core/aes/AESCrypt.cpp",
        "MMKV/Core/aes/openssl/openssl_aes_core.cpp",
        "MMKV/Core/aes/openssl/openssl_cfb128.cpp",
        "MMKV/Core/aes/openssl/openssl_md5_dgst.cpp",
        "MMKV/Core/aes/openssl/openssl_md5_one.cpp",
        "MMKV/Core/crc32/crc32_armv8.cpp",
        "MMKV/Core/crc32/zlib/crc32.cpp"
      ],
      "include_dirs": [
        "src",
        "MMKV/Core",
        "<!@(node -p \"require('node-addon-api').include\")"
      ],
      "dependencies": [
        "<!(node -p \"require('node-addon-api').gyp\")"
      ],
      "defines": [
        "NAPI_CPP_EXCEPTIONS"
      ],
      "cflags_cc": [
        "-std=c++20",
        "-fexceptions"
      ],
      "conditions": [
        [
          "OS!='win'",
          {
            "libraries": [
              "-lz"
            ]
          }
        ],
        [
          "OS=='mac'",
          {
            "defines": [
              "FORCE_POSIX"
            ]
          }
        ],
        [
          "OS=='win'",
          {
            "defines": [
              "UNICODE",
              "_UNICODE"
            ],
            "msvs_settings": {
              "VCCLCompilerTool": {
                "ExceptionHandling": 1,
                "AdditionalOptions": [
                  "/std:c++20"
                ]
              }
            }
          }
        ],
        [
          "target_arch=='arm64' and OS!='win'",
          {
            "sources": [
              "MMKV/Core/aes/openssl/openssl_aesv8-armx.S"
            ]
          }
        ]
      ],
      "xcode_settings": {
        "CLANG_CXX_LANGUAGE_STANDARD": "c++20",
        "GCC_ENABLE_CPP_EXCEPTIONS": "YES",
        "GCC_GENERATE_DEBUGGING_SYMBOLS": "NO",
        "DEAD_CODE_STRIPPING": "YES",
        "STRIP_INSTALLED_PRODUCT": "YES",
        "COPY_PHASE_STRIP": "YES",
        "GCC_INLINES_ARE_PRIVATE_EXTERN": "YES",
        "GCC_SYMBOLS_PRIVATE_EXTERN": "YES",
        "MACOSX_DEPLOYMENT_TARGET": "11.0"
      },
      "product_extension": "node"
    }
  ]
}
