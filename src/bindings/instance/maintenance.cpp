#include "core/node_mmkv_binding.h"

namespace node_mmkv {

Napi::Value NodeMMKV::TotalSize(const Napi::CallbackInfo &info) {
    auto env = info.Env();
    auto mmkv = RequireOpen(env);
    return Napi::Number::New(env, static_cast<double>(mmkv->totalSize()));
}

Napi::Value NodeMMKV::ActualSize(const Napi::CallbackInfo &info) {
    auto env = info.Env();
    auto mmkv = RequireOpen(env);
    return Napi::Number::New(env, static_cast<double>(mmkv->actualSize()));
}

Napi::Value NodeMMKV::ClearAll(const Napi::CallbackInfo &info) {
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

Napi::Value NodeMMKV::Trim(const Napi::CallbackInfo &info) {
    auto env = info.Env();
    auto mmkv = RequireOpen(env);
    mmkv->trim();
    return env.Undefined();
}

Napi::Value NodeMMKV::Sync(const Napi::CallbackInfo &info) {
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

Napi::Value NodeMMKV::Lock(const Napi::CallbackInfo &info) {
    auto env = info.Env();
    auto mmkv = RequireOpen(env);
    mmkv->lock();
    return env.Undefined();
}

Napi::Value NodeMMKV::Unlock(const Napi::CallbackInfo &info) {
    auto env = info.Env();
    auto mmkv = RequireOpen(env);
    mmkv->unlock();
    return env.Undefined();
}

Napi::Value NodeMMKV::TryLock(const Napi::CallbackInfo &info) {
    auto env = info.Env();
    auto mmkv = RequireOpen(env);
    return Napi::Boolean::New(env, mmkv->try_lock());
}

Napi::Value NodeMMKV::ReKey(const Napi::CallbackInfo &info) {
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

Napi::Value NodeMMKV::CryptKey(const Napi::CallbackInfo &info) {
    auto env = info.Env();
    RequireOpen(env);
    if (!knownCryptKey_) {
        return env.Null();
    }
    return Napi::String::New(env, *knownCryptKey_);
}

Napi::Value NodeMMKV::Close(const Napi::CallbackInfo &info) {
    auto env = info.Env();
    if (!closed_ && instance_ != nullptr) {
        instance_->close();
        closed_ = true;
    }
    return env.Undefined();
}

} // namespace node_mmkv
