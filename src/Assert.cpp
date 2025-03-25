#include <Daisy/Assert.hpp>

namespace ds {
#if defined(__GNUC__)
void DebugBreak() { __builtin_trap(); }
#else
void DebugBreak() { __debugbreak(); }
#endif
} // namespace ds