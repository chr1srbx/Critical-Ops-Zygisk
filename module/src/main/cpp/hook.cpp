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
#include "Include/Unity.h"
#include "Misc.h"
#include "Include/Vector3.h"
#include "hook.h"
#include "Include/Roboto-Regular.h"
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
fov, ggod, killnotes,crosshair, supressor, rifleb, bonesp, viewmodel, viewmodelfov;
float speedval = 1, fovModifier, viemodelposx, viemodelposy, viemodelposz, viewmodelfovval;
int glHeight, glWidth;
monoList<void**>* (*getAllCharacters)(void* obj);
Vector3 (*get_Position)(void* transform);
Vector3 (*WorldToScreen)(void*, Vector3 worldPos, int);
void* (*get_camera)();
void* (*get_CharacterBodyPart)(void* obj, int);

float (*old_get_fieldOfView)(void *instance);
float get_fieldOfView(void *instance) {
    if (instance != nullptr && fov) {
        return fovModifier;
    }
    return old_get_fieldOfView(instance);
}
void (*SetResolution)(int widht, int height, bool fullscreen, float refreshRate);
int (*get_Width)();
int (*get_Height)();
int (*getLocalId)(void* obj);
void* (*getPlayer)(void* obj, int id);
void* (*getLocalPlayer)(void* obj);
int (*getCharacterCount)(void* obj);
int (*get_Health)(void* character);
void* (*get_Player)(void* character);

void(*set_targetFrameRate)(int frames);
bool (*get_IsInitialized)(void* character);
// character functions
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

void Pointers()
{
    SetResolution = (void(*)(int, int, bool, float)) get_absolute_address(0x1A0C224);
    get_Width = (int(*)()) get_absolute_address(0x1A0BF90);
    get_Height = (int(*)()) get_absolute_address(0x1A0BFB8);
    getAllCharacters = (monoList<void**>*(*)(void*)) get_absolute_address(0x10C993C);
    getLocalId= (int(*)(void*)) get_absolute_address(0x10BDBEC);
    getPlayer = (void*(*)(void*,int))  get_absolute_address(0x10CCBDC);
    getLocalPlayer = (void*(*)(void*)) get_absolute_address(0x10BD65C);
    getCharacterCount = (int(*)(void*)) get_absolute_address(0x10C994C);
    get_Health = (int(*)(void*)) get_absolute_address(0x1A74DA0);
    get_Player = (void*(*)(void*)) get_absolute_address(0x1A74D88);
    get_IsInitialized = (bool(*)(void*)) get_absolute_address(0x1A74C10);
    get_Position = (Vector3(*)(void*)) get_absolute_address(0x1A3B390);
    get_camera = (void*(*)()) get_absolute_address(0x1A13C5C);
    WorldToScreen = (Vector3(*)(void*, Vector3, int)) get_absolute_address(0x1A13060);
    set_targetFrameRate = (void(*)(int)) get_absolute_address(0x1A3A5BC);
    get_CharacterBodyPart =(void*(*)(void*, int)) get_absolute_address(0x1A74DFC);
}

void DrawBones(void* character, int bone1, int bone2){
    Vector3 bone1Pos = getBonePosition(character, bone1);
    Vector3 bone2Pos = getBonePosition(character, bone2);
    Vector3 wsbone1 = WorldToScreen(get_camera(), bone1Pos, 2);
    Vector3 wsbone2 = WorldToScreen(get_camera(), bone2Pos, 2);
    wsbone1.Y = glHeight - wsbone1.Y;
    wsbone2.Y = glHeight - wsbone2.Y;
    if(wsbone1.Z > 0 && wsbone2.Z > 0){
        DrawLine(ImVec2(wsbone1.X, wsbone1.Y), ImVec2(wsbone2.X, wsbone2.Y), ImColor(172, 204, 255), 3);
    }
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

void* (*oldShaderFind)(std::string name);
void* ShaderFind(std::string name)
{
    LOGE("Shader logged: %s", name.c_str());
    oldShaderFind(name);
}

void(*oldGameSystemUpdate)(void* obj);
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
        }
    }
    return oldGameSystemUpdate(obj);
}


void(*oldRenderOverlayFlashbang)(void* obj);
void RenderOverlayFlashbang(void* obj){
    if(obj != nullptr && flash){
        *(float*)((uint64_t) obj + 0x38) = 0;//m_flashTime
    }
    oldRenderOverlayFlashbang(obj);
}

void(*oldset_Spread)(void*obj);
void set_Spread(void* obj){
    if(obj != nullptr){
        *(float*)((uint64_t) obj + 0x24) = 0;//m_maxSpread
        *(float*)((uint64_t) obj + 0x20) = 0;//m_spreadFactor
    }
    oldset_Spread(obj);
}

void(*oldRenderOverlaySmoke)(void* obj);
void RenderOverlaySmoke(void* obj) {
    if (obj != nullptr && smoke) {
        *(float *) ((uint64_t) obj + 0x20) = 9999;//m_fadeSpeed
    }
    oldRenderOverlaySmoke(obj);
}

void(*oldDrawRenderer)(void* obj);
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

void Hooks() {
    HOOK("0x19AFC90", RenderOverlayFlashbang, oldRenderOverlayFlashbang);
    HOOK("0x19AFED8", set_Spread, oldset_Spread);
    HOOK("0x19AFC90", RenderOverlayFlashbang, oldRenderOverlayFlashbang);
    HOOK("0x19B6090", RenderOverlaySmoke, oldRenderOverlaySmoke);
    HOOK("0x10CE734", GameSystemUpdate, oldGameSystemUpdate);
    //HOOK("0x1B9D608", DrawRenderer, oldDrawRenderer); need args
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
    PATCH_SWITCH("0x1D8507C", "200080D2C0035FD6", supressor);//get_GrenadeSmokeAreaOfEffect
    PATCH_SWITCH("0x1D8506C", "400080D2C0035FD6", rifleb);//get_GrenadeSmokeAreaOfEffect
    PATCH("0x10DCE9C", "000080D2C0035FD6");//HasBadWord
    PATCH("0x10DCF88", "000080D2C0035FD6");//IsBadWord
}

Vector3 getBonePosition(void* character, int bone){
    void* curBone = get_CharacterBodyPart(character, bone);
    int boneValue = *(int*)((uint64_t)curBone + 0x18);
    void* hitSphere = *(void**)((uint64_t)curBone + 0x20);
    void* transform = *(void**)((uint64_t)hitSphere + 0x30);
    Vector3 bonePos = get_Position(transform);
    return bonePos;
}



enum BodyPart
{
    LOWERLEG_LEFT,
    LOWERLEG_RIGHT,
    UPPERLEG_LEFT,
    UPPERLEG_RIGHT,
    STOMACH,
    CHEST,
    UPPERARM_LEFT,
    UPPERARM_RIGHT,
    LOWERARM_LEFT,
    LOWERARM_RIGHT,
    HEAD
};

void DrawMenu(){
    if(pSys != nullptr){
        void* GamePlayModule = *(void**)((uint64_t)pSys + 0x80);
        void* GameModeSystem = *(void**)((uint64_t)GamePlayModule + 0x38);
        bool inGame = *(bool*)((uint64_t)GameModeSystem + 0x69);
        if (inGame) {
            ImGui::GetBackgroundDrawList()->AddLine(ImVec2(0, 0), ImVec2(200, 200), ImColor(255, 255, 0), 1);
            int id = getLocalId(pSys);
            void *localPlayer = getPlayer(pSys, id);
            int localTeam = get_PlayerTeam(localPlayer);
            monoList<void **> *characterList = getAllCharacters(pSys);
            for (int i = 0; i < characterList->getSize(); i++) {
                void *currentCharacter = (monoList<void **> *) characterList->getItems()[i];
                int curTeam = get_CharacterTeam(currentCharacter);
                if (get_Health(currentCharacter) > 0 && get_IsInitialized(currentCharacter) &&
                    localTeam != curTeam && curTeam != -1) {
                    void *transform = getTransform(currentCharacter);
                    Vector3 position = get_Position(transform);
                    Vector3 screenPos = WorldToScreen(get_camera(), position, 2);
                    screenPos.Y = glHeight - screenPos.Y;
                    if (snaplines && screenPos.Z > 0) {
                        DrawLine(ImVec2(glWidth / 2, glHeight), ImVec2(screenPos.X, screenPos.Y), ImColor(172, 204, 255), 5);
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
                        Vector3 chestPos = getBonePosition(currentCharacter, CHEST);
                        Vector3 headPos = getBonePosition(currentCharacter, HEAD);
                        Vector3 wschestPos = WorldToScreen(get_camera(), chestPos, 2);
                        Vector3 wsheadPos = WorldToScreen(get_camera(), headPos, 2);
                        Vector3 diff = wschestPos - wsheadPos;
                        if(wsheadPos.Z > 0 && wschestPos.Z > 0 ){
                            float radius = sqrt(diff.X * diff.X + diff.Y * diff.Y);
                            auto background = ImGui::GetBackgroundDrawList();
                            wsheadPos.Y = glHeight - wsheadPos.Y;
                            background->AddCircle(ImVec2(wsheadPos.X, wsheadPos.Y), radius, IM_COL32(172, 204, 255, 255), 0, 5.0f);
                        }
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
                ImGui::EndTabItem();
            }
            
            if (ImGui::BeginTabItem(OBFUSCATE("Visual Mods"))) {
                if (ImGui::CollapsingHeader(OBFUSCATE("ESP"))) {
                    ImGui::Checkbox(OBFUSCATE(" · Snaplines"), &snaplines);
                    ImGui::Checkbox(OBFUSCATE(" · Bones"), &bonesp);
                }
                if (ImGui::CollapsingHeader(OBFUSCATE("ViewModel"))) {
                    ImGui::Checkbox(OBFUSCATE("Viewmodel Position"), &viewmodel);
                    if(viewmodel){
                        ImGui::SliderFloat(OBFUSCATE(" · Value X"), &viemodelposx, 1.0, 360.0);
                        ImGui::SliderFloat(OBFUSCATE(" · Value Y"), &viemodelposy, 1.0, 360.0);
                        ImGui::SliderFloat(OBFUSCATE(" · Value Z"), &viemodelposz, 1.0, 360.0);
                    }
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
}

EGLBoolean (*old_eglSwapBuffers)(EGLDisplay dpy, EGLSurface surface);
EGLBoolean hook_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    eglQuerySurface(dpy, surface, EGL_WIDTH, &glWidth);
    eglQuerySurface(dpy, surface, EGL_HEIGHT, &glHeight);

    int width = get_Width();
    int height = get_Height();
    if (!setupimg) {
        SetupImgui();
        SetResolution(width, height, true, 999);
        setupimg = true;
    }

    ImGuiIO &io = ImGui::GetIO();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();

    DrawMenu();

    ImGui::EndFrame();
    ImGui::Render();
    glViewport(0, 0, (int)width, (int)height);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    return old_eglSwapBuffers(dpy, surface);
}

void *hack_thread(void *arg) {
    do {
        sleep(1);
        auto maps =  KittyMemory::getMapsByName("split_config.arm64_v8a.apk");
        int lowest = ULONG_MAX;
        int highest = 0;
        for(std::vector<ProcMap>::iterator it = maps.begin(); it != maps.end(); ++it) {
            auto address = KittyScanner::findHexFirst(it->startAddress, it->endAddress, "7F 45 4C 46 02 01 01 00 00 00 00 00 00 00 00 00 03 00 B7 00 01 00 00 00 90 06 7A 00 00 00 00 00 40 00 00 00 00 00 00 00 B8 6A E1 02 00 00 00 00", "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");
            if(address != 0){
                libBaseAddress = address;
            }
        }
    } while (libBaseAddress == 0);
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
