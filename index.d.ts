export declare enum MMKVLogLevel {
  Debug = 0,
  Info = 1,
  Warning = 2,
  Error = 3,
  None = 4,
}

export interface MMKVOptions {
  multiProcess?: boolean;
  readOnly?: boolean;
  expectedCapacity?: number;
  rootPath?: string;
  cryptKey?: string;
  aes256?: boolean;
}

export type MMKVIntegerLike = bigint | number;

export declare class MMKV {
  constructor(id: string, options?: MMKVOptions);

  static initialize(rootDir: string, logLevel?: MMKVLogLevel | number): string;
  static onExit(): void;
  static version(): string;
  static defaultMMKV(options?: MMKVOptions): MMKV;
  static backupOneToDirectory(mmapID: string, dstDir: string, srcDir?: string): boolean;
  static restoreOneFromDirectory(mmapID: string, srcDir: string, dstDir?: string): boolean;
  static backupAllToDirectory(dstDir: string, srcDir?: string): number;
  static restoreAllFromDirectory(srcDir: string, dstDir?: string): number;

  setBool(key: string, value: boolean): boolean;
  getBool(key: string, defaultValue?: boolean): boolean;
  setInt32(key: string, value: number): boolean;
  getInt32(key: string, defaultValue?: number): number;
  setUInt32(key: string, value: number): boolean;
  getUInt32(key: string, defaultValue?: number): number;
  setUInt64(key: string, value: MMKVIntegerLike): boolean;
  getUInt64(key: string, defaultValue?: MMKVIntegerLike): bigint;
  setInt64(key: string, value: MMKVIntegerLike): boolean;
  getInt64(key: string, defaultValue?: MMKVIntegerLike): bigint;
  setFloat(key: string, value: number): boolean;
  getFloat(key: string, defaultValue?: number): number;
  setDouble(key: string, value: number): boolean;
  getDouble(key: string, defaultValue?: number): number;
  setString(key: string, value: string): boolean;
  getString(key: string): string | null;
  setBuffer(key: string, value: Buffer): boolean;
  getBuffer(key: string): Buffer | null;
  containsKey(key: string): boolean;
  removeValueForKey(key: string): boolean;
  removeValuesForKeys(keys: string[]): boolean;
  allKeys(): string[];
  count(): number;
  totalSize(): number;
  actualSize(): number;
  clearAll(keepSpace?: boolean): void;
  trim(): void;
  sync(async?: boolean): void;
  lock(): void;
  unlock(): void;
  tryLock(): boolean;
  reKey(cryptKey?: string | null, aes256?: boolean): boolean;
  cryptKey(): string | null;
  close(): void;
}

export interface MMKVModule {
  MMKV: typeof MMKV;
  version: string;
}

export declare const version: string;
