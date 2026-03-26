#include "helpers/value_utils.h"

#include <cmath>
#include <limits>

namespace node_mmkv {

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
    return value.As<Napi::Number>().Uint32Value();
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

MMKVPath_t ToMMKVPath(const std::string &value) {
    return string2MMKVPath_t(value);
}

} // namespace node_mmkv
