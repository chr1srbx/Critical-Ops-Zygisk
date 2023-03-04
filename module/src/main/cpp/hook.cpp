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

#define GamePackageName "com.criticalforceentertainment.criticalops"

monoString* CreateIl2cppString(const char* str)
{
    static monoString* (*CreateIl2cppString)(const char* str, int *startIndex, int *length) =
    (monoString* (*)(const char* str, int *startIndex, int *length))(g_il2cppBaseMap.startAddress + string2Offset(OBFUSCATE("0x44865B4")));
    int* startIndex = 0;
    int* length = (int *)strlen(str);
    return CreateIl2cppString(str, startIndex, length);
}

int (*get_Width)();
int (*get_Height)();
void* (*getAllCharacters)(void* pSys);
int (*getLocalId)(void* pSys);
void* (*getPlayer)(void* pSys, int id);
void* (*getLocalPlayer)(void* pSys, int64_t a2, int64_t a3, int64_t a4, int64_t a5, int64_t a6, int64_t a7, int64_t a8, int64_t a9, int64_t a10);
int (*getCharacterCount)(void* pSys);
int (*get_Health)(void* character);
void* (*get_Player)(void* character);
bool (*get_IsInitialized)(void* character);
Vector3 (*get_Position)(void* transform);
Vector3 (*WorldToScreen)(Vector3 worldPos);


bool recoil, radar, flash, smoke, scope, setupimg;
float recoilVal;
int glHeight, glWidth;


