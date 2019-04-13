#pragma once

#include "nodes.h"

class ConstructorsHolder {

    public:
        ConstructorsHolder(v8::Isolate* isolate, v8::Local<v8::Object> exports);
        virtual ~ConstructorsHolder();

        // the interesting stuff (feel free to set/get directly)
        v8::Persistent<v8::Function> PDFWriter_constructor;

 private:
    static void DeleteMe(const v8::WeakCallbackInfo<ConstructorsHolder>& info);

    v8::Persistent<v8::Object> mExports;

};