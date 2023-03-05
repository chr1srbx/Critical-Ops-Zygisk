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
#include "hook.h"
#include "Include/Roboto-Regular.h"
#include "ESP.h"
#include "Include/Vector3.h"

#define GamePackageName "com.criticalforceentertainment.criticalops"

monoString* CreateIl2cppString(const char* str)
{
    static monoString* (*CreateIl2cppString)(const char* str, int *startIndex, int *length) =
    (monoString* (*)(const char* str, int *startIndex, int *length))(get_absolute_address(string2Offset(OBFUSCATE("0x44865B4"))));
    int* startIndex = 0;
    int* length = (int *)strlen(str);
    return CreateIl2cppString(str, startIndex, length);
}
void (*SetResolution)(int widht, int height, bool fullscreen, float refreshRate);
int (*get_Width)();
int (*get_Height)();
monoList<void**>* (*getAllCharacters)(void* obj);
int (*getLocalId)(void* obj);
void* (*getPlayer)(void* obj, int id);
void* (*getLocalPlayer)(void* obj);
int (*getCharacterCount)(void* obj);
int (*get_Health)(void* character);
void* (*get_Player)(void* character);
void(*set_targetFrameRate)(int frames);
bool (*get_IsInitialized)(void* character);
Vector3 (*get_Position)(void* transform);
Vector2 (*WorldToScreen)(void*, Vector3 worldPos);
void* (*get_camera)();
void* (*SetRotation)(void* obj, Vector2 rotation);

void* pSys = nullptr;
void* pBones = nullptr;
bool recoil, radar, flash, smoke, scope, setupimg, spread, aimpunch, speed, reload, esp, snaplines, kickback, crouch, wallbang,
fov, ggod, killnotes,crosshair, supressor, rifleb;
float speedval = 1, fovModifier;
int glHeight, glWidth;

float (*old_get_fieldOfView)(void *instance);
float get_fieldOfView(void *instance) {
    if (instance != nullptr && fov) {
        return fovModifier;
    }
    return old_get_fieldOfView(instance);
}

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
    SetRotation = (void*(*)(void*, Vector2)) get_absolute_address(0x1A75780);
    WorldToScreen = (Vector2(*)(void*, Vector3)) get_absolute_address(0x160D4CC);
    set_targetFrameRate = (void(*)(int)) get_absolute_address(0x1A3A5BC);
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
void RenderOverlaySmoke(void* obj){
    if(obj != nullptr && smoke){
        *(float*)((uint64_t) obj + 0x20) = 9999;//m_fadeSpeed
    }
    oldRenderOverlaySmoke(obj);
}

void(*oldUpdateWeapon)(void* obj);
void UpdateWeapon(void* obj){
    if(obj != nullptr){
        monoArray<void**> *CharcacterBodyPart = *(monoArray<void**>**)((uint64_t) obj + 0x78);
        if(CharcacterBodyPart != nullptr){
            LOGE("CHAR BODY");
            *(int*)((uint64_t) CharcacterBodyPart + 0x18) = 10;
            pBones = obj;
        }
    }
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
    HOOK("0x1A78DD8", UpdateWeapon, oldUpdateWeapon);
    HOOK("0x1A10C54", get_fieldOfView, old_get_fieldOfView);
}

