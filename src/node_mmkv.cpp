#include <napi.h>

#include "MMKV.h"

#include "MMBuffer.h"

#include <filesystem>
#include <cmath>
#include <limits>
#include <optional>
#include <string>
#include <vector>

namespace {

using MMKVNative = MMKV_NAMESPACE_PREFIX::MMKV;
using MMKVMode = MMKV_NAMESPACE_PREFIX::MMKVMode;
using MMKVConfig = MMKV_NAMESPACE_PREFIX::MMKVConfig;
using MMKVLogLevel = MMKV_NAMESPACE_PREFIX::MMKVLogLevel;
using NativeBuffer = mmkv::MMBuffer;

std::string RequireString(const Napi::Value &value, const char *name) {
    if (!value.IsString()) {
        throw Napi::TypeError::New(value.Env(), std::string(name) + " must be a string");
    }
    return value.As<Napi::String>().Utf8Value();
}

bool GetOptionalBool(const Napi::Object &options, const char *name, bool defaultValue) {
    if (!options.Has(name)) {
        return defaultValue;
    }
    auto value = options.Get(name);
    if (!value.IsBoolean()) {
        throw Napi::TypeError::New(options.Env(), std::string(name) + " must be a boolean");
    }
    return value.As<Napi::Boolean>().Value();
}

uint32_t GetOptionalUint32(const Napi::Object &options, const char *name, uint32_t defaultValue) {
    if (!options.Has(name)) {
        return defaultValue;
    }
    auto value = options.Get(name);
    if (!value.IsNumber()) {
        throw Napi::TypeError::New(options.Env(), std::string(name) + " must be a number");
    }
    auto number = value.As<Napi::Number>().Uint32Value();
    return number;
}

int64_t GetInt64Value(const Napi::Value &value, const char *name) {
    auto env = value.Env();
    if (value.IsBigInt()) {
        bool lossless = false;
        auto result = value.As<Napi::BigInt>().Int64Value(&lossless);
        if (!lossless) {
            throw Napi::RangeError::New(env, std::string(name) + " is out of int64 range");
        }
        return result;
    }
    if (value.IsNumber()) {
        auto number = value.As<Napi::Number>().DoubleValue();
        if (!std::isfinite(number) || std::floor(number) != number) {
            throw Napi::TypeError::New(env, std::string(name) + " must be an integer");
        }
        if (number < static_cast<double>(std::numeric_limits<int64_t>::min()) ||
            number > static_cast<double>(std::numeric_limits<int64_t>::max())) {
            throw Napi::RangeError::New(env, std::string(name) + " is out of int64 range");
        }
        return static_cast<int64_t>(number);
    }
    throw Napi::TypeError::New(env, std::string(name) + " must be a bigint or integer number");
}

uint64_t GetUInt64Value(const Napi::Value &value, const char *name) {
    auto env = value.Env();
    if (value.IsBigInt()) {
        bool lossless = false;
        auto result = value.As<Napi::BigInt>().Uint64Value(&lossless);
        if (!lossless) {
            throw Napi::RangeError::New(env, std::string(name) + " is out of uint64 range");
        }
        return result;
    }
    if (value.IsNumber()) {
        auto number = value.As<Napi::Number>().DoubleValue();
        if (!std::isfinite(number) || std::floor(number) != number) {
            throw Napi::TypeError::New(env, std::string(name) + " must be an integer");
        }
        if (number < 0 || number > static_cast<double>(std::numeric_limits<uint64_t>::max())) {
            throw Napi::RangeError::New(env, std::string(name) + " is out of uint64 range");
        }
        return static_cast<uint64_t>(number);
    }
    throw Napi::TypeError::New(env, std::string(name) + " must be a bigint or integer number");
}

double GetDoubleValue(const Napi::Value &value, const char *name) {
    if (!value.IsNumber()) {
        throw Napi::TypeError::New(value.Env(), std::string(name) + " must be a number");
    }
    return value.As<Napi::Number>().DoubleValue();
}

float GetFloatValue(const Napi::Value &value, const char *name) {
    return static_cast<float>(GetDoubleValue(value, name));
}

std::optional<std::string> GetOptionalString(const Napi::Object &options, const char *name) {
    if (!options.Has(name)) {
        return std::nullopt;
    }
    auto value = options.Get(name);
    if (value.IsNull() || value.IsUndefined()) {
        return std::nullopt;
    }
    if (!value.IsString()) {
        throw Napi::TypeError::New(options.Env(), std::string(name) + " must be a string");
    }
    return value.As<Napi::String>().Utf8Value();
}

struct ParsedConfig {
    MMKVConfig config;
    std::optional<std::string> rootPath;
    std::optional<std::string> cryptKey;
};

ParsedConfig ParseConfig(const Napi::Env &env, const Napi::Value &value) {
    ParsedConfig parsed;
    if (value.IsUndefined() || value.IsNull()) {
        return parsed;
    }
    if (!value.IsObject()) {
        throw Napi::TypeError::New(env, "options must be an object");
    }

    auto options = value.As<Napi::Object>();
    MMKVMode mode = MMKV_NAMESPACE_PREFIX::MMKV_SINGLE_PROCESS;

    if (GetOptionalBool(options, "multiProcess", false)) {
        mode = static_cast<MMKVMode>(mode | MMKV_NAMESPACE_PREFIX::MMKV_MULTI_PROCESS);
    }
    if (GetOptionalBool(options, "readOnly", false)) {
        mode = static_cast<MMKVMode>(mode | MMKV_NAMESPACE_PREFIX::MMKV_READ_ONLY);
    }

    parsed.config.mode = mode;
    parsed.config.expectedCapacity = GetOptionalUint32(options, "expectedCapacity", 0);
    parsed.rootPath = GetOptionalString(options, "rootPath");
    parsed.cryptKey = GetOptionalString(options, "cryptKey");
    parsed.config.aes256 = GetOptionalBool(options, "aes256", false);

    return parsed;
}

class NodeMMKV : public Napi::ObjectWrap<NodeMMKV> {
public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports) {
        auto ctor = DefineClass(
            env,
            "MMKV",
            {
                StaticMethod("initialize", &NodeMMKV::Initialize),
                StaticMethod("onExit", &NodeMMKV::OnExit),
                StaticMethod("version", &NodeMMKV::Version),
                StaticMethod("defaultMMKV", &NodeMMKV::DefaultMMKV),
                StaticMethod("backupOneToDirectory", &NodeMMKV::BackupOneToDirectory),
                StaticMethod("restoreOneFromDirectory", &NodeMMKV::RestoreOneFromDirectory),
                StaticMethod("backupAllToDirectory", &NodeMMKV::BackupAllToDirectory),
                StaticMethod("restoreAllFromDirectory", &NodeMMKV::RestoreAllFromDirectory),
                InstanceMethod("setBool", &NodeMMKV::SetBool),
                InstanceMethod("getBool", &NodeMMKV::GetBool),
                InstanceMethod("setInt32", &NodeMMKV::SetInt32),
                InstanceMethod("getInt32", &NodeMMKV::GetInt32),
                InstanceMethod("setUInt32", &NodeMMKV::SetUInt32),
                InstanceMethod("getUInt32", &NodeMMKV::GetUInt32),
                InstanceMethod("setUInt64", &NodeMMKV::SetUInt64),
                InstanceMethod("getUInt64", &NodeMMKV::GetUInt64),
                InstanceMethod("setInt64", &NodeMMKV::SetInt64),
                InstanceMethod("getInt64", &NodeMMKV::GetInt64),
                InstanceMethod("setFloat", &NodeMMKV::SetFloat),
                InstanceMethod("getFloat", &NodeMMKV::GetFloat),
                InstanceMethod("setDouble", &NodeMMKV::SetDouble),
                InstanceMethod("getDouble", &NodeMMKV::GetDouble),
                InstanceMethod("setString", &NodeMMKV::SetString),
                InstanceMethod("getString", &NodeMMKV::GetString),
                InstanceMethod("setBuffer", &NodeMMKV::SetBuffer),
                InstanceMethod("getBuffer", &NodeMMKV::GetBuffer),
                InstanceMethod("containsKey", &NodeMMKV::ContainsKey),
                InstanceMethod("removeValueForKey", &NodeMMKV::RemoveValueForKey),
                InstanceMethod("removeValuesForKeys", &NodeMMKV::RemoveValuesForKeys),
                InstanceMethod("allKeys", &NodeMMKV::AllKeys),
                InstanceMethod("count", &NodeMMKV::Count),
                InstanceMethod("totalSize", &NodeMMKV::TotalSize),
                InstanceMethod("actualSize", &NodeMMKV::ActualSize),
                InstanceMethod("clearAll", &NodeMMKV::ClearAll),
                InstanceMethod("trim", &NodeMMKV::Trim),
                InstanceMethod("sync", &NodeMMKV::Sync),
                InstanceMethod("lock", &NodeMMKV::Lock),
                InstanceMethod("unlock", &NodeMMKV::Unlock),
                InstanceMethod("tryLock", &NodeMMKV::TryLock),
                InstanceMethod("reKey", &NodeMMKV::ReKey),
                InstanceMethod("cryptKey", &NodeMMKV::CryptKey),
                InstanceMethod("close", &NodeMMKV::Close)
            });

        constructor = Napi::Persistent(ctor);
        constructor.SuppressDestruct();

        exports.Set("MMKV", ctor);
        exports.Set("version", Napi::String::New(env, MMKV_VERSION));
        return exports;
    }

