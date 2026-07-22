import path = require("node:path");
import load = require("node-gyp-build");

import type { MMKVModule } from "../index";

const binding = (load as (dir: string) => MMKVModule)(path.join(__dirname, ".."));

export = binding;
