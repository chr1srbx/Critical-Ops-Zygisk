#include <cstring>
#include <cstdio>
#include <unistd.h>
#include <sys/system_properties.h>
#include <dlfcn.h>
#include <dlfcn.h>
#include <cstdlib>
#include <cinttypes>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include "imgui.h"
#include "imgui_internal.h"
#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_android.h"
#include "KittyMemory/KittyMemory.h"
#include "KittyMemory/MemoryPatch.h"
#include "KittyMemory/KittyScanner.h"
#include "KittyMemory/KittyUtils.h"
#include"Includes/Dobby/dobbyForHooks.h"
#include <cmath>
#include "Include/Unity.h"
#include "Misc.h"
#include "Include/Vector3.h"
#include "hook.h"
#include "Include/Roboto-Regular.h"
#include "Include/RetroGaming.h"
#include "Include/Minecraftia-Regular.h"
#include "ESP.h"
#include <thread>
#include <chrono>
#include <time.h>

#define GamePackageName "com.criticalforceentertainment.criticalops"

monoString* CreateIl2cppString(const char* str)
{
    static monoString* (*CreateIl2cppString)(const char* str, int *startIndex, int *length) =
    (monoString* (*)(const char* str, int *startIndex, int *length))(get_absolute_address(string2Offset(OBFUSCATE("0x44865B4"))));
    int* startIndex = 0;
    int* length = (int *)strlen(str);
    return CreateIl2cppString(str, startIndex, length);
}

bool forceSetSpeed = false;

void* pSys = nullptr;
void* pSpeed = nullptr;
void* localCharacter = nullptr;
bool unsafe,recoil, radar, flash, smoke, scope, setupimg, spread, aimpunch, speed, reload, esp, snaplines, kickback, crouch, wallbang,
        fov, ggod, killnotes,crosshair, supressor, rifleb, bonesp, viewmodel, viewmodelfov, boxesp, healthesp, healthNumber, espName, weaponEsp, armroFlag, spawnbullets,
        canmove,isPurchasingSkins, fly, removecharacter, tutorial, freeshop, gravity, dropweapon,ragdoll, crouchheight, cameraheight, interactionrange, jumpheight, bhop,
        noslow, god, ccollision, aimbot, freearmor, firerate, drawFov, p100Crosshair, vischeck, fovCheck, aimbotSmooth, applied = false;

float speedval = 5.100000, fovModifier, viemodelposx, viemodelposy, viemodelposz, viewmodelfovval, gravityval, flyval ,crouchval, camval, jumpval = 4.100000, firerateval, fovValue = 360, smoothValue = 1;
int screenscale = 0, callCount = 0;

extern int glHeight;
extern int glWidth;
ImFont* espFont;
ImFont* flagFont;

float get_fieldOfView(void *instance) {
    if (instance != nullptr && fov) {
        return fovModifier;
    }
    return old_get_fieldOfView(instance);
}

void* getTransform(void* character)
{
    return *(void**)((uint64_t)character + 0x70);
}

int get_CharacterTeam(void* character)
{
    void* player = get_Player(character);
    void* boxedValueName = *(void**)((uint64_t)player + 0x118);
    return *(int*)((uint64_t)boxedValueName + 0x1C);
}

int get_PlayerTeam(void* player)
{
    void* boxedValueName = *(void**)((uint64_t)player + 0x118);
    return *(int*)((uint64_t)boxedValueName + 0x1C);
}

std::string get_CharacterName(void* character)
{
    void* player = get_Player(character);
    void* boxedValueName = *(void**)((uint64_t)player + 0xA8);
    monoString* username = *(monoString**)((uint64_t)boxedValueName + 0x19);
    return username->getString();
}

std::string get_characterWeaponName(void* character)
{
    void* characterData = *(void**)((uint64_t)character + 0x98);
    void* m_wpn = *(void**)((uint64_t)characterData + 0x80);
    if (m_wpn)
    {
        monoString* weaponName = *(monoString**)((uint64_t)m_wpn + 0x10);
        return weaponName->getString();
    }
    std::string filler = "";
    return filler;
}

Vector3 getBonePosition(void* character, int bone){
    void* curBone = get_CharacterBodyPart(character, bone);
    void* hitSphere = *(void**)((uint64_t)curBone + 0x20);
    void* transform = *(void**)((uint64_t)hitSphere + 0x30);
    Vector3 bonePos = get_Position(transform);
    return bonePos;
}

float NormalizeAngle (float angle){
    while (angle>360)
        angle -= 360;
    while (angle<0)
        angle += 360;
    return angle;
}

Vector2 NormalizeAngles (Vector2 angles)
{
    angles.X = NormalizeAngle(angles.X);
    angles.Y = NormalizeAngle(angles.Y);
    return angles;
}

int isGame(JNIEnv *env, jstring appDataDir) {
    if (!appDataDir)
        return 0;
    const char *app_data_dir = env->GetStringUTFChars(appDataDir, nullptr);
    int user = 0;
    static char package_name[256];
    if (sscanf(app_data_dir, "/data/%*[^/]/%d/%s", &user, package_name) != 2) {
        if (sscanf(app_data_dir, "/data/%*[^/]/%s", package_name) != 1) {
            package_name[0] = '\0';
            LOGW(OBFUSCATE("can't parse %s"), app_data_dir);
            return 0;
        }
    }
    if (strcmp(package_name, GamePackageName) == 0) {
        LOGI(OBFUSCATE("detect game: %s"), package_name);
        game_data_dir = new char[strlen(app_data_dir) + 1];
        strcpy(game_data_dir, app_data_dir);
        env->ReleaseStringUTFChars(appDataDir, app_data_dir);
        return 1;
    } else {
        env->ReleaseStringUTFChars(appDataDir, app_data_dir);
        return 0;
    }
}

void* ShaderFind(std::string name)
{
    LOGE("Shader logged: %s", name.c_str());
    oldShaderFind(name);
}