    NodeMMKV(const Napi::CallbackInfo &info) : Napi::ObjectWrap<NodeMMKV>(info) {
        auto env = info.Env();
        if (info.Length() < 1) {
            throw Napi::TypeError::New(env, "MMKV constructor expects an id string");
        }

        if (info[0].IsExternal()) {
            instance_ = info[0].As<Napi::External<MMKVNative>>().Data();
            if (instance_ == nullptr) {
                throw Napi::Error::New(env, "Failed to create MMKV instance");
            }
            if (info.Length() > 1 && info[1].IsString()) {
                knownCryptKey_ = info[1].As<Napi::String>().Utf8Value();
            }
            return;
        }

        const auto id = RequireString(info[0], "id");
        auto parsed = ParseConfig(env, info.Length() > 1 ? info[1] : env.Undefined());
        if (parsed.rootPath) {
            parsed.config.rootPath = &(*parsed.rootPath);
        }
        if (parsed.cryptKey) {
            parsed.config.cryptKey = &(*parsed.cryptKey);
            knownCryptKey_ = parsed.cryptKey;
        }
        instance_ = MMKVNative::mmkvWithID(id, parsed.config);

        if (instance_ == nullptr) {
            throw Napi::Error::New(env, "Failed to create MMKV instance");
        }
    }

private:
    static Napi::FunctionReference constructor;

