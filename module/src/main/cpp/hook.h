
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
std::u16string get_CharacterName(void* character);
std::u16string get_characterWeaponName(void* character);
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


#define LOG_TAG "ZygiskCops"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

#define HOOKAF(ret, func, ...) \
    ret (*orig##func)(__VA_ARGS__); \
    ret my##func(__VA_ARGS__)


#endif //ZygiskImGui_HOOK_H