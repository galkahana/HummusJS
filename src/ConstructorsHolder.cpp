#include "ConstructorsHolder.h"

using namespace v8;

ConstructorsHolder::ConstructorsHolder(Isolate* isolate, Local<Object> exports) {
    mExports.Reset(isolate, exports);
    mExports.SetWeak(this, DeleteMe, WeakCallbackType::kParameter);
}

ConstructorsHolder::~ConstructorsHolder() {
    if (!mExports.IsEmpty()) {
        mExports.ClearWeak();
        mExports.Reset();
    }
}


void ConstructorsHolder::DeleteMe(const WeakCallbackInfo<ConstructorsHolder>& info) {
    delete info.GetParameter();
}