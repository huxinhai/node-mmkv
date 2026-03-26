#include "helpers/config_parser.h"

#include "helpers/value_utils.h"

namespace node_mmkv {

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
    if (auto rootPath = GetOptionalString(options, "rootPath")) {
        parsed.rootPath = ToMMKVPath(*rootPath);
    }
    parsed.cryptKey = GetOptionalString(options, "cryptKey");
    parsed.config.aes256 = GetOptionalBool(options, "aes256", false);

    return parsed;
}

} // namespace node_mmkv