    MMKVNative *instance_ = nullptr;
    bool closed_ = false;
    std::optional<std::string> knownCryptKey_;

    static Napi::Object NewWrappedInstance(Napi::Env env, MMKVNative *native, const std::optional<std::string> &cryptKey = std::nullopt) {
        if (native == nullptr) {
            throw Napi::Error::New(env, "Failed to create MMKV instance");
        }
        std::vector<napi_value> args;
        args.push_back(Napi::External<MMKVNative>::New(env, native));
        if (cryptKey) {
            args.push_back(Napi::String::New(env, *cryptKey));
        }
        return constructor.New(args);
    }

    MMKVNative *RequireOpen(const Napi::Env &env) const {
        if (closed_ || instance_ == nullptr) {
            throw Napi::Error::New(env, "MMKV instance is closed");
        }
        return instance_;
    }

    static Napi::Value Initialize(const Napi::CallbackInfo &info) {
        auto env = info.Env();
        if (info.Length() < 1) {
            throw Napi::TypeError::New(env, "initialize expects a rootDir string");
        }

        auto rootDir = RequireString(info[0], "rootDir");
        auto logLevel = MMKV_NAMESPACE_PREFIX::MMKVLogInfo;
        if (info.Length() > 1 && !info[1].IsUndefined()) {
            if (!info[1].IsNumber()) {
                throw Napi::TypeError::New(env, "logLevel must be a number");
            }
            logLevel = static_cast<MMKVLogLevel>(info[1].As<Napi::Number>().Int32Value());
        }

        std::filesystem::create_directories(rootDir);
        MMKVNative::initializeMMKV(rootDir, logLevel);
        return Napi::String::New(env, rootDir);
    }

