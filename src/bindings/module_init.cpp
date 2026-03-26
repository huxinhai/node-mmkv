#include "core/node_mmkv_binding.h"

namespace node_mmkv {

Napi::Object NodeMMKV::Init(Napi::Env env, Napi::Object exports) {
    auto ctor = DefineClass(
        env,
        "MMKV",
        {
            StaticMethod("initialize", &NodeMMKV::Initialize),
            StaticMethod("onExit", &NodeMMKV::OnExit),
            StaticMethod("version", &NodeMMKV::Version),
            StaticMethod("defaultMMKV", &NodeMMKV::DefaultMMKV),
            StaticMethod("backupOneToDirectory", &NodeMMKV::BackupOneToDirectory),
            StaticMethod("restoreOneFromDirectory", &NodeMMKV::RestoreOneFromDirectory),
            StaticMethod("backupAllToDirectory", &NodeMMKV::BackupAllToDirectory),
            StaticMethod("restoreAllFromDirectory", &NodeMMKV::RestoreAllFromDirectory),
            InstanceMethod("setBool", &NodeMMKV::SetBool),
            InstanceMethod("getBool", &NodeMMKV::GetBool),
            InstanceMethod("setInt32", &NodeMMKV::SetInt32),
            InstanceMethod("getInt32", &NodeMMKV::GetInt32),
            InstanceMethod("setUInt32", &NodeMMKV::SetUInt32),
            InstanceMethod("getUInt32", &NodeMMKV::GetUInt32),
            InstanceMethod("setUInt64", &NodeMMKV::SetUInt64),
            InstanceMethod("getUInt64", &NodeMMKV::GetUInt64),
            InstanceMethod("setInt64", &NodeMMKV::SetInt64),
            InstanceMethod("getInt64", &NodeMMKV::GetInt64),
            InstanceMethod("setFloat", &NodeMMKV::SetFloat),
            InstanceMethod("getFloat", &NodeMMKV::GetFloat),
            InstanceMethod("setDouble", &NodeMMKV::SetDouble),
            InstanceMethod("getDouble", &NodeMMKV::GetDouble),
            InstanceMethod("setString", &NodeMMKV::SetString),
            InstanceMethod("getString", &NodeMMKV::GetString),
            InstanceMethod("setBuffer", &NodeMMKV::SetBuffer),
            InstanceMethod("getBuffer", &NodeMMKV::GetBuffer),
            InstanceMethod("containsKey", &NodeMMKV::ContainsKey),
            InstanceMethod("removeValueForKey", &NodeMMKV::RemoveValueForKey),
            InstanceMethod("removeValuesForKeys", &NodeMMKV::RemoveValuesForKeys),
            InstanceMethod("allKeys", &NodeMMKV::AllKeys),
            InstanceMethod("count", &NodeMMKV::Count),
            InstanceMethod("totalSize", &NodeMMKV::TotalSize),
            InstanceMethod("actualSize", &NodeMMKV::ActualSize),
            InstanceMethod("clearAll", &NodeMMKV::ClearAll),
            InstanceMethod("trim", &NodeMMKV::Trim),
            InstanceMethod("sync", &NodeMMKV::Sync),
            InstanceMethod("lock", &NodeMMKV::Lock),
            InstanceMethod("unlock", &NodeMMKV::Unlock),
            InstanceMethod("tryLock", &NodeMMKV::TryLock),
            InstanceMethod("reKey", &NodeMMKV::ReKey),
            InstanceMethod("cryptKey", &NodeMMKV::CryptKey),
            InstanceMethod("close", &NodeMMKV::Close),
        });

    constructor = Napi::Persistent(ctor);
    constructor.SuppressDestruct();

    exports.Set("MMKV", ctor);
    exports.Set("version", Napi::String::New(env, MMKV_VERSION));
    return exports;
}

} // namespace node_mmkv

Napi::Object InitModule(Napi::Env env, Napi::Object exports) {
    return node_mmkv::NodeMMKV::Init(env, exports);
}

NODE_API_MODULE(mmkv, InitModule)
