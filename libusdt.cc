#include <v8.h>
#include <node.h>

#include <string.h>

extern "C" {
#include <usdt.h>
}


namespace libusdt {

  using namespace node;
  using namespace v8;

  /**
   * @param args[0] <String> is the PROVIDER
   * @param args[1] <String> is the MODULE
   */
  static Handle<Value> CreateProvider(const Arguments& args){
    HandleScope scope;

    String::Utf8Value name   (args[0]->ToString());
    String::Utf8Value module (args[1]->ToString());

    // create a probe identified as PROVIDER:MODULE
    usdt_provider_t *provider = usdt_create_provider(*name, *module);

    Handle<Value> ext = External::Wrap(provider);

    return scope.Close(ext);
  };

  /**
   * @param args[0] <String> is the FUNCTION
   * @param args[1] <String> is the NAME
   * @return <External> a provider external object
   */
  static Handle<Value> CreateProbe(const Arguments& args){
    HandleScope scope;

    String::Utf8Value func (args[0]->ToString());
    String::Utf8Value name (args[1]->ToString());

    Local<Array> array = Local<Array>::Cast(args[2]);

    // the number of arguments
    size_t argc = array->Length();

    // these are pointers to argument types
    // e.g.
    // types[0] -> "char *"
    // tyeps[1] -> "int"
    const char *types[USDT_ARG_MAX];

    for (size_t i = 0; i < argc; i++) {
      String::Utf8Value type(array->Get(i));
      types[i] = strdup(*type);
    };

    // create a probe identified as FUNCTION:NAME
    usdt_probedef_t *probe = usdt_create_probe(*func, *name, argc, types);

    for (size_t i = 0; i < argc; i++) {
      free((void *) types[i]);
    }

    Handle<Value> ext = External::Wrap(probe);

    return scope.Close(ext);
  };

  /**
   * @param args[0] <External> is the provider object returned by CreateProvider
   * @param args[1] <External> is the probe object returned by CreateProbe
   * @return <External> a probe external object
   */
  static Handle<Value> AddProbe(const Arguments& args){
    HandleScope scope;

    Local<External> ext0 = Local<External>::Cast(args[0]);
    Local<External> ext1 = Local<External>::Cast(args[1]);

    usdt_provider_t *provider = (usdt_provider_t*) External::Unwrap(ext0);
    usdt_probedef_t *probedef = (usdt_probedef_t*) External::Unwrap(ext1);

    int res = usdt_provider_add_probe(provider, probedef);

    return scope.Close(Number::New(res));
  };

  /**
   * @param args[0] <External> is a probe object
   * @return <Bool> true if the probe is enabled by DTrace
   */
  static Handle<Value> Enabled(const Arguments& args) {
    HandleScope scope;
    Local<External> ext0 = Local<External>::Cast(args[0]);
    usdt_probedef_t *probe = (usdt_probedef_t *) External::Unwrap(ext0);

    int ret = usdt_is_enabled(probe->probe);

    return scope.Close(Number::New(ret));
  };

  // should have size of void *
  // i don't know how to enforce this
  typedef struct trace_type {
    union {
      bool     b;
      char *   s;
      void *   v;
      uint64_t i64;
    };
  } trace_type_t;

  // // these are used in FireProbe
  void stringify(Handle<Value> val, trace_type_t *t) {
    String::Utf8Value name(val->ToString());
    t->s = *name;
  };
  void intify(Handle<Value> val, trace_type_t *t) {
    t->i64 = val->ToInt32()->Value();
  };

  /**
   * @param args[0] <External> is the probe added in AddProbe
   * @param args[1] <Array> is an array matching the type signature of CreateProbe
   */
  static Handle<Value> FireProbe(const Arguments& args){
    HandleScope scope;

    Local<External> ext0 = Local<External>::Cast(args[0]);
    usdt_probedef_t *probe = (usdt_probedef_t *) External::Unwrap(ext0);

    Local<Array> array = Local<Array>::Cast(args[1]);
    size_t argc = array->Length();

    trace_type_t argv[USDT_ARG_MAX];

    // copy the items from the array into the probe payload
    for (size_t i = 0; i < argc; i++) {
      Local<Value> item = array->Get(i);

      // convert types as necessary
      // make sure you pass in the right types to start with
      if (item->IsInt32()) {
        intify(item, &argv[i]);
      } else {
        stringify(item, &argv[i]);
      }
    }

    // emit the probe event
    usdt_fire_probe(probe->probe, argc, (void**) argv);

    return scope.Close(Undefined());
  };

  /**
   * @param args[0] <External> is a provider object
   */
  static Handle<Value> Enable(const Arguments& args){
    HandleScope scope;

    Local<External> ext = Local<External>::Cast(args[0]);
    usdt_provider_t * provider = (usdt_provider_t *) External::Unwrap(ext);

    int ret = usdt_provider_enable(provider);

    return scope.Close(Number::New(ret));
  };

  static void init(Handle<Object> target) {
    target->Set(String::NewSymbol("create_provider"),
      FunctionTemplate::New(CreateProvider)->GetFunction() );
    target->Set(String::NewSymbol("create_probe"),
      FunctionTemplate::New(CreateProbe)->GetFunction() );
    target->Set(String::NewSymbol("add_probe"),
      FunctionTemplate::New(AddProbe)->GetFunction() );
    target->Set(String::NewSymbol("enable"),
      FunctionTemplate::New(Enable)->GetFunction() );
    target->Set(String::NewSymbol("enabled"),
      FunctionTemplate::New(Enabled)->GetFunction() );
    target->Set(String::NewSymbol("fire"),
      FunctionTemplate::New(FireProbe)->GetFunction() );
  }

  NODE_MODULE(USDT, init)
};