    static Napi::Value OnExit(const Napi::CallbackInfo &info) {
        MMKVNative::onExit();
        return info.Env().Undefined();
    }

    static Napi::Value Version(const Napi::CallbackInfo &info) {
        return Napi::String::New(info.Env(), MMKV_VERSION);
    }

    static Napi::Value DefaultMMKV(const Napi::CallbackInfo &info) {
        auto env = info.Env();
        auto parsed = ParseConfig(env, info.Length() > 0 ? info[0] : env.Undefined());
        if (parsed.rootPath) {
            parsed.config.rootPath = &(*parsed.rootPath);
        }
        if (parsed.cryptKey) {
            parsed.config.cryptKey = &(*parsed.cryptKey);
        }
        auto instance = MMKVNative::defaultMMKV(parsed.config);
        return NewWrappedInstance(env, instance, parsed.cryptKey);
    }

    static Napi::Value BackupOneToDirectory(const Napi::CallbackInfo &info) {
        auto env = info.Env();
        if (info.Length() < 2) {
            throw Napi::TypeError::New(env, "backupOneToDirectory expects mmapID and dstDir");
        }
        auto mmapID = RequireString(info[0], "mmapID");
        auto dstDir = RequireString(info[1], "dstDir");
        std::optional<std::string> srcDir;
        if (info.Length() > 2 && !info[2].IsUndefined() && !info[2].IsNull()) {
            srcDir = RequireString(info[2], "srcDir");
        }
        std::filesystem::create_directories(dstDir);
        auto ok = srcDir ? MMKVNative::backupOneToDirectory(mmapID, dstDir, &(*srcDir))
                         : MMKVNative::backupOneToDirectory(mmapID, dstDir, nullptr);
        return Napi::Boolean::New(env, ok);
    }

    static Napi::Value RestoreOneFromDirectory(const Napi::CallbackInfo &info) {
        auto env = info.Env();
        if (info.Length() < 2) {
            throw Napi::TypeError::New(env, "restoreOneFromDirectory expects mmapID and srcDir");
        }
        auto mmapID = RequireString(info[0], "mmapID");
        auto srcDir = RequireString(info[1], "srcDir");
        std::optional<std::string> dstDir;
        if (info.Length() > 2 && !info[2].IsUndefined() && !info[2].IsNull()) {
            dstDir = RequireString(info[2], "dstDir");
            std::filesystem::create_directories(*dstDir);
        }
        auto ok = dstDir ? MMKVNative::restoreOneFromDirectory(mmapID, srcDir, &(*dstDir))
                         : MMKVNative::restoreOneFromDirectory(mmapID, srcDir, nullptr);
        return Napi::Boolean::New(env, ok);
    }

    static Napi::Value BackupAllToDirectory(const Napi::CallbackInfo &info) {
        auto env = info.Env();
        if (info.Length() < 1) {
            throw Napi::TypeError::New(env, "backupAllToDirectory expects dstDir");
        }
        auto dstDir = RequireString(info[0], "dstDir");
        std::optional<std::string> srcDir;
        if (info.Length() > 1 && !info[1].IsUndefined() && !info[1].IsNull()) {
            srcDir = RequireString(info[1], "srcDir");
        }
        std::filesystem::create_directories(dstDir);
        auto count = srcDir ? MMKVNative::backupAllToDirectory(dstDir, &(*srcDir))
                            : MMKVNative::backupAllToDirectory(dstDir, nullptr);
        return Napi::Number::New(env, static_cast<double>(count));
    }

    static Napi::Value RestoreAllFromDirectory(const Napi::CallbackInfo &info) {
        auto env = info.Env();
        if (info.Length() < 1) {
            throw Napi::TypeError::New(env, "restoreAllFromDirectory expects srcDir");
        }
        auto srcDir = RequireString(info[0], "srcDir");
        std::optional<std::string> dstDir;
        if (info.Length() > 1 && !info[1].IsUndefined() && !info[1].IsNull()) {
            dstDir = RequireString(info[1], "dstDir");
            std::filesystem::create_directories(*dstDir);
        }
        auto count = dstDir ? MMKVNative::restoreAllFromDirectory(srcDir, &(*dstDir))
                            : MMKVNative::restoreAllFromDirectory(srcDir, nullptr);
        return Napi::Number::New(env, static_cast<double>(count));
    }

    Napi::Value SetBool(const Napi::CallbackInfo &info) {
        auto env = info.Env();
        auto mmkv = RequireOpen(env);
        if (info.Length() < 2) {
            throw Napi::TypeError::New(env, "setBool expects key and value");
        }
        auto key = RequireString(info[0], "key");
        if (!info[1].IsBoolean()) {
            throw Napi::TypeError::New(env, "value must be a boolean");
        }
        auto value = info[1].As<Napi::Boolean>().Value();
        return Napi::Boolean::New(env, mmkv->set(value, key));
    }

    Napi::Value GetBool(const Napi::CallbackInfo &info) {
        auto env = info.Env();
        auto mmkv = RequireOpen(env);
        if (info.Length() < 1) {
            throw Napi::TypeError::New(env, "getBool expects a key");
        }
        auto key = RequireString(info[0], "key");
        bool defaultValue = false;
        if (info.Length() > 1 && !info[1].IsUndefined()) {
            if (!info[1].IsBoolean()) {
                throw Napi::TypeError::New(env, "defaultValue must be a boolean");
            }
            defaultValue = info[1].As<Napi::Boolean>().Value();
        }
        return Napi::Boolean::New(env, mmkv->getBool(key, defaultValue));
    }

    Napi::Value SetInt32(const Napi::CallbackInfo &info) {
        auto env = info.Env();
        auto mmkv = RequireOpen(env);
        if (info.Length() < 2) {
            throw Napi::TypeError::New(env, "setInt32 expects key and value");
        }
        auto key = RequireString(info[0], "key");
        if (!info[1].IsNumber()) {
            throw Napi::TypeError::New(env, "value must be a number");
        }
        auto value = info[1].As<Napi::Number>().Int32Value();
        return Napi::Boolean::New(env, mmkv->set(value, key));
    }

    Napi::Value GetInt32(const Napi::CallbackInfo &info) {
        auto env = info.Env();
        auto mmkv = RequireOpen(env);
        if (info.Length() < 1) {
            throw Napi::TypeError::New(env, "getInt32 expects a key");
        }
        auto key = RequireString(info[0], "key");
        int32_t defaultValue = 0;
        if (info.Length() > 1 && !info[1].IsUndefined()) {
            if (!info[1].IsNumber()) {
                throw Napi::TypeError::New(env, "defaultValue must be a number");
            }
            defaultValue = info[1].As<Napi::Number>().Int32Value();
        }
        return Napi::Number::New(env, mmkv->getInt32(key, defaultValue));
    }

    Napi::Value SetUInt32(const Napi::CallbackInfo &info) {
        auto env = info.Env();
        auto mmkv = RequireOpen(env);
        if (info.Length() < 2) {
            throw Napi::TypeError::New(env, "setUInt32 expects key and value");
        }
        auto key = RequireString(info[0], "key");
        if (!info[1].IsNumber()) {
            throw Napi::TypeError::New(env, "value must be a number");
        }
        auto value = info[1].As<Napi::Number>().Uint32Value();
        return Napi::Boolean::New(env, mmkv->set(value, key));
    }

    Napi::Value GetUInt32(const Napi::CallbackInfo &info) {
        auto env = info.Env();
        auto mmkv = RequireOpen(env);
        if (info.Length() < 1) {
            throw Napi::TypeError::New(env, "getUInt32 expects a key");
        }
        auto key = RequireString(info[0], "key");
        uint32_t defaultValue = 0;
        if (info.Length() > 1 && !info[1].IsUndefined()) {
            if (!info[1].IsNumber()) {
                throw Napi::TypeError::New(env, "defaultValue must be a number");
            }
            defaultValue = info[1].As<Napi::Number>().Uint32Value();
        }
        return Napi::Number::New(env, mmkv->getUInt32(key, defaultValue));
    }

