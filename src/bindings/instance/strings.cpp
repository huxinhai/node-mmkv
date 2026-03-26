#include "core/node_mmkv_binding.h"

#include "helpers/value_utils.h"

namespace node_mmkv {

Napi::Value NodeMMKV::SetString(const Napi::CallbackInfo &info) {
    auto env = info.Env();
    auto mmkv = RequireOpen(env);
    if (info.Length() < 2) {
        throw Napi::TypeError::New(env, "setString expects key and value");
    }
    auto key = RequireString(info[0], "key");
    auto value = RequireString(info[1], "value");
    return Napi::Boolean::New(env, mmkv->set(value, key));
}

Napi::Value NodeMMKV::GetString(const Napi::CallbackInfo &info) {
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

} // namespace node_mmkv
