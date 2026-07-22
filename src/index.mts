import { createRequire } from "node:module";
import { fileURLToPath } from "node:url";

const require = createRequire(import.meta.url);
const load = require("node-gyp-build") as (dir: string) => import("../index.d.ts").MMKVModule;
const packageRoot = fileURLToPath(new URL("..", import.meta.url));
const binding = load(packageRoot);

export const MMKV = binding.MMKV;
export const version = binding.version;
export default binding;