    Napi::Value SetString(const Napi::CallbackInfo &info) {
        auto env = info.Env();
        auto mmkv = RequireOpen(env);
        if (info.Length() < 2) {
            throw Napi::TypeError::New(env, "setString expects key and value");
        }
        auto key = RequireString(info[0], "key");
        auto value = RequireString(info[1], "value");
        return Napi::Boolean::New(env, mmkv->set(value, key));
    }

    Napi::Value SetInt64(const Napi::CallbackInfo &info) {
        auto env = info.Env();
        auto mmkv = RequireOpen(env);
        if (info.Length() < 2) {
            throw Napi::TypeError::New(env, "setInt64 expects key and value");
        }
        auto key = RequireString(info[0], "key");
        auto value = GetInt64Value(info[1], "value");
        return Napi::Boolean::New(env, mmkv->set(value, key));
    }

    Napi::Value GetInt64(const Napi::CallbackInfo &info) {
        auto env = info.Env();
        auto mmkv = RequireOpen(env);
        if (info.Length() < 1) {
            throw Napi::TypeError::New(env, "getInt64 expects a key");
        }
        auto key = RequireString(info[0], "key");
        int64_t defaultValue = 0;
        if (info.Length() > 1 && !info[1].IsUndefined()) {
            defaultValue = GetInt64Value(info[1], "defaultValue");
        }
        return Napi::BigInt::New(env, mmkv->getInt64(key, defaultValue));
    }

    Napi::Value SetUInt64(const Napi::CallbackInfo &info) {
        auto env = info.Env();
        auto mmkv = RequireOpen(env);
        if (info.Length() < 2) {
            throw Napi::TypeError::New(env, "setUInt64 expects key and value");
        }
        auto key = RequireString(info[0], "key");
        auto value = GetUInt64Value(info[1], "value");
        return Napi::Boolean::New(env, mmkv->set(value, key));
    }

    Napi::Value GetUInt64(const Napi::CallbackInfo &info) {
        auto env = info.Env();
        auto mmkv = RequireOpen(env);
        if (info.Length() < 1) {
            throw Napi::TypeError::New(env, "getUInt64 expects a key");
        }
        auto key = RequireString(info[0], "key");
        uint64_t defaultValue = 0;
        if (info.Length() > 1 && !info[1].IsUndefined()) {
            defaultValue = GetUInt64Value(info[1], "defaultValue");
        }
        return Napi::BigInt::New(env, mmkv->getUInt64(key, defaultValue));
    }

    Napi::Value SetFloat(const Napi::CallbackInfo &info) {
        auto env = info.Env();
        auto mmkv = RequireOpen(env);
        if (info.Length() < 2) {
            throw Napi::TypeError::New(env, "setFloat expects key and value");
        }
        auto key = RequireString(info[0], "key");
        auto value = GetFloatValue(info[1], "value");
        return Napi::Boolean::New(env, mmkv->set(value, key));
    }

    Napi::Value GetFloat(const Napi::CallbackInfo &info) {
        auto env = info.Env();
        auto mmkv = RequireOpen(env);
        if (info.Length() < 1) {
            throw Napi::TypeError::New(env, "getFloat expects a key");
        }
        auto key = RequireString(info[0], "key");
        float defaultValue = 0;
        if (info.Length() > 1 && !info[1].IsUndefined()) {
            defaultValue = GetFloatValue(info[1], "defaultValue");
        }
        return Napi::Number::New(env, mmkv->getFloat(key, defaultValue));
    }

    Napi::Value SetDouble(const Napi::CallbackInfo &info) {
        auto env = info.Env();
        auto mmkv = RequireOpen(env);
        if (info.Length() < 2) {
            throw Napi::TypeError::New(env, "setDouble expects key and value");
        }
        auto key = RequireString(info[0], "key");
        auto value = GetDoubleValue(info[1], "value");
        return Napi::Boolean::New(env, mmkv->set(value, key));
    }

    Napi::Value GetDouble(const Napi::CallbackInfo &info) {
        auto env = info.Env();
        auto mmkv = RequireOpen(env);
        if (info.Length() < 1) {
            throw Napi::TypeError::New(env, "getDouble expects a key");
        }
        auto key = RequireString(info[0], "key");
        double defaultValue = 0;
        if (info.Length() > 1 && !info[1].IsUndefined()) {
            defaultValue = GetDoubleValue(info[1], "defaultValue");
        }
        return Napi::Number::New(env, mmkv->getDouble(key, defaultValue));
    }