void Patches(){
    PATCH_SWITCH("0x1D4FD28", "000080D2C0035FD6", spread);//UpdateSpread
    PATCH_SWITCH("0x1D4FBF8", "1F2003D5C0035FD6", recoil);//AddRecoil
    PATCH_SWITCH("0x17292FC", "000080D2C0035FD6", recoil);//get_RecoilRotatesCamera
    PATCH_SWITCH("0x172930C", "000080D2C0035FD6", recoil);//get_RecoilRotatesViewModel
    PATCH_SWITCH("0x17257C8", "000080D2C0035FD6", recoil);//get_RecoilRotatesCamera
    PATCH_SWITCH("0x17257D8", "000080D2C0035FD6", recoil);//get_RecoilRotatesViewModel
    PATCH_SWITCH("0x10BEED4", "1F2003D5C0035FD6", recoil);//UpdateCameraShake
    PATCH_SWITCH("0x1D4FCC4", "000080D2C0035FD6", aimpunch);//AimPunchRecover
    PATCH_SWITCH("0x1D4FDC4", "000080D2C0035FD6", kickback);//ApplyKickBack
    PATCH_SWITCH("0x19BAB70", "1F2003D5C0035FD6", crouch);//UpdateCrouch
    PATCH_SWITCH("0x10D2730", "1F2003D5C0035FD6", wallbang);//ProcessHitBuffers
    PATCH_SWITCH("0x10D2C58", "1F2003D5C0035FD6", wallbang);//UpdateWallHit
    PATCH_SWITCH("0x10D2228", "1F2003D5C0035FD6", wallbang);//ProcessHitBuffers
    PATCH_SWITCH("0x10D1664", "000080D2C0035FD6", ggod);//GrenadeHitCharacter
    PATCH_SWITCH("0xF3C32C", "1F2003D5C0035FD6", killnotes);//SetKillNotification
    PATCH_SWITCH("0x1D8507C", "200080D2C0035FD6", crosshair);//get_Crosshair
    PATCH_SWITCH("0x1D8502C", "000080D2C0035FD6", smoke);//get_GrenadeSmokeAreaOfEffect
    PATCH_SWITCH("0x1D8507C", "200080D2C0035FD6", supressor);//get_GrenadeSmokeAreaOfEffect
    PATCH_SWITCH("0x1D8506C", "400080D2C0035FD6", rifleb);//get_GrenadeSmokeAreaOfEffect
    PATCH("0x10DCE9C", "000080D2C0035FD6");//HasBadWord
    PATCH("0x10DCF88", "000080D2C0035FD6");//IsBadWord
}


void DrawMenu(){
    if(pSys != nullptr) {
        if (esp) {
            ImGui::GetBackgroundDrawList()->AddLine(ImVec2(0, 0), ImVec2(200, 200), ImColor(255, 255, 0), 1);
            DrawLine(ImVec2(0, 0), ImVec2(200, 200), ImVec4(1, 1, 1, 1));
            int id = getLocalId(pSys);
            void *localPlayer = getPlayer(pSys, id);
            int localTeam = get_PlayerTeam(localPlayer);
            monoList<void **> *characterList = getAllCharacters(pSys);
            for (int i = 0; i < characterList->getSize(); i++) {
                void *currentCharacter = (monoList<void **> *) characterList->getItems()[i];
                if (get_Health(currentCharacter) > 0 && get_IsInitialized(currentCharacter) &&
                    localTeam != get_CharacterTeam(currentCharacter)) {
                    void *transform = getTransform(currentCharacter);
                    Vector3 position = get_Position(transform);
                    Vector2 screenPos = WorldToScreen(get_camera(), position);
                    screenPos.Y = glHeight - screenPos.Y;
                    if (snaplines) {
                        ImVec2 fromLine = ImVec2(glWidth / 2, 0);
                        ImVec2 linePosition = ImVec2(screenPos.X, screenPos.Y);
                        ImGui::GetBackgroundDrawList()->AddLine(fromLine, linePosition, ImColor(172, 204, 255), 5);
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
                    ImGui::Checkbox(OBFUSCATE("No Recoil"), &recoil);
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
                if(ImGui::Checkbox(OBFUSCATE("ESP"), &esp))
                if(esp){
                    ImGui::Checkbox(OBFUSCATE("Snaplines"), &snaplines);
                }
                if(ImGui::Checkbox(OBFUSCATE("Field Of View"), &fov))
                if(fov){
                    ImGui::SliderFloat(OBFUSCATE("Value"), &fovModifier, 0.0, 360.0);
                }
                ImGui::Checkbox(OBFUSCATE("Radar"), &radar);
                ImGui::Checkbox(OBFUSCATE("No Flashbang"), &flash);
                ImGui::Checkbox(OBFUSCATE("No Smoke"), &smoke);
                ImGui::Checkbox(OBFUSCATE("No Scope"), &scope);
                ImGui::Checkbox(OBFUSCATE("No Aimpunch"), &aimpunch);
                ImGui::Checkbox(OBFUSCATE("No Kickback"), &kickback);
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
    set_targetFrameRate(999);

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
