#pragma once

#include <node.h> 
#include <node_object_wrap.h>


#define NODE_2_5_0_MODULE_VERSION 44
#define NODE_10_0_0_MODULE_VERSION 64
#define NODE_11_0_0_MODULE_VERSION 67
#define NODE_CONTEXT_AWARE_VERSION NODE_10_0_0_MODULE_VERSION
#define IS_CONTEXT_AWARE NODE_MODULE_VERSION >= NODE_CONTEXT_AWARE_VERSION
#ifdef NODE_MODULE_INIT
    #define IS_CONTEXT_AWARE_MODULE 1
#else 
    #define IS_CONTEXT_AWARE_MODULE 0
#endif


#define ARGS_TYPE v8::FunctionCallbackInfo<v8::Value>
#define PROPERTY_TYPE v8::PropertyCallbackInfo<v8::Value>
#define PROPERTY_SETTER_TYPE v8::PropertyCallbackInfo<void>
#define METHOD_RETURN_TYPE void
#define CREATE_ISOLATE_CONTEXT Isolate* isolate = Isolate::GetCurrent()
#define NEW_FUNCTION_TEMPLATE(X) FunctionTemplate::New(isolate, X)
#define NEW_STRING(X) String::NewFromUtf8(isolate, X, v8::NewStringType::kNormal).ToLocalChecked()
#define NEW_SYMBOL(X) NEW_STRING(X)
#define NEW_NUMBER(X) Number::New(isolate,X)
#define NEW_INTEGER(X) Integer::New(isolate,X)
#define NEW_ARRAY(X) Array::New(isolate,X)
#define NEW_BOOLEAN(X) Boolean::New(isolate,X)
#define NEW_OBJECT Object::New(isolate)
#define SET_ACCESSOR_METHOD(t,s,f) t->InstanceTemplate()->SetAccessor(NEW_STRING(s), f);
#define SET_ACCESSOR_METHODS(t,s,f,g) t->InstanceTemplate()->SetAccessor(NEW_STRING(s), f,g);
#define SET_CONSTRUCTOR_EXPORT(s,c) EXPORTS_SET(exports,NEW_STRING(s),c->GetFunction(GET_CURRENT_CONTEXT).ToLocalChecked())
#define SET_PROTOTYPE_METHOD(t, s, f) NODE_SET_PROTOTYPE_METHOD(t,s,f)
#define SET_PERSISTENT_OBJECT(c,ot,t) c.Reset(isolate,t)
#define CREATE_SCOPE HandleScope scope(isolate)
#define CREATE_ESCAPABLE_SCOPE v8::EscapableHandleScope scope(isolate)
#define SET_FUNCTION_RETURN_VALUE(v) {args.GetReturnValue().Set(v); return;}
#define SET_ACCESSOR_RETURN_VALUE(v) {info.GetReturnValue().Set(v); return;}
#define HAS_INSTANCE(c,o) Local<FunctionTemplate>::New(isolate, c)->HasInstance(o->TO_OBJECT())
#define UNDEFINED Undefined(isolate)
#define THROW_EXCEPTION(s) isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,s,v8::NewStringType::kNormal).ToLocalChecked()))
#define DISPOSE_PERSISTENT(p) p.Reset()
#define OBJECT_FROM_PERSISTENT(p) Local<Object>::New(isolate, p)
#define GET_CURRENT_CONTEXT v8::Isolate::GetCurrent()->GetCurrentContext()
#define CLOSE_SCOPE(v) scope.Escape(v)
#define ESCAPABLE_HANDLE(v) Local<v>
#define THIS_HANDLE (this->handle())
#define SET_CONSTRUCTOR_TEMPLATE(c,t) SET_PERSISTENT_OBJECT(c,FunctionTemplate,t)

// some conversions
#if NODE_MODULE_VERSION > NODE_2_5_0_MODULE_VERSION

#define SET_CONSTRUCTOR(c,t) c.Reset(isolate, t->GetFunction(GET_CURRENT_CONTEXT).ToLocalChecked())
#define NEW_INSTANCE(c,i) Local<Function> c1 = Local<Function>::New(isolate, c); Local<Object> i = Local<Function>::New(isolate, c1)->NewInstance(GET_CURRENT_CONTEXT).ToLocalChecked()
#define NEW_INSTANCE_ARGS(c,i,argc,argv) Local<Function> c1 = Local<Function>::New(isolate, c); Local<Object> i = Local<Function>::New(isolate, c1)->NewInstance(GET_CURRENT_CONTEXT,argc,argv).ToLocalChecked()
#define TO_NUMBER(x) x->ToNumber(GET_CURRENT_CONTEXT).ToLocalChecked()
#define TO_UINT32(x) x->ToUint32(GET_CURRENT_CONTEXT).ToLocalChecked()
#define TO_INT32(x) x->ToInt32(GET_CURRENT_CONTEXT).ToLocalChecked()
#define TO_UINT32Value() ToUint32(GET_CURRENT_CONTEXT).ToLocalChecked()->Value()

#else 