    Napi::Value GetString(const Napi::CallbackInfo &info) {
        auto env = info.Env();
        auto mmkv = RequireOpen(env);
        if (info.Length() < 1) {
            throw Napi::TypeError::New(env, "getString expects a key");
        }
        auto key = RequireString(info[0], "key");
        std::string result;
        if (!mmkv->getString(key, result)) {
            return env.Null();
        }
        return Napi::String::New(env, result);
    }

    Napi::Value SetBuffer(const Napi::CallbackInfo &info) {
        auto env = info.Env();
        auto mmkv = RequireOpen(env);
        if (info.Length() < 2) {
            throw Napi::TypeError::New(env, "setBuffer expects key and value");
        }
        auto key = RequireString(info[0], "key");
        if (!info[1].IsBuffer()) {
            throw Napi::TypeError::New(env, "value must be a Buffer");
        }
        auto buffer = info[1].As<Napi::Buffer<uint8_t>>();
        NativeBuffer value(buffer.Data(), buffer.Length(), mmkv::MMBufferCopy);
        return Napi::Boolean::New(env, mmkv->set(value, key));
    }

    Napi::Value GetBuffer(const Napi::CallbackInfo &info) {
        auto env = info.Env();
        auto mmkv = RequireOpen(env);
        if (info.Length() < 1) {
            throw Napi::TypeError::New(env, "getBuffer expects a key");
        }
        auto key = RequireString(info[0], "key");
        auto value = mmkv->getBytes(key);
        if (value.length() == 0) {
            return env.Null();
        }
        auto data = static_cast<uint8_t *>(value.getPtr());
        return Napi::Buffer<uint8_t>::Copy(env, data, value.length());
    }

    Napi::Value ContainsKey(const Napi::CallbackInfo &info) {
        auto env = info.Env();
        auto mmkv = RequireOpen(env);
        if (info.Length() < 1) {
            throw Napi::TypeError::New(env, "containsKey expects a key");
        }
        auto key = RequireString(info[0], "key");
        return Napi::Boolean::New(env, mmkv->containsKey(key));
    }

    Napi::Value RemoveValueForKey(const Napi::CallbackInfo &info) {
        auto env = info.Env();
        auto mmkv = RequireOpen(env);
        if (info.Length() < 1) {
            throw Napi::TypeError::New(env, "removeValueForKey expects a key");
        }
        auto key = RequireString(info[0], "key");
        return Napi::Boolean::New(env, mmkv->removeValueForKey(key));
    }

    Napi::Value RemoveValuesForKeys(const Napi::CallbackInfo &info) {
        auto env = info.Env();
        auto mmkv = RequireOpen(env);
        if (info.Length() < 1 || !info[0].IsArray()) {
            throw Napi::TypeError::New(env, "removeValuesForKeys expects a string array");
        }
        auto input = info[0].As<Napi::Array>();
        std::vector<std::string> keys;
        keys.reserve(input.Length());
        for (uint32_t i = 0; i < input.Length(); ++i) {
            keys.push_back(RequireString(input.Get(i), "key"));
        }
        return Napi::Boolean::New(env, mmkv->removeValuesForKeys(keys));
    }

    Napi::Value AllKeys(const Napi::CallbackInfo &info) {
        auto env = info.Env();
        auto mmkv = RequireOpen(env);
        auto keys = mmkv->allKeys();
        auto result = Napi::Array::New(env, keys.size());
        for (size_t i = 0; i < keys.size(); ++i) {
            result.Set(static_cast<uint32_t>(i), Napi::String::New(env, keys[i]));
        }
        return result;
    }

    Napi::Value Count(const Napi::CallbackInfo &info) {
        auto env = info.Env();
        auto mmkv = RequireOpen(env);
        return Napi::Number::New(env, static_cast<double>(mmkv->count()));
    }

    Napi::Value TotalSize(const Napi::CallbackInfo &info) {
        auto env = info.Env();
        auto mmkv = RequireOpen(env);
        return Napi::Number::New(env, static_cast<double>(mmkv->totalSize()));
    }

