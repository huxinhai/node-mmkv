declare module "node-gyp-build" {
  function load(dir: string): unknown;
  export = load;
}
