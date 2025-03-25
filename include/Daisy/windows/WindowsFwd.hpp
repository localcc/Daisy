#pragma once
#include <cstdint>

namespace ds::wt {

using PVOID = void*;
using HANDLE = PVOID;
using HHOOK = HANDLE;
using HCMNOTIFICATION = HANDLE;
using HDEVINFO = PVOID;

using UINT_PTR = uint64_t;
using LONG_PTR = int64_t;

using WPARAM = UINT_PTR;
using LPARAM = LONG_PTR;
using LRESULT = LONG_PTR;

} // namespace ds::wt