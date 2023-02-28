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

#define GamePackageName "com.criticalforceentertainment.criticalops"

/*
 *
 * LIST OF NOTES:
 * GAMEMODES LIST: 丂丛万世丝七丒与丈, GameMode.cs << DONT TRUST THIS, ITS 丈下东与三丆下丛丄
 * 丒丘丁丄丂丑丛丄东, ItemReward.cs (Scripts\\Store\\MainStore\\ItemReward.cs)
 * 丒下丗丈丕三丌丏不, ProgressUpdater.Currency.cs (Scripts\\ServerProgressSync\\ProgressUpdater.Currency.cs)
 * 丐三丐丄丝丅丈丄丞, OfferData.cs (Scripts/OfferPacks/OfferData.cs)
 * SCENEINFO IS SceneInfo, FULLY OBFUSCATED
 * SCENEINFO 且与丒与丈下丈丁一 = AddMode(GameMode)
 *
 */

monoString* CreateIl2cppString(const char* str)
{
    static monoString* (*CreateIl2cppString)(const char* str, int *startIndex, int *length) =
    (monoString* (*)(const char* str, int *startIndex, int *length))(g_il2cppBaseMap.startAddress + string2Offset(OBFUSCATE("0x44865B4")));
    int* startIndex = 0;
    int* length = (int *)strlen(str);
    return CreateIl2cppString(str, startIndex, length);
}

bool recoil, radar, flash, smoke;
float recoilVal;
int(*get_Width)();
int(*get_Height)();
void Pointers() {

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

    }
}

void(*oldUpdateCameraEffects)(void* obj);
void UpdateCameraEffects(void* obj){
    if(obj != nullptr){
        LOGE("UPDATE CAMERA FX");
        if(radar){
            *(int*)((uint64_t) obj + 0x6C) = -1;//m_currentTeamIndex
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
}

void(*oldRenderOverlaySmoke)(void* obj);
void RenderOverlaySmoke(void* obj){
    if(obj != nullptr && smoke){
        *(float*)((uint64_t) obj + 0x20) = 9999;//m_fadeSpeed
    }
    oldRenderOverlaySmoke(obj);
}

int glHeight, glWidth;
bool setupimg;

HOOKAF(void, Input, void *thiz, void *ex_ab, void *ex_ac) {
    origInput(thiz, ex_ab, ex_ac);
    ImGui_ImplAndroid_HandleInputEvent((AInputEvent *)thiz);
    return;
}


void Hooks() {
    DobbyHook((void*) get_absolute_address(0x10CE734), (void*) GameLogic, (void**) &oldGameLogic);
    DobbyHook((void*) get_absolute_address(0x10BE384), (void*) UpdateCameraEffects, (void**) &oldUpdateCameraEffects);
    DobbyHook((void*) get_absolute_address(0x19AFC90), (void*) RenderOverlayFlashbang, (void**) &oldRenderOverlayFlashbang);
    DobbyHook((void*) get_absolute_address(0x19AFED8), (void*) set_Spread, (void**) &oldset_Spread);
    DobbyHook((void*) get_absolute_address(0x19B6090), (void*) RenderOverlaySmoke, (void**) &oldRenderOverlaySmoke);
}

void Patches(){
    PATCH("0xA8AC30", )
}

void DrawMenu(){
    static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    {
        ImGui::Begin(OBFUSCATE("Critical Ops 1.0a (23.1) - chr1s#4191 && networkCommand()#7611 && ohmyfajett#3500"));
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
        }
        Patches();
        ImGui::End();
    }
}

void SetupImgui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.DisplaySize = ImVec2((float) 2560.0, (float) 1440.0);
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