void* getValidEnt()
{
    int id = getLocalId(pSys);
    void *localPlayer = getPlayer(pSys, id);
    int localTeam = get_PlayerTeam(localPlayer);
    float closestEntDist = 99999.0f;
    void* closestCharacter = nullptr;
    monoList<void **> *characterList = getAllCharacters(pSys);
    for (int i = 0; i < characterList->getSize(); i++) {
        void *currentCharacter = (monoList<void **> *) characterList->getItems()[i];
        if (get_Player(currentCharacter) == localPlayer) {
            localCharacter = currentCharacter;
        }
        int curTeam = get_CharacterTeam(currentCharacter);
        int health = get_Health(currentCharacter);
        bool canSet = false;
        if (aimbot && localCharacter && health > 0 && localTeam != curTeam && curTeam != -1) {
            if (vischeck) {
                if (isCharacterVisible(currentCharacter, pSys)) {
                    canSet = true;
                }

            }
            Vector3 localPosition = get_Position(getTransform(localCharacter));
            Vector3 currentCharacterPosition = get_Position(getTransform(currentCharacter));
            Vector3 currentEntDist = Vector3::Distance(localPosition, currentCharacterPosition);
            if (Vector3::Magnitude(currentEntDist) < closestEntDist) {
                if (vischeck && !canSet) continue;
                closestEntDist = Vector3::Magnitude(currentEntDist);
                closestCharacter = currentCharacter;
            }
        }

    }
    return closestCharacter;
}


void GameSystemUpdate(void* obj){
    if(obj != nullptr){
        pSys = obj;
        void* GamePlayModule = *(void**)((uint64_t) obj + 0x80);
        if(GamePlayModule != nullptr){
            void* CameraSystem = *(void**)((uint64_t) GamePlayModule + 0x30);
            if(CameraSystem != nullptr){
                if(fov){
                    *(float*)((uint64_t) CameraSystem + 0x8C) = fovModifier;//m_horizontalFieldOfView
                }

                if(viewmodelfov){
                    *(float*)((uint64_t) CameraSystem + 0x90) = viewmodelfovval;//m_viewModelFieldOfView
                }
            }

            if(removecharacter){
                RemoveCharacter(obj, getLocalId(obj));
                removecharacter = false;
            }

            if (spawnbullets) {
                int id = getLocalId(pSys);
                void *localPlayer = getPlayer(pSys, id);
                int localTeam = get_PlayerTeam(localPlayer);
                // spawn bullets in ppls headlol
                monoList<void **> *characterList = getAllCharacters(pSys);
                void *localCharacter = nullptr;
                for (int i = 0; i < characterList->getSize(); i++) {
                    void *currentCharacter = (monoList<void **> *) characterList->getItems()[i];
                    if (get_Player(currentCharacter) == localPlayer) {
                        localCharacter = currentCharacter;
                    }
                }
                for (int i = 0; i < characterList->getSize(); i++) {
                    void *currentCharacter = (monoList<void **> *) characterList->getItems()[i];
                    int curTeam = get_CharacterTeam(currentCharacter);
                    if (curTeam != localTeam) {
                        Vector3 headPos = getBonePosition(currentCharacter, 10);
                        Ray ray;
                        ray.origin = headPos;
                        ray.direction = Vector3(1, 1, 1);
                        if (localCharacter) {

                        }
                    }
                }
            }
        }
    }
    return oldGameSystemUpdate(obj);
}

void wait(int seconds){
    clock_t endwait;
    endwait=clock()+seconds*CLOCKS_PER_SEC;
    while (clock()<endwait);
}

void UpdateWeapon(void* obj, float deltatime){
    if(obj != nullptr) {
        void *CharacterData = *(void **) ((uint64_t) obj + 0x98);
        if (CharacterData != nullptr) {
            void *CharacterSettingsData = *(void **) ((uint64_t) CharacterData + 0x78);
            if (CharacterSettingsData != nullptr) {
                if (speed) {
                    *(float *) ((uint64_t) CharacterSettingsData + 0x14) = speedval;
                }

                if (jumpheight) {
                    *(float *) ((uint64_t) CharacterSettingsData + 0x4C) = jumpval;
                    *(float *) ((uint64_t) CharacterSettingsData + 0x50) = jumpval;
                }

                if (noslow) {
                    *(float *) ((uint64_t) CharacterSettingsData + 0x60) = 0;
                    *(float *) ((uint64_t) CharacterSettingsData + 0x64) = 0;
                    *(float *) ((uint64_t) CharacterSettingsData + 0x5C) = 0;
                }

                if(forceSetSpeed == true){
                    if(speed){
                        speed = false;
                        *(float *) ((uint64_t) CharacterSettingsData + 0x64) = 12.500000;
                        *(float *) ((uint64_t) CharacterSettingsData + 0x60) = 2.000000;
                        *(float *) ((uint64_t) CharacterSettingsData + 0x5C) = 0.100000;
                        *(float *) ((uint64_t) CharacterSettingsData + 0x14) = 5.100000;
                        *(float *) ((uint64_t) CharacterSettingsData + 0x1C) = 1.850000;
                        *(float *) ((uint64_t) CharacterSettingsData + 0x4C) = 4.100000;
                        *(float *) ((uint64_t) CharacterSettingsData + 0x50) = 3.700000;
                        wait(1.5);
                        speed = true;
                    }
                    forceSetSpeed = false;
                }
            }
        }
    }
    oldUpdateWeapon(obj, deltatime);

}

