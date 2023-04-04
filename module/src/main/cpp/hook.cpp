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
#include "user_interface.h";

#define GamePackageName "com.criticalforceentertainment.criticalops"


monoString* CreateIl2cppString(const char* str)
{
    static monoString* (*CreateIl2cppString)(const char* str, int *startIndex, int *length) =
    (monoString* (*)(const char* str, int *startIndex, int *length))(get_absolute_address(string2Offset(OBFUSCATE("0x1AA45C4"))));
    int* startIndex = nullptr;
    int* length = (int *)strlen(str);
    return CreateIl2cppString(str, startIndex, length);
}

ESPCfg visibleCfg, invisibleCfg, espcfg;
AimbotCfg pistolCfg, smgCfg, arCFg, shotgunCfg, sniperCfg;
bool firsttime = false;
static int selected = 0;
static int sub_selected = 10;
const char *Shader = OBFUSCATE("unity_SHC");
void* TouchControls = nullptr;
void* pSys = nullptr;
void* pSpeed = nullptr;
void* localCharacter = nullptr;
bool unsafe,recoil, radar, flash, smoke, scope, setupimg, spread, aimpunch, speed, reload, esp, forcebuy, kickback, crouch, wallbang, rain, rain1,
        fov, ggod, killnotes,crosshair, moneyreward, mindamage, maxdamage, viewmodel, viewmodelfov, boxesp, healthesp, healthNumber, espName, weaponEsp, armroFlag, spawnbullets,
        canmove,isPurchasingSkins, fly, removecharacter, tutorial, freeshop, gravity, dropweapon,ragdoll, crouchheight, cameraheight, interactionrange, jumpheight, bhop,
        noslow, shake, eoi, gbounciness, ammo, firerate, iea, p100Crosshair, tradar, fovCheck, fscope, applied = false, nosway, burstfire,rain2,
        pickup, silentknife, armorpen, chams, headhitbox, bodyhitbox, silentknife1, addmone;

float speedval = 5.100000, fovModifier, recoilval, spreadval, viemodelposz, viewmodelfovval, gravityval, flyval ,crouchval, camval, jumpval = 4.100000,
firerateval = 500, fovValue = 360, bounceval, gundmgm;
int ammocount = 0, callCount = 0, burstfireval, LocalTeam, aimPos = 0, shootControl = 0;

extern int glHeight;
extern int glWidth;
ImFont* espFont;

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

void TouchControlsUpdate(void* obj)
{
    if (obj) {
        TouchControls = obj;
    }
    return oTouchControlsUpdate(obj);
}

