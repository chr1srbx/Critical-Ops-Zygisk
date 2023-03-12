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



#define GamePackageName "com.criticalforceentertainment.criticalops"

monoString* CreateIl2cppString(const char* str)
{
    static monoString* (*CreateIl2cppString)(const char* str, int *startIndex, int *length) =
    (monoString* (*)(const char* str, int *startIndex, int *length))(get_absolute_address(string2Offset(OBFUSCATE("0x44865B4"))));
    int* startIndex = 0;
    int* length = (int *)strlen(str);
    return CreateIl2cppString(str, startIndex, length);
}

void* pSys = nullptr;
void* pBones = nullptr;
bool unsafe,recoil, radar, flash, smoke, scope, setupimg, spread, aimpunch, speed, reload, esp, snaplines, kickback, crouch, wallbang,
fov, ggod, killnotes,crosshair, supressor, rifleb, bonesp, viewmodel, viewmodelfov, boxesp, healthesp, healthNumber, espName, weaponEsp, armroFlag, spawnbullets,
canmove,isPurchasingSkins, fly, removecharacter, tutorial, freeshop, gravity;

float speedval = 1, fovModifier, viemodelposx, viemodelposy, viemodelposz, viewmodelfovval, gravityval, flyval;
int screenscale = 0;

extern int glHeight;
extern int glWidth;
ImFont* espFont;
ImFont* flagFont;

uintptr_t find_pattern_in_module(const char* lib_name, const char* pattern) {
    lib_info lib_info = find_library(lib_name);
    return find_pattern((uint8_t*)lib_info.start_address, lib_info.size, pattern);
}

uintptr_t find_pattern_with_lib(const char* pattern) {
    return find_pattern((uint8_t*)libBaseAddress, libSize, pattern);
}

float get_fieldOfView(void *instance) {
    if (instance != nullptr && fov) {
        return fovModifier;
    }
    return old_get_fieldOfView(instance);
}

// Custom functions
void* getTransform(void* character)
{
    return *(void**)((uint64_t)character + 0x70);
}

int get_CharacterTeam(void* character)
{
    void* player = get_Player(character);
    void* boxedValueName = *(void**)((uint64_t)player + 0x150);
    return *(int*)((uint64_t)boxedValueName + 0x1C);
}

int get_PlayerTeam(void* player)
{
    void* boxedValueName = *(void**)((uint64_t)player + 0x150);
    return *(int*)((uint64_t)boxedValueName + 0x1C);
}

std::string get_CharacterName(void* character)
{
    void* player = get_Player(character);
    void* boxedValueName = *(void**)((uint64_t)player + 0x118);
    monoString *username = *(monoString **)((uint64_t)boxedValueName + 0x1C);
    return username->getString();
}

std::string get_characterWeaponName(void* character)
{
    void* characterData = *(void**)((uint64_t)character + 0x98);
    void* m_wpn = *(void**)((uint64_t)characterData + 0x80);
    monoString *weaponName = *(monoString **)((uint64_t)m_wpn + 0x10);
    return weaponName->getString();
}

const char* get_characterArmors(void* character)
{
    void* CharacterArmors = *(void**)((uint64_t)character + 0xE8);
    monoArray<void **> *ArmorList = *(monoArray<void **>**)((uint64_t)CharacterArmors + 0xE8);
    bool hasHelmet, hasKevlar;
    for (int i = 0; i < ArmorList->getLength(); i++)
    {
        void* currentArmor = (monoList<void **> *) ArmorList->getPointer()[i];
        void* ArmorDef = *(void**)((uint64_t)currentArmor + 0x10);
        void* ArmorDefData = *(void**)((uint64_t)ArmorDef + 0x30);
        int armorType = *(int*)((uint64_t)ArmorDefData + 0x30);
        if (armorType == 0)
        {
            hasHelmet = true;
        }
        else
        {
            hasKevlar = true;
        }
    }


    if (hasKevlar && hasHelmet) return "HK";
    if (hasKevlar && !hasHelmet) return "K";
    if (!hasKevlar && hasHelmet) return "H";
    return "NONE";
}

Vector3 getBonePosition(void* character, int bone){
    void* curBone = get_CharacterBodyPart(character, bone);
    void* hitSphere = *(void**)((uint64_t)curBone + 0x20);
    void* transform = *(void**)((uint64_t)hitSphere + 0x30);
    Vector3 bonePos = get_Position(transform);
    return bonePos;
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

void(*oldUpdateAimedCharacter)(void* obj);
void UpdateAimedCharacter(void* obj){
    if(obj != nullptr) {
        LOGE("PLAYER MOVEMENT");
        void *CharacterData = *(void **) ((uint64_t) obj + 0x98);
        if (CharacterData != nullptr) {
            void *StateData = *(void **) ((uint64_t) obj + 0x98);
            if (StateData != nullptr) {
                if (fly) {
                    *(bool *) ((uint64_t) obj + 0x0) = true;
                }
            }
        }
    }
    oldUpdateAimedCharacter(obj);
}
void RenderOverlayFlashbang(void* obj){
    if(obj != nullptr && flash){
        *(float*)((uint64_t) obj + 0x38) = 0;//m_flashTime
    }
    oldRenderOverlayFlashbang(obj);
}

void set_Spread(void* obj){
    if(obj != nullptr){
        *(float*)((uint64_t) obj + 0x24) = 0;//m_maxSpread
        *(float*)((uint64_t) obj + 0x20) = 0;//m_spreadFactor
    }
    oldset_Spread(obj);
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

float(*oldGetCurrentMaxSpeed)(void* obj, float speed);
float GetCurrentMaxSpeed(void* obj, float speed){
    if(obj != nullptr){
        if(speed){
            return 20;
        }
    }
    oldGetCurrentMaxSpeed(obj, speed);
}

int (*oldGetPlayerMoney)(void* obj);
int GetPlayerMoney(void* obj){
    if(obj != nullptr){
        if(freeshop){
            return 10000;
        }
    }
    oldGetPlayerMoney(obj);
}

Vector3 (*oldget_gravity)();
Vector3 get_gravity(){
    if(gravity){
        return gravityval;
    }
}

Vector3 (*oldget_height)();
Vector3 get_height(){
    if(fly){
        return flyval;
    }
}

void(*oldInit)(void* obj);
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

HOOKAF(void, Input, void *thiz, void *ex_ab, void *ex_ac) {
    origInput(thiz, ex_ab, ex_ac);
    ImGui_ImplAndroid_HandleInputEvent((AInputEvent *)thiz);
    return;
}

// Initilizers with patterns <3
void Hooks()
{
    HOOK("0x1B872E8", BackendManager, oldBackendManager);
    HOOK("0x1BACFCC", set_Spread, oldset_Spread); // Overlay Scope set spread
    HOOK("0x1BACD84", RenderOverlayFlashbang, oldRenderOverlayFlashbang); // flash render overlay
    HOOK("0x1BB31C8", RenderOverlaySmoke, oldRenderOverlaySmoke); // smoke render overlay
    HOOK("0x106431C", GameSystemUpdate, oldGameSystemUpdate); // GameSystem Update
    HOOK("0x1BC0334", UpdateAimedCharacter, oldUpdateAimedCharacter); // character
   // HOOK("0x1D6EA74", GetCurrentMaxSpeed, oldGetCurrentMaxSpeed); // speed
    HOOK("0x1F99474", GetPlayerMoney, oldGetPlayerMoney); // freshop
    //HOOK("0x1AF318C", get_gravity, oldget_gravity); // gravity
   // HOOK("0x1AF71A4", get_height, oldget_height); // height
    //HOOK("0x1A76E68", Init, oldInit); // for scale
    //HOOK("0x1B9D608", DrawRenderer, oldDrawRenderer); need args
}

void Pointers()
{
    RequestPurchaseSkin = (void(*)(void*, int, int, bool)) get_absolute_address(string2Offset(OBFUSCATE("0x1B80760")));
    SetResolution = (void(*)(int, int, bool)) get_absolute_address(string2Offset(OBFUSCATE("0x1A268A4"))); // SetResolution
    get_Width = (int(*)()) get_absolute_address(string2Offset(OBFUSCATE("0x1A265AC"))); // screen get_Width
    get_Height = (int(*)()) get_absolute_address(string2Offset(OBFUSCATE("0x1A265D4"))); // screen get_Height
    getAllCharacters = (monoList<void**>*(*)(void*)) get_absolute_address(string2Offset(OBFUSCATE("0x105F4FC"))); // get_AllCharacters
    getLocalId= (int(*)(void*)) get_absolute_address(string2Offset(OBFUSCATE("0x10537AC"))); // get_LocalId
    getPlayer = (void*(*)(void*,int)) get_absolute_address(string2Offset(OBFUSCATE("0x10627C4"))); // GameSystem GetPlayer
    getLocalPlayer = (void*(*)(void*)) get_absolute_address(string2Offset(OBFUSCATE("0x105321C"))); // GameSystem get_LocalPlayer
    getCharacterCount = (int(*)(void*)) get_absolute_address(string2Offset(OBFUSCATE("0x105F50C"))); // GameSystem get_CharacterCount
    get_Health = (int(*)(void*)) get_absolute_address(string2Offset(OBFUSCATE("0x19D8E10"))); // get_Health
    get_Player = (void*(*)(void*)) get_absolute_address(string2Offset(OBFUSCATE("0x19D8DF8"))); // Gameplay Character get_Player
    get_IsInitialized = (bool(*)(void*)) get_absolute_address(string2Offset(OBFUSCATE("0x19D8C80"))); // Gameplay Character get_IsInitialized
    get_Position = (Vector3(*)(void*)) get_absolute_address(string2Offset(OBFUSCATE("0x1A56A4C"))); // Transform get_position
    get_camera = (void*(*)()) get_absolute_address(string2Offset(OBFUSCATE("0x1A2E278"))); // get camera main
    WorldToScreen = (Vector3(*)(void*, Vector3, int)) get_absolute_address(string2Offset(OBFUSCATE("0x1A2D67C"))); // WorldToScreenPoint
    //set_targetFrameRate = (void(*)(int)) get_absolute_address(""); NOT USED
    get_CharacterBodyPart =(void*(*)(void*, int)) get_absolute_address(string2Offset(OBFUSCATE("0x19D8E6C"))); // Character GetBodyPart
    RaycastCharacter = (void(*)(void*,void*,Ray, int)) get_absolute_address(string2Offset(OBFUSCATE("0x1068A2C"))); // RaycastCharacters
    RemoveCharacter = (void(*)(void*, int)) get_absolute_address(string2Offset(OBFUSCATE("0x106194C")));//RemovePlayer
}

void Patches(){
    PATCH_SWITCH("0x1D6EF8C", "1F2003D5C0035FD6", spread);//UpdateSpread
    PATCH_SWITCH("0x1D6F028", "1F2003D5C0035FD6", recoil);//ApplyRecoil
    PATCH_SWITCH("0x1D6EF60", "000080D2C0035FD6", recoil);//Evaluate
    PATCH_SWITCH("0x10BEED4", "000080D2C0035FD6", recoil);//UpdateCameraShake
    PATCH_SWITCH("0x1D4FCC4", "000080D2C0035FD6", aimpunch);//AimPunchRecover
    PATCH_SWITCH("0x1BB86FC", "200080D2C0035FD6", canmove);//CanMove
    PATCH_SWITCH("0x1BB86FC", "200080D2C0035FD6", canmove);//CanShoot
    PATCH_SWITCH("0x10D2C58", "1F2003D5C0035FD6", wallbang);//UpdateWallHit
    PATCH_SWITCH("0x10D2228", "1F2003D5C0035FD6", wallbang);//ProcessWallhit
  //  PATCH_SWITCH("0x10695C0", "1F2003D5C0035FD6", wallbang);//CheckWallHits
    PATCH_SWITCH("0x106724C", "000080D2C0035FD6", ggod);//GrenadeHitCharacter
    PATCH_SWITCH("0x1067820", "000080D2C0035FD6", ggod);//OnGrenadeExploded
    PATCH_SWITCH("0xF3D9F4", "1F2003D5C0035FD6", killnotes);//SetKillNotification
    PATCH_SWITCH("0x1A7C36C", "200080D2C0035FD6", crosshair);//get_Crosshair weapondef
    PATCH_SWITCH("0x16CDFBC", "000080D2C0035FD6", smoke);//get_GrenadeSmokeAreaOfEffect
    PATCH_SWITCH("0x1D8507C", "200080D2C0035FD6", supressor);//isSupressor
    PATCH_SWITCH("0x1F96504", "200080D2C0035FD6", tutorial);//get_Tutorial
    PATCH("0x10DC594", "000080D2C0035FD6");//bad word
    PATCH("0x10DC680", "000080D2C0035FD6");//bad word
    PATCH("0x19DEC30", "200080D2C0035FD6");//character visi
    PATCH("0x1BB5B54", "000080D2C0035FD6");//deploying
    PATCH("0x1D6BBF8", "1F2003D5C0035FD6");//ProcessMovement
    PATCH("0x208BB6C", "1F2003D5C0035FD6");//ApplyRestorePriceToArmor
    PATCH("0x208BB28", "1F2003D5C0035FD6");//ApplyNormalPriceToArmor
}

void DrawMenu(){
    if(pSys != nullptr){
            int id = getLocalId(pSys);
            void *localPlayer = getPlayer(pSys, id);
            int localTeam = get_PlayerTeam(localPlayer);
            monoList<void **> *characterList = getAllCharacters(pSys);
            for (int i = 0; i < characterList->getSize(); i++)
            {
                void *currentCharacter = (monoList<void **> *) characterList->getItems()[i];
                //if(currentCharacter == localPlayer){
                //    void* myPlayer = currentCharacter;
              //  }
                int curTeam = get_CharacterTeam(currentCharacter);
                int health = get_Health(currentCharacter);
                if (health > 0 && get_IsInitialized(currentCharacter) && localTeam != curTeam && curTeam != -1)
                {
                    void *transform = getTransform(currentCharacter);
                    Vector3 position = get_Position(transform);
                    Vector3 transformPos = WorldToScreen(get_camera(), position, 2);
                    transformPos.Y = glHeight - transformPos.Y;
                    Vector3 headPos = getBonePosition(currentCharacter, HEAD);
                    Vector3 chestPos = getBonePosition(currentCharacter, CHEST);
                    Vector3 wschestPos = WorldToScreen(get_camera(), chestPos, 2);
                    Vector3 wsheadPos = WorldToScreen(get_camera(), headPos, 2);
                    Vector3 wsheadPos2 = wsheadPos;
                    Vector3 aboveHead = headPos + Vector3(0,0.2,0); // estimate
                    Vector3 headEstimate = position + Vector3(0,1.48,0);// estimate
                    Vector3 wsAboveHead = WorldToScreen(get_camera(), aboveHead, 2);
                    Vector3 wsheadEstimate = WorldToScreen(get_camera(), headEstimate, 2);
                    wsAboveHead.Y = glHeight - wsAboveHead.Y;
                    wsheadEstimate.Y = glHeight - wsheadEstimate.Y;
                    float height = transformPos.Y - wsAboveHead.Y;
                    float width = (transformPos.Y - wsheadEstimate.Y)/2;
                    int hitindex;
                    //Vector3 currentheadPos = getBonePosition(, 10);
                   // Vector3 enemyheadPos = getBonePosition(currentCharacter, 10);
                    //Ray ray;
                    //ray.origin = currentheadPos;
                    //ray.direction = currentheadPos - enemyheadPos;
                   // RaycastCharacter(pSys, currentCharacter, ray, hitindex);
                    //LOGE("hitindex %d", hitindex);
                    if (snaplines && transformPos.Z > 0)
                    {
                        DrawLine(ImVec2(glWidth / 2, glHeight), ImVec2(transformPos.X, transformPos.Y), ImColor(172, 204, 255), 3);
                    }
                    if (bonesp && transformPos.Z > 0)
                    {
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
                        Vector3 neck = (chestPos + headPos)/2;
                        Vector3 wsneck = WorldToScreen(get_camera(), neck, 2);
                        wsneck.Y = glHeight - wsneck.Y;
                        wschestPos.Y = glHeight - wschestPos.Y;
                        wsheadPos.Y = glHeight - wsheadPos.Y;
                        if(wschestPos.Z > 0 && wsneck.Z)
                        {
                            DrawLine(ImVec2(wschestPos.X, wschestPos.Y),ImVec2(wsneck.X,wsneck.Y),ImColor(172, 204, 255), 3);
                        }
                        if(wsheadPos.Z > 0 && wschestPos.Z > 0 ){
                            float radius = sqrt(diff.X * diff.X + diff.Y * diff.Y);
                            auto background = ImGui::GetBackgroundDrawList();

                            background->AddCircle(ImVec2(wsheadPos.X, wsheadPos.Y), radius/2, IM_COL32(172, 204, 255, 255), 0, 3.0f);
                        }
                    }
                    if (boxesp && transformPos.Z > 0 && wsAboveHead.Z > 0)
                    {
                        DrawOutlinedBox2(wsAboveHead.X - width/2, wsAboveHead.Y, width, height, ImVec4(255,255,255,255), 3);
                    }
                    if (healthesp && transformPos.Z > 0 && wsAboveHead.Z > 0)
                    {
                        DrawOutlinedFilledRect(wsAboveHead.X - width/2 - 12, wsAboveHead.Y + height*(1 - (static_cast<float>(health)/100.0f)), 3, height*(static_cast<float>(health)/100.0f), HealthToColor(health));
                    }
                    if (healthNumber && transformPos.Z > 0 && wsAboveHead.Z > 0)
                    {
                        if (health < 100)
                        {
                            DrawText(ImVec2(wsAboveHead.X - width/2 - 15, wsAboveHead.Y + height*(1 - static_cast<float>(health)/100.0f)), ImVec4(255,255,255,255), std::to_string(health), espFont);
                        }
                    }
                    if (espName && transformPos.Z > 0 && wsAboveHead.Z > 0)
                    {
                        std::string name = get_CharacterName(currentCharacter);
                        DrawText(ImVec2(wsheadPos.X, wsAboveHead.Y - 7), ImVec4(255,255,255,255), name.c_str(), espFont);
                    }
                    if (weaponEsp && transformPos.Z > 0 && wsAboveHead.Z > 0)
                    {
                        std::string weapon = get_characterWeaponName(currentCharacter);
                        DrawText(ImVec2(wsheadPos.X, transformPos.Y + 7), ImVec4(255,255,255,255), weapon.c_str(), espFont);
                    }
                    if (armroFlag && transformPos.Z > 0 && wsAboveHead.Z > 0)
                    {
                        const char* armor = get_characterArmors(currentCharacter);
                        // sorcery calculations to make the text left allign
                        int length = strlen(armor);

                        // flag font is 8px
                        // we want to have 3 px between the furthest border of the box

                        DrawText(ImVec2(wsheadPos.X + width + 4 + 4*length, wsheadPos.Y + 4), ImVec4(255,255,255,255), armor, flagFont);
                    }
                }

        }
    }
    {
        if(unsafe){ImGui::Begin(OBFUSCATE("(UNSAFE HOOK) zyCheats Rage - 1.37.1f2091 - chr1s#4191 & 077 Icemods && faggosito"));}
        else{ImGui::Begin(OBFUSCATE("zyCheats Rage - 1.37.1f2091 - chr1s#4191 & 077 Icemods && faggosito"));}
        ImGui::TextUnformatted("If the menu is broken, set the screenscale in settings to 100.");
        if (ImGui::Button(OBFUSCATE("Join Discord")))
        {
            //isDiscordPressed = true;
        }
        ImGui::TextUnformatted("Its Recommended to join the discord server for mod updates etc.");
        ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_FittingPolicyResizeDown;
        if (ImGui::BeginTabBar("Menu", tab_bar_flags)) {
            if (ImGui::BeginTabItem(OBFUSCATE("Legit"))) {
                if (ImGui::CollapsingHeader(OBFUSCATE("Weapon Mods"))) {
                    if(ImGui::Checkbox(OBFUSCATE("Less Recoil"), &recoil)){ Patches(); }
                    if(ImGui::Checkbox(OBFUSCATE("No Spread"), &spread)){ Patches(); }
                    if(ImGui::Checkbox(OBFUSCATE("Force Supressor"), &supressor)){ Patches(); }
                }
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem(OBFUSCATE("Rage"))) {
                if (ImGui::CollapsingHeader(OBFUSCATE("Player Mods"))) {
                    if(ImGui::Checkbox(OBFUSCATE("No Grenade Damage"), &ggod)){ Patches(); }
                    //if( ImGui::Checkbox(OBFUSCATE("Spoof Crouch"), &crouch)){ Patches(); }
                    ImGui::Checkbox(OBFUSCATE("Speed"), &speed);
                    ImGui::Checkbox(OBFUSCATE("Gravity"), &gravity);
                    if(gravity){
                        ImGui::SliderFloat(OBFUSCATE(" · Value"), &gravityval, 0.0, 100.0);
                    }
                    ImGui::Checkbox(OBFUSCATE("Fly"), &fly);
                    if(flyval){
                        ImGui::SliderFloat(OBFUSCATE(" · Height"), &flyval, 0.0, 10.0);
                    }
                }
                if (ImGui::CollapsingHeader(OBFUSCATE("Weapon Mods"))) {
                    if(ImGui::Checkbox(OBFUSCATE("Wallbang"), &wallbang)){ Patches(); }
                }
                if (ImGui::CollapsingHeader(OBFUSCATE("Game Mods"))) {
                    if(ImGui::Checkbox(OBFUSCATE("Spawn Bullets In Enemy"), &spawnbullets));
                    if(ImGui::Checkbox(OBFUSCATE("Move/Shoot before timer"), &canmove)){ Patches(); }
                    if(ImGui::Button(OBFUSCATE("Godmode"))){
                        removecharacter = true;
                    }
                }

                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem(OBFUSCATE("Visual"))) {
                if (ImGui::CollapsingHeader(OBFUSCATE("ESP"))) {
                    ImGui::Checkbox(OBFUSCATE("Snaplines"), &snaplines);
                    ImGui::Checkbox(OBFUSCATE("Bones"), &bonesp);
                    ImGui::Checkbox(OBFUSCATE("Boxes"), &boxesp);
                    ImGui::Checkbox(OBFUSCATE("Show Names"), &espName);
                    ImGui::Checkbox(OBFUSCATE("Show Health"), &healthesp);
                    if(healthesp){
                        ImGui::Checkbox(OBFUSCATE("Health Numbers"), &healthNumber);
                    }
                    ImGui::Checkbox(OBFUSCATE(" Show Armor"), &armroFlag);
                    ImGui::Checkbox(OBFUSCATE(" Show Weapon"), &weaponEsp);
                }
                if (ImGui::CollapsingHeader(OBFUSCATE("ViewModel"))) {
                    ImGui::Checkbox(OBFUSCATE("View Model FOV"), &viewmodelfov);
                    if(viewmodelfov){
                        ImGui::SliderFloat(OBFUSCATE(" · Value"), &viewmodelfovval, 1.0, 360.0);
                    }
                }
                ImGui::Checkbox(OBFUSCATE("Field Of View"), &fov);
                if(fov){
                    ImGui::SliderFloat(OBFUSCATE(" · Value"), &fovModifier, 1.0, 360.0);
                }
                if(ImGui::Checkbox(OBFUSCATE("Radar"), &radar)){ Patches(); }
                ImGui::Checkbox(OBFUSCATE("No Flashbang"), &flash);
                ImGui::Checkbox(OBFUSCATE("No Smoke"), &smoke);
                ImGui::Checkbox(OBFUSCATE("No Scope"), &scope);
                if(ImGui::Checkbox(OBFUSCATE("Force Crosshair"), &scope)){ Patches(); }
                if(ImGui::Checkbox(OBFUSCATE("No Aimpunch"), &aimpunch)){ Patches(); }
                ImGui::Checkbox(OBFUSCATE("Hide Kill Notifications"), &killnotes);{Patches();}
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem(OBFUSCATE("Account"))) {
                if(ImGui::Button(OBFUSCATE("Purchase all skins"))){
                    isPurchasingSkins = true;
                }
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem(OBFUSCATE("Misc"))) {
                if(ImGui::Checkbox(OBFUSCATE("Tutorial Shop"), &tutorial)){Patches();}
                if(ImGui::Checkbox(OBFUSCATE("Free shop"), &freeshop)){ Patches(); }
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
            auto address = KittyScanner::findHexFirst(it->startAddress, it->endAddress,"7F 45 4C 46 02 01 01 00 00 00 00 00 00 00 00 00 03 00 B7 00 01 00 00 00 D0 70 7A 00 00 00 00 00 40 00 00 00 00 00 00 00 B0 DB E2 02 00 00 00 00", "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");
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