void ReturnValues(void* obj, float MinDamage, float MaxDamage, float FireRate, float deployTime, int ArmorPenetration, int WallPenetration, int BurstFireShots/*, bool CanBuy*/, float RecoilYFactor, float RecoilXFactor, /*float RecoilPerShot, float RecoilRecover*/ float ShakeFactor, float burstSpread, float SpreadFactor,/* float SpreadPerShot*/ float SpreadRecover) {
    *(float *) ((uint64_t) obj + 0x50) = MinDamage;
    *(float *) ((uint64_t) obj + 0x4C) = MaxDamage;
    *(float *) ((uint64_t) obj + 0x64) = FireRate;
    *(float *) ((uint64_t) obj + 0x88) = deployTime;
    *(int *) ((uint64_t) obj + 0x80) = ArmorPenetration;
    *(int *) ((uint64_t) obj + 0x7C) = WallPenetration;
    *(int *) ((uint64_t) obj + 0x11C) = BurstFireShots;
  //  *(bool *) ((uint64_t) obj + 0x40) = CanBuy;
     *(float *) ((uint64_t) obj + 0xF0) = RecoilYFactor;
    *(float *) ((uint64_t) obj + 0x100) = RecoilXFactor;
   // *(float *) ((uint64_t) obj + 0x104) = RecoilPerShot;
  //  *(float *) ((uint64_t) obj + 0x108) = RecoilRecover;
    *(float *) ((uint64_t) obj + 0x110) = ShakeFactor;
    *(float *) ((uint64_t) obj + 0x6C) = burstSpread;
    *(float *) ((uint64_t) obj + 0xD8) = SpreadFactor;
    //*(float *) ((uint64_t) obj + 0xDC) = SpreadPerShot;
    *(float *) ((uint64_t) obj + 0xE0) = SpreadRecover;
}

void (*oCopyHashData)(void* obj, void* from, void* to);
void CopyHashData(void* obj, void* from, void* to)
{
    if(obj != nullptr) {
        forceSetSpeed = true;
        void *fromCharSett = *(void **) ((uint64_t) from + 0x38);
        if (fromCharSett != nullptr) {
            if (speedval != 5.100000 || jumpval != 4.100000 || noslow) {
                *(float *) ((uint64_t) fromCharSett + 0x64) = 12.500000;
                *(float *) ((uint64_t) fromCharSett + 0x60) = 2.000000;
                *(float *) ((uint64_t) fromCharSett + 0x5C) = 0.100000;
                *(float *) ((uint64_t) fromCharSett + 0x14) = 5.100000;
                *(float *) ((uint64_t) fromCharSett + 0x1C) = 1.850000;
                *(float *) ((uint64_t) fromCharSett + 0x4C) = 4.100000;
                *(float *) ((uint64_t) fromCharSett + 0x50) = 3.700000;
                forceSetSpeed = true;
            }
        }
    }
    oCopyHashData(obj, from, to);
}

void RenderOverlayFlashbang(void* obj){
    if(obj != nullptr && flash){
        *(float*)((uint64_t) obj + string2Offset(OBFUSCATE("0x38"))) = 0;//m_flashTime
    }
    oldRenderOverlayFlashbang(obj);
}

void set_Spread(void* obj, float value){
    if(obj != nullptr){
        *(float*)((uint64_t) obj + 0x24) = 0;//m_maxSpread
        *(float*)((uint64_t) obj + 0x20) = 0;//m_spreadFactor
    }
    oldset_Spread(obj, value);
}


bool isCharacterVisible(void* character, void* pSys)
{
    void* localCharacter = get_LocalCharacter(pSys);
    return !isHeadBehindWall(localCharacter, character);
}

void RenderOverlaySmoke(void* obj) {
    if (obj != nullptr && smoke) {
        *(float *) ((uint64_t) obj + 0x20) = 9999;//m_fadeSpeed
    }
    oldRenderOverlaySmoke(obj);
}

void DrawRenderer(void* obj){
    if(obj != nullptr){
        LOGE("DRENDER");
        void* m_offset = *(void**)((uint64_t) obj + 0x64);
        if(viewmodel){
            *(float*)((uint64_t)m_offset + 0x0) = viemodelposx;
            *(float*)((uint64_t)m_offset + 0x4) = viemodelposy;
            *(float*)((uint64_t)m_offset + 0x8) = viemodelposz;
        }
    }
    oldDrawRenderer(obj);
}

void(*oldFpsLimitUpdate)(void* obj);
void FpsLimitUpdate(void* obj){
    if(obj != nullptr){
        *(int*)((uint64_t) obj + 0x18) = 999;
    }
    oldFpsLimitUpdate(obj);
}


Vector3 get_gravity(){
    if(gravity){
        return gravityval;
    }
    oldget_gravity();
}

Vector3 get_height(){
    if(fly){
        return flyval;
    }
    oldget_height();
}

void Init(void* obj){
    if(obj != nullptr){
        LOGE("CALLED");
        void* GraphicsProfile = *(void**)((uint64_t) obj + 0x38);
        if(GraphicsProfile != nullptr){
            LOGE("GPROFILE");
            int scscale = (int)((uint64_t) GraphicsProfile + 0x30);
            LOGE("SSCALE, %d", scscale);
            screenscale = scscale / 100;
        }
    }
    oldInit(obj);
}

void BackendManager(void* obj){
    if(obj != nullptr && isPurchasingSkins){
        for (int i = 0; i < 9999; i++)
        {
            LOGE("trying to force purchase skins...");
            RequestPurchaseSkin(obj, i, 0, false);
        }
        isPurchasingSkins = false;
    }
    oldBackendManager(obj);
}

Vector2 isInFov(Vector2 rotation, Vector2 newAngle)
{
    Vector2 difference = newAngle - rotation;
    if(sqrt(difference.X * difference.X + difference.Y * difference.Y) > fovValue)
    {
        rotation += 180;
        newAngle += 180;
        difference = newAngle - rotation;
        if(sqrt(difference.X * difference.X + difference.Y * difference.Y) > fovValue)
        {
            return Vector2(0,0);
        }
    }
    return difference;
}

void setRotation(void* character, Vector2 rotation)
{
    Vector2 newAngle;
    Vector2 difference;
    difference.X = 0;
    difference.Y = 0;
    if(pSys) {
        if (character && localCharacter != nullptr && get_IsInitialized(localCharacter)) {
            if (get_Health(localCharacter) > 0) {
                void *closestCharacter = getValidEnt();
                if (aimbot && closestCharacter) {
                    Vector3 localHead = getBonePosition(localCharacter, 10);
                    Vector3 enemyBone = getBonePosition(closestCharacter, 10);

                    Vector3 deltavec = enemyBone - localHead;
                    float deltLength = sqrt(deltavec.X * deltavec.X + deltavec.Y * deltavec.Y +
                                            deltavec.Z * deltavec.Z);

                    newAngle.X = -asin(deltavec.Y / deltLength) * (180.0 / PI);
                    newAngle.Y = atan2(deltavec.X, deltavec.Z) * 180.0 / PI;

                }
                if (aimbot && character == localCharacter && closestCharacter &&
                    get_Health(localCharacter) > 0) {
                    if (fovCheck) {
                        difference = isInFov(rotation, newAngle);
                    } else {
                        difference = newAngle - rotation;
                    }

                    if (aimbotSmooth) {
                        difference = difference / smoothValue;
                    }
                }
            }
        }
    }

    oSetRotation(character, rotation + difference);
}


void(*oldApplyKickBack)(void* obj, bool* applied);
void ApplyKickBack(void* obj, bool* applied)
{
    if(obj != nullptr && recoil){
        *applied = true;
        return;
    }
    oldApplyKickBack(obj, applied);
}

void GameSystemDestroy(void* obj)
{
    pSys = nullptr;
    applied = false;
    oGameSystemDestroy(obj);
}


float(*oldFovWorld)(void* obj);
float FovWorld(void* obj){
    if(obj != nullptr && fov){
        return fovModifier;
    }
    return oldFovWorld(obj);
}

float(*oldFovViewModel)(void* obj);
float FovViewModel(void* obj){
    if(obj != nullptr && fov){
        return fovModifier;
    }
    return oldFovViewModel(obj);
}
void* (*acRealOld)(monoString* status);
void* (*acFakeOld)(void* obj);
void acFirstHook(void* obj){
    if (obj){
      *(bool*)((uint64_t)obj + 0x10) = 0;
      *(bool*)((uint64_t)obj + 0x11) = 1;
      *(bool*)((uint64_t)obj + 0x12) = 0;
      *(int*)((uint64_t)obj + 0x14) = 1;
    }
}

void* susFirst(monoString* status){
    LOGE("STRING IS: %s", status->getString().c_str()); return acRealOld(status);
}

void acSecondHook(void* obj){
if (obj){
        *(bool*)((uint64_t)obj + 0x10) = 1;
    }
}

void acThirdHook(void* obj){
if (obj){
    *(bool*)((uint64_t)obj + 0x10) = 1;
    }
}

void acFourthHook(void* obj)
{
    if (obj){
        *(bool*)((uint64_t)obj + 0x10) = 1;
    }
}

void acSixthhHook(void* obj){
    if (obj){
        *(bool*)((uint64_t)obj + 0x10) = 1;
    }
}

void(*oldGenerateGameDataHashThread)(void* obj);
void GenerateGameDataHashThread(void* obj){
    if(obj != nullptr){
        LOGE("I AM THE FUCKING ANTICHEAT1");
        *(bool*)((uint64_t) obj + 0x30) = false;
    }
    oldGenerateGameDataHashThread(obj);
}

float get_cameraFov(void* pSys)
{
    void* GameplayModule = *(void**)((uint64_t)pSys + 0x80);
    void* CameraSystem = *(void**)((uint64_t)GameplayModule + 0x30);
    return *(float*)((uint64_t)CameraSystem + 0x8C); // m_horizontalFieldOfView
}

float GetCurrentMaxSpeed(void* obj, float speed){
    if(obj != nullptr){
        return speedval;
    }
    return oldGetCurrentMaxSpeed(obj, speed);
}

HOOKAF(void, Input, void *thiz, void *ex_ab, void *ex_ac) {
    origInput(thiz, ex_ab, ex_ac);
    ImGui_ImplAndroid_HandleInputEvent((AInputEvent *)thiz);
    return;
}

// Initilizers with patterns <3
void Hooks()
{
   // HOOK("0x1B872E8", BackendManager, oldBackendManager);
  //  HOOK("0x19B97B8", set_Spread, oldset_Spread);// Overlay Scope set spread
   // HOOK("0x19B9570", RenderOverlayFlashbang, oldRenderOverlayFlashbang); // flash render overlay
    //HOOK("0x19BF970", RenderOverlaySmoke, oldRenderOverlaySmoke); // smoke render overlay
    HOOK("0x10D3390", GameSystemUpdate, oldGameSystemUpdate); // GameSystem Update
    HOOK("0x1A80624", UpdateWeapon, oldUpdateWeapon); // character
    //HOOK("0x10C06F4", FovViewModel, oldFovViewModel); // speed
    HOOK("0x10CF518", GameSystemDestroy, oGameSystemDestroy); // GameSystem Destroy
    HOOK("0x10C06B4", FovWorld, oldFovWorld); // speed
    HOOK("0x1A7CFCC", setRotation, oSetRotation);
    HOOK("0x1D56428", ApplyKickBack, oldApplyKickBack); // speed
    HOOK("0x1E5D240",CopyHashData , oCopyHashData);
   // HOOK("0x1BA8AB4", GenerateGameDataHashThread , oldGenerateGameDataHashThread);
   // HOOK("0x1D55E74", GetCurrentMaxSpeed, oldGetCurrentMaxSpeed); // speed

      //HOOK("0x19D84B4", susFirst, acRealOld); // speed
    /*   //  HOOK("0x19D825C", susFirst, acRealOld); // speed
      HOOK("0x1A55FE4", acSecondHook, acFakeOld); // speed
      HOOK("0x1A560B0", acSecondHook, acFakeOld); // speed
        HOOK("0x1A5A1CC", acThirdHook, acFakeOld); // speed
       HOOK("0x1A5A9E8", acThirdHook, acFakeOld); // speed
       HOOK("0x1A5643C", acFourthHook, acFakeOld); // speed
       HOOK("0x1A56430", acFourthHook, acFakeOld); // speed
       HOOK("0x1A5646C", acFourthHook, acFakeOld); // speed
       HOOK("0x1A56938", acFourthHook, acFakeOld); // speed
       HOOK("0x1A56E04", acFourthHook, acFakeOld); // speed
       HOOK("0x1A572D0", acFourthHook, acFakeOld); // speed
       HOOK("0x1A5779C", acFourthHook, acFakeOld); // speed
       HOOK("0x1A57C68", acFourthHook, acFakeOld); // speed
       HOOK("0x1A58134", acFourthHook, acFakeOld); // speed
       HOOK("0x1A58600", acFourthHook, acFakeOld); // speed
       HOOK("0x1A58ACC", acFourthHook, acFakeOld); // speed
       HOOK("0x1A58F98", acFourthHook, acFakeOld); // speed*/
    //HOOK("0x1A5A1CC", Bypass1, oldBypass1); // speed
    //HOOK("0x1B9D608", DrawRenderer, oldDrawRenderer); need args
}

