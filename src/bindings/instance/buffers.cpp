#include "core/node_mmkv_binding.h"

#include "helpers/value_utils.h"

namespace node_mmkv {

Napi::Value NodeMMKV::SetBuffer(const Napi::CallbackInfo &info) {
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

Napi::Value NodeMMKV::GetBuffer(const Napi::CallbackInfo &info) {
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

} // namespace node_mmkv
