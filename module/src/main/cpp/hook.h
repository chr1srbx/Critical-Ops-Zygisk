
#ifndef ZygiskImGui_HOOK_H
#define ZygiskImGui_HOOK_H

#define PI 3.14159265

#include <jni.h>
#include "Include/Vector3.h"
#include "Include/Unity.h"
#include "Include/GameTypes.h"

// function predefines
static int enable_hack;
static char *game_data_dir = NULL;
int isGame(JNIEnv *env, jstring appDataDir);
void *hack_thread(void *arg);
void* triggerbot_thread();

// In Game Function Predefines
monoList<void**>* (*getAllCharacters)(void* obj);
Vector3 (*get_Position)(void* transform);
Vector3 (*WorldToScreen)(void*, Vector3 worldPos, int);
void* (*get_camera)();
void* (*get_CharacterBodyPart)(void* obj, int);
void (*SetResolution)(int widht, int height, bool fullscreen);
int (*get_Width)();
int (*get_Height)();
float (*get_focalLength)(void* camera);
int (*getLocalId)(void* obj);
void* (*getPlayer)(void* obj, int id);
void* (*getLocalPlayer)(void* obj);
int (*getCharacterCount)(void* obj);
void(*oldLoadSettings)(void* obj);
int (*get_Health)(void* character);
void* (*get_Player)(void* character);
void (*RaycastCharacter)(void* pSys, void* shooter , Ray ray, int hitdex);
void(*set_targetFrameRate)(int frames);
bool (*get_IsInitialized)(void* character);
void (*RequestPurchaseSkin)(void* obj, int skinID, int price, bool isSeasonalSkin);
monoString* (*getNameAndTag)(void* player);
void(*RemoveCharacter)(void* obj, int playerID);
void*(*get_LocalCharacter)(void* obj);
bool(*isHeadBehindWall)(void* localCharacter, void* character);
float(*get_FovWorld)(void* cameraSettings);
void(*onInputButtons)(void* TouchControls, int Input, bool down);
Ray (*ScreenPointToRay)(void* camera, Vector2 pos, int eye);
void (*UpdateCharacterHitBuffer)(void* pSys, void* character, Ray ray, int* hitIndex);
bool (*getIsCrouched)(void* character);
void* (*GetWeaponByID)(void* character, int id);
int (*FindWeaponID)(void* pSys, void* EGID);
void* (*TraceShot)(void* pSys, void* shooter, Ray ray);
std::uintptr_t (*get_AllHits)(void* peesys);
int (*get_LocalID)(void* obj);
void (*set_Position)(void* obj, Vector3 pos);
void (*AddMoney)(void* player, int amount);
void* (*get_LocalPlayer)(void* pSys);
monoString* (*getDeviceUniqueIdentifier)();
void (*OpenURL)(monoString* url);
void* (*getCharacterIndicatorData)(void* obj, void* character);
int(*getTeamIndex)(PlayerAdapter* player);
monoString* (*get_Username)(PlayerAdapter* player);
monoString* (*get_ClanTag)(PlayerAdapter* player);
void* (*CreateMessage)(monoString* message, ChatMessageType type, bool commandUIUsed);
void (*SendMessage) (void* obj);
bool (*get_Invulnerable) (void* character);
void (*PlayerAdapter_CTOR) (void* obj, void*);
void (*Clear) (void* obj);
void (*AddHit) (void* obj, HitData HitData);
int (*get_ID) (void* obj);

// Custom functions
void* getTransform(void* character);
int get_CharacterTeam(void* character);
int get_PlayerTeam(void* player);
std::string get_CharacterName(void* character);
std::string get_characterWeaponName(void* character);
const char* get_characterArmors(void* character);
Vector3 getBonePosition(void* character, int bone);
bool isCharacterVisible(void* character, void* pSys);
long get_invincibilityTime(void* character);
int getCurrentWeaponCategory(void* character);
Vector2 isInFov(Vector2 rotation, Vector2 newAngle, AimbotCfg cfg);
bool isInFov2(Vector2 rotation, Vector2 newAngle, AimbotCfg cfg);


// oHooks
void(*oldGameSystemUpdate)(void* obj);
void(*oldRenderOverlayFlashbang)(void* obj, void* IRender);
void(*oldset_Spread)(void*obj, float value);
void(*oldRenderOverlaySmoke)(void* obj, void* IRender, void* IEntityDB, Vector3 origin);
void(*oldUpdateWeapon)(void* obj, float deltatime);
void(*oSetRotation)(void* obj, Vector2 rotation);
void (*oGameSystemDestroy)(void* obj);
EGLBoolean (*old_eglSwapBuffers)(EGLDisplay dpy, EGLSurface surface);
void (*oTouchControlsUpdate)(void* obj);
void (*oTouchControlsDestroy)(void* obj);
float(*oldget_Height)(void* obj);
float(*oldFovViewModel)(void* obj);
float(*oldFovWorld)(void* obj);
void (*oldCheckCharacterVisibility)(void* obj, bool* visibility);
void(*oldGenerateHash)(void* obj);
void(*oldFPSMeter)(void* obj);


// Hooks
void GameSystemUpdate(void* obj);
void Init(void* obj);
void setRotation(void* obj, Vector2 rotation);
EGLBoolean hook_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface);

// Initializers
void Hooks();
void Pointers();
void Patches();

#include <android/log.h>



#define LOG_TAG OBFUSCATE("prime")
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

#define HOOKAF(ret, func, ...) \
    ret (*orig##func)(__VA_ARGS__);\
    ret my##func(__VA_ARGS__)


#endif //ZygiskImGui_HOOK_H