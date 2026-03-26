#include "core/node_mmkv_binding.h"

#include "helpers/config_parser.h"
#include "helpers/value_utils.h"

namespace node_mmkv {

Napi::FunctionReference NodeMMKV::constructor;

NodeMMKV::NodeMMKV(const Napi::CallbackInfo &info) : Napi::ObjectWrap<NodeMMKV>(info) {
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
        rootPathHolder_ = std::move(parsed.rootPath);
        parsed.config.rootPath = &(*rootPathHolder_);
    }
    if (parsed.cryptKey) {
        knownCryptKey_ = std::move(parsed.cryptKey);
        parsed.config.cryptKey = &(*knownCryptKey_);
    }
    instance_ = MMKVNative::mmkvWithID(id, parsed.config);

    if (instance_ == nullptr) {
        throw Napi::Error::New(env, "Failed to create MMKV instance");
    }
}

Napi::Object NodeMMKV::NewWrappedInstance(
    Napi::Env env,
    MMKVNative *native,
    const std::optional<std::string> &cryptKey) {
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

MMKVNative *NodeMMKV::RequireOpen(const Napi::Env &env) const {
    if (closed_ || instance_ == nullptr) {
        throw Napi::Error::New(env, "MMKV instance is closed");
    }
    return instance_;
}

} // namespace node_mmkv