void Pointers()
{
//   RequestPurchaseSkin = (void(*)(void*, int, int, bool)) get_absolute_address(string2Offset(OBFUSCATE("0x1B80760")));
    SetResolution = (void(*)(int, int, bool)) get_absolute_address(string2Offset(OBFUSCATE("0x1A16B5C"))); // SetResolution
    get_Width = (int(*)()) get_absolute_address(string2Offset(OBFUSCATE("0x1A16864"))); // screen get_Width
    get_Height = (int(*)()) get_absolute_address(string2Offset(OBFUSCATE("0x1A1688C"))); // screen get_Height
    get_focalLength = (float(*)(void*)) get_absolute_address(string2Offset(OBFUSCATE("0x1A1CA98"))); // get focal length
    getAllCharacters = (monoList<void**>*(*)(void*)) get_absolute_address(string2Offset(OBFUSCATE("0x10CE598"))); // get_AllCharacters
    getLocalId= (int(*)(void*)) get_absolute_address(string2Offset(OBFUSCATE("0x10C2848"))); // get_LocalId
    getPlayer = (void*(*)(void*,int)) get_absolute_address(string2Offset(OBFUSCATE("0x10D1838"))); // GameSystem GetPlayer
    getLocalPlayer = (void*(*)(void*)) get_absolute_address(string2Offset(OBFUSCATE("0x10C22B8"))); // GameSystem get_LocalPlayer
    getCharacterCount = (int(*)(void*)) get_absolute_address(string2Offset(OBFUSCATE("0x10CE5A8"))); // GameSystem get_CharacterCount
    get_Health = (int(*)(void*)) get_absolute_address(string2Offset(OBFUSCATE("0x1A7C5EC"))); // get_Health
    get_Player = (void*(*)(void*)) get_absolute_address(string2Offset(OBFUSCATE("0x1A7C5D4"))); // Gameplay Character get_Player
    get_IsInitialized = (bool(*)(void*)) get_absolute_address(string2Offset(OBFUSCATE("0x1A7C45C"))); // Gameplay Character get_IsInitialized
    get_Position = (Vector3(*)(void*)) get_absolute_address(string2Offset(OBFUSCATE("0x1A44BDC"))); // Transform get_position
    get_camera = (void*(*)()) get_absolute_address(string2Offset(OBFUSCATE("0x1A1E52C"))); // get camera main
    WorldToScreen = (Vector3(*)(void*, Vector3, int)) get_absolute_address(string2Offset(OBFUSCATE("0x1A1D930"))); // WorldToScreenPoint
    //set_targetFrameRate = (void(*)(int)) get_absolute_address(""); NOT USED
    get_CharacterBodyPart =(void*(*)(void*, int)) get_absolute_address(string2Offset(OBFUSCATE("0x1A7C648"))); // Character GetBodyPart
    getNameAndTag = (monoString*(*)(void*)) get_absolute_address(string2Offset(OBFUSCATE("0xBCCEA0"))); // get_UsernameWithClanTag
    RaycastCharacter = (void(*)(void*,void*,Ray, int)) get_absolute_address(string2Offset(OBFUSCATE("0x10D80B0"))); // RaycastCharacters
    RemoveCharacter = (void(*)(void*, int)) get_absolute_address(string2Offset(OBFUSCATE("0x10D09C0")));//RemovePlayer
    get_LocalCharacter = (void*(*)(void*)) get_absolute_address(string2Offset(OBFUSCATE("0x10C4DEC"))); // get_LocalCharacter
    isHeadBehindWall = (bool(*)(void*, void*)) get_absolute_address(string2Offset(OBFUSCATE("0x19CCDA4"))); // IsHeadBehindWall
    get_FovWorld = (float(*)(void*)) get_absolute_address(string2Offset(OBFUSCATE("0x10C06B4"))); // get_FovWorld
}
/*   PATCH_SWITCH("0x1D5638C", "1F2003D5C0035FD6", spread);//UpdateSpread
    PATCH_SWITCH("0x1D56328", "000080D2C0035FD6", aimpunch);//AimPunchRecover
    PATCH_SWITCH("0x19C46E0", "200080D2C0035FD6", canmove);//CanMove
    PATCH_SWITCH("0x19C605C", "200080D2C0035FD6", canmove);//CanShoot
    PATCH_SWITCH("0x19C1FFC", "200080D2C0035FD6", canmove);//ShootingAllowed
    PATCH_SWITCH("0x19C1FE8", "200080D2C0035FD6", canmove);//MovementAllowed
    PATCH_SWITCH("0x10D78B4", "1F2003D5C0035FD6", wallbang);//UpdateWallHit
    PATCH_SWITCH("0x10D6E84", "1F2003D5C0035FD6", wallbang);//ProcessWallhit
    //  PATCH_SWITCH("0x10695C0", "1F2003D5C0035FD6", wallbang);//CheckWallHits
    PATCH_SWITCH("0x10D62C0", "000080D2C0035FD6", ggod);//GrenadeHitCharacter
    PATCH_SWITCH("0x10D6868", "000080D2C0035FD6", ggod);//OnGrenadeExploded
  //  PATCH_SWITCH("0xF3CB7C", "000080D2C0035FD6", killnotes);//SetKillNotification
   // PATCH_SWITCH("0x1DB3698", "1F2003D5C0035FD6", killnotes);//Init
    PATCH_SWITCH("0x172D454", "200080D2C0035FD6", crosshair);//get_Crosshair
    PATCH_SWITCH("0x1BFDF2C", "1F2003D5C0035FD6", smoke);//SmokeGrenadeEffect
  //  PATCH_SWITCH("0x1D8507C", "200080D2C0035FD6", supressor);//isSupressor
    PATCH_SWITCH("0x1A7CB20", "1F2003D5C0035FD6", crouch);//isSupressor
   // PATCH_SWITCH("0x1BB5A00", "200080D2C0035FD6", dropweapon);//WeaponDroppingAllowed
 //   PATCH_SWITCH("0x1BB5A14", "200080D2C0035FD6", dropweapon);//WeaponPickupAllowed
   // PATCH_SWITCH("0x19DFB00", "1F2003D5C0035FD6", ragdoll);//Ragdoll
    PATCH_SWITCH("0x1FA30E0", "1F2003D5C0035FD6", freearmor);//ApplyRestorePriceToArmor
    PATCH_SWITCH("0x1FA3124", "1F2003D5C0035FD6", freearmor);//ApplyNormalPriceToArmor*/
