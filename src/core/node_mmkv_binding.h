#pragma once

#include <napi.h>

#include "MMBuffer.h"
#include "MMKV.h"

#include <optional>
#include <string>
#include <vector>

namespace node_mmkv {

using MMKVNative = MMKV_NAMESPACE_PREFIX::MMKV;
using MMKVMode = MMKV_NAMESPACE_PREFIX::MMKVMode;
using MMKVConfig = MMKV_NAMESPACE_PREFIX::MMKVConfig;
using MMKVLogLevel = MMKV_NAMESPACE_PREFIX::MMKVLogLevel;
using NativeBuffer = mmkv::MMBuffer;

struct ParsedConfig {
    MMKVConfig config;
    std::optional<MMKVPath_t> rootPath;
    std::optional<std::string> cryptKey;
};

class NodeMMKV : public Napi::ObjectWrap<NodeMMKV> {
public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    static Napi::FunctionReference constructor;

    explicit NodeMMKV(const Napi::CallbackInfo &info);

    static Napi::Object NewWrappedInstance(
        Napi::Env env,
        MMKVNative *native,
        const std::optional<std::string> &cryptKey = std::nullopt);

    MMKVNative *RequireOpen(const Napi::Env &env) const;

    static Napi::Value Initialize(const Napi::CallbackInfo &info);
    static Napi::Value OnExit(const Napi::CallbackInfo &info);
    static Napi::Value Version(const Napi::CallbackInfo &info);
    static Napi::Value DefaultMMKV(const Napi::CallbackInfo &info);
    static Napi::Value BackupOneToDirectory(const Napi::CallbackInfo &info);
    static Napi::Value RestoreOneFromDirectory(const Napi::CallbackInfo &info);
    static Napi::Value BackupAllToDirectory(const Napi::CallbackInfo &info);
    static Napi::Value RestoreAllFromDirectory(const Napi::CallbackInfo &info);

    Napi::Value SetBool(const Napi::CallbackInfo &info);
    Napi::Value GetBool(const Napi::CallbackInfo &info);
    Napi::Value SetInt32(const Napi::CallbackInfo &info);
    Napi::Value GetInt32(const Napi::CallbackInfo &info);
    Napi::Value SetUInt32(const Napi::CallbackInfo &info);
    Napi::Value GetUInt32(const Napi::CallbackInfo &info);
    Napi::Value SetString(const Napi::CallbackInfo &info);
    Napi::Value GetString(const Napi::CallbackInfo &info);
    Napi::Value SetInt64(const Napi::CallbackInfo &info);
    Napi::Value GetInt64(const Napi::CallbackInfo &info);
    Napi::Value SetUInt64(const Napi::CallbackInfo &info);
    Napi::Value GetUInt64(const Napi::CallbackInfo &info);
    Napi::Value SetFloat(const Napi::CallbackInfo &info);
    Napi::Value GetFloat(const Napi::CallbackInfo &info);
    Napi::Value SetDouble(const Napi::CallbackInfo &info);
    Napi::Value GetDouble(const Napi::CallbackInfo &info);
    Napi::Value SetBuffer(const Napi::CallbackInfo &info);
    Napi::Value GetBuffer(const Napi::CallbackInfo &info);
    Napi::Value ContainsKey(const Napi::CallbackInfo &info);
    Napi::Value RemoveValueForKey(const Napi::CallbackInfo &info);
    Napi::Value RemoveValuesForKeys(const Napi::CallbackInfo &info);
    Napi::Value AllKeys(const Napi::CallbackInfo &info);
    Napi::Value Count(const Napi::CallbackInfo &info);
    Napi::Value TotalSize(const Napi::CallbackInfo &info);
    Napi::Value ActualSize(const Napi::CallbackInfo &info);
    Napi::Value ClearAll(const Napi::CallbackInfo &info);
    Napi::Value Trim(const Napi::CallbackInfo &info);
    Napi::Value Sync(const Napi::CallbackInfo &info);
    Napi::Value Lock(const Napi::CallbackInfo &info);
    Napi::Value Unlock(const Napi::CallbackInfo &info);
    Napi::Value TryLock(const Napi::CallbackInfo &info);
    Napi::Value ReKey(const Napi::CallbackInfo &info);
    Napi::Value CryptKey(const Napi::CallbackInfo &info);
    Napi::Value Close(const Napi::CallbackInfo &info);

private:
    MMKVNative *instance_ = nullptr;
    bool closed_ = false;
    std::optional<MMKVPath_t> rootPathHolder_;
    std::optional<std::string> knownCryptKey_;
};

} // namespace node_mmkv