#define SET_CONSTRUCTOR(c,t) c.Reset(isolate, t->GetFunction(GET_CURRENT_CONTEXT).ToLocalChecked())
#define NEW_INSTANCE(c,i) Local<Object> i = Local<Function>::New(isolate, c)->NewInstance()
#define NEW_INSTANCE_ARGS(c,i,argc,argv) Local<Object> i = Local<Function>::New(isolate, c)->NewInstance(argc,argv)
#define TO_NUMBER(x) x->ToNumber()
#define TO_UINT32(x) x->ToUint32()
#define TO_INT32(x) x->ToInt32()
#define TO_UINT32Value() ToUint32()->Value()

#endif

// Some more conversions
#if IS_CONTEXT_AWARE
    #define TO_STRING() ToString(GET_CURRENT_CONTEXT).FromMaybe(Local<String>())
    #define TO_OBJECT() ToObject(GET_CURRENT_CONTEXT).FromMaybe(Local<Object>())
    #define TO_BOOLEAN() ToBoolean(isolate)
    #define UTF_8_VALUE(x) String::Utf8Value(isolate, x)

#else 
    #define TO_STRING() ToString()
    #define TO_OBJECT() ToObject()
    #define TO_BOOLEAN() ToBoolean()
    #define UTF_8_VALUE(x) String::Utf8Value(x)

#endif

// Strict context aware definitions, to support node threadening
#if IS_CONTEXT_AWARE_MODULE
	#define NODES_MODULE(m,f) NODE_MODULE_INIT() {f(exports, context);}
    #define EXPORTS_SET(e,k,v) e->Set(context, k,v);
    #define CALL_INIT_WITH_EXPORTS(f) f(exports, context, external);
    #define DEF_INIT(f) void f(Local<Object> exports, Local<Context> context)
    #define DEF_SUBORDINATE_INIT(f) void f(Local<Object> exports, Local<Context> context, Local<External> external)
    #define DEC_SUBORDINATE_INIT(f) static void f(v8::Local<v8::Object> exports, v8::Local<v8::Context> context, v8::Local<v8::External> external);
    #define NEW_FUNCTION_TEMPLATE_EXTERNAL(X) FunctionTemplate::New(isolate, X, external)

    #define EXPOSE_EXTERNAL(C, c, e) C* c = reinterpret_cast<C*>(e->Value());
    #define EXPOSE_EXTERNAL_FOR_INIT(C, c) EXPOSE_EXTERNAL(C, c, external)
    #define EXPOSE_EXTERNAL_ARGS(C, c) EXPOSE_EXTERNAL(C, c, args.Data().As<External>())
    #define DECLARE_EXTERNAL_DE_CON_STRUCTORS(C) v8::Persistent<v8::Object> mExports; \
                                            C(v8::Isolate* isolate, v8::Local<v8::Object> exports); \
                                            static void DeleteMe(const v8::WeakCallbackInfo<C>& info); \
                                            virtual ~C();
    #define DEFINE_EXTERNAL_DE_CON_STRUCTORS(C) \
        C::C(Isolate* isolate, Local<Object> exports) { \
            mExports.Reset(isolate, exports); \
            mExports.SetWeak(this, DeleteMe, WeakCallbackType::kParameter); \
        } \
        C::~C() { \
            if (!mExports.IsEmpty()) { \
                mExports.ClearWeak(); \
                mExports.Reset(); \
            } \
        } \
        void C::DeleteMe(const WeakCallbackInfo<C>& info) { \
            delete info.GetParameter(); \
        }

    // creates external instance on exports, when using externals
    #define DECLARE_EXTERNAL(C) C* c1 = new C(isolate, exports); Local<External> external = External::New(isolate, c1); 
#else 
	#define NODES_MODULE(m,f) NODE_MODULE(m, f)
    #define EXPORTS_SET(e,k,v) e->Set(k,v);
    #define CALL_INIT_WITH_EXPORTS(f) f(exports);
    #define DEF_INIT(f) void f(Local<Object> exports)
    #define DEF_SUBORDINATE_INIT(f) void f(Local<Object> exports)
    #define DEC_SUBORDINATE_INIT(f) static void f(v8::Local<v8::Object> exports);
    #define NEW_FUNCTION_TEMPLATE_EXTERNAL(X) NEW_FUNCTION_TEMPLATE(X)

    #define EXPOSE_EXTERNAL(C, c) C* c = C::GetInstance();
    #define EXPOSE_EXTERNAL_FOR_INIT(C, c) EXPOSE_EXTERNAL(C, c)
    #define EXPOSE_EXTERNAL_ARGS(C, c) EXPOSE_EXTERNAL(C, c)
    #define DECLARE_EXTERNAL_DE_CON_STRUCTORS(C) C(); \
                                            virtual ~C(); \
                                            static C* GetInstance();        
    #define DEFINE_EXTERNAL_DE_CON_STRUCTORS(C) \
        C::C(){} \
        C::~C(){} \
        C _instance; \
        C* C::GetInstance(){return &_instance;} 

    // the following is empty because external is defined as a shared static instance, when external is not used
    #define DECLARE_EXTERNAL(C) 
 
#endif