void Patches(){


    PATCH("0x20C9830", "000080D2C0035FD6");//t
    PATCH("0x20C95D0", "000080D2C0035FD6");//t
    PATCH("0x20CA2AC", "000080D2C0035FD6");//t
    PATCH("0x20CA0E8", "000080D2C0035FD6");//t
    PATCH("0x20C9700", "000080D2C0035FD6");//t
    PATCH("0x20C9528", "000080D2C0035FD6");//t

  //  PATCH("0x1A5C210", "1F2003D5C0035FD6");
   // PATCH("0x1A71B50", "1F2003D5C0035FD6");
   // PATCH("0x1A5D454", "1F2003D5C0035FD6");
  //  PATCH("0x1A71A84", "1F2003D5C0035FD6");
}

void DrawMenu() {
    if(pSys != nullptr)
    {
        int id = getLocalId(pSys);
        void *localPlayer = getPlayer(pSys, id);
        int localTeam = get_PlayerTeam(localPlayer);
        float closestEntDist = 99999.0f;
        monoList<void **> *characterList = getAllCharacters(pSys);
        if(!applied){
            applied = true;
        }

        for (int i = 0; i < characterList->getSize(); i++) {
            void *currentCharacter = (monoList<void **> *) characterList->getItems()[i];
            if(get_Player(currentCharacter) == localPlayer)
            {
                localCharacter = currentCharacter;
            }
            int curTeam = get_CharacterTeam(currentCharacter);
            int health = get_Health(currentCharacter);
            if (health > 0 && get_IsInitialized(currentCharacter) && localTeam != curTeam &&
                curTeam != -1) {
                void *transform = getTransform(currentCharacter);
                Vector3 position = get_Position(transform);
                Vector3 transformPos = WorldToScreen(get_camera(), position, 2);
                transformPos.Y = glHeight - transformPos.Y;
                Vector3 headPos = getBonePosition(currentCharacter, HEAD);
                Vector3 chestPos = getBonePosition(currentCharacter, CHEST);
                Vector3 wschestPos = WorldToScreen(get_camera(), chestPos, 2);
                Vector3 wsheadPos = WorldToScreen(get_camera(), headPos, 2);
                Vector3 aboveHead = headPos + Vector3(0, 0.2, 0); // estimate
                Vector3 headEstimate = position + Vector3(0, 1.48, 0);// estimate
                Vector3 wsAboveHead = WorldToScreen(get_camera(), aboveHead, 2);
                Vector3 wsheadEstimate = WorldToScreen(get_camera(), headEstimate, 2);
                wsAboveHead.Y = glHeight - wsAboveHead.Y;
                wsheadEstimate.Y = glHeight - wsheadEstimate.Y;
                float height = transformPos.Y - wsAboveHead.Y;
                float width = (transformPos.Y - wsheadEstimate.Y) / 2;
                if (snaplines && transformPos.Z > 0) {
                    DrawLine(ImVec2(glWidth / 2, glHeight), ImVec2(transformPos.X, transformPos.Y),
                             ImColor(172, 204, 255), 3);
                }
                if (bonesp && transformPos.Z > 0) {
                    DrawBones(currentCharacter, LOWERLEG_LEFT, UPPERLEG_LEFT);
                    DrawBones(currentCharacter, LOWERLEG_RIGHT, UPPERLEG_RIGHT);
                    DrawBones(currentCharacter, UPPERLEG_LEFT, STOMACH);
                    DrawBones(currentCharacter, UPPERLEG_RIGHT, STOMACH);
                    DrawBones(currentCharacter, STOMACH, CHEST);
                    DrawBones(currentCharacter, LOWERARM_LEFT, UPPERARM_LEFT);
                    DrawBones(currentCharacter, LOWERARM_RIGHT, UPPERARM_RIGHT);
                    DrawBones(currentCharacter, UPPERARM_LEFT, CHEST);
                    DrawBones(currentCharacter, UPPERARM_RIGHT, CHEST);
                    Vector3 diff = wschestPos - wsheadPos;
                    Vector3 neck = (chestPos + headPos) / 2;
                    Vector3 wsneck = WorldToScreen(get_camera(), neck, 2);
                    wsneck.Y = glHeight - wsneck.Y;
                    wschestPos.Y = glHeight - wschestPos.Y;
                    wsheadPos.Y = glHeight - wsheadPos.Y;
                    if (wschestPos.Z > 0 && wsneck.Z) {
                        DrawLine(ImVec2(wschestPos.X, wschestPos.Y), ImVec2(wsneck.X, wsneck.Y),
                                 ImColor(172, 204, 255), 3);
                    }
                    if (wsheadPos.Z > 0 && wschestPos.Z > 0) {
                        float radius = sqrt(diff.X * diff.X + diff.Y * diff.Y);
                        auto background = ImGui::GetBackgroundDrawList();

                        background->AddCircle(ImVec2(wsheadPos.X, wsheadPos.Y), radius / 2, IM_COL32(172, 204, 255, 255), 0, 3.0f);
                    }
                }
                if (boxesp && transformPos.Z > 0 && wsAboveHead.Z > 0) {
                    DrawOutlinedBox2(wsAboveHead.X - width / 2, wsAboveHead.Y, width, height,
                                     ImVec4(255, 255, 255, 255), 3);
                }
                if (healthesp && transformPos.Z > 0 && wsAboveHead.Z > 0) {
                    DrawOutlinedFilledRect(wsAboveHead.X - width / 2 - 12, wsAboveHead.Y + height *(1 -(static_cast<float>(health) / 100.0f)),3, height * (static_cast<float>(health) / 100.0f),
                                           HealthToColor(health));
                }
                if (healthNumber && transformPos.Z > 0 && wsAboveHead.Z > 0) {
                    if (health < 100) {
                        DrawText(ImVec2(wsAboveHead.X - width / 2 - 17, wsAboveHead.Y + height *(1 -static_cast<float>(health) /100.0f) -3),
                                  ImVec4(255, 255, 255, 255), std::to_string(health), espFont);
                    }
                }
                if (espName && transformPos.Z > 0 && wsAboveHead.Z > 0) {
                    void *player = get_Player(currentCharacter);
                    monoString *mono = getNameAndTag(player);
                    LOGE("Monostring at: %p", mono);
                    std::string name = mono->getString();
                    LOGE("Player Name: %s", name.c_str());
                    // font is 15 px has 2 px outline so has 16px per character, to center the font we do
                    float compensation = name.length() * 4.0f;
                    DrawText(ImVec2(wsheadPos.X - compensation, wsAboveHead.Y - 20),ImVec4(255, 255, 255, 255), name, espFont);
                }
                if (weaponEsp && transformPos.Z > 0 && wsAboveHead.Z > 0) {
                    std::string weapon = get_characterWeaponName(currentCharacter);
                    // font is 15 px has 2 px outline so has 16px per character, to center the font we do
                    float compensation = weapon.length() * 4.0f;

                    DrawText(ImVec2(wsAboveHead.X - compensation, transformPos.Y + 7), ImVec4(255, 255, 255, 255), weapon, espFont);
                }

            }
        }
        if(drawFov)
        {
            float worldFov = get_cameraFov(pSys);
            float radius = tan( fovValue*(PI/ 180) / 2 ) / tan( worldFov / 2 ) * glWidth / 2;
            auto background = ImGui::GetBackgroundDrawList();
            background->AddCircle(ImVec2(glWidth/2, glHeight/2), radius, IM_COL32(255, 255, 255, 255), 0, 3.0f);
        }
    }
    if(p100Crosshair)
    {
        ImVec2 Middle = ImVec2(glWidth/2, glHeight/2);

        CoolCrosshair(ImVec2(Middle.x -2, Middle.y), ImVec4(0,0,0,255));
        CoolCrosshair(ImVec2(Middle.x -2, Middle.y -2), ImVec4(0,0,0,255));
        CoolCrosshair(ImVec2(Middle.x +2, Middle.y), ImVec4(0,0,0,255));
        CoolCrosshair(ImVec2(Middle.x +2, Middle.y + 2), ImVec4(0,0,0,255));
        CoolCrosshair(ImVec2(Middle.x, Middle.y-2), ImVec4(0,0,0,255));
        CoolCrosshair(ImVec2(Middle.x + 2, Middle.y-2), ImVec4(0,0,0,255));
        CoolCrosshair(ImVec2(Middle.x, Middle.y+2), ImVec4(0,0,0,255));
        CoolCrosshair(ImVec2(Middle.x -2, Middle.y+2), ImVec4(0,0,0,255));
        CoolCrosshair(Middle, ImVec4(255,255,255,255));
    }
    {
        if (unsafe) { ImGui::Begin(OBFUSCATE("(UNSAFE HOOK) zyCheats Rage - 1.37.1f2091 - chr1s#4191 & 077 Icemods && faggosito"));
        }
        else { ImGui::Begin(OBFUSCATE("zyCheats Rage - 1.37.1f2091 - chr1s#4191 & 077 Icemods && faggosito"));
        }
        ImGui::TextUnformatted("Set the screenscale in settings to 100.");
        if (ImGui::Button(OBFUSCATE("Join Discord"))) {
            //isDiscordPressed = true;
        }
        ImGui::TextUnformatted("Its Recommended to join the discord server for mod updates etc.");
        ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_FittingPolicyResizeDown;
        if (ImGui::BeginTabBar("Menu", tab_bar_flags)) {
            if (ImGui::BeginTabItem(OBFUSCATE("Aimbot"))) {
                ImGui::Checkbox(OBFUSCATE("Aimbot"), &aimbot);
                if (aimbot) {
                    ImGui::Checkbox(OBFUSCATE("Vis Check"), &vischeck);
                    ImGui::Checkbox(OBFUSCATE("Crosshair"), &p100Crosshair);
                    ImGui::Checkbox(OBFUSCATE("Smooth"), &aimbotSmooth);
                    if(aimbotSmooth)
                    {
                        ImGui::SliderFloat(OBFUSCATE("Smooth Amount"), &smoothValue, 1.0, 30.0);
                    }
                    ImGui::Checkbox(OBFUSCATE("Fov"), &fovCheck);
                    if(fovCheck)
                    {
                        ImGui::SliderFloat(OBFUSCATE("Fov Amount"), &fovValue, 0.0, 360.0);
                        ImGui::Checkbox(OBFUSCATE("Draw Fov"), &drawFov);
                    }
                }
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem(OBFUSCATE("Player"))) {
                if (ImGui::Checkbox(OBFUSCATE("No Grenade Damage"), &ggod)) { Patches(); }
                if (ImGui::Checkbox(OBFUSCATE("Spoof Crouch"), &crouch)) { Patches(); }
                ImGui::Checkbox(OBFUSCATE("Speed"), &speed);
                if (speed) {
                    ImGui::SliderFloat(OBFUSCATE(" · Speed"), &speedval, 0.0, 20.0);
                }
                ImGui::Checkbox(OBFUSCATE("Jump Force"), &jumpheight);
                if (jumpheight) {
                    ImGui::SliderFloat(OBFUSCATE(" · Force"), &jumpval, 0.0, 20.0);
                }
                ImGui::Checkbox(OBFUSCATE("No Slow-Down"), &noslow);
                ImGui::Checkbox(OBFUSCATE("BunnyHop"), &bhop);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem(OBFUSCATE("Weapon"))) {
                if (ImGui::Checkbox(OBFUSCATE("No Recoil"), &recoil)) { Patches(); }
                if (ImGui::Checkbox(OBFUSCATE("No Spread"), &spread)) { Patches(); }
                if (ImGui::Checkbox(OBFUSCATE("Force Supressor"), &supressor)) { Patches(); }
                ImGui::Checkbox(OBFUSCATE("Force Supressor"), &firerate);
                if (firerate) {
                    ImGui::SliderFloat(OBFUSCATE(" · Firerate"), &firerateval, 0.1, 50.0);
                }
                ImGui::Checkbox(OBFUSCATE("No Reload"), &reload);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem(OBFUSCATE("Visual"))) {
                if (ImGui::CollapsingHeader(OBFUSCATE("ESP"))) {
                    ImGui::Checkbox(OBFUSCATE("Snaplines"), &snaplines);
                    ImGui::Checkbox(OBFUSCATE("Bones"), &bonesp);
                    ImGui::Checkbox(OBFUSCATE("Boxes"), &boxesp);
                    ImGui::Checkbox(OBFUSCATE("Show Names"), &espName);
                    ImGui::Checkbox(OBFUSCATE("Show Health"), &healthesp);
                    if (healthesp) {
                        ImGui::Checkbox(OBFUSCATE("Health Numbers"), &healthNumber);
                    }
                    ImGui::Checkbox(OBFUSCATE(" Show Weapon"), &weaponEsp);
                }
                if (ImGui::CollapsingHeader(OBFUSCATE("Camera Mods"))) {
                    ImGui::Checkbox(OBFUSCATE("Camera Height"), &cameraheight);
                    if (viewmodelfov) {
                        ImGui::SliderFloat(OBFUSCATE(" · Viewmodel Value"), &camval, 1.0, 360.0);
                    }
                    ImGui::Checkbox(OBFUSCATE("View Model FOV"), &viewmodelfov);
                    if (viewmodelfov) {
                        ImGui::SliderFloat(OBFUSCATE(" · Viewmodel Value"), &viewmodelfovval, 1.0,360.0);
                    }
                    ImGui::Checkbox(OBFUSCATE("Field Of View"), &fov);
                    if (fov) {
                        ImGui::SliderFloat(OBFUSCATE(" · FOV Value"), &fovModifier, 1.0, 360.0);
                    }
                }
                if (ImGui::Checkbox(OBFUSCATE("Force Crosshair"), &crosshair)) { Patches(); }
                if (ImGui::Checkbox(OBFUSCATE("No Aimpunch"), &aimpunch)) { Patches(); }
                if (ImGui::Checkbox(OBFUSCATE("Hide Kill Notifications"),
                                    &killnotes)) { Patches(); }
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem(OBFUSCATE("Misc"))) {
                if (ImGui::Checkbox(OBFUSCATE("Disable Ragdoll"), &ragdoll)) { Patches(); }
                ImGui::Checkbox(OBFUSCATE("Interaction Range"), &interactionrange);
                ImGui::Checkbox(OBFUSCATE("Free Armor"), &freearmor);
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
    }
    ImGui::End();
}


void SetupImgui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    glWidth = get_Width();
    glHeight = get_Height();
    io.DisplaySize = ImVec2((float)glWidth , (float)glHeight);
    ImGui_ImplOpenGL3_Init("#version 100");
    ImGui::StyleColorsDark();
    ImGui::GetStyle().ScaleAllSizes(6.0f);
    io.Fonts->AddFontFromMemoryTTF(Roboto_Regular, 30, 30.0f);
    espFont = io.Fonts->AddFontFromMemoryCompressedTTF(RetroGaming, compressedRetroGamingSize, 15);
    flagFont = io.Fonts->AddFontFromMemoryCompressedTTF(Minecraftia_Regular, compressedMinecraftia_RegularSize, 15);
}

EGLBoolean hook_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    eglQuerySurface(dpy, surface, EGL_WIDTH, &glWidth);
    eglQuerySurface(dpy, surface, EGL_HEIGHT, &glHeight);


    if (!setupimg) {
        SetupImgui();
        setupimg = true;
    }

    ImGuiIO &io = ImGui::GetIO();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();

    DrawMenu();

    ImGui::EndFrame();
    ImGui::Render();
    glViewport(0, 0, (int)glWidth, (int)glHeight);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    return old_eglSwapBuffers(dpy, surface);
}

