#include "AddressSpace.h"
#include <Ty/Defer.h>
#include <Ty/ErrorOr.h>
#include <Ty/System.h>

namespace Mem::Internal {

ErrorOr<void> init(uptr base, uptr size)
{
    auto page_size = TRY(System::page_size());
    auto flags = MAP_PRIVATE | MAP_ANONYMOUS | MAP_32BIT;
#if __linux__
    flags |= MAP_FIXED_NOREPLACE;
#endif
    TRY(System::mmap((void*)base, size, PROT_READ | PROT_WRITE,
        flags));
    bool should_unmap_region = true;
    Defer unmap_region = [&] {
        if (should_unmap_region) {
            System::munmap((void*)base, size).ignore();
        }
    };
    TRY(System::mprotect((void*)(base + size - page_size),
        page_size, PROT_NONE));
    should_unmap_region = false;
    return {};
}

ErrorOr<void> deinit(uptr base, uptr size)
{
    TRY(System::munmap((void*)base, size));
    return {};
}

}
