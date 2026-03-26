#include "core/node_mmkv_binding.h"

#include "helpers/value_utils.h"

namespace node_mmkv {

Napi::Value NodeMMKV::ContainsKey(const Napi::CallbackInfo &info) {
    auto env = info.Env();
    auto mmkv = RequireOpen(env);
    if (info.Length() < 1) {
        throw Napi::TypeError::New(env, "containsKey expects a key");
    }
    auto key = RequireString(info[0], "key");
    return Napi::Boolean::New(env, mmkv->containsKey(key));
}

Napi::Value NodeMMKV::RemoveValueForKey(const Napi::CallbackInfo &info) {
    auto env = info.Env();
    auto mmkv = RequireOpen(env);
    if (info.Length() < 1) {
        throw Napi::TypeError::New(env, "removeValueForKey expects a key");
    }
    auto key = RequireString(info[0], "key");
    return Napi::Boolean::New(env, mmkv->removeValueForKey(key));
}

Napi::Value NodeMMKV::RemoveValuesForKeys(const Napi::CallbackInfo &info) {
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

Napi::Value NodeMMKV::AllKeys(const Napi::CallbackInfo &info) {
    auto env = info.Env();
    auto mmkv = RequireOpen(env);
    auto keys = mmkv->allKeys();
    auto result = Napi::Array::New(env, keys.size());
    for (size_t i = 0; i < keys.size(); ++i) {
        result.Set(static_cast<uint32_t>(i), Napi::String::New(env, keys[i]));
    }
    return result;
}

Napi::Value NodeMMKV::Count(const Napi::CallbackInfo &info) {
    auto env = info.Env();
    auto mmkv = RequireOpen(env);
    return Napi::Number::New(env, static_cast<double>(mmkv->count()));
}

} // namespace node_mmkv
