#pragma once

#include <napi.h>

#include "MMKV.h"

#include <cstdint>
#include <optional>
#include <string>

namespace node_mmkv {

std::string RequireString(const Napi::Value &value, const char *name);
bool GetOptionalBool(const Napi::Object &options, const char *name, bool defaultValue);
uint32_t GetOptionalUint32(const Napi::Object &options, const char *name, uint32_t defaultValue);
int64_t GetInt64Value(const Napi::Value &value, const char *name);
uint64_t GetUInt64Value(const Napi::Value &value, const char *name);
double GetDoubleValue(const Napi::Value &value, const char *name);
float GetFloatValue(const Napi::Value &value, const char *name);
std::optional<std::string> GetOptionalString(const Napi::Object &options, const char *name);
MMKVPath_t ToMMKVPath(const std::string &value);

} // namespace node_mmkv
