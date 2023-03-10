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
bool recoil, radar, flash, smoke, scope, setupimg, spread, aimpunch, speed, reload, esp, snaplines, kickback, crouch, wallbang,
fov, ggod, killnotes,crosshair, supressor, rifleb, bonesp, viewmodel, viewmodelfov, boxesp, healthesp, healthNumber, espName, weaponEsp, armroFlag, spawnbullets;

float speedval = 1, fovModifier, viemodelposx, viemodelposy, viemodelposz, viewmodelfovval;

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

// Custom functions
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

std::u16string get_CharacterName(void* character)
{
    void* player = get_Player(character);
    void* boxedValueName = *(void**)((uint64_t)player + 0x118);
    return *(std::u16string*)((uint64_t)boxedValueName + 0x1C);
}

std::u16string get_characterWeaponName(void* character)
{
    void* characterData = *(void**)((uint64_t)character + 0x98);
    void* m_wpn = *(void**)((uint64_t)characterData + 0x80);
    return *(std::u16string*)((uint64_t)m_wpn + 0x10);
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
            LOGE("GAMEPLAYMODULE");
            void* CameraSystem = *(void**)((uint64_t) GamePlayModule + 0x30);
            if(CameraSystem != nullptr){
                if(fov){
                    *(float*)((uint64_t) CameraSystem + 0x8C) = fovModifier;//m_horizontalFieldOfView
                }

                if(viewmodelfov){
                    *(float*)((uint64_t) CameraSystem + 0x90) = viewmodelfovval;//m_viewModelFieldOfView
                }
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
                            RaycastCharacters(pSys, localCharacter, ray);
                        }
                    }
                }
            }
        }
    }
    return oldGameSystemUpdate(obj);
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

HOOKAF(void, Input, void *thiz, void *ex_ab, void *ex_ac) {
    origInput(thiz, ex_ab, ex_ac);
    ImGui_ImplAndroid_HandleInputEvent((AInputEvent *)thiz);
    return;
}

// Initilizers with patterns <3
void Hooks()
{
    Hook("E9 23 BD 6D F5 53 01 A9 F3 7B 02 A9 74 94 00 B0", (void*)set_Spread, (void**)&oldset_Spread, "xxxxxxxxxxxxxxxx");
    Hook("E8 0F 1C FC FC 07 00 F9", (void*)RenderOverlayFlashbang, (void**)&oldRenderOverlayFlashbang, "xxxxxxxx");
    Hook("F6 0F 1D F8 F5 53 01 A9 F3 7B 02 A9 34 94 00 D0 D5 81 00 F0 88 DE", (void*)RenderOverlaySmoke, (void**)&oldRenderOverlaySmoke, "xxxxxxxxxxxxxxxxxxxxxx");
    Hook("FF C3 03 D1 E8 43 00 FD FC 6F 09 A9 FA 67 0A A9 F8 5F 0B A9 F6 57 0C A9 F4 4F 0D A9 FD 7B 0E A9 74 EE 00 B0", (void*)GameSystemUpdate, (void**)&oldGameSystemUpdate, "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");
    //HOOK("0x1B9D608", DrawRenderer, oldDrawRenderer); need args
}

void Pointers()
{
    SetResolution = (void(*)(int, int, bool, int)) KittyScanner::findHexFirst(libBaseAddress, libBaseEndAddress, "F7 5B BD A9 F5 53 01 A9 F3 7B 02 A9 97 A0 00 B0 E4 B6 43 F9 5F 00 00 72", "xxxxxxxxxxxxxxxxxxxxxxxx");
    get_Width = (int(*)()) KittyScanner::findHexFirst(libBaseAddress, libBaseEndAddress, "F3 7B BF A9 93 A0 00 B0 60 9E", "xxxxxxxxxx");
    get_Height = (int(*)()) KittyScanner::findHexFirst(libBaseAddress, libBaseEndAddress, "F3 7B BF A9 93 A0 00 B0 60 A2", "xxxxxxxxxx");
    getAllCharacters = (monoList<void**>*(*)(void*)) KittyScanner::findHexFirst(libBaseAddress, libBaseEndAddress, "", "xxxxxxxxxx");
    getLocalId= (int(*)(void*)) KittyScanner::findHexFirst(libBaseAddress, libBaseEndAddress, "F4 0F 1E F8 F3 7B 01 A9 F4 EE 00 D0 88 86 46 39", "xxxxxxxxxxxxxxxx");
    getPlayer = (void*(*)(void*,int))  KittyScanner::findHexFirst(libBaseAddress, libBaseEndAddress, "FF C3 00 D1 F5 53 01 A9 F3 7B 02 A9 75 EE 00 F0", "xxxxxxxxxxxxxxxx");
    getLocalPlayer = (void*(*)(void*)) KittyScanner::findHexFirst(libBaseAddress, libBaseEndAddress, "F4 0F 1E F8 F3 7B 01 A9 F3 EE 00 D0", "xxxxxxxxxxxx");
    getCharacterCount = (int(*)(void*)) KittyScanner::findHexFirst(libBaseAddress, libBaseEndAddress, "F4 0F 1E F8 F3 7B 01 A9 94 EE 00 D0 88 7E 46 39", "xxxxxxxxxxxxxxxx");
    get_Health = (int(*)(void*)) KittyScanner::findHexFirst(libBaseAddress, libBaseEndAddress, "00 C0 40 B9 C0 03 5F D6 01 C0 00 B9 C0 03 5F D6", "xxxxxxxxxxxxxxxx");
    get_Player = (void*(*)(void*)) KittyScanner::findHexFirst(libBaseAddress, libBaseEndAddress, "00 48 40 F9 C0 03 5F D6 00 48 41 BD C0 03 5F D6", "xxxxxxxxxxxxxxxx");
    get_IsInitialized = (bool(*)(void*)) KittyScanner::findHexFirst(libBaseAddress, libBaseEndAddress, "00 60 43 39 C0 03 5F D6  08 C0 40 B9 1F 05 00 71", "xxxxxxxxxxxxxxxx");
    get_Position = (Vector3(*)(void*)) KittyScanner::findHexFirst(libBaseAddress, libBaseEndAddress, "FF C3 00 D1 F4 0B 00 F9 F3 7B 02 A9 14 9F 00 D0 88 2E 44 F9", "xxxxxxxxxxxxxxxxxxxx");
    get_camera = (void*(*)()) KittyScanner::findHexFirst(libBaseAddress, libBaseEndAddress, "F3 7B BF A9 53 A0 00 B0 60 F6 46 F9", "xxxxxxxxxxxx");
    WorldToScreen = (Vector3(*)(void*, Vector3, int)) KittyScanner::findHexFirst(libBaseAddress, libBaseEndAddress, "FF 03 01 D1 F5 53 02 A9 F3 7B 03 A9 55 A0 00 D0 A8 F6 47 F9", "xxxxxxxxxxxxxxxxxxxx");
    set_targetFrameRate = (void(*)(int))  KittyScanner::findHexFirst(libBaseAddress, libBaseEndAddress, "F4 0F 1E F8 F3 7B 01 A9 14 9F 00 F0 81 D6 43 F9", "xxxxxxxxxxxxxxxx");
    get_CharacterBodyPart =(void*(*)(void*, int)) KittyScanner::findHexFirst(libBaseAddress, libBaseEndAddress, "FE 0F 1F F8 08 3C 40 F9 08 01 00 B4 09 19 40 B9 3F 01 01 6B C9 00 00 54 08 CD 21 8B 00 11 40 F9", "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");
    RaycastCharacters = (void(*)(void*,void*,Ray)) KittyScanner::findHexFirst(libBaseAddress, libBaseEndAddress, "FF 43 06 D1 ED 33 10 6D", "xxxxxxxx");
}

void Patches(){
    PATCH_SWITCH("0x1D4FD28", "000080D2C0035FD6", spread);//UpdateSpread
    PATCH_SWITCH("0x1D50130", "000080D2C0035FD6", recoil);//ApplyRecoil
    PATCH_SWITCH("0x1D4FC38", "000080D2C0035FD6", recoil);//RecoilRecover
    PATCH_SWITCH("0x10BEED4", "000080D2C0035FD6", recoil);//UpdateCameraShake
    PATCH_SWITCH("0x1D4FCC4", "000080D2C0035FD6", aimpunch);//AimPunchRecover
    PATCH_SWITCH("0x19BAB70", "1F2003D5C0035FD6", crouch);//UpdateCrouch
    //PATCH_SWITCH("0x10D2730", "1F2003D5C0035FD6", wallbang);//ProcessHitBuffers
    PATCH_SWITCH("0x10D2C58", "1F2003D5C0035FD6", wallbang);//UpdateWallHit
    PATCH_SWITCH("0x10D2228", "1F2003D5C0035FD6", wallbang);//ProcessWallhit
    PATCH_SWITCH("0x10D1664", "000080D2C0035FD6", ggod);//GrenadeHitCharacter
    PATCH_SWITCH("0xF3C32C", "1F2003D5C0035FD6", killnotes);//SetKillNotification
    PATCH_SWITCH("0x1D8507C", "200080D2C0035FD6", crosshair);//get_Crosshair
    PATCH_SWITCH("0x1D8502C", "000080D2C0035FD6", smoke);//get_GrenadeSmokeAreaOfEffect
    PATCH_SWITCH("0x1D8507C", "200080D2C0035FD6", supressor);//isSupressor
    PATCH("0x10DCE9C", "000080D2C0035FD6");//HasBadWord
    PATCH("0x10DCF88", "000080D2C0035FD6");//IsBadWord
    PATCH("0x19A8B20", "200080D2C0035FD6");//IsVisible CharacterModel
}

void DrawMenu(){
    if(pSys != nullptr){
        void* GamePlayModule = *(void**)((uint64_t)pSys + 0x80);
        void* GameModeSystem = *(void**)((uint64_t)GamePlayModule + 0x38);
        bool inGame = *(bool*)((uint64_t)GameModeSystem + 0x69);
        if (inGame) {
            int id = getLocalId(pSys);
            void *localPlayer = getPlayer(pSys, id);
            int localTeam = get_PlayerTeam(localPlayer);
            monoList<void **> *characterList = getAllCharacters(pSys);
            for (int i = 0; i < characterList->getSize(); i++) {
                void *currentCharacter = (monoList<void **> *) characterList->getItems()[i];
                int curTeam = get_CharacterTeam(currentCharacter);
                int health = get_Health(currentCharacter);
                if (health > 0 && get_IsInitialized(currentCharacter) &&
                    localTeam != curTeam && curTeam != -1) {
                    void *transform = getTransform(currentCharacter);
                    Vector3 position = get_Position(transform);
                    Vector3 transformPos = WorldToScreen(get_camera(), position, 2);
                    transformPos.Y = glHeight - transformPos.Y;
                    Vector3 headPos = getBonePosition(currentCharacter, HEAD);
                    Vector3 chestPos = getBonePosition(currentCharacter, CHEST);
                    Vector3 wschestPos = WorldToScreen(get_camera(), chestPos, 2);
                    Vector3 wsheadPos = WorldToScreen(get_camera(), headPos, 2);
                    Vector3 wsheadPos2 = wsheadPos;
                    Vector3 aboveHead = headPos + Vector3(0,0,1); // estimate
                    Vector3 headEstimate = position + Vector3(0,0,1.48);// estimate
                    Vector3 wsAboveHead = WorldToScreen(get_camera(), aboveHead, 2);
                    Vector3 wsheadEstimate = WorldToScreen(get_camera(), headEstimate, 2);
                    wsAboveHead.Y = glHeight - wsAboveHead.Y;
                    wsheadEstimate.Y = glHeight - wsheadEstimate.Y;
                    float height = transformPos.Y - wsAboveHead.Y;
                    float width = (transformPos.Y - wsheadEstimate.Y);
                    if (snaplines && transformPos.Z > 0) {
                        DrawLine(ImVec2(glWidth / 2, glHeight), ImVec2(transformPos.X, transformPos.Y), ImColor(172, 204, 255), 3);
                    }
                    if (bonesp) {
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
                            DrawLine(ImVec2(wschestPos.X, wschestPos.Y),ImVec2(wsneck.X,wsneck.Y),ImVec4(172, 204, 255, 255), 3);
                        }
                        if(wsheadPos.Z > 0 && wschestPos.Z > 0 ){
                            float radius = sqrt(diff.X * diff.X + diff.Y * diff.Y);
                            auto background = ImGui::GetBackgroundDrawList();

                            background->AddCircle(ImVec2(wsheadPos.X, wsheadPos.Y), radius/2, IM_COL32(172, 204, 255, 255), 0, 3.0f);
                        }
                    }
                    if (boxesp && transformPos.Z > 0 && wsAboveHead.Z > 0)
                    {
                        DrawOutlinedBox2(wsAboveHead.X, wsAboveHead.Y, width, height, ImVec4(255,255,255,255), 3);
                    }
                    if (healthesp && transformPos.Z > 0 && wsAboveHead.Z > 0)
                    {
                        DrawOutlinedBox2(wsheadPos.X - 12, wsheadPos.Y - height*(1 - (health/100)), 3, height*(health/100), HealthToColor(health), 3);
                    }
                    if (healthNumber && transformPos.Z > 0 && wsAboveHead.Z > 0)
                    {
                        if (health < 100)
                        {
                            DrawText(ImVec2(wsheadPos.X - 10, wsheadPos.Y - height*(1 - (health/100))), ImVec4(255,255,255,255), std::to_string(health), espFont);
                        }
                    }
                    if (espName && transformPos.Z > 0 && wsAboveHead.Z > 0)
                    {
                        std::u16string name = get_CharacterName(currentCharacter);
                        DrawText(ImVec2(wsheadPos.X, wsAboveHead.Y - 7), ImVec4(255,255,255,255), name.c_str(), espFont);
                    }
                    if (weaponEsp && transformPos.Z > 0 && wsAboveHead.Z > 0)
                    {
                        std::u16string weapon = get_characterWeaponName(currentCharacter);
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
    }
    static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    {
        ImGui::Begin(OBFUSCATE("Critical Ops 1.0a (1.37.0.f2085) - chr1s#4191 && 077 Icemods"));
        ImGui::TextUnformatted("If the menu touch is broken, set the screenscale in settings to 100.");
        if (ImGui::Button(OBFUSCATE("Join Discord")))
        {
            //isDiscordPressed = true;
        }
        ImGui::TextUnformatted("Its Recommended to join the discord server for mod updates etc.");
        ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_FittingPolicyResizeDown;
        if (ImGui::BeginTabBar("Menu", tab_bar_flags)) {
            if (ImGui::BeginTabItem(OBFUSCATE("Legit Mods"))) {
                if (ImGui::CollapsingHeader(OBFUSCATE("Weapon Mods"))) {
                    ImGui::Checkbox(OBFUSCATE("Less Recoil"), &recoil);
                    ImGui::Checkbox(OBFUSCATE("No Spread"), &spread);
                    ImGui::Checkbox(OBFUSCATE("Force Supressor"), &supressor);
                }
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem(OBFUSCATE("Rage Mods"))) {
                if (ImGui::CollapsingHeader(OBFUSCATE("Player Mods"))) {
                    ImGui::Checkbox(OBFUSCATE("No Grenade Damage"), &ggod);
                    ImGui::Checkbox(OBFUSCATE("Spoof Crouch"), &crouch);
                }
                if (ImGui::CollapsingHeader(OBFUSCATE("Weapon Mods"))) {
                    ImGui::Checkbox(OBFUSCATE("Wallbang"), &wallbang);
                }
                if (ImGui::CollapsingHeader(OBFUSCATE("Game Mods"))) {
                    ImGui::Checkbox(OBFUSCATE("Spawn Bullets In Enemy"), &spawnbullets);
                }
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem(OBFUSCATE("Visual Mods"))) {
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
                ImGui::Checkbox(OBFUSCATE("Radar"), &radar);
                ImGui::Checkbox(OBFUSCATE("No Flashbang"), &flash);
                ImGui::Checkbox(OBFUSCATE("No Smoke"), &smoke);
                ImGui::Checkbox(OBFUSCATE("No Scope"), &scope);
                ImGui::Checkbox(OBFUSCATE("No Aimpunch"), &aimpunch);
                ImGui::Checkbox(OBFUSCATE("Hide Kill Notifications"), &killnotes);
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
        Patches();
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
    espFont = io.Fonts->AddFontFromMemoryCompressedTTF(RetroGaming, compressedRetroGamingSize, 30);
    flagFont = io.Fonts->AddFontFromMemoryCompressedTTF(Minecraftia_Regular, compressedMinecraftia_RegularSize, 30);
}

EGLBoolean hook_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    eglQuerySurface(dpy, surface, EGL_WIDTH, &glWidth);
    eglQuerySurface(dpy, surface, EGL_HEIGHT, &glHeight);

    LOGE("swapBuffers Hooked");

    if (!setupimg)
    {
        LOGE("1");
        glHeight = get_Height();
        LOGE("2");
        glWidth = get_Width();
        LOGE("3");
        SetupImgui();
        LOGE("4");
        SetResolution(glWidth, glHeight, true, 999);
        LOGE("5");
        setupimg = true;
        LOGE("6");
    }

    ImGuiIO &io = ImGui::GetIO();
    LOGE("7");

    ImGui_ImplOpenGL3_NewFrame();
    LOGE("8");
    ImGui::NewFrame();
    LOGE("9");

    DrawMenu();
    LOGE("10");

    ImGui::EndFrame();
    ImGui::Render();
    LOGE("height: %d, width: %d", glHeight, glWidth);
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
            auto address = KittyScanner::findHexFirst(it->startAddress, it->endAddress, "7F 45 4C 46 02 01 01 00 00 00 00 00 00 00 00 00 03 00 B7 00 01 00 00 00 90 06 7A 00 00 00 00 00 40 00 00 00 00 00 00 00 B8 6A E1 02 00 00 00 00", "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");
            if(address != 0)
            {
                libBaseAddress = address;
                libBaseEndAddress = it->endAddress;
            }
        }
        if (tries > 10)
        {
            auto map = KittyMemory::getLibraryBaseMap("libil2cpp.so");
            libBaseAddress = map.startAddress;
            libBaseEndAddress = map.endAddress;
        }
        tries++;
    } while (libBaseAddress == 0);
    LOGE("LIB base at: %p", (void*)get_absolute_address(0x0));
    Hooks();
    Pointers();
    auto eglhandle = dlopen("libunity.so", RTLD_LAZY);
    auto eglSwapBuffers = dlsym(eglhandle, "eglSwapBuffers");
    DobbyHook((void*)eglSwapBuffers,(void*)hook_eglSwapBuffers,
              (void**)&old_eglSwapBuffers);
    void *sym_input = DobbySymbolResolver(("/system/lib/libinput.so"), ("_ZN7android13InputConsumer21initializeMotionEventEPNS_11MotionEventEPKNS_12InputMessageE"));
    if (NULL != sym_input) {
        DobbyHook(sym_input,(void*)myInput,(void**)&origInput);
    }
    return nullptr;
}
