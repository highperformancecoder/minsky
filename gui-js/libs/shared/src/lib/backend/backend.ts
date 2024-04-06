import JSON5 from 'json5';

export class CppClass
{
  public static backend : (...args)=>any; // pass command to destination: see backend-init and electron.service
  public static backendSync : (...args)=>any; // pass command to destination: see backend-init and electron.service
  public static record=(cmd: string)=>{}; // recording support: see RecordingsManager
  public static logMessage : (message:string)=>void;
  protected m_prefix: string;
  constructor(prefix: string) {this.m_prefix=prefix;}

  // the following methods start with $ to avoid clashing with any auto-generated C++ methods in derived classes
  protected async $callMethod(method: string,...args): Promise<any>
  {
    return CppClass.backend(`${this.m_prefix}.${method}`, ...args);
  }
  // calls the method on this thread
  public $callMethodSync(method: string,...args)
  {
    return CppClass.backendSync(`${this.m_prefix}.${method}`, ...args);
  }
  public async $properties(...args) {return CppClass.backend(this.m_prefix, ...args);}
  public async $list(): Promise<string[]> {return this.$callMethod("@list");} // $ prevents this method from being shadowed by a C++ method
  /// returns if this proxy object and x refer to the same backend object
  public $equal(x: CppClass): boolean {return this.m_prefix===x.m_prefix;}
  public $prefix(): string {return this.m_prefix;}
};


export class Pair<Key, Value> {
  first: Key;
  second: Value;
  constructor(key: Key, value: Value) {
    this.first=key;
    this.second=value;
  }
};

export class Map<Key, Value> extends CppClass
{
  valueType: any;  // stash a reference to the actual type here, for use in a new expression
  constructor(prefix: string, valueType: any=null) {super(prefix); this.valueType=valueType;}
  elem(key: Key) {
    const cmd=`${this.m_prefix}.@elem.${JSON5.stringify(key)}`;
    // if proxy type provided, instantiate that, otherwise return the current value
    return this.valueType? new this.valueType(cmd): this.$callMethod(cmd);
  }
  insert(key: Key, value: Value) {this.$callMethod("@insert",{first: key, second:value});}
  erase(key: Key) {this.$callMethod("@erase",key);}
  size(): Promise<number> {return this.$callMethod("@size");}
  keys(): Promise<Key[]> {return this.$callMethod("@keys");}
};

export class Container<Key,Value=Key> extends CppClass
{
  type: any;
  constructor(prefix: string, type: any=null) {super(prefix); this.type=type}
  elem(key: Key) {
    // if proxy type provided, instantiate that, otherwise return the current value
    const cmd=`${this.m_prefix}.@elem.${JSON5.stringify(key)}`;
    return this.type? new this.type(cmd): this.$callMethod(cmd);
  }
  insert(key: Key) {this.$callMethod("@insert",key);}
  erase(key: Key) {this.$callMethod("@erase",key);}
  size(): Promise<number> {return this.$callMethod("@size");}
  properties(...args): Promise<Value[]> {return super.$properties(...args) as Promise<Value[]>;}
};

export class Sequence<Value> extends Container<number,Value>
  {
    constructor(prefix: string, type: any=null) {super(prefix,type);}
  }