    Napi::Value ActualSize(const Napi::CallbackInfo &info) {
        auto env = info.Env();
        auto mmkv = RequireOpen(env);
        return Napi::Number::New(env, static_cast<double>(mmkv->actualSize()));
    }

    Napi::Value ClearAll(const Napi::CallbackInfo &info) {
        auto env = info.Env();
        auto mmkv = RequireOpen(env);
        bool keepSpace = false;
        if (info.Length() > 0 && !info[0].IsUndefined()) {
            if (!info[0].IsBoolean()) {
                throw Napi::TypeError::New(env, "keepSpace must be a boolean");
            }
            keepSpace = info[0].As<Napi::Boolean>().Value();
        }
        mmkv->clearAll(keepSpace);
        return env.Undefined();
    }

    Napi::Value Trim(const Napi::CallbackInfo &info) {
        auto env = info.Env();
        auto mmkv = RequireOpen(env);
        mmkv->trim();
        return env.Undefined();
    }

    Napi::Value Sync(const Napi::CallbackInfo &info) {
        auto env = info.Env();
        auto mmkv = RequireOpen(env);
        bool async = false;
        if (info.Length() > 0 && !info[0].IsUndefined()) {
            if (!info[0].IsBoolean()) {
                throw Napi::TypeError::New(env, "async must be a boolean");
            }
            async = info[0].As<Napi::Boolean>().Value();
        }
        mmkv->sync(async ? MMKV_NAMESPACE_PREFIX::MMKV_ASYNC : MMKV_NAMESPACE_PREFIX::MMKV_SYNC);
        return env.Undefined();
    }

    Napi::Value Lock(const Napi::CallbackInfo &info) {
        auto env = info.Env();
        auto mmkv = RequireOpen(env);
        mmkv->lock();
        return env.Undefined();
    }

    Napi::Value Unlock(const Napi::CallbackInfo &info) {
        auto env = info.Env();
        auto mmkv = RequireOpen(env);
        mmkv->unlock();
        return env.Undefined();
    }

    Napi::Value TryLock(const Napi::CallbackInfo &info) {
        auto env = info.Env();
        auto mmkv = RequireOpen(env);
        return Napi::Boolean::New(env, mmkv->try_lock());
    }

    Napi::Value ReKey(const Napi::CallbackInfo &info) {
        auto env = info.Env();
        auto mmkv = RequireOpen(env);
        std::optional<std::string> cryptKey;
        bool aes256 = false;

        if (info.Length() > 0 && !info[0].IsUndefined()) {
            if (info[0].IsNull()) {
                cryptKey = std::nullopt;
            } else if (info[0].IsString()) {
                cryptKey = info[0].As<Napi::String>().Utf8Value();
            } else {
                throw Napi::TypeError::New(env, "cryptKey must be a string, null, or undefined");
            }
        }

        if (info.Length() > 1 && !info[1].IsUndefined()) {
            if (!info[1].IsBoolean()) {
                throw Napi::TypeError::New(env, "aes256 must be a boolean");
            }
            aes256 = info[1].As<Napi::Boolean>().Value();
        }

        if (!cryptKey || cryptKey->empty()) {
            auto ok = mmkv->reKey("", aes256);
            if (ok) {
                knownCryptKey_.reset();
            }
            return Napi::Boolean::New(env, ok);
        }
        auto ok = mmkv->reKey(*cryptKey, aes256);
        if (ok) {
            knownCryptKey_ = cryptKey;
        }
        return Napi::Boolean::New(env, ok);
    }

    Napi::Value CryptKey(const Napi::CallbackInfo &info) {
        auto env = info.Env();
        RequireOpen(env);
        if (!knownCryptKey_) {
            return env.Null();
        }
        return Napi::String::New(env, *knownCryptKey_);
    }

    Napi::Value Close(const Napi::CallbackInfo &info) {
        auto env = info.Env();
        if (!closed_ && instance_ != nullptr) {
            instance_->close();
            closed_ = true;
        }
        return env.Undefined();
    }
};

Napi::FunctionReference NodeMMKV::constructor;

Napi::Object InitModule(Napi::Env env, Napi::Object exports) {
    return NodeMMKV::Init(env, exports);
}

} // namespace

NODE_API_MODULE(mmkv, InitModule)
