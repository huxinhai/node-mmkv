{
  "targets": [
    {
      "target_name": "node_mmkv",
      "sources": [
        "src/node_mmkv.cpp",
        "MMKV/Core/MMKV.cpp",
        "MMKV/Core/MMKV_IO.cpp",
        "MMKV/Core/MMKVLog.cpp",
        "MMKV/Core/CodedInputData.cpp",
        "MMKV/Core/CodedInputDataCrypt.cpp",
        "MMKV/Core/CodedOutputData.cpp",
        "MMKV/Core/KeyValueHolder.cpp",
        "MMKV/Core/PBUtility.cpp",
        "MMKV/Core/MiniPBCoder.cpp",
        "MMKV/Core/MMBuffer.cpp",
        "MMKV/Core/MemoryFile.cpp",
        "MMKV/Core/MemoryFile_Linux.cpp",
        "MMKV/Core/MemoryFile_OSX.cpp",
        "MMKV/Core/ThreadLock.cpp",
        "MMKV/Core/InterProcessLock.cpp",
        "MMKV/Core/aes/AESCrypt.cpp",
        "MMKV/Core/aes/openssl/openssl_aes_core.cpp",
        "MMKV/Core/aes/openssl/openssl_cfb128.cpp",
        "MMKV/Core/aes/openssl/openssl_md5_dgst.cpp",
        "MMKV/Core/aes/openssl/openssl_md5_one.cpp",
        "MMKV/Core/crc32/crc32_armv8.cpp",
        "MMKV/Core/crc32/zlib/crc32.cpp"
      ],
      "include_dirs": [
        "MMKV/Core",
        "<!@(node -p \"require('node-addon-api').include\")"
      ],
      "dependencies": [
        "<!(node -p \"require('node-addon-api').gyp\")"
      ],
      "defines": [
        "NAPI_CPP_EXCEPTIONS",
        "FORCE_POSIX"
      ],
      "cflags_cc": [
        "-std=c++20",
        "-fexceptions"
      ],
      "xcode_settings": {
        "CLANG_CXX_LANGUAGE_STANDARD": "c++20",
        "GCC_ENABLE_CPP_EXCEPTIONS": "YES",
        "MACOSX_DEPLOYMENT_TARGET": "11.0"
      },
      "conditions": [
        [
          "OS=='mac'",
          {
            "libraries": [
              "-lz"
            ]
          }
        ]
      ]
    }
  ]
}
