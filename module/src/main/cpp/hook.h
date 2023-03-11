
#ifndef ZygiskImGui_HOOK_H
#define ZygiskImGui_HOOK_H

#include <jni.h>
#include "Include/Vector3.h"
#include "Include/Unity.h"
#include "Include/GameTypes.h"

// function predefines
static int enable_hack;
static char *game_data_dir = NULL;
int isGame(JNIEnv *env, jstring appDataDir);
void *hack_thread(void *arg);

// In Game Function Predefines
monoList<void**>* (*getAllCharacters)(void* obj);
Vector3 (*get_Position)(void* transform);
Vector3 (*WorldToScreen)(void*, Vector3 worldPos, int);
void* (*get_camera)();
void* (*get_CharacterBodyPart)(void* obj, int);
void (*SetResolution)(int widht, int height, bool fullscreen);
int (*get_Width)();
int (*get_Height)();
int (*getLocalId)(void* obj);
void* (*getPlayer)(void* obj, int id);
void* (*getLocalPlayer)(void* obj);
int (*getCharacterCount)(void* obj);
int (*get_Health)(void* character);
void* (*get_Player)(void* character);
void (*RaycastCharacters)(void* pSys, void* shooter , Ray ray);
void(*set_targetFrameRate)(int frames);
bool (*get_IsInitialized)(void* character);

// Custom functions
void* getTransform(void* character);
int get_CharacterTeam(void* character);
int get_PlayerTeam(void* player);
std::string get_CharacterName(void* character);
std::string get_characterWeaponName(void* character);
const char* get_characterArmors(void* character);
Vector3 getBonePosition(void* character, int bone);



// oHooks
float (*old_get_fieldOfView)(void *instance);
void* (*oldShaderFind)(std::string name);
void(*oldGameSystemUpdate)(void* obj);
void(*oldRenderOverlayFlashbang)(void* obj);
void(*oldset_Spread)(void*obj);
void(*oldRenderOverlaySmoke)(void* obj);
void(*oldDrawRenderer)(void* obj);
EGLBoolean (*old_eglSwapBuffers)(EGLDisplay dpy, EGLSurface surface);

// Hooks
float get_fieldOfView(void *instance);
void* ShaderFind(std::string name);
void GameSystemUpdate(void* obj);
void RenderOverlayFlashbang(void* obj);
void set_Spread(void* obj);
void RenderOverlaySmoke(void* obj);
void DrawRenderer(void* obj);
EGLBoolean hook_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface);

// Initializers
void Hooks();
void Pointers();
void Patches();

#include <android/log.h>

uintptr_t find_pattern(uint8_t* start, const size_t length, const char* pattern) {
    const char* pat = pattern;
    uint8_t* first_match = 0;
    for (auto current_byte = start; current_byte < (start + length); ++current_byte) {
        if (*pat == '?' || *current_byte == strtoul(pat, NULL, 16)) {
            if (!first_match)
                first_match = current_byte;
            if (!pat[2])
                return (uintptr_t)first_match;
            pat += *(uint16_t*)pat == 16191 || *pat != '?' ? 3 : 2;
        }
        else if (first_match) {
            current_byte = first_match;
            pat = pattern;
            first_match = 0;
        }
    } return 0;
}

struct lib_info{
    void* start_address;
    void* end_address;
    intptr_t size;
    std::string name;
};

lib_info find_library(const char* module_name) {
    lib_info library_info{};
    char line[512], mod_name[64];

    FILE* fp = fopen("/proc/self/maps", "rt");
    if (fp != nullptr) {
        while (fgets(line, sizeof(line), fp)) {
            if (strstr(line, module_name)) {
                sscanf(line, "%lx-%lx %*s %*s %*s %*d %s",
                       (long unsigned *)&library_info.start_address,
                       (long unsigned*)&library_info.end_address, mod_name);

                library_info.size = reinterpret_cast<uintptr_t>(library_info.end_address) -
                                    reinterpret_cast<uintptr_t>(library_info.start_address);

                if (library_info.name.empty()) {
                    library_info.name = mod_name;
                }

                break;
            }
        }
        fclose(fp);
    }

    return library_info;
}




#define LOG_TAG "ZygiskCops"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

#define HOOKAF(ret, func, ...) \
    ret (*orig##func)(__VA_ARGS__); \
    ret my##func(__VA_ARGS__)


#endif //ZygiskImGui_HOOK_H