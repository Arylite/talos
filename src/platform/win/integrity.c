#include "platform/talos_platform.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <bcrypt.h>

#include <stdint.h>
#include <string.h>

static int locate_own_text_section(void **out_address, size_t *out_size)
{
    HMODULE self_module = NULL;
    /*
     * GetModuleHandleExA's "from address" mode treats this as an
     * address, not a string; routing through uintptr_t avoids relying
     * on a direct function-pointer-to-object-pointer conversion, which
     * ISO C does not guarantee.
     */
    if (!GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
                             (LPCSTR)(uintptr_t)&locate_own_text_section, &self_module)) {
        return 0;
    }

    BYTE *base = (BYTE *)self_module;
    IMAGE_DOS_HEADER *dos_header = (IMAGE_DOS_HEADER *)base;
    if (dos_header->e_magic != IMAGE_DOS_SIGNATURE) {
        return 0;
    }

    IMAGE_NT_HEADERS *nt_headers = (IMAGE_NT_HEADERS *)(base + dos_header->e_lfanew);
    if (nt_headers->Signature != IMAGE_NT_SIGNATURE) {
        return 0;
    }

    IMAGE_SECTION_HEADER *section = IMAGE_FIRST_SECTION(nt_headers);
    for (WORD i = 0; i < nt_headers->FileHeader.NumberOfSections; i++, section++) {
        if (memcmp(section->Name, ".text", 5) == 0) {
            *out_address = base + section->VirtualAddress;
            *out_size = (size_t)section->Misc.VirtualSize;
            return 1;
        }
    }

    return 0;
}

talos_status talos_platform_hash_self_code(unsigned char out_hash[32])
{
    void *code_address = NULL;
    size_t code_size = 0;
    if (!locate_own_text_section(&code_address, &code_size)) {
        return TALOS_ERROR_UNKNOWN;
    }

    BCRYPT_ALG_HANDLE alg = NULL;
    if (BCryptOpenAlgorithmProvider(&alg, BCRYPT_SHA256_ALGORITHM, NULL, 0) != 0) {
        return TALOS_ERROR_UNKNOWN;
    }

    talos_status result = TALOS_ERROR_UNKNOWN;
    BCRYPT_HASH_HANDLE hash = NULL;
    if (BCryptCreateHash(alg, &hash, NULL, 0, NULL, 0, 0) == 0) {
        if (BCryptHashData(hash, (PUCHAR)code_address, (ULONG)code_size, 0) == 0 &&
            BCryptFinishHash(hash, out_hash, 32, 0) == 0) {
            result = TALOS_OK;
        }
        BCryptDestroyHash(hash);
    }

    BCryptCloseAlgorithmProvider(alg, 0);
    return result;
}
