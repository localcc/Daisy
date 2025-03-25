#pragma once

namespace ds {
void DebugBreak();
}

#define DS_UNUSED(x) (void)x

#define DS_ASSERT(x, msg)                                                                                                                                      \
    if (!(x)) {                                                                                                                                                \
        ::ds::DebugBreak();                                                                                                                                    \
    }
