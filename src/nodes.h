#pragma once

#include <node.h> 


#define NODE_0_10_MODULE_VERSION 11

#if NODE_MODULE_VERSION > NODE_0_10_MODULE_VERSION
#include <node_object_wrap.h>

#define ARGS_TYPE v8::FunctionCallbackInfo<v8::Value>
#define PROPERTY_TYPE v8::PropertyCallbackInfo<v8::Value>
#define PROPERTY_SETTER_TYPE v8::PropertyCallbackInfo<void>
#define METHOD_RETURN_TYPE void
#define CREATE_ISOLATE_CONTEXT Isolate* isolate = Isolate::GetCurrent()
#define NEW_FUNCTION_TEMPLATE(X) FunctionTemplate::New(isolate, X)
#define NEW_STRING(X) String::NewFromUtf8(isolate, X)
#define NEW_SYMBOL(X) NEW_STRING(X)
#define NEW_NUMBER(X) Number::New(isolate,X)
#define NEW_INTEGER(X) Integer::New(isolate,X)
#define NEW_ARRAY(X) Array::New(isolate,X)
#define NEW_BOOLEAN(X) Boolean::New(isolate,X)
#define NEW_OBJECT Object::New(isolate)
#define SET_ACCESSOR_METHOD(t,s,f) t->InstanceTemplate()->SetAccessor(NEW_STRING(s), f);
#define SET_ACCESSOR_METHODS(t,s,f,g) t->InstanceTemplate()->SetAccessor(NEW_STRING(s), f,g);
#define SET_CONSTRUCTOR_EXPORT(t,s,c) t->Set(NEW_STRING(s),c->GetFunction())
#define SET_PROTOTYPE_METHOD(t, s, f) NODE_SET_PROTOTYPE_METHOD(t,s,f)
#define SET_PERSISTENT_OBJECT(c,ot,t) c.Reset(isolate,t)
#define SET_CONSTRUCTOR(c,t) c.Reset(isolate, t->GetFunction())
#define CREATE_SCOPE HandleScope scope(isolate)
#define CREATE_ESCAPABLE_SCOPE v8::EscapableHandleScope scope(isolate)
#define NEW_INSTANCE(c) Local<Function>::New(isolate, c)->NewInstance()
#define NEW_INSTANCE_ARGS(c,argc,argv) Local<Function>::New(isolate, c)->NewInstance(argc,argv)
#define SET_FUNCTION_RETURN_VALUE(v) args.GetReturnValue().Set(v)
#define SET_ACCESSOR_RETURN_VALUE(v) info.GetReturnValue().Set(v)
#define HAS_INSTANCE(c,o) Local<FunctionTemplate>::New(isolate, c)->HasInstance(o->ToObject())
#define UNDEFINED Undefined(isolate)
#define THROW_EXCEPTION(s) isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,s)))
#define DISPOSE_PERSISTENT(p) p.Reset()
#define OBJECT_FROM_PERSISTENT(p) Local<Object>::New(isolate, p)
#define GET_CURRENT_CONTEXT v8::Isolate::GetCurrent()->GetCurrentContext()
#define CLOSE_SCOPE(v) scope.Escape(v)
#define ESCAPABLE_HANDLE(v) Local<v>

#else

#define ARGS_TYPE v8::Arguments
#define PROPERTY_TYPE v8::AccessorInfo
#define PROPERTY_SETTER_TYPE v8::AccessorInfo
#define METHOD_RETURN_TYPE v8::Handle<v8::Value>
#define CREATE_ISOLATE_CONTEXT
#define NEW_FUNCTION_TEMPLATE(X) FunctionTemplate::New(X)
#define NEW_STRING(X) String::New(X)
#define NEW_SYMBOL(X) String::NewSymbol(X)
#define NEW_NUMBER(X) Number::New(X)
#define NEW_INTEGER(X) Integer::New(X)
#define NEW_ARRAY(X) Array::New(X)
#define NEW_BOOLEAN(X) Boolean::New(X)
#define NEW_OBJECT Object::New()
#define SET_ACCESSOR_METHOD(t,s,f) t->InstanceTemplate()->SetAccessor(String::NewSymbol(s), f);
#define SET_ACCESSOR_METHODS(t,s,f,g) t->InstanceTemplate()->SetAccessor(String::NewSymbol(s), f,g);
#define SET_CONSTRUCTOR_EXPORT(t,s,c) t->Set(String::NewSymbol(s),c->GetFunction())
#define SET_PROTOTYPE_METHOD(t,s,f) t->PrototypeTemplate()->Set(String::NewSymbol(s),FunctionTemplate::New(f)->GetFunction())
#define SET_PERSISTENT_OBJECT(c,ot,t) c =Persistent<ot>::New(t);
#define SET_CONSTRUCTOR(c,t) SET_PERSISTENT_OBJECT(c,Function,t->GetFunction())
#define CREATE_SCOPE v8::HandleScope scope
#define CREATE_ESCAPABLE_SCOPE v8::HandleScope scope
#define NEW_INSTANCE(c) c->NewInstance()
#define NEW_INSTANCE_ARGS(c,argc,argv) c->NewInstance(argc,argv)
#define SET_ACCESSOR_RETURN_VALUE(v) return scope.Close(v)
#define SET_FUNCTION_RETURN_VALUE(v) return scope.Close(v)
#define HAS_INSTANCE(c,o) c->HasInstance(o->ToObject())
#define UNDEFINED Undefined()
#define THROW_EXCEPTION(s) ThrowException(Exception::TypeError(String::New(s)))
#define DISPOSE_PERSISTENT(p) p.Dispose()
#define OBJECT_FROM_PERSISTENT(p) p
#define GET_CURRENT_CONTEXT v8::Context::GetCurrent()
#define CLOSE_SCOPE(v) scope.Close(v)
#define ESCAPABLE_HANDLE(v) Handle<v>
#endif

#define SET_CONSTRUCTOR_TEMPLATE(c,t) SET_PERSISTENT_OBJECT(c,FunctionTemplate,t)
