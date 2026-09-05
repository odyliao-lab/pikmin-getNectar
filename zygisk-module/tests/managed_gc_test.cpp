#include "../cpp/managed_gc.h"
#include <cassert>
#include <cstdio>
#include <type_traits>

using namespace pikmin;
static_assert(sizeof(GcHandle) == 8, "This regression test targets v152 arm64");
static_assert(!std::is_copy_constructible<ScopedManagedRoot>::value, "Never double-free copied roots");

constexpr GcHandle kHandle = UINT64_C(0x00000071f1347738);
int creates{}, frees{};
GcHandle released{};
GcHandle create(void *object, bool pinned) {
    assert(object && !pinned);
    ++creates;
    return kHandle;
}
GcHandle create_zero(void *, bool) { ++creates; return 0; }
void destroy(GcHandle handle) { ++frees; released = handle; }

int main() {
    int object{};
    {
        ScopedManagedRoot root(&object, create, destroy);
        assert(root.handle == kHandle);
        assert(creates == 1 && frees == 0);
    }
    assert(frees == 1 && released == kHandle);
    assert(released != static_cast<uint32_t>(kHandle));
    {
        ScopedManagedRoot no_object(nullptr, create, destroy);
        ScopedManagedRoot no_create(&object, nullptr, destroy);
        ScopedManagedRoot no_destroy(&object, create, nullptr);
        assert(!no_object.handle && !no_create.handle && !no_destroy.handle);
    }
    assert(creates == 1 && frees == 1);
    { ScopedManagedRoot zero(&object, create_zero, destroy); assert(!zero.handle); }
    assert(creates == 2 && frees == 1);
    std::puts("PASS: 64-bit handle survives create/storage/free; null guards; exactly-once release");
}