void *hack_thread(void *arg) {
    int tries = 0;
    do {
        sleep(1);
        auto maps =  KittyMemory::getMapsByName("split_config.arm64_v8a.apk");
        for(std::vector<ProcMap>::iterator it = maps.begin(); it != maps.end(); ++it) {
            auto address = KittyScanner::findHexFirst(it->startAddress, it->endAddress,"7F 45 4C 46 02 01 01 00 00 00 00 00 00 00 00 00 03 00 B7 00 01 00 00 00 00 F2 79 00 00 00 00 00 40 00 00 00 00 00 00 00 88 D8 E1 02 00 00 00 00", "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");
            if(address != 0)
            {
                libBaseAddress = address;
                libBaseEndAddress = it->endAddress;
                libSize = it->length;
            }
        }
        if (tries > 10)
        {
            auto map = KittyMemory::getLibraryBaseMap("libil2cpp.so");
            libBaseAddress = map.startAddress;
            libBaseEndAddress = map.endAddress;
            unsafe = true;
        }
        tries++;
    } while (libBaseAddress == 0);
    Hooks();
    Pointers();
    Patches();
    auto eglhandle = dlopen("libunity.so", RTLD_LAZY);
    auto eglSwapBuffers = dlsym(eglhandle, "eglSwapBuffers");
    DobbyHook((void*)eglSwapBuffers,(void*)hook_eglSwapBuffers, (void**)&old_eglSwapBuffers);
    void *sym_input = DobbySymbolResolver(("/system/lib/libinput.so"), ("_ZN7android13InputConsumer21initializeMotionEventEPNS_11MotionEventEPKNS_12InputMessageE"));
    if (NULL != sym_input) {
        DobbyHook(sym_input,(void*)myInput,(void**)&origInput);
    }
    return nullptr;
}
