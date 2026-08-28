#pragma once

#include <cstdint>
#include <jni.h>

// Installs the v152 expedition-list GPS observer into the already verified
// Pikmin process.  It shares the main module's libil2cpp base and never hooks
// when the two instruction signatures do not match.
void gps_copy_prepare(uintptr_t il2cpp_base, const char *game_data_dir, JavaVM *vm);
