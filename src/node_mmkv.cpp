#include <napi.h>

#include "MMKV.h"

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace {

using MMKVNative = MMKV_NAMESPACE_PREFIX::MMKV;
using MMKVMode = MMKV_NAMESPACE_PREFIX::MMKVMode;
using MMKVConfig = MMKV_NAMESPACE_PREFIX::MMKVConfig;
using MMKVLogLevel = MMKV_NAMESPACE_PREFIX::MMKVLogLevel;

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
                InstanceMethod("setBool", &NodeMMKV::SetBool),
                InstanceMethod("getBool", &NodeMMKV::GetBool),
                InstanceMethod("setInt32", &NodeMMKV::SetInt32),
                InstanceMethod("getInt32", &NodeMMKV::GetInt32),
                InstanceMethod("setString", &NodeMMKV::SetString),
                InstanceMethod("getString", &NodeMMKV::GetString),
                InstanceMethod("containsKey", &NodeMMKV::ContainsKey),
                InstanceMethod("removeValueForKey", &NodeMMKV::RemoveValueForKey),
                InstanceMethod("allKeys", &NodeMMKV::AllKeys),
                InstanceMethod("count", &NodeMMKV::Count),
                InstanceMethod("totalSize", &NodeMMKV::TotalSize),
                InstanceMethod("actualSize", &NodeMMKV::ActualSize),
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

        const auto id = RequireString(info[0], "id");
        auto parsed = ParseConfig(env, info.Length() > 1 ? info[1] : env.Undefined());
        if (parsed.rootPath) {
            parsed.config.rootPath = &(*parsed.rootPath);
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

NODE_API_MODULE(node_mmkv, InitModule)
