//
// Created by lbert on 2/9/2023.
//
#ifndef ZYGISKPG_MISC_H
#define ZYGISKPG_MISC_H

#include "Includes/Dobby/dobbyForHooks.h"
#include "Include/Unity.h"
#include "KittyMemory/KittyMemory.h"
#include "KittyMemory/MemoryPatch.h"
#include "KittyMemory/KittyScanner.h"
#include "Include/obfuscate.h"
#include "hook.h"

using KittyMemory::ProcMap;

uint64_t libBaseAddress;
uint64_t libBaseEndAddress;
size_t libSize;
uintptr_t get_absolute_address(uintptr_t relative_addr){
    return (reinterpret_cast<uintptr_t>(libBaseAddress) + relative_addr);
}

uintptr_t* get_absolute_addresss(uintptr_t relative_addr){
    return (reinterpret_cast<uintptr_t*>(libBaseAddress) + relative_addr);
}

ProcMap g_il2cppBaseMap ;
void hook(void *offset, void* ptr, void **orig)
{
    DobbyHook(offset, ptr, orig);
}

void Hook(const char* pattern, void* ptr, void **orig, std::string mask)
{
    void* offset = (void*)KittyScanner::findHexFirst(libBaseAddress,libBaseEndAddress,std::string(pattern), mask);
    DobbyHook(offset, ptr, orig);
}

std::vector<MemoryPatch> memoryPatches;
std::vector<uint64_t> offsetVector;

// Patching a offset with switch.
void patchOffset(uint64_t offset, std::string hexBytes, bool isOn) {

    MemoryPatch patch = MemoryPatch::createWithHex(get_absolute_address(offset), hexBytes);

    //Check if offset exists in the offsetVector
    if (std::find(offsetVector.begin(), offsetVector.end(), offset) != offsetVector.end()) {
        //LOGE(OBFUSCATE("Already exists"));
        std::vector<uint64_t>::iterator itr = std::find(offsetVector.begin(), offsetVector.end(), offset);
        patch = memoryPatches[std::distance(offsetVector.begin(), itr)]; //Get index of memoryPatches vector
    } else {
        memoryPatches.push_back(patch);
        offsetVector.push_back(offset);
        //LOGI(OBFUSCATE("Added"));
    }

    if (isOn) {
        if (!patch.Modify()) {

        }
    } else {
        if (!patch.Restore()) {

        }
    }
}

uintptr_t string2Offset(const char *c) {
    int base = 16;
    // See if this function catches all possibilities.
    // If it doesn't, the function would have to be amended
    // whenever you add a combination of architecture and
    // compiler that is not yet addressed.
    static_assert(sizeof(uintptr_t) == sizeof(unsigned long)
                  || sizeof(uintptr_t) == sizeof(unsigned long long),
                  "");

    // Now choose the correct function ...
    if (sizeof(uintptr_t) == sizeof(unsigned long)) {
        return strtoul(c, nullptr, base);
    }

    // All other options exhausted, sizeof(uintptr_t) == sizeof(unsigned long long))
    return strtoull(c, nullptr, base);
}
#define HOOK(offset, ptr, orig) hook((void*)get_absolute_address(string2Offset(OBFUSCATE(offset))), (void*)ptr, (void **)&orig)
#define PHOOK(pattern, ptr, orig) Hook((void*)find_pattern(libBaseAddress,libBaseEndAddress,(OBFUSCATE(pattern))), (void *)ptr, (void **)&orig)
#define PATCH(offset, hex) patchOffset((string2Offset(OBFUSCATE(offset))), OBFUSCATE(hex), true)
#define PATCH_SWITCH(offset, hex, boolean) patchOffset(string2Offset(OBFUSCATE(offset)), OBFUSCATE(hex), boolean)
#define RESTORE(offset) patchOffset(string2Offset(OBFUSCATE(offset)), "", false)

#endif //ZYGISKPG_MISC_H
