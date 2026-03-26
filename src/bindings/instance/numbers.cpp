#include "core/node_mmkv_binding.h"

#include "helpers/value_utils.h"

namespace node_mmkv {

Napi::Value NodeMMKV::SetBool(const Napi::CallbackInfo &info) {
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

Napi::Value NodeMMKV::GetBool(const Napi::CallbackInfo &info) {
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

Napi::Value NodeMMKV::SetInt32(const Napi::CallbackInfo &info) {
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

Napi::Value NodeMMKV::GetInt32(const Napi::CallbackInfo &info) {
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

Napi::Value NodeMMKV::SetUInt32(const Napi::CallbackInfo &info) {
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

Napi::Value NodeMMKV::GetUInt32(const Napi::CallbackInfo &info) {
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

Napi::Value NodeMMKV::SetInt64(const Napi::CallbackInfo &info) {
    auto env = info.Env();
    auto mmkv = RequireOpen(env);
    if (info.Length() < 2) {
        throw Napi::TypeError::New(env, "setInt64 expects key and value");
    }
    auto key = RequireString(info[0], "key");
    auto value = GetInt64Value(info[1], "value");
    return Napi::Boolean::New(env, mmkv->set(value, key));
}

Napi::Value NodeMMKV::GetInt64(const Napi::CallbackInfo &info) {
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

Napi::Value NodeMMKV::SetUInt64(const Napi::CallbackInfo &info) {
    auto env = info.Env();
    auto mmkv = RequireOpen(env);
    if (info.Length() < 2) {
        throw Napi::TypeError::New(env, "setUInt64 expects key and value");
    }
    auto key = RequireString(info[0], "key");
    auto value = GetUInt64Value(info[1], "value");
    return Napi::Boolean::New(env, mmkv->set(value, key));
}

Napi::Value NodeMMKV::GetUInt64(const Napi::CallbackInfo &info) {
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

Napi::Value NodeMMKV::SetFloat(const Napi::CallbackInfo &info) {
    auto env = info.Env();
    auto mmkv = RequireOpen(env);
    if (info.Length() < 2) {
        throw Napi::TypeError::New(env, "setFloat expects key and value");
    }
    auto key = RequireString(info[0], "key");
    auto value = GetFloatValue(info[1], "value");
    return Napi::Boolean::New(env, mmkv->set(value, key));
}

Napi::Value NodeMMKV::GetFloat(const Napi::CallbackInfo &info) {
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

Napi::Value NodeMMKV::SetDouble(const Napi::CallbackInfo &info) {
    auto env = info.Env();
    auto mmkv = RequireOpen(env);
    if (info.Length() < 2) {
        throw Napi::TypeError::New(env, "setDouble expects key and value");
    }
    auto key = RequireString(info[0], "key");
    auto value = GetDoubleValue(info[1], "value");
    return Napi::Boolean::New(env, mmkv->set(value, key));
}

Napi::Value NodeMMKV::GetDouble(const Napi::CallbackInfo &info) {
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

} // namespace node_mmkv
