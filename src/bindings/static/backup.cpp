#include "core/node_mmkv_binding.h"

#include "helpers/value_utils.h"

#include <filesystem>

namespace node_mmkv {

Napi::Value NodeMMKV::BackupOneToDirectory(const Napi::CallbackInfo &info) {
    auto env = info.Env();
    if (info.Length() < 2) {
        throw Napi::TypeError::New(env, "backupOneToDirectory expects mmapID and dstDir");
    }
    auto mmapID = RequireString(info[0], "mmapID");
    auto dstDir = ToMMKVPath(RequireString(info[1], "dstDir"));
    std::optional<MMKVPath_t> srcDir;
    if (info.Length() > 2 && !info[2].IsUndefined() && !info[2].IsNull()) {
        srcDir = ToMMKVPath(RequireString(info[2], "srcDir"));
    }
    std::filesystem::create_directories(std::filesystem::path(dstDir));
    auto ok = srcDir ? MMKVNative::backupOneToDirectory(mmapID, dstDir, &(*srcDir))
                     : MMKVNative::backupOneToDirectory(mmapID, dstDir, nullptr);
    return Napi::Boolean::New(env, ok);
}

Napi::Value NodeMMKV::RestoreOneFromDirectory(const Napi::CallbackInfo &info) {
    auto env = info.Env();
    if (info.Length() < 2) {
        throw Napi::TypeError::New(env, "restoreOneFromDirectory expects mmapID and srcDir");
    }
    auto mmapID = RequireString(info[0], "mmapID");
    auto srcDir = ToMMKVPath(RequireString(info[1], "srcDir"));
    std::optional<MMKVPath_t> dstDir;
    if (info.Length() > 2 && !info[2].IsUndefined() && !info[2].IsNull()) {
        dstDir = ToMMKVPath(RequireString(info[2], "dstDir"));
        std::filesystem::create_directories(std::filesystem::path(*dstDir));
    }
    auto ok = dstDir ? MMKVNative::restoreOneFromDirectory(mmapID, srcDir, &(*dstDir))
                     : MMKVNative::restoreOneFromDirectory(mmapID, srcDir, nullptr);
    return Napi::Boolean::New(env, ok);
}

Napi::Value NodeMMKV::BackupAllToDirectory(const Napi::CallbackInfo &info) {
    auto env = info.Env();
    if (info.Length() < 1) {
        throw Napi::TypeError::New(env, "backupAllToDirectory expects dstDir");
    }
    auto dstDir = ToMMKVPath(RequireString(info[0], "dstDir"));
    std::optional<MMKVPath_t> srcDir;
    if (info.Length() > 1 && !info[1].IsUndefined() && !info[1].IsNull()) {
        srcDir = ToMMKVPath(RequireString(info[1], "srcDir"));
    }
    std::filesystem::create_directories(std::filesystem::path(dstDir));
    auto count = srcDir ? MMKVNative::backupAllToDirectory(dstDir, &(*srcDir))
                        : MMKVNative::backupAllToDirectory(dstDir, nullptr);
    return Napi::Number::New(env, static_cast<double>(count));
}

Napi::Value NodeMMKV::RestoreAllFromDirectory(const Napi::CallbackInfo &info) {
    auto env = info.Env();
    if (info.Length() < 1) {
        throw Napi::TypeError::New(env, "restoreAllFromDirectory expects srcDir");
    }
    auto srcDir = ToMMKVPath(RequireString(info[0], "srcDir"));
    std::optional<MMKVPath_t> dstDir;
    if (info.Length() > 1 && !info[1].IsUndefined() && !info[1].IsNull()) {
        dstDir = ToMMKVPath(RequireString(info[1], "dstDir"));
        std::filesystem::create_directories(std::filesystem::path(*dstDir));
    }
    auto count = dstDir ? MMKVNative::restoreAllFromDirectory(srcDir, &(*dstDir))
                        : MMKVNative::restoreAllFromDirectory(srcDir, nullptr);
    return Napi::Number::New(env, static_cast<double>(count));
}

} // namespace node_mmkv
