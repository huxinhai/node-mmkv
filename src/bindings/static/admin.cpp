#include "core/node_mmkv_binding.h"

#include "helpers/config_parser.h"
#include "helpers/value_utils.h"

#include <filesystem>

namespace node_mmkv {

Napi::Value NodeMMKV::Initialize(const Napi::CallbackInfo &info) {
    auto env = info.Env();
    if (info.Length() < 1) {
        throw Napi::TypeError::New(env, "initialize expects a rootDir string");
    }

    auto rootDir = RequireString(info[0], "rootDir");
    auto rootPath = ToMMKVPath(rootDir);
    auto logLevel = MMKV_NAMESPACE_PREFIX::MMKVLogInfo;
    if (info.Length() > 1 && !info[1].IsUndefined()) {
        if (!info[1].IsNumber()) {
            throw Napi::TypeError::New(env, "logLevel must be a number");
        }
        logLevel = static_cast<MMKVLogLevel>(info[1].As<Napi::Number>().Int32Value());
    }

    std::filesystem::create_directories(std::filesystem::path(rootPath));
    MMKVNative::initializeMMKV(rootPath, logLevel);
    return Napi::String::New(env, rootDir);
}

Napi::Value NodeMMKV::OnExit(const Napi::CallbackInfo &info) {
    MMKVNative::onExit();
    return info.Env().Undefined();
}

Napi::Value NodeMMKV::Version(const Napi::CallbackInfo &info) {
    return Napi::String::New(info.Env(), MMKV_VERSION);
}

Napi::Value NodeMMKV::DefaultMMKV(const Napi::CallbackInfo &info) {
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

} // namespace node_mmkv
