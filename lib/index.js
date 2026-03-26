"use strict";

const path = require("node:path");

const bindingPath = path.join(__dirname, "..", "build", "Release", "node_mmkv.node");
const native = require(bindingPath);

module.exports = native;
