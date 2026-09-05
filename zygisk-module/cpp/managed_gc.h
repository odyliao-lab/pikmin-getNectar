#pragma once

#include <cstdint>

namespace pikmin {

// APK-derived v152 arm64 GC handles are pointer-sized, not uint32_t indices.
// il2cpp_gchandle_free masks x0 to an 8 KiB page then reads its header.
using GcHandle = uintptr_t;
using GcHandleNew = GcHandle (*)(void *, bool);
using GcHandleFree = void (*)(GcHandle);
static_assert(sizeof(GcHandle) == sizeof(void *), "GC handle must retain every pointer bit");

class ScopedManagedRoot {
public:
    const GcHandle handle;
    ScopedManagedRoot(void *object, GcHandleNew create, GcHandleFree destroy)
        : handle(object && create && destroy ? create(object, false) : 0), destroy_(destroy) {}
    ~ScopedManagedRoot() { if (handle) destroy_(handle); }
    ScopedManagedRoot(const ScopedManagedRoot &) = delete;
    ScopedManagedRoot &operator=(const ScopedManagedRoot &) = delete;

private:
    GcHandleFree destroy_;
};

} // namespace pikmin