void Pointers()
{
    get_Width = (int(*)()) get_absolute_address(0x1A0BF90);
    get_Height = (int(*)()) get_absolute_address(0x1A0BFB8);
    getAllCharacters = (void*(*)(void*)) get_absolute_address(0x1128D88);
    getLocalId= (int(*)(void*)) (void*) get_absolute_address( 0x111D1DC);
    getPlayer = (void*(*)(void*,int)) (void*) get_absolute_address( 0x112BFE4);
    getLocalPlayer = (void*(*)(void*,int64_t,int64_t,int64_t,int64_t,int64_t,int64_t,int64_t,int64_t,int64_t)) get_absolute_address(0x111CC4C);
    getCharacterCount = (int(*)(void*)) get_absolute_address(0x1128D98);
    get_Health = (int(*)(void*)) get_absolute_address(0x1A74DA0);
    get_Player = (void*(*)(void*)) get_absolute_address(0x1A74D88);
    get_IsInitialized = (bool(*)(void*)) get_absolute_address(0x1A74C10);
    get_Position = (Vector3(*)(void*)) get_absolute_address(0x1A3B390);


    WorldToScreen = (Vector3(*)(Vector3)) get_absolute_address(0x1A13390);
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

void(*oldGameLogic)(void* obj);
void GameLogic(void* obj){
    if(obj != nullptr){
        LOGE("GAME LOGIC");
        void* GameData = *(void**)((uint64_t) obj + 0xE8);
        if(GameData != nullptr){
            LOGE("GAME DATA");
            monoArray<void **> *weaponDefsData = *(monoArray<void **> **)((uint64_t)GameData + 0x28);
            if(weaponDefsData != nullptr){
                LOGE("WDD");
                if(recoil){
                    *(float*)((uint64_t) weaponDefsData + 0x104) = recoilVal;
                    *(float*)((uint64_t) weaponDefsData + 0x108) = recoilVal;
                    *(float*)((uint64_t) weaponDefsData + 0x100) = recoilVal;
                    *(float*)((uint64_t) weaponDefsData + 0xF0) = recoilVal;
                }
            }
        }
    }
    oldGameLogic(obj);
}

void* oldShaderFind(std::string name);
void* ShaderFind(std::string name)
{
    LOGE("Shader logged: %s", name.c_str());
    oldShaderFind(name);
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



void(*oldGameSystemUpdate)(void* pSys, float deltaTime);
void GameSystemUpdate(void* pSys, float deltaTime)
{
    LOGE("GameSystemUpdate called");

    if(pSys)
    {
        int id = getLocalId(pSys);
        LOGE("Player Id is: %d", id);
        void* localPlayer = getPlayer(pSys, id);
        LOGE("Player class is at: %p", localPlayer);
        void* boxedValueName = *(void**)((uint64_t)localPlayer + 0x118);
        LOGE("Player Boxed Value is at: %p: ", boxedValueName);
        uint8_t realValue = *(uint8_t*)((uint64_t)boxedValueName + 0x1C); // NOTE the get value offset from the boxed data type depends on the boxed datatype refer to dump.cs for more info
        LOGE("Player team is: %d", realValue);

        /*
         int localTeam = get_PlayerTeam(localPlayer);
         LOGE("Character LocalTeam: %d", int);
         void* characterList = getAllCharacters(pSys);
         LOGE("Charcter List found at: %p", characterList);
         int characterCount = getCharacterCount(pSys);
         LOGE("Character Count found: %d", character)
         for(int i= 0;i < characterCount; i++)
         {
           LOGE("Character Iteration loop: %d", i)
           currentCharcter* = *(void**)((uint64_t)characterList + i);
           LOGE("Character Current is at: %p", currentCharcter);
           if (get_Health(currentCharcter) > 0 && get_IsInitialized(currentCharcter) && localTeam != get_CharacterTeam(currentCharcter))
           {
               LOGE("Character Health: %d", get_Health(currentCharcter));
               LOGE("Character Initialized: %d", get_IsInitialized(currentCharcter));
               void* transform = getTransform(currentCharcter);
               LOGE("Character Transform at: %p", transform);
               Vector3 position = get_Position(transform);
               LOGE("Character position X: %f, Y: %f, Z: %f", position.x, position.y, position.z);
               Vector3 screenPos = WorldToScreen(position);
               LOGE("Character  Screen position X: %f, Y: %f, Z: %f", screenPos.x, screenPos.y, screenPos.z);

               // Snaplines (Ew)
               DrawLine(ImVec2(glWidth/2,glHeight), ImVec2(screenPos.x,screenPos.y), ImVec4(127/255,204/255,1,1));
               LOGE("Character Snapoline Drawn");
           }
          }



         
         */



    }
    return oldGameSystemUpdate(pSys, deltaTime);
}




void(*oldUpdateCameraEffects)(void* obj);
void UpdateCameraEffects(void* obj){
    if(obj != nullptr){
        LOGE("UPDATE CAMERA FX");
        if(radar){
            *(int*)((uint64_t) obj + 0x6C) = 1;//m_currentTeamIndex
        }
    }
    oldUpdateCameraEffects(obj);
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

HOOKAF(void, Input, void *thiz, void *ex_ab, void *ex_ac) {
    origInput(thiz, ex_ab, ex_ac);
    ImGui_ImplAndroid_HandleInputEvent((AInputEvent *)thiz);
    return;
}


void Hooks() {
    DobbyHook((void*) get_absolute_address(0x10BE384), (void*) UpdateCameraEffects, (void**) &oldUpdateCameraEffects);
    // reinterpret_cast<uintptr_t>(KittyMemory::findHexFirst(libBaseAddress, libBaseAddress + 10000000, "FF 03 01 D1 E9 23 01 6D F5 53 02 A9 F3 7B 03 A9 F5", "xxxxxxxxxxxxxxxxx"))
    DobbyHook((void*) get_absolute_address(0x19AFC90), (void*) RenderOverlayFlashbang, (void**) &oldRenderOverlayFlashbang);
    // reinterpret_cast<uintptr_t>(KittyMemory::findHexFirst(libBaseAddress, libBaseAddress + 10000000, "", ""))
    DobbyHook((void*) get_absolute_address(0x19AFED8), (void*) set_Spread, (void**) &oldset_Spread);
    DobbyHook((void*) get_absolute_address(0x19B6090), (void*) RenderOverlaySmoke, (void**) &oldRenderOverlaySmoke);
    DobbyHook((void*) get_absolute_address(0x1D7005C), (void*) GameLogic, (void**) &oldGameLogic);
    // reinterpret_cast<uintptr_t>(KittyMemory::findHexFirst(libBaseAddress, libBaseAddress + 10000000, "F8 0F 1C F8 F7 5B 01 A9 F5 53 02 A9 F3 7B 03 A9 95 85", "xxxxxxxxxxxxxxxxxx"))
    DobbyHook((void*) get_absolute_address(0x10CE734), (void*) GameSystemUpdate, (void**) &oldGameSystemUpdate);
    DobbyHook((void*) get_absolute_address(0x1A0C310), (void*) ShaderFind, (void**) &oldShaderFind);
}

void Patches(){
   // PATCH("0xA8AC30", "")
}

void DrawMenu(){
    static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    {
        ImGui::Begin(OBFUSCATE("Critical Ops 1.0a (23.1) - chr1s#4191 && 077 Icemods && networkCommand()#7611 && ohmyfajett#3500"));
        if (ImGui::Button(OBFUSCATE("Join Discord")))
        {
            //isDiscordPressed = true;
        }
        ImGui::TextUnformatted("Its Recommended to join the discord server for mod updates etc.");
        if (ImGui::CollapsingHeader(OBFUSCATE("Legit Mods"))) {
            ImGui::Checkbox(OBFUSCATE("Change Recoil"), &recoil);
            if(recoil){
                ImGui::SliderFloat(OBFUSCATE("Recoil Intensity"), &recoilVal, 0.0f, 100.0f);
            }
        }
        if (ImGui::CollapsingHeader(OBFUSCATE("Rage Mods"))) {

        }
        if (ImGui::CollapsingHeader(OBFUSCATE("Visual Mods"))) {
            ImGui::Checkbox(OBFUSCATE("Radar"), &radar);
            ImGui::Checkbox(OBFUSCATE("No Flashbang"), &flash);
            ImGui::Checkbox(OBFUSCATE("No Smoke"), &smoke);
            ImGui::Checkbox(OBFUSCATE("No Scope"), &scope);
        }
        Patches();
        ImGui::End();
    }
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
    glViewport(0, 0, (int)2560, (int)1440);
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
