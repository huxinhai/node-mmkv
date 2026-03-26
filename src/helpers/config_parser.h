#pragma once

#include "core/node_mmkv_binding.h"

namespace node_mmkv {

ParsedConfig ParseConfig(const Napi::Env &env, const Napi::Value &value);

} // namespace node_mmkv