void TouchControlsDestroy(void* obj)
{
    if (obj)
    {
        TouchControls = nullptr;
    }
    return oTouchControlsDestroy(obj);
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

bool isInFov2(Vector2 rotation, Vector2 newAngle, AimbotCfg cfg)
{
    if (!cfg.fovCheck) return true;

    Vector2 difference = newAngle - rotation;

    if (difference.Y > 180) difference.Y -= 360;
    if (difference.Y < -180) difference.Y += 360;

    if(sqrt(difference.X * difference.X + difference.Y * difference.Y) > cfg.fovValue)
    {
        return false;
    }
    return true;
}

void* getValidEnt3(AimbotCfg cfg, Vector2 rotation) {
    int id = getLocalId(pSys);
    void *localPlayer = getPlayer(pSys, id);
    int localTeam = get_PlayerTeam(localPlayer);
    float closestEntDist = 99999.0f;
    void *closestCharacter = nullptr;
    monoList<void **> *characterList = getAllCharacters(pSys);
    for (int i = 0; i < characterList->getSize(); i++) {
        void *currentCharacter = (monoList<void **> *) characterList->getItems()[i];
        int curTeam = get_CharacterTeam(currentCharacter);
        int health = get_Health(currentCharacter);
        bool canSet = false;
        Vector2 newAngle;
        // check if entity is in fov;

        if (cfg.aimbot && localCharacter && currentCharacter)
        {
            if (get_Health(localCharacter) > 0 && get_Health(currentCharacter) > 0)
            {
                Vector3 localHead = getBonePosition(localCharacter, 10);
                if(getIsCrouched(localCharacter))
                {
                    localHead = localHead - Vector3(0, 0.5, 0);
                }
                Vector3 enemyBone = getBonePosition(currentCharacter, cfg.aimBone);
                Vector3 deltavec = enemyBone - localHead;
                float deltLength = sqrt(deltavec.X * deltavec.X + deltavec.Y * deltavec.Y +
                                        deltavec.Z * deltavec.Z);

                newAngle.X = -asin(deltavec.Y / deltLength) * (180.0 / PI);
                newAngle.Y = atan2(deltavec.X, deltavec.Z) * 180.0 / PI;
                if (isInFov2(rotation, newAngle, cfg))
                {
                    if (localCharacter && health > 0 && localTeam != curTeam && curTeam != -1) {
                        if (cfg.visCheck && get_Health(localCharacter) > 0) {
                            if (isCharacterVisible(currentCharacter, pSys)) {
                                canSet = true;
                            }
                        }
                        Vector3 localPosition = get_Position(getTransform(localCharacter));
                        Vector3 currentCharacterPosition = get_Position(
                                getTransform(currentCharacter));
                        Vector3 currentEntDist = Vector3::Distance(localPosition,
                                                                   currentCharacterPosition);
                        if (Vector3::Magnitude(currentEntDist) < closestEntDist) {
                            if (cfg.visCheck && !canSet) continue;
                            closestEntDist = Vector3::Magnitude(currentEntDist);
                            closestCharacter = currentCharacter;
                        }
                    }
                }
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
            }

            void *WeaponDefData = *(void **) ((uint64_t) CharacterData + 0x80);
            if(WeaponDefData != nullptr){
                //add reload time
                if(recoil){
                    *(float *) ((uint64_t) WeaponDefData + 0x100) = recoilval;
                    *(float *) ((uint64_t) WeaponDefData + 0xF0) = recoilval;
                }

                if(moneyreward){
                    *(int *) ((uint64_t) WeaponDefData + 0x48) = 100;
                }

                if(forcebuy){
                    *(bool*)((uint64_t) WeaponDefData + 0x40) = true;
                    *(int *) ((uint64_t) WeaponDefData + 0x44) = 200;
                }

                if(mindamage){
                    *(float *) ((uint64_t) WeaponDefData + 0x50) = mindamage;
                }

                if(maxdamage){
                    *(float *) ((uint64_t) WeaponDefData + 0x4C) = maxdamage;
                }

                if(firerate){
                    *(float *) ((uint64_t) WeaponDefData + 0x64) = 2000;
                }

                if(burstfire){
                    *(int *) ((uint64_t) WeaponDefData + 0x11C) = burstfireval;
                    *(float *) ((uint64_t) WeaponDefData + 0x120) = 0;
                }

                if(ammo){
                    *(bool*)((uint64_t) WeaponDefData + 0x94) = false;
                }




                if(fscope){
                    *(int *) ((uint64_t) WeaponDefData + 0x98) = 1;
                }

                if(wallbang){
                    //*(int *) ((uint64_t) WeaponDefData + 0x7C) = 3;
                }

                if(armorpen){
                    *(int *) ((uint64_t) WeaponDefData + 0x80) = 4;
                }

                if(pickup){
                    *(float *) ((uint64_t) WeaponDefData + 0x88) = 0;
                    *(float *) ((uint64_t) WeaponDefData + 0x8C) = 0;
                }

                if(burstfire){
                    *(int *) ((uint64_t) WeaponDefData + 0x11C) = burstfireval;
                    *(int *) ((uint64_t) WeaponDefData + 0x120) = 0;
                }

                if(shake){
                    *(float *) ((uint64_t) WeaponDefData + 0x110) = 0;
                }

                if(gbounciness){
                    *(float *) ((uint64_t) WeaponDefData + 0x14C) = bounceval;
                }

                if(eoi){
                    *(bool*)((uint64_t) WeaponDefData + 0x140) = true;
                }

                if(iea){
                    *(bool*)((uint64_t) WeaponDefData + 0x144) = 99999;
                }
            }
        }
    }
    oldUpdateWeapon(obj, deltatime);
}

void UpdateGame(void* obj, float dt){
    if(obj != nullptr){
        if(addmone){
            AddMoney(get_LocalPlayer(pSys), 1000);
            addmone = false;
        }
    }
    oldUpdateGame(obj, dt);
}

void RenderOverlayFlashbang(void* obj){
    if(obj != nullptr && flash){
        *(float*)((uint64_t) obj + 0x38) = 0;//m_flashTime
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

int getCurrentWeaponCategory(void* character)
{
    void* characterData = *(void**)((uint64_t)character + 0x98);
    void* m_wpn = *(void**)((uint64_t)characterData + 0x80);
    if (m_wpn)
    {
        return *(int*)((uint64_t)m_wpn + 0x38);
    }
    return -1;
}

bool isCharacterShooting(void* character)
{
    void* characterData = *(void**)((uint64_t)character + 0x98);;
    return *(bool*)((uint64_t)characterData + 0x6C);
}

Vector2 isInFov(Vector2 rotation, Vector2 newAngle, AimbotCfg cfg)
{
    Vector2 difference = newAngle - rotation;

    if (difference.Y > 180) difference.Y -= 360;
    if (difference.Y < -180) difference.Y += 360;

    if(sqrt(difference.X * difference.X + difference.Y * difference.Y) > cfg.fovValue)
    {
        return Vector2(0,0);
    }
    return difference;
}

void setRotation(void* character, Vector2 rotation)
{
    Vector2 newAngle;
    Vector2 difference;
    difference.X = 0;
    difference.Y = 0;
    AimbotCfg cfg;
    if (localCharacter) {
        int currWeapon = getCurrentWeaponCategory(localCharacter);
        if (currWeapon != -1) {
            switch (currWeapon) {
                case 0:
                    cfg = pistolCfg;
                    break;
                case 1:
                    cfg = arCFg;
                    break;
                case 2:
                    cfg = smgCfg;
                    break;
                case 3:
                    cfg = shotgunCfg;
                    break;
                case 4:
                    cfg = sniperCfg;
                    break;
            }
        }
    }
    void* closestEnt = nullptr;
    if(pSys != nullptr) {
        if (character && localCharacter != nullptr && get_IsInitialized(localCharacter)) {

            closestEnt = getValidEnt3(cfg, rotation);

            if (get_Health(localCharacter) > 0) {
                if (cfg.aimbot && closestEnt) {

                    Vector3 localHead = getBonePosition(localCharacter, 10);
                    if(getIsCrouched(localCharacter))
                    {
                        localHead = localHead - Vector3(0, 0.5, 0);
                    }
                    Vector3 enemyBone;
                    if(aimPos == 0){
                        enemyBone = getBonePosition(closestEnt, HEAD);
                    } else if(aimPos == 1){
                        enemyBone = getBonePosition(closestEnt, CHEST);
                    }
                    else if(aimPos == 2){
                        enemyBone = getBonePosition(closestEnt, STOMACH);
                    }

                    Vector3 deltavec = enemyBone - localHead;
                    float deltLength = sqrt(deltavec.X * deltavec.X + deltavec.Y * deltavec.Y + deltavec.Z * deltavec.Z);

                    newAngle.X = -asin(deltavec.Y / deltLength) * (180.0 / PI);
                    newAngle.Y = atan2(deltavec.X, deltavec.Z) * 180.0 / PI;

                }

                if (cfg.aimbot && character == localCharacter && closestEnt && get_Health(localCharacter) > 0) {
                    if (cfg.onShoot) {
                        if (isCharacterShooting(localCharacter)) {
                            if (cfg.fovCheck) {
                                difference = isInFov(rotation, newAngle, cfg);
                            } else {
                                difference = newAngle - rotation;
                            }
                        }
                    }
                    else
                    {
                        if (cfg.fovCheck) {
                            difference = isInFov(rotation, newAngle, cfg);
                        } else {
                            difference = newAngle - rotation;
                        }

                    }

                    if (cfg.aimbotSmooth) {
                        difference = difference / cfg.smoothAmount;
                    }

                }
            }
        }
        if(cfg.triggerbot && closestEnt && localCharacter && get_Health(localCharacter) > 0)
        {
            int hitIndex = 0;
            Ray ray = ScreenPointToRay(get_camera(), Vector2(glWidth/2, glHeight/2), 2);
            UpdateCharacterHitBuffer(pSys, closestEnt, ray, &hitIndex);

            if(hitIndex && !shootControl)
            {
                shootControl = 1;
            }
        }
    }
    oSetRotation(character, rotation + difference);
}

void GameSystemDestroy(void* obj)
{
    oGameSystemDestroy(obj);
    pSys = nullptr;
    localCharacter = nullptr;
}

float FovWorld(void* obj){
    if(obj != nullptr && fov){
        return fovModifier;
    }
    return oldFovWorld(obj);
}

float FovViewModel(void* obj){
    if(obj != nullptr && fov){
        return viewmodelfovval;
    }
    return oldFovViewModel(obj);
}

float get_cameraFov(void* pSys)
{
    void* GameplayModule = *(void**)((uint64_t)pSys + 0x80);
    void* CameraSystem = *(void**)((uint64_t)GameplayModule + 0x30);
    return *(float*)((uint64_t)CameraSystem + 0x8C); // m_horizontalFieldOfView
}


float get_Heightt(void* obj){
    if(obj != nullptr ){
        if(fly){
            return flyval;
        }
        else{
            return 1.5f;
        }
    }

    return oldget_Height(obj);
}


void LoadSettings(void* obj){
    if(obj != nullptr){
        void* GraphicsPorfile = *(void**)((uint64_t) obj + 0x38);
        if(GraphicsPorfile != nullptr){
            *(float*)((uint64_t) GraphicsPorfile + 0x38) = 100;
        }
    }
}

std::string weaponDefToStr(int weaponDef)
{
    switch (weaponDef)
    {
        case 4:return "AK47";
        case 5:return "M14";
        case 6:return "M4";
        case 7:return "SA58";
        case 8:return "MR96";
        case 9:return "HK417";
        case 10:return "SG551";
        case 11:return "URatio";
        case 12:return "SmokeGrenade";
        case 13:return "Flashbang";
        case 14:return "MTX";
        case 15:return "MP5";
        case 16:return "XD45";
        case 17:return "FP6";
        case 18:return "Super90";
        case 19:return "AUG";
        case 20:return "P90";
        case 21:return "P250";
        case 22:return "Frag";
        case 103:return "Knife";
        case 106:return "MP7";
        case 3078:return "TRG";
        case 4009:return "Vector";
        case 6525:return "Winchester";
        case 6712:return "MPX";
        case 7073:return "SVD";
        case 7519:return "Deagle";
        case 13707:return "AR15";
        case 14680:return "SCARH";
        case 15079:return "KSG";
    }
    return "Invalid Weapon";
}


void CheckCharacterVisiblity(void* obj, bool* visibility){
    oldCheckCharacterVisibility(obj, visibility);
    if(obj != nullptr){
        *visibility = true;
    }
}


void GenerateHash(void* obj){
    oldGenerateHash(obj);
    if(obj != nullptr){
        *(monoString**)((uint64_t) obj + 0x60) = CreateIl2cppString(OBFUSCATE("67FA5C67C843980A4D20390A27D5A728"));
        *(monoString**)((uint64_t) obj + 0x50) = CreateIl2cppString(OBFUSCATE("67FA5C67C843980A4D20390A27D5A728"));
    }
}

HOOKAF(void, Input, void *thiz, void *ex_ab, void *ex_ac) {
    origInput(thiz, ex_ab, ex_ac);
    ImGui_ImplAndroid_HandleInputEvent((AInputEvent *)thiz);
    return;
}

const char* combo_items[3] = { "head", "chest", "stomach" };
const char* expand[3] = { "head", "body" };
// Initilizers with patterns <3
void Hooks()
{
   // HOOK("0x1B872E8", BackendManager, oldBackendManager);
  //  HOOK("0x19B97B8", set_Spread, oldset_Spread);// Overlay Scope set spread
   // HOOK("0x19B9570", RenderOverlayFlashbang, oldRenderOverlayFlashbang); // flash render overlay
    //HOOK("0x19BF970", RenderOverlaySmoke, oldRenderOverlaySmoke); // smoke render overlay
    HOOK("0x10D3390", GameSystemUpdate, oldGameSystemUpdate); // GameSystem Update
    HOOK("0x1A80624", UpdateWeapon, oldUpdateWeapon); // character
    HOOK("0x10C06F4", FovViewModel, oldFovViewModel); // speed
    HOOK("0x10CF518", GameSystemDestroy, oGameSystemDestroy); // GameSystem Destroy
    HOOK("0x10C06B4", FovWorld, oldFovWorld); // speed
    HOOK("0x1A7CFCC", setRotation, oSetRotation);
    //HOOK("0x10D101C", CreateCharacter, oCreateCharacter);
   // HOOK("0x10D3DCC", DestroyCharacter, oDestroyCharacter);
    //HOOK("0x1E5F1CC", AddHit, oAddHit);
    HOOK("0x1AEC7D4", get_Heightt , oldget_Height);
    HOOK("0x1D871D4", LoadSettings , oldLoadSettings);
    //HOOK("0x10D738C", ProcessHitBuffers , oldProcessHitBuffers);
    HOOK("0xACB1AC", TouchControlsUpdate, oTouchControlsUpdate); // touchcontrols update
    HOOK("0xACAAC4", TouchControlsDestroy, oTouchControlsDestroy); // touchcontrols destroy
    //HOOK("0x10D80B0", RaycastCharacterr , oldRaycastCharacter);
   // HOOK("0x1F5E8B0", SendShoot , oldSendShoot);
    HOOK("0x1D80CB8", UpdateGame , oldUpdateGame);
    HOOK("0x19B2708", CheckCharacterVisiblity , oldCheckCharacterVisibility);
    HOOK("0x1E5DA3C", GenerateHash , oldGenerateHash);
}


void Pointers()
{
//RequestPurchaseSkin = (void(*)(void*, int, int, bool)) get_absolute_address(string2Offset(OBFUSCATE("0x1B80760")));
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
    getIsCrouched = (bool(*)(void*)) get_absolute_address(string2Offset(OBFUSCATE("0x1A7C474"))); // get crouched
    GetWeaponByID = (void*(*)(void*, int)) get_absolute_address(string2Offset(OBFUSCATE("0x1A7D03C"))); // get crouched
    FindWeaponID = (int(*)(void*,void*)) get_absolute_address(string2Offset(OBFUSCATE("0x10D5164"))); // GameSystem FindWeaponID
    UpdateCharacterHitBuffer = (void(*)(void*, void*, Ray, int*)) get_absolute_address(string2Offset(OBFUSCATE("0x10D7D20"))); // UpdateCharacterHitBuffer
    ScreenPointToRay = (Ray(*)(void*, Vector2, int)) get_absolute_address(string2Offset(OBFUSCATE("0x1A1E008"))); // ScreenPointToRay
    onInputButtons = (void(*)(void*, int, bool)) get_absolute_address(string2Offset(OBFUSCATE("0xACDF74"))); // OnInputButton
    TraceShot = (void*(*)(void*,void*, Ray)) get_absolute_address(string2Offset(OBFUSCATE("0x10D57B0"))); // OnInputButton
    get_AllHits =(std::uintptr_t(*)(void*)) get_absolute_address(string2Offset(OBFUSCATE("0x1E5EFF0"))); // Character GetBodyPart
    get_LocalID = (int(*)(void*)) get_absolute_address(string2Offset(OBFUSCATE("0x10C2848")));
    set_Position = (void(*)(void*, Vector3)) get_absolute_address(string2Offset(OBFUSCATE("0x1A44C7C")));
    AddMoney = (void(*)(void*, int)) get_absolute_address(string2Offset(OBFUSCATE("0x1D58174")));
    get_LocalPlayer =(void*(*)(void*)) get_absolute_address(string2Offset(OBFUSCATE("0x1D58174")));
}

void Patches(){
    PATCH_SWITCH("0x1D5638C", "1F2003D5C0035FD6", spread);//UpdateSpread
    PATCH_SWITCH("0x1D56328", "000080D2C0035FD6", aimpunch);//AimPunchRecover
    PATCH_SWITCH("0x19C46E0", "200080D2C0035FD6", canmove);//CanMove
    PATCH_SWITCH("0x19C605C", "200080D2C0035FD6", canmove);//CanShoot
    PATCH_SWITCH("0x19C1FFC", "200080D2C0035FD6", canmove);//ShootingAllowed
    PATCH_SWITCH("0x19C1FE8", "200080D2C0035FD6", canmove);//MovementAllowed
    PATCH_SWITCH("0x19C20E4", "200080D2C0035FD6", canmove);//PlantingAllowed
    PATCH_SWITCH("0x19C20E4", "200080D2C0035FD6", canmove);//PlantingAllowed
    PATCH_SWITCH("0x10D62C0", "000080D2C0035FD6", ggod);//GrenadeHitCharacter
    PATCH_SWITCH("0xF34034", "1F2003D5C0035FD6", killnotes);//SetKillNotification
    PATCH_SWITCH("0x172D454", "200080D2C0035FD6", crosshair);//get_Crosshair
    PATCH_SWITCH("0x1BFDF2C", "1F2003D5C0035FD6", smoke);//SmokeGrenadeEffect
    PATCH_SWITCH("0x1A7CB20", "1F2003D5C0035FD6", crouch);//isSupressor
    PATCH_SWITCH("0x173109C", "E003271EC0035FD6", nosway);//ViewModelAimSway
    PATCH_SWITCH("0x10D7444", "1F2003D5", wallbang);//ProcessHitBuffers + 0xB8
    PATCH_SWITCH("0x10D7F3C", "01F0271E", headhitbox);//UpdateCharacterHitBuffer + 0x21C
    PATCH_SWITCH("0x10D7F40", "01F0271E", bodyhitbox);//UpdateCharacterHitBuffer + 0x220
    PATCH_SWITCH("0x1A7B7B8", "200080D2C0035FD6", headhitbox);//Intersects
    PATCH_SWITCH("0x1A7B7B8", "200080D2C0035FD6", bodyhitbox);//Intersects
    PATCH_SWITCH("0x10C29A4", "E0031FAA", radar);//FetchFollowedCharacterTeamIndex + 0x8C
    PATCH_SWITCH("0x10D61E8", "01F0271E", silentknife);//MeleeHit + 0x808
    PATCH_SWITCH("0x10D5C24", "08F0271E", silentknife1);//MeleeHit + 0x244
    PATCH_SWITCH("0x19B278C", "1F01086B", tradar);//CheckCharacterVisiblity + 0x84
    PATCH_SWITCH("0x1AE98AC", "000080D2C0035FD6", tradar);//Linecast(Vector3 start, Vector3 end, int layerMask)
    PATCH_SWITCH("0x1DAFBAC", "000080D2C0035FD6", killnotes);
    PATCH("0x20C9830", "000080D2C0035FD6");//t
    PATCH("0x20C95D0", "000080D2C0035FD6");//t
    PATCH("0x20CA2AC", "000080D2C0035FD6");//t
    PATCH("0x20CA0E8", "000080D2C0035FD6");//t
    PATCH("0x20C9700", "000080D2C0035FD6");//t
    PATCH("0x20C9528", "000080D2C0035FD6");//t
    PATCH("0x1D74F1C", "200080D2C0035FD6");
  //  PATCH("0x10D7444", "1F2003D5");
    // PATCH("0x1A71B50", "1F2003D5C0035FD6");
  //  PATCH("0x1A5C210", "1F2003D5C0035FD6");
   // PATCH("0x1A5D454", "1F2003D5C0035FD6");
  //  PATCH("0x1A71A84", "1F2003D5C0035FD6");
}

void ESP()
{
    AimbotCfg cfg;
    if (pSys != nullptr) {
        if (!firsttime) {
            wait(2);
            firsttime = true;
        }

        int id = getLocalId(pSys);
        void* localPlayer = getPlayer(pSys, id);
        if (localPlayer != nullptr) {
            int localTeam = get_PlayerTeam(localPlayer);
            monoList<void**>* characterList = getAllCharacters(pSys);
            for (int i = 0; i < characterList->getSize(); i++) {
                void* currentCharacter = (monoList<void**>*)characterList->getItems()[i];
                if (get_Player(currentCharacter) == localPlayer) {
                    localCharacter = currentCharacter;
                }

                if (localCharacter != nullptr) {
                    int curTeam = get_CharacterTeam(currentCharacter);
                    int health = get_Health(currentCharacter);
                    if (health > 0 && get_IsInitialized(currentCharacter) && localTeam != curTeam && curTeam != -1) {
                        void* transform = getTransform(currentCharacter);
                        Vector3 position = get_Position(transform);
                        Vector3 transformPos = WorldToScreen(get_camera(), position, 2);
                        transformPos.Y = glHeight - transformPos.Y;
                        Vector3 headPos = getBonePosition(currentCharacter, HEAD);
                        Vector3 chestPos = getBonePosition(currentCharacter, CHEST);
                        Vector3 wschestPos = WorldToScreen(get_camera(), chestPos, 2);
                        Vector3 wsheadPos = WorldToScreen(get_camera(), headPos, 2);
                        Vector3 aboveHead = headPos + Vector3(0, 0.2, 0); // estimate
                        Vector3 headEstimate = position + Vector3(0, 1.48, 0); // estimate
                        Vector3 wsAboveHead = WorldToScreen(get_camera(), aboveHead, 2);
                        Vector3 wsheadEstimate = WorldToScreen(get_camera(), headEstimate, 2);

                        wsAboveHead.Y = glHeight - wsAboveHead.Y;
                        wsheadEstimate.Y = glHeight - wsheadEstimate.Y;

                        float height = transformPos.Y - wsAboveHead.Y;
                        float width = (transformPos.Y - wsheadEstimate.Y) / 2;

                        Vector3 localPosition = get_Position(getTransform(localCharacter));

                        Vector3 currentCharacterPosition = get_Position(getTransform(currentCharacter));
                        float currentEntDist = Vector3::Distance(localPosition, currentCharacterPosition);

                        espcfg = invisibleCfg;

                        if (espcfg.snapline && transformPos.Z > 0) {
                            DrawLine(ImVec2(glWidth / 2, glHeight),
                                     ImVec2(transformPos.X, transformPos.Y),
                                     ImColor(espcfg.snaplineColor.x, espcfg.snaplineColor.y,
                                             espcfg.snaplineColor.z, (255 - currentEntDist * 5.0)),
                                     3);
                        }

                        if (espcfg.bone && transformPos.Z > 0) {
                            DrawBones(currentCharacter, LOWERLEG_LEFT, UPPERLEG_LEFT, espcfg);
                            DrawBones(currentCharacter, LOWERLEG_RIGHT, UPPERLEG_RIGHT, espcfg);
                            DrawBones(currentCharacter, UPPERLEG_LEFT, STOMACH, espcfg);
                            DrawBones(currentCharacter, UPPERLEG_RIGHT, STOMACH, espcfg);
                            DrawBones(currentCharacter, STOMACH, CHEST, espcfg);
                            DrawBones(currentCharacter, LOWERARM_LEFT, UPPERARM_LEFT, espcfg);
                            DrawBones(currentCharacter, LOWERARM_RIGHT, UPPERARM_RIGHT, espcfg);
                            DrawBones(currentCharacter, UPPERARM_LEFT, CHEST, espcfg);
                            DrawBones(currentCharacter, UPPERARM_RIGHT, CHEST, espcfg);
                            Vector3 diff = wschestPos - wsheadPos;
                            Vector3 neck = (chestPos + headPos) / 2;
                            Vector3 wsneck = WorldToScreen(get_camera(), neck, 2);
                            wsneck.Y = glHeight - wsneck.Y;
                            wschestPos.Y = glHeight - wschestPos.Y;
                            wsheadPos.Y = glHeight - wsheadPos.Y;
                            if (wschestPos.Z > 0 && wsneck.Z) {
                                DrawLine(ImVec2(wschestPos.X, wschestPos.Y),
                                         ImVec2(wsneck.X, wsneck.Y),
                                         ImColor(espcfg.boneColor.x, espcfg.boneColor.y,
                                                 espcfg.boneColor.z),
                                         3);
                            }

                            if (wsheadPos.Z > 0 && wschestPos.Z > 0) {
                                float radius = sqrt(diff.X * diff.X + diff.Y * diff.Y);
                                auto background = ImGui::GetBackgroundDrawList();

                                background->AddCircle(ImVec2(wsheadPos.X, wsheadPos.Y), radius / 2, IM_COL32(espcfg.boneColor.x * 255,espcfg.boneColor.y * 255,espcfg.boneColor.z * 255, 255),0,3.0f);
                            }
                        }

                        if (espcfg.box && transformPos.Z > 0 && wsAboveHead.Z > 0) {
                            DrawOutlinedBox2(wsAboveHead.X - width / 2, wsAboveHead.Y, width,
                                             height,
                                             ImVec4(espcfg.boxColor.x, espcfg.boxColor.y, espcfg.boxColor.z,255),3);
                        }

                        if (espcfg.healthesp && transformPos.Z > 0 && wsAboveHead.Z > 0) {
                            DrawOutlinedFilledRect(wsAboveHead.X - width / 2 - 12, wsAboveHead.Y + height * (1 - (static_cast<float>(health) / 100.0f)), 3, height * (static_cast<float>(health) / 100.0f), HealthToColor(health));
                        }

                        if (espcfg.healthNumber && transformPos.Z > 0 && wsAboveHead.Z > 0) {
                            if (health < 100) {
                                DrawText(ImVec2(wsAboveHead.X - width / 2 - 17, wsAboveHead.Y + height * (1 - static_cast<float>(health) / 100.0f) - 3), ImVec4(255, 255, 255, 255), std::to_string(health), espFont);
                            }
                        }

                        if (espcfg.name && transformPos.Z > 0 && wsAboveHead.Z > 0) {
                            void* player = get_Player(currentCharacter);
                            void* boxedValueName = *(void**)((uint64_t)player + 0x70);
                            monoString* username = *(monoString**)((uint64_t)boxedValueName + 0x20);
                            float compensation = username->getLength() * 4.0f;
                            DrawText(ImVec2(wsheadPos.X - compensation, wsAboveHead.Y - 20),ImVec4(espcfg.nameColor.x, espcfg.nameColor.y, espcfg.nameColor.z,255), username->getString(), espFont);
                        }

                        /* if (espcfg.weapon && transformPos.Z > 0) { 	std::string weapon = get_characterWeaponName(pSys, currentCharacter);
						     	// font is 15 px has 2 px outline so has 16px per character, to center the font we do
						      float compensation = weapon.length() *4.0f;

						      DrawText(ImVec2(wsAboveHead.X - compensation, transformPos.Y + 7),
						               ImVec4(espcfg.weaponColor.x, espcfg.weaponColor.y,
						                      espcfg.weaponColor.z, 255), weapon, espFont);
						  }*/
                    }

                    AimbotCfg cfg;
                    if (localCharacter && get_Health(localCharacter)) {
                        int currWeapon = getCurrentWeaponCategory(localCharacter);
                        if (currWeapon != -1) {
                            switch (currWeapon) {
                                case 0:
                                    cfg = pistolCfg;
                                    break;
                                case 1:
                                    cfg = arCFg;
                                    break;
                                case 2:
                                    cfg = smgCfg;
                                    break;
                                case 3:
                                    cfg = shotgunCfg;
                                    break;
                                case 4:
                                    cfg = sniperCfg;
                                    break;
                            }
                        }
                    }

                    if (cfg.fovCheck) {
                        float worldFov = get_cameraFov(pSys);
                        float radius = tan(fovValue * (PI / 180) / 2) / tan(worldFov / 2) * glWidth / 2;
                        auto background = ImGui::GetBackgroundDrawList();
                        background->AddCircle(ImVec2(glWidth / 2, glHeight / 2), radius, IM_COL32(255, 255, 255, 255), 0, 3.0f);
                        if (cfg.drawFov) {
                            float worldFov = get_cameraFov(pSys);
                            float radius = tan(cfg.fovValue * (PI / 180) / 2) / tan(90 / 2) * glWidth / 2;
                            auto background = ImGui::GetBackgroundDrawList();
                            float correction = 0;
                            if ((worldFov - 90) < 0) {
                                correction = worldFov - 90;
                            }

                            background->AddCircle(ImVec2(glWidth / 2, glHeight / 2), (radius)*PI - correction / PI * 2, IM_COL32(255, 255, 255, 255), 0, 3.0f);
                        }
                    }
                }
            }
        }
    }

    if (p100Crosshair) {
        ImVec2 Middle = ImVec2(glWidth / 2, glHeight / 2);

        CoolCrosshair(ImVec2(Middle.x - 2, Middle.y), ImVec4(0, 0, 0, 255));
        CoolCrosshair(ImVec2(Middle.x - 2, Middle.y - 2), ImVec4(0, 0, 0, 255));
        CoolCrosshair(ImVec2(Middle.x + 2, Middle.y), ImVec4(0, 0, 0, 255));
        CoolCrosshair(ImVec2(Middle.x + 2, Middle.y + 2), ImVec4(0, 0, 0, 255));
        CoolCrosshair(ImVec2(Middle.x, Middle.y - 2), ImVec4(0, 0, 0, 255));
        CoolCrosshair(ImVec2(Middle.x + 2, Middle.y - 2), ImVec4(0, 0, 0, 255));
        CoolCrosshair(ImVec2(Middle.x, Middle.y + 2), ImVec4(0, 0, 0, 255));
        CoolCrosshair(ImVec2(Middle.x - 2, Middle.y + 2), ImVec4(0, 0, 0, 255));
        CoolCrosshair(Middle, ImVec4(255, 255, 255, 255));
    }
}

void DrawMenu() {
    ImGui::Begin(OBFUSCATE("menu"), nullptr, ImGuiWindowFlags_NoDecoration);
    {
        auto draw = ImGui::GetWindowDrawList();
        ImVec2 pos = ImGui::GetWindowPos();
        ImGui::SetWindowSize(ImVec2(900, 500));
        ImVec2 size = ImGui::GetWindowSize();

        draw->AddRectFilled(pos, ImVec2(pos.x + 55, pos.y + size.y),
                            ImColor(20, 20, 20, 255)); // left rect
        draw->AddRectFilled(ImVec2(pos.x + 55, pos.y), ImVec2(pos.x + size.x, pos.y + 55),
                            ImColor(20, 20, 20, 255)); // upper rect

        draw->AddLine(ImVec2(pos.x + 55, pos.y + 55), ImVec2(pos.x + size.x, pos.y + 55),
                      ImColor(255, 255, 255, 15)); // upper line
        draw->AddLine(ImVec2(pos.x + 55, pos.y), ImVec2(pos.x + 55, pos.y + size.y),
                      ImColor(255, 255, 255, 15)); // left line

        ImGui::SetCursorPos(ImVec2(-1, 11));
        ImGui::BeginGroup();
        if (custom_interface::tab("A", 0 == selected)) {
            selected = 0;
            sub_selected = 10;
        }
        if (custom_interface::tab("V", 1 == selected)) {
            selected = 1;
            sub_selected = 1;
        }
        if (custom_interface::tab("M", 2 == selected)) {
            selected = 2;
            sub_selected = 2;
        }

        ImGui::EndGroup();

        if (selected == 0) {
            ImGui::BeginGroup();
            ImGui::SetCursorPos(ImVec2(65, 14));
            if (custom_interface::subtab("Rifle", 10 == sub_selected)) {
                sub_selected = 10;
            }
            ImGui::SameLine();
            if (custom_interface::subtab("SMG", 11 == sub_selected)) {
                sub_selected = 11;
            }
            ImGui::SameLine();
            if (custom_interface::subtab("Pistol", 12 == sub_selected)) {
                sub_selected = 12;
            }
            ImGui::SameLine();
            if (custom_interface::subtab("Shotgun", 13 == sub_selected)) {
                sub_selected = 13;
            }
            ImGui::SameLine();
            if (custom_interface::subtab("Sniper", 14 == sub_selected)) {
                sub_selected = 14;
            }
            ImGui::EndGroup();
        }

        if (sub_selected == 10) {
            ImGui::SetCursorPos(ImVec2(67, 74));
            ImGui::BeginChild("Aim Assistance", ImVec2(405.5, 406));
            {
                ImGui::Checkbox(OBFUSCATE("Aimbot"), &arCFg.aimbot);
                ImGui::Checkbox(OBFUSCATE("VisCheck"), &arCFg.visCheck);
                ImGui::TextUnformatted(OBFUSCATE(""));
                ImGui::Combo(OBFUSCATE("Aim Position"), &aimPos, combo_items, 3);
                ImGui::Checkbox(OBFUSCATE("Smooth"), &arCFg.aimbotSmooth);
                if (arCFg.aimbotSmooth) {
                    ImGui::SliderFloat(OBFUSCATE("Smooth Amount"), &arCFg.smoothAmount, 1.0, 30.0);
                }
                ImGui::Checkbox(OBFUSCATE("Fov"), &arCFg.fovCheck);
                if (arCFg.fovCheck) {
                    ImGui::SliderFloat(OBFUSCATE("Fov Amount"), &arCFg.fovValue, 0.0, 65.0);
                    ImGui::Checkbox(OBFUSCATE("Draw Fov"), &arCFg.drawFov);
                }
                ImGui::Checkbox(OBFUSCATE("Aim on shoot"), &arCFg.onShoot);
            }
            ImGui::EndChild();

            ImGui::SetCursorPos(ImVec2(481.5, 74));
            ImGui::BeginChild("Triggerbot", ImVec2(405.5, 406));
            {
                ImGui::Checkbox(OBFUSCATE("Triggerbot"), &arCFg.triggerbot);
            }
            ImGui::EndChild();
        }

        if (sub_selected == 11) {
            ImGui::SetCursorPos(ImVec2(67, 74));
            ImGui::BeginChild("Aim Assistance", ImVec2(405.5, 406));
            {
                ImGui::Checkbox(OBFUSCATE("Aimbot"), &smgCfg.aimbot);
                ImGui::Checkbox(OBFUSCATE("VisCheck"), &smgCfg.visCheck);
                ImGui::TextUnformatted(OBFUSCATE(""));
                ImGui::Combo(OBFUSCATE("Aim Position"), &aimPos, combo_items, 3);
                ImGui::Checkbox(OBFUSCATE("Smooth"), &smgCfg.aimbotSmooth);
                if (smgCfg.aimbotSmooth) {
                    ImGui::SliderFloat(OBFUSCATE("Smooth Amount"), &smgCfg.smoothAmount, 1.0, 30.0);
                }
                ImGui::Checkbox(OBFUSCATE("Fov"), &smgCfg.fovCheck);
                if (smgCfg.fovCheck) {
                    ImGui::SliderFloat(OBFUSCATE("Fov Amount"), &smgCfg.fovValue, 0.0, 65.0);
                    ImGui::Checkbox(OBFUSCATE("Draw Fov"), &smgCfg.drawFov);
                }
                ImGui::Checkbox(OBFUSCATE("Aim on shoot"), &smgCfg.onShoot);
            }
            ImGui::EndChild();

            ImGui::SetCursorPos(ImVec2(481.5, 74));
            ImGui::BeginChild("Triggerbot", ImVec2(405.5, 406));
            {
                ImGui::Checkbox(OBFUSCATE("Triggerbot"), &smgCfg.triggerbot);
            }
            ImGui::EndChild();
        }
        if (sub_selected == 12) {
            ImGui::SetCursorPos(ImVec2(67, 74));
            ImGui::BeginChild("Aim Assistance", ImVec2(405.5, 406));
            {
                ImGui::Checkbox(OBFUSCATE("Aimbot"), &pistolCfg.aimbot);
                ImGui::Checkbox(OBFUSCATE("VisCheck"), &pistolCfg.visCheck);
                ImGui::TextUnformatted(OBFUSCATE(""));
                ImGui::Combo(OBFUSCATE("Aim Position"), &aimPos, combo_items, 3);
                ImGui::Checkbox(OBFUSCATE("Smooth"), &pistolCfg.aimbotSmooth);
                if (pistolCfg.aimbotSmooth) {
                    ImGui::SliderFloat(OBFUSCATE("Smooth Amount"), &pistolCfg.smoothAmount, 1.0,
                                       30.0);
                }
                ImGui::Checkbox(OBFUSCATE("Fov"), &pistolCfg.fovCheck);
                if (pistolCfg.fovCheck) {
                    ImGui::SliderFloat(OBFUSCATE("Fov Amount"), &pistolCfg.fovValue, 0.0, 65.0);
                    ImGui::Checkbox(OBFUSCATE("Draw Fov"), &pistolCfg.drawFov);
                }
                ImGui::Checkbox(OBFUSCATE("Aim on shoot"), &pistolCfg.onShoot);
            }
            ImGui::EndChild();

            ImGui::SetCursorPos(ImVec2(481.5, 74));
            ImGui::BeginChild("Triggerbot", ImVec2(405.5, 406));
            {
                ImGui::Checkbox(OBFUSCATE("Triggerbot"), &pistolCfg.triggerbot);
            }
            ImGui::EndChild();
        }

        if (sub_selected == 13) {
            ImGui::SetCursorPos(ImVec2(67, 74));
            ImGui::BeginChild("Aim Assistance", ImVec2(405.5, 406));
            {
                ImGui::Checkbox(OBFUSCATE("Aimbot"), &shotgunCfg.aimbot);
                ImGui::Checkbox(OBFUSCATE("VisCheck"), &shotgunCfg.visCheck);
                ImGui::TextUnformatted(OBFUSCATE(""));
                ImGui::Combo(OBFUSCATE("Aim Position"), &aimPos, combo_items, 3);
                ImGui::Checkbox(OBFUSCATE("Smooth"), &shotgunCfg.aimbotSmooth);
                if (shotgunCfg.aimbotSmooth) {
                    ImGui::SliderFloat(OBFUSCATE("Smooth Amount"), &shotgunCfg.smoothAmount, 1.0,
                                       30.0);
                }
                ImGui::Checkbox(OBFUSCATE("Fov"), &shotgunCfg.fovCheck);
                if (shotgunCfg.fovCheck) {
                    ImGui::SliderFloat(OBFUSCATE("Fov Amount"), &shotgunCfg.fovValue, 0.0, 65.0);
                    ImGui::Checkbox(OBFUSCATE("Draw Fov"), &shotgunCfg.drawFov);
                }
                ImGui::Checkbox(OBFUSCATE("Aim on shoot"), &shotgunCfg.onShoot);
            }
            ImGui::EndChild();

            ImGui::SetCursorPos(ImVec2(481.5, 74));
            ImGui::BeginChild("Triggerbot", ImVec2(405.5, 406));
            {
                ImGui::Checkbox(OBFUSCATE("Triggerbot"), &shotgunCfg.triggerbot);
            }
            ImGui::EndChild();
        }

        if (sub_selected == 14) {
            ImGui::SetCursorPos(ImVec2(67, 74));
            ImGui::BeginChild("Aim Assistance", ImVec2(405.5, 406));
            ImGui::Checkbox(OBFUSCATE("Aimbot"), &sniperCfg.aimbot);
            ImGui::Checkbox(OBFUSCATE("VisCheck"), &sniperCfg.visCheck);
            ImGui::TextUnformatted(OBFUSCATE(""));
            ImGui::Combo(OBFUSCATE("Aim Position"), &aimPos, combo_items, 3);
            ImGui::Checkbox(OBFUSCATE("Smooth"), &sniperCfg.aimbotSmooth);
            if (sniperCfg.aimbotSmooth) {
                ImGui::SliderFloat(OBFUSCATE("Smooth Amount"), &sniperCfg.smoothAmount, 1.0, 30.0);
            }
            ImGui::Checkbox(OBFUSCATE("Fov"), &sniperCfg.fovCheck);
            if (sniperCfg.fovCheck) {
                ImGui::SliderFloat(OBFUSCATE("Fov Amount"), &sniperCfg.fovValue, 0.0, 65.0);
                ImGui::Checkbox(OBFUSCATE("Draw Fov"), &sniperCfg.drawFov);
            }
            ImGui::Checkbox(OBFUSCATE("Aim on shoot"), &sniperCfg.onShoot);
            ImGui::EndChild();

            ImGui::SetCursorPos(ImVec2(481.5, 74));
            ImGui::BeginChild("Triggerbot", ImVec2(405.5, 406));
            {
                ImGui::Checkbox(OBFUSCATE("Triggerbot"), &sniperCfg.triggerbot);
            }
            ImGui::EndChild();
        }

        if (selected == 1) {
            ImGui::BeginGroup();
            ImGui::SetCursorPos(ImVec2(65, 14));
            if (custom_interface::subtab("Visuals", 1 == sub_selected)) {
                sub_selected = 1;
            }
            ImGui::SameLine();
            if (custom_interface::subtab("Configuration", 8 == sub_selected)) {
                sub_selected = 8;
            }
            ImGui::EndGroup();
        }


        if (sub_selected == 1) {
            ImGui::SetCursorPos(ImVec2(67, 74));
            ImGui::BeginChild("ESP", ImVec2(405.5, 406));
            {
                ImGui::Checkbox(OBFUSCATE("Chams"), &chams);
                ImGui::Checkbox(OBFUSCATE("Bones"), &invisibleCfg.bone);
                ImGui::Checkbox(OBFUSCATE("Boxes"), &invisibleCfg.box);
                ImGui::Checkbox(OBFUSCATE("Snaplines"), &invisibleCfg.snapline);
                ImGui::Checkbox(OBFUSCATE("Names"), &invisibleCfg.name);
             //   ImGui::Checkbox(OBFUSCATE("Weapons"), &invisibleCfg.weapon);
                ImGui::Checkbox(OBFUSCATE("Health"), &invisibleCfg.healthesp);
                if (&invisibleCfg.healthesp) {
                    ImGui::Checkbox(OBFUSCATE("Health Numbers"), &invisibleCfg.healthNumber);
                }
                //ImGui::Checkbox(OBFUSCATE(" Show Weapon"), &weaponEsp);
                ImGui::EndChild();

                ImGui::SetCursorPos(ImVec2(481.5, 74));
                ImGui::BeginChild("Other", ImVec2(405.5, 406));
                {
                   if(ImGui::Checkbox(OBFUSCATE("Radar"), &radar)){ Patches(); }
                   if(ImGui::Checkbox(OBFUSCATE("Team Radar"), &tradar)){ Patches(); }
                   ImGui::Checkbox(OBFUSCATE("View Model FOV"), &viewmodelfov);
                    if (viewmodelfov) {
                        ImGui::SliderFloat(OBFUSCATE(" · Viewmodel Value"), &viewmodelfovval, 1.0,220.0);
                    }
                    ImGui::Checkbox(OBFUSCATE("Field Of View"), &fov);
                    if (fov) {
                        ImGui::SliderFloat(OBFUSCATE(" · FOV Value"), &fovModifier, 1.0, 220.0);
                    }
                  if (ImGui::Checkbox(OBFUSCATE("Force Crosshair"), &crosshair)) { Patches(); }
                  if (ImGui::Checkbox(OBFUSCATE("No Aimpunch"), &aimpunch)) { Patches(); }
                  ImGui::Checkbox(OBFUSCATE("Nazi Crosshair"), &p100Crosshair);
                  //if (ImGui::Checkbox(OBFUSCATE("Hide Kill Notifications"), &killnotes)) { Patches(); }
                  ImGui::Checkbox(OBFUSCATE("No Camera Shake"), &shake);
                }
                ImGui::EndChild();
            }
        }

        if(sub_selected == 8){
            ImGui::SetCursorPos(ImVec2(67, 74));
            ImGui::BeginChild("ESP Configuration", ImVec2(811, 406));
            ImGui::ColorEdit4("Snapline Color", &invisibleCfg.snaplineColor.x);
            ImGui::ColorEdit4("Bone Color", &invisibleCfg.boneColor.x);
            ImGui::ColorEdit4("Box Color", &invisibleCfg.boxColor.x);
            ImGui::ColorEdit4("Name Color", &invisibleCfg.nameColor.x);
            ImGui::ColorEdit4("Weapon Color", &invisibleCfg.weaponColor.x);
            ImGui::EndChild();
        }


        if (selected == 2) {
            ImGui::BeginGroup();
            ImGui::SetCursorPos(ImVec2(65, 14));
            if (custom_interface::subtab(OBFUSCATE("Weapon"), 2 == sub_selected)) {
                sub_selected = 2;
            }
            ImGui::SameLine();
            if (custom_interface::subtab(OBFUSCATE("Player"), 3 == sub_selected)) {
                sub_selected = 3;
            }
            ImGui::EndGroup();

            if (sub_selected == 2) {
                ImGui::SetCursorPos(ImVec2(67, 74));
                ImGui::BeginChild("Legit", ImVec2(405.5, 406));
                ImGui::Checkbox(OBFUSCATE("Recoil"), &recoil);
                if(recoil){
                    ImGui::SliderFloat(OBFUSCATE(" Recoil"), &recoilval, 1.0, 15.0);
                }
                if(ImGui::Checkbox(OBFUSCATE("Spread"), &spread)){ Patches(); }
                ImGui::Checkbox(OBFUSCATE("Force Scope"), &fscope);
                ImGui::Checkbox(OBFUSCATE("Burst Shots"), &burstfire);
                if(burstfire){
                    ImGui::SliderInt(OBFUSCATE(" shots"), &burstfireval, 1.0, 30.0);
                }
                ImGui::EndChild();

                ImGui::SetCursorPos(ImVec2(481.5, 74));
                ImGui::BeginChild("Rage", ImVec2(405.5, 406));
                if(ImGui::Checkbox(OBFUSCATE("Expand Head Hitbox"), &headhitbox)){Patches();};
                if(ImGui::Checkbox(OBFUSCATE("Expand Body Hitbox"), &bodyhitbox)){Patches();};
                if(ImGui::Checkbox(OBFUSCATE("Wallbang"), &wallbang)) { Patches(); }
                ImGui::Checkbox(OBFUSCATE("Firerate"), &firerate);
                ImGui::Checkbox(OBFUSCATE("Instant Pick-up"), &pickup);
                if(ImGui::Checkbox(OBFUSCATE("Always Backstab"), &silentknife)){Patches();};
                if(ImGui::Checkbox(OBFUSCATE("Long Knife Range"), &silentknife1)){Patches();};
                ImGui::EndChild();
            }

            if (sub_selected == 3) {
                ImGui::SetCursorPos(ImVec2(67, 74));
                if(ImGui::Button(OBFUSCATE("Mone"))){addmone = true;}
                ImGui::Checkbox(OBFUSCATE("Speed"), &speed);
                if (speed) {
                    ImGui::SliderFloat(OBFUSCATE(" · Speed"), &speedval, 0.0, 30.0);
                }
                ImGui::Checkbox(OBFUSCATE("Jump Force"), &jumpheight);
                if (jumpheight) {
                    ImGui::SliderFloat(OBFUSCATE(" · Force"), &jumpval, 0.0, 7.0);
                }
                ImGui::Checkbox(OBFUSCATE("Player Height"), &fly);
                if(fly){
                    ImGui::SliderFloat(OBFUSCATE(" · Height"), &flyval, 0.0, 6.0);
                }
                ImGui::Checkbox(OBFUSCATE("No Slow-Down"), &noslow);
                ImGui::Checkbox(OBFUSCATE("Instant Crouch"), &crouch);
                ImGui::EndChild();
            }
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
    ImGui::GetStyle().ScaleAllSizes(15.0f);
    io.Fonts->AddFontFromMemoryTTF(Roboto_Regular, 30, 30.0f);
    espFont = io.Fonts->AddFontFromMemoryCompressedTTF(RetroGaming, compressedRetroGamingSize, 20);
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
    ESP();
    DrawText(ImVec2(10,10), ImVec4(255, 255, 255, 255), "PrimeTools BETA", espFont);

    ImGui::EndFrame();
    ImGui::Render();
    glViewport(0, 0, (int)glWidth, (int)glHeight);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    return old_eglSwapBuffers(dpy, surface);
}

GLint (*old_glGetUniformLocation)(GLuint program, const GLchar *name);
GLint glGetUniformLocation(GLuint program, const GLchar *name) // returns location of a shader/uniform.
{
    return old_glGetUniformLocation(program, name);
}

bool Shaders()
{
    GLint program;
    glGetIntegerv(GL_CURRENT_PROGRAM, &program);

    return old_glGetUniformLocation(program, Shader) != -1;
}

void (*old_glDrawElements)(GLenum mode, GLsizei count, GLenum type, const void *indices);
void glDrawElements(GLenum mode, GLsizei count, GLenum type, const void *indices)
{
    old_glDrawElements(mode, count, type, indices);

    if(Shaders() > 0)
    {
        if (chams) {
            LOGE("COEMS");
            glDepthRangef(1, 0.5);

            glEnable(GL_BLEND);
            glBlendFunc(GL_CONSTANT_COLOR, GL_CONSTANT_COLOR);
            glBlendEquation(GL_FUNC_ADD);
            glBlendColor(255, 255, 255, 1);
            glDepthFunc(GL_ALWAYS);
            old_glDrawElements(GL_TRIANGLES, count, type, indices);
            glColorMask(124, 252, 0, 255);
            glBlendFunc(GL_DST_COLOR, GL_ONE);
            glDepthFunc(GL_LESS);
            glBlendColor(0.0, 0.0, 0.0, 0.0);
        }
        old_glDrawElements(mode, count, type, indices);

        glDepthRangef(0.5, 1);
        glColorMask(1, 1, 1, 1);
        glDisable(GL_BLEND);
    }

}


void *hack_thread(void *arg) {
    int tries = 0;
    do {
        sleep(1);
        auto maps =  KittyMemory::getMapsByName("split_config.arm64_v8a.apk");
        for(std::vector<ProcMap>::iterator it = maps.begin(); it != maps.end(); ++it) {
            auto address = KittyScanner::findHexFirst(it->startAddress, it->endAddress,"7F 45 4C 46 02 01 01 00 00 00 00 00 00 00 00 00 03 00 B7 00 01 00 00 00 00 F2 79 00 00 00 00 00 40 00 00 00 00 00 00 00 88 D8 E1 02 00 00 00 00", "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");
            auto libcf = KittyScanner::findHexFirst(it->startAddress, it->endAddress,"7F 45 4C 46 02 01 01 00 00 00 00 00 00 00 00 00 03 00 B7 00 01 00 00 00 30 11 00 00 00 00 00 00 40 00 00 00 00 00 00 00 48 41 00 00 00 00 00 00", "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");
            auto libgl = KittyScanner::findHexFirst(it->startAddress, it->endAddress,"7F 45 4C 46 02 01 01 00 00 00 00 00 00 00 00 00 03 00 B7 00 01 00 00 00 00 F2 79 00 00 00 00 00 40 00 00 00 00 00 00 00 88 D8 E1 02 00 00 00 00", "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");
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
    auto renderHandle = dlopen("libGLES.so", RTLD_LAZY);
    LOGE("RenderHandle %p", renderHandle);
    auto glDrawElement = dlsym(renderHandle, "glDrawElements");
    auto glGetUniformLocations = dlsym(renderHandle, "glGetUniformLocation");
    LOGE("glDrawElement %p", glDrawElement);
    LOGE("glGetUniformLocation %p", glGetUniformLocation);
    DobbyHook((void*)eglSwapBuffers,(void*)hook_eglSwapBuffers, (void**)&old_eglSwapBuffers);
    DobbyHook((void*)glDrawElement,(void*)glDrawElements, (void**)&old_glDrawElements);
    DobbyHook((void*)glGetUniformLocations,(void*)glGetUniformLocation, (void**)&old_glGetUniformLocation);
    void *sym_input = DobbySymbolResolver(("/system/lib/libinput.so"), ("_ZN7android13InputConsumer21initializeMotionEventEPNS_11MotionEventEPKNS_12InputMessageE"));

    if (NULL != sym_input) {
        DobbyHook(sym_input,(void*)myInput,(void**)&origInput);
    }

    while (true)
    {
        if(p100Crosshair)
        {
            if (crosshairRotation > 89)
            {
                crosshairRotation = 0;
            }
            if (crosshairRotation1 > 134)
            {
                crosshairRotation1 = 45;
            }
            crosshairRotation++;
            crosshairRotation1++;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    return nullptr;
}

void* triggerbot_thread(void* arg)
{
    while(true)
    {
        if(TouchControls && shootControl && localCharacter && get_Health(localCharacter))
        {
            onInputButtons(TouchControls, 7, 1);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            onInputButtons(TouchControls, 7, 0);
            shootControl = 0;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return nullptr;
}

