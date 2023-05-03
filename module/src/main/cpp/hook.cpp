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
#include <zlib.h>
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
#include <mutex>
#include "Include/curl/curl.h"
#include "Include/json.h"
#define GamePackageName OBFUSCATE("com.criticalforceentertainment.criticalops")

static std::mutex esp_mtx;
static std::mutex aimbot_mtx;

monoString *CreateIl2cppString(const char *str) {
    static monoString *(*CreateIl2cppString)(const char *str, int *startIndex, int *length) =
    (monoString *(*)(const char *str, int *startIndex, int *length)) (get_absolute_address(
            string2Offset(OBFUSCATE("0xC8E8A4"))));
    int *startIndex = nullptr;
    int *length = (int *) strlen(str);
    return CreateIl2cppString(str, startIndex, length);
}

ESPCfg invisibleCfg, espcfg;
AimbotCfg pistolCfg, smgCfg, arCFg, shotgunCfg, sniperCfg;
bool firsttime = false;
static int selected = 0;
static int sub_selected = 10;
const char *Shader = OBFUSCATE("_PBS_Character_Indirect_Base");
//const char *Shader = OBFUSCATE("unity_SHC");
void *TouchControls = nullptr;
void *pSys = nullptr;
void *localCharacter = nullptr;
void *esp_localCharacter = nullptr;
bool unsafe, recoil, radar, flash, smoke, scope, setupimg, spread, aimpunch, speed, reload, esp, forcebuy, crouch, wallbang, rain, rain1,
        fov, ggod, killnotes, crosshair, moneyreward, mindamage, maxdamage, viewmodelfov, spawnbullets,
        canmove, isPurchasingSkins, fly, removecharacter, jumpheight, noslow, shake, eoi, gbounciness, ammo, firerate, iea,
        p100Crosshair, tradar, fscope, applied = false, nosway, burstfire, pickup, silentknife, armorpen, chams,
        headhitbox, bodyhitbox, silentknife1, addmone, openurls, ready = false, uptodate = true;

bool hidden, aimbotenabled, espenabled;

float speedval = 5.100000, fovModifier, recoilval, viewmodelfovval, flyval, jumpval = 4.100000, fovValue = 360, bounceval, gundmgm;
int burstfireval, aimPos = 0, shootControl = 0, trollage;

extern int glHeight;
extern int glWidth;
ImFont *espFont;

void *getTransform(void *character) {
    if (character) {
        return *(void **) ((uint64_t) character + string2Offset(OBFUSCATE("0x70")));
    }
    return nullptr;
}

int get_CharacterTeam(void* character)
{
    int team = -1;
    if (character)
    {
        void* player = *(void**)((uint64_t)character + string2Offset(OBFUSCATE("0x90")));
        if (player)
        {
            PlayerAdapter* playerAdapter = new PlayerAdapter;
            playerAdapter->Player = player;
            team = getTeamIndex(playerAdapter);
            delete playerAdapter;
        }
    }

    return team;
}

int get_PlayerTeam(void* player)
{
    int team = std::stoi(OBFUSCATE("-1"));
    if (player)
    {
        PlayerAdapter* playerAdapter = new PlayerAdapter;
        playerAdapter->Player = player;
        team = getTeamIndex(playerAdapter);
        delete playerAdapter;
    }

    return team;
}

std::string get_PlayerUsername(void* player){
    std::string username = OBFUSCATE("");
    std::string clantag = OBFUSCATE("");
    if(player != nullptr){
        PlayerAdapter* playerAdapter = new PlayerAdapter;
        playerAdapter->Player = player;
        username = get_Username(playerAdapter)->getString();
        clantag = get_ClanTag(playerAdapter)->getString();
        if(clantag != std::string(OBFUSCATE(""))){
            username = std::string(OBFUSCATE("[")) + clantag + std::string(OBFUSCATE("]")) + std::string(OBFUSCATE(" ")) +  username;
        }
        delete playerAdapter;
    }
    return username;
}

void TouchControlsUpdate(void *obj) {
    if (obj) {
        TouchControls = obj;
    }
    return oTouchControlsUpdate(obj);
}

void TouchControlsDestroy(void *obj) {
    if (obj) {
        TouchControls = nullptr;
    }
    return oTouchControlsDestroy(obj);
}

Vector3 getBonePosition(void *character, int bone) {
    void *hitSphere = NULL;
    void *transform = NULL;
    Vector3 bonePos = Vector3(0, 0, 0);
    if (character != nullptr) {
        void *curBone = get_CharacterBodyPart(character, bone);
        if (curBone != nullptr) { hitSphere = *(void **) ((uint64_t) curBone + 0x20); }
        if (hitSphere != nullptr) { transform = *(void **) ((uint64_t) hitSphere + 0x30); }
        if (transform != nullptr) {
            bonePos = get_Position(transform);
            return bonePos;
        }
        return Vector3(0, 0, 0);
    }
    return Vector3(0, 0, 0);
}

int isGame(JNIEnv *env, jstring appDataDir) {
    if (!appDataDir)
        return 0;
    const char *app_data_dir = env->GetStringUTFChars(appDataDir, nullptr);
    int user = 0;
    static char package_name[256];
    if (sscanf(app_data_dir, OBFUSCATE("/data/%*[^/]/%d/%s"), &user, package_name) != 2) {
        if (sscanf(app_data_dir, OBFUSCATE("/data/%*[^/]/%s"), package_name) != 1) {
            package_name[0] = '\0';
            return 0;
        }
    }
    if (strcmp(package_name, GamePackageName) == 0) {
        game_data_dir = new char[strlen(app_data_dir) + 1];
        strcpy(game_data_dir, app_data_dir);
        env->ReleaseStringUTFChars(appDataDir, app_data_dir);
        return 1;
    } else {
        env->ReleaseStringUTFChars(appDataDir, app_data_dir);
        return 0;
    }
}

bool isInFov2(Vector2 rotation, Vector2 newAngle, AimbotCfg cfg) {
    if (!cfg.fovCheck) return true;

    Vector2 difference = newAngle - rotation;

    if (difference.Y > 180) difference.Y -= 360;
    if (difference.Y < -180) difference.Y += 360;

    if (sqrt(difference.X * difference.X + difference.Y * difference.Y) > cfg.fovValue) {
        return false;
    }
    return true;
}

void *getValidEnt3(AimbotCfg cfg, Vector2 rotation) {
    int id = getLocalId(pSys);
    if (id == 0) return nullptr;

    void *localPlayer = getPlayer(pSys, id);
    if (localPlayer == nullptr) return nullptr;
    int localTeam = get_PlayerTeam(localPlayer);
    float closestEntDist = 99999.0f;
    void *closestCharacter = nullptr;
    monoList<void **> *characterList = getAllCharacters(pSys);
    if (characterList == nullptr) return nullptr;
    for (int i = 0; i < characterList->getSize(); i++) {
        void *currentCharacter = (monoList<void **> *) characterList->getItems()[i];
        if (currentCharacter == nullptr) continue;
        int curTeam = get_CharacterTeam(currentCharacter);
        int health = get_Health(currentCharacter);
        bool canSet = false;
        Vector2 newAngle;

        if (cfg.aimbot && esp_localCharacter != nullptr) {
            if (get_Health(esp_localCharacter) > 0 && health > 0) {
                Vector3 localHead = getBonePosition(esp_localCharacter, 10);

                if (getIsCrouched(esp_localCharacter)) {
                    localHead = localHead - Vector3(0, 0.5, 0);
                }
                Vector3 enemyBone = getBonePosition(currentCharacter, cfg.aimBone);
                Vector3 deltavec = enemyBone - localHead;
                float deltLength = sqrt(deltavec.X * deltavec.X + deltavec.Y * deltavec.Y +
                                        deltavec.Z * deltavec.Z);

                newAngle.X = -asin(deltavec.Y / deltLength) * (180.0 / PI);
                newAngle.Y = atan2(deltavec.X, deltavec.Z) * 180.0 / PI;
                if (isInFov2(rotation, newAngle, cfg)) {
                    if (esp_localCharacter && health > 0 && localTeam != curTeam && curTeam != -1) {
                        if (cfg.visCheck && get_Health(esp_localCharacter) > 0) {
                            if (isCharacterVisible(currentCharacter, pSys)) {
                                canSet = true;
                            }
                        }

                        void *transform = getTransform(esp_localCharacter);
                        if (transform) {
                            Vector3 localPosition = get_Position(transform);
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
    }
    return closestCharacter;
}

void GameSystemUpdate(void *obj) {
    if (obj != nullptr) {
        if (pSys != obj) {
            std::lock_guard<std::mutex> guard(esp_mtx);
            pSys = obj;
        }
        void *GamePlayModule = *(void **) ((uint64_t) obj + string2Offset(OBFUSCATE("0x80")));
        if (GamePlayModule != nullptr) {
            void *CameraSystem = *(void **) ((uint64_t) GamePlayModule + string2Offset(OBFUSCATE("0x30")));
            if (CameraSystem != nullptr) {
                if (fov) {
                    *(float *) ((uint64_t) CameraSystem + string2Offset(OBFUSCATE("0x8C"))) = fovModifier; // m_horizontalFieldOfView
                }

                if (viewmodelfov) {
                    *(float *) ((uint64_t) CameraSystem + string2Offset(OBFUSCATE("0x90"))) = viewmodelfovval; // m_viewModelFieldOfView
                }
            }
        }
    }
    oldGameSystemUpdate(obj);
}

void wait(int seconds) {
    clock_t endwait;
    endwait = clock() + seconds * CLOCKS_PER_SEC;
    while (clock() < endwait);
}

void* event;
void*(*oldCM)(monoString* message, ChatMessageType yes, bool funny);
void* CreateEMessage(monoString* message, ChatMessageType yes, bool funny){

    event = oldCM(CreateIl2cppString(OBFUSCATE("suganigadick")), PUBLIC_CHAT, funny);
    return event;
}


void UpdateWeapon(void* obj, float deltatime) {
    if (obj != nullptr) {
        void* CharacterData = *(void**)((uint64_t)obj + string2Offset(OBFUSCATE("0x98")));
        if (CharacterData != nullptr) {
            void* CharacterSettingsData = *(void**)((uint64_t)CharacterData + string2Offset(OBFUSCATE("0x78")));
            if (CharacterSettingsData != nullptr) {
                if (speed) {
                    *(float*)((uint64_t)CharacterSettingsData + string2Offset(OBFUSCATE("0x14"))) = speedval;
                }

                if (jumpheight) {
                    *(float*)((uint64_t)CharacterSettingsData + string2Offset(OBFUSCATE("0x4C"))) = jumpval;
                    *(float*)((uint64_t)CharacterSettingsData + string2Offset(OBFUSCATE("0x50"))) = jumpval;
                }

                if (noslow) {
                    *(float*)((uint64_t)CharacterSettingsData + string2Offset(OBFUSCATE("0x60"))) = 0;
                    *(float*)((uint64_t)CharacterSettingsData + string2Offset(OBFUSCATE("0x64"))) = 0;
                    *(float*)((uint64_t)CharacterSettingsData + string2Offset(OBFUSCATE("0x5C"))) = 0;
                }
            }

            void* WeaponDefData = *(void**)((uint64_t)CharacterData + string2Offset(OBFUSCATE("0x80")));
            if (WeaponDefData != nullptr) {
                //add reload time
                if (recoil) {
                    *(float*)((uint64_t)WeaponDefData + string2Offset(OBFUSCATE("0xF0"))) = recoilval;
                    *(float*)((uint64_t)WeaponDefData + string2Offset(OBFUSCATE("0x100"))) = recoilval;
                }

                if (mindamage) {
                    *(float*)((uint64_t)WeaponDefData + string2Offset(OBFUSCATE("0x50"))) = mindamage;
                }

                if (maxdamage) {
                    *(float*)((uint64_t)WeaponDefData + string2Offset(OBFUSCATE("0x4C"))) = maxdamage;
                }

                if (firerate) {
                    *(float*)((uint64_t)WeaponDefData + string2Offset(OBFUSCATE("0x64"))) = 1600;
                    *(float*)((uint64_t)WeaponDefData + string2Offset(OBFUSCATE("0x8C"))) = 0;
                }

                if (burstfire) {
                    *(int*)((uint64_t)WeaponDefData + string2Offset(OBFUSCATE("0x11C"))) = burstfireval;
                    *(float*)((uint64_t)WeaponDefData + string2Offset(OBFUSCATE("0x120"))) = 0;
                }

                if (ammo) {
                    *(bool*)((uint64_t)WeaponDefData + string2Offset(OBFUSCATE("0x94"))) = false;
                }

                if (fscope) {
                    *(int *) ((uint64_t) WeaponDefData + string2Offset(OBFUSCATE("0x98"))) = 1;
                }

                if (pickup) {
                    *(float *) ((uint64_t) WeaponDefData + string2Offset(OBFUSCATE("0x88"))) = 0;
                    *(float *) ((uint64_t) WeaponDefData + string2Offset(OBFUSCATE("0x8C"))) = 0;
                }

                if (shake) {
                    *(float *) ((uint64_t) WeaponDefData + string2Offset(OBFUSCATE("0x110"))) = 0;
                }
            }
        }
    }
    oldUpdateWeapon(obj, deltatime);
}

void RenderOverlayFlashbang(void *obj, void* IRender) {
    if (obj != nullptr && flash) {
        *(float *) ((uint64_t) obj + string2Offset(OBFUSCATE("0x38"))) = 0;//m_flashTime
    }
    oldRenderOverlayFlashbang(obj, IRender);
}

void set_Spread(void *obj, float value) {
    if (obj != nullptr && scope) {
        *(float *) ((uint64_t) obj + string2Offset(OBFUSCATE("0x24"))) = 0;//m_maxSpread
        *(float *) ((uint64_t) obj + string2Offset(OBFUSCATE("0x20"))) = 0;//m_spreadFactor
    }
    oldset_Spread(obj, value);
}


bool isCharacterVisible(void *character, void *pSys) {
    void *localCharacter = get_LocalCharacter(pSys);
    if (localCharacter) {
        return !isHeadBehindWall(localCharacter, character);
    }
    return 0;
}

void RenderOverlaySmoke(void *obj, void* IRender, void* IEntityDB, Vector3 origin) {
    if (obj != nullptr && smoke) {
        *(float *) ((uint64_t) obj + string2Offset(OBFUSCATE("0x20"))) = 9999;//m_fadeSpeed
    }
    oldRenderOverlaySmoke(obj, IRender, IEntityDB, origin);
}

int getCurrentWeaponCategory(void *character) {
    void *characterData = *(void **) ((uint64_t) character + string2Offset(OBFUSCATE("0x98")));
    if (characterData) {
        void *m_wpn = *(void **) ((uint64_t) characterData + string2Offset(OBFUSCATE("0x80")));
        if (m_wpn) {
            return *(int *) ((uint64_t) m_wpn + string2Offset(OBFUSCATE("0x38")));
        }
    }
    return -1;
}

bool isCharacterShooting(void *character) {
    void *characterData = *(void **) ((uint64_t) character + string2Offset(OBFUSCATE("0x98")));
    if (characterData) {
        return *(bool *) ((uint64_t) characterData + string2Offset(OBFUSCATE("0x6C")));
    }
    return 0;
}

Vector2 isInFov(Vector2 rotation, Vector2 newAngle, AimbotCfg cfg) {
    Vector2 difference = newAngle - rotation;

    if (difference.Y > 180) difference.Y -= 360;
    if (difference.Y < -180) difference.Y += 360;

    if (sqrt(difference.X * difference.X + difference.Y * difference.Y) > cfg.fovValue) {
        return Vector2(0, 0);
    }
    return difference;
}

int currWeapon = -1;
void setRotation(void *character, Vector2 rotation) {
    std::lock_guard<std::mutex> guard(aimbot_mtx);
    Vector2 newAngle;
    Vector2 difference;
    difference.X = 0;
    difference.Y = 0;
    AimbotCfg cfg;
    if (esp_localCharacter != nullptr) {
        currWeapon = getCurrentWeaponCategory(esp_localCharacter);
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
    void *closestEnt = nullptr;
    if (character != nullptr && esp_localCharacter != nullptr && get_IsInitialized(esp_localCharacter) && (pistolCfg.aimbot || shotgunCfg.aimbot || smgCfg.aimbot || arCFg.aimbot || sniperCfg.aimbot)) {
        closestEnt = getValidEnt3(cfg, rotation);
    }

    if (esp_localCharacter != nullptr && get_Health(esp_localCharacter) > 0 && closestEnt != nullptr) {
        Vector3 localHead = getBonePosition(esp_localCharacter, 10);
        if (localHead != Vector3(0, 0, 0) && getIsCrouched(esp_localCharacter)) {
            localHead = localHead - Vector3(0, 0.5, 0);
        }

        Vector3 enemyBone;
        if (aimPos == 0) {
            enemyBone = getBonePosition(closestEnt, HEAD);
        } else if (aimPos == 1) {
            enemyBone = getBonePosition(closestEnt, CHEST);
        } else if (aimPos == 2) {
            enemyBone = getBonePosition(closestEnt, STOMACH);
        }

        Vector3 deltavec = enemyBone - localHead;
        float deltLength = sqrt(deltavec.X * deltavec.X + deltavec.Y * deltavec.Y +
                                deltavec.Z * deltavec.Z);

        newAngle.X = -asin(deltavec.Y / deltLength) * (180.0 / PI);
        newAngle.Y = atan2(deltavec.X, deltavec.Z) * 180.0 / PI;

        if (character != nullptr) {
            if (cfg.aimbot && character == esp_localCharacter) {
                if (cfg.onShoot) {
                    if (isCharacterShooting(esp_localCharacter)) {
                        if (cfg.fovCheck) {
                            difference = isInFov(rotation, newAngle, cfg);
                        } else {
                            difference = newAngle - rotation;
                            cfg.fovValue = 360;
                        }
                    }
                } else {
                    if (cfg.fovCheck) {
                        difference = isInFov(rotation, newAngle, cfg);
                    } else {
                        difference = newAngle - rotation;
                        cfg.fovValue = 360;
                    }
                }

                if (cfg.aimbotSmooth) {
                    difference = difference / cfg.smoothAmount;
                }
            }
        }
    }


    if (cfg.triggerbot && closestEnt != nullptr && esp_localCharacter != nullptr && pSys != nullptr && get_Health(esp_localCharacter) > 0 && !get_Invulnerable(closestEnt)){
        int hitIndex = 0;
        void *camera = get_camera();
        if (camera != nullptr) {
            Ray ray = ScreenPointToRay(camera, Vector2(glWidth / 2, glHeight / 2), 2);

            if(closestEnt != nullptr){
                UpdateCharacterHitBuffer(pSys, closestEnt, ray, &hitIndex);
            }
            if (hitIndex && !shootControl) {
                shootControl = 1;
            }
        }
    }
    oSetRotation(character, rotation + difference);
}

void GameSystemDestroy(void *obj) {
    oGameSystemDestroy(obj);
    pistolCfg.aimbot = false; arCFg.aimbot = false; shotgunCfg.aimbot = false; sniperCfg.aimbot = false; smgCfg.aimbot = false;
    esp = false;
    std::lock_guard<std::mutex> guard(esp_mtx);
    pSys = nullptr;
    localCharacter = nullptr;
    esp_localCharacter = nullptr;
}

float FovWorld(void *obj) {
    if (obj != nullptr && fov) {
        return fovModifier;
    }
    return oldFovWorld(obj);
}

float FovViewModel(void *obj) {
    if (obj != nullptr && fov) {
        return viewmodelfovval;
    }
    return oldFovViewModel(obj);
}


float get_Heightt(void *obj) {
    if (obj != nullptr) {
        if (fly) {
            return flyval;
        } else {
            return 1.5f;
        }
    }

    return oldget_Height(obj);
}


void LoadSettings(void *obj) {
    if (obj != nullptr) {
        void *GraphicsPorfile = *(void **) ((uint64_t) obj + string2Offset(OBFUSCATE("0x38")));
        if (GraphicsPorfile != nullptr) {
            *(int *) ((uint64_t) GraphicsPorfile + string2Offset(OBFUSCATE("0x30"))) = 100;
        }
    }
}

void CheckCharacterVisiblity(void *obj, bool *visibility) {
    oldCheckCharacterVisibility(obj, visibility);
    if (obj != nullptr) {
        *visibility = true;
    }
}

const char* deviceid = "";
void(*oldAppManager)(void* obj);
void AppManager(void* obj){
    if(obj != nullptr){
        if(openurls){
            OpenURL(CreateIl2cppString((std::string(OBFUSCATE("https://www.spdmteam.com/PT-key-system-1?hwid=")) + getDeviceUniqueIdentifier()->getString()).c_str()));
            openurls = false;
        }

        if(deviceid == OBFUSCATE(""))
            deviceid = getDeviceUniqueIdentifier()->getString().c_str();

    }
    oldAppManager(obj);
}

void* getValidEnt()
{
    int id = getLocalId(pSys);
    void *localPlayer = getPlayer(pSys, id);
    int localTeam = get_PlayerTeam(localPlayer);
    float closestEntDist = 10.0f;
    void* closestCharacter = nullptr;
    monoList<void **> *characterList = getAllCharacters(pSys);
    for (int i = 0; i < characterList->getSize(); i++) {
        void *currentCharacter = (monoList<void **> *) characterList->getItems()[i];
        if (get_Player(currentCharacter) == localPlayer) {
                localCharacter = currentCharacter;
        }
        int curTeam = get_CharacterTeam(currentCharacter);
        int health = get_Health(currentCharacter);
        if (curTeam != localTeam && curTeam != -1 && health > 0) {
            Vector3 localPosition = get_Position(getTransform(localCharacter));
            Vector3 currentCharacterPosition = get_Position(getTransform(currentCharacter));
            Vector3 currentEntDist = Vector3::Distance(localPosition, currentCharacterPosition);
            if (Vector3::Magnitude(currentEntDist) < closestEntDist) {
                closestEntDist = Vector3::Magnitude(currentEntDist);
                closestCharacter = currentCharacter;
            }
        }
    }
    return closestCharacter;
}


void (*oldMeeleHit)(void* obj, void* shooter, Ray ray);
void MeleeHit(void* obj, void* shooter, Ray ray){
    if(obj != nullptr && silentknife1 && esp_localCharacter != nullptr){
        if(shooter == esp_localCharacter && getValidEnt() != nullptr ){
            ray.origin = get_Position(getTransform(getValidEnt()));
            ray.direction = Vector3(0, 1, 0);
        }
    }
    return oldMeeleHit(obj, shooter, ray);
}

void GenerateHash(void *obj) {
    oldGenerateHash(obj);
    if (obj != nullptr) {
        *(monoString **) ((uint64_t) obj + string2Offset(OBFUSCATE("0x60"))) = CreateIl2cppString(OBFUSCATE("81C4D6F1A802B49339E4DCCADE4B1263"));
       // monoString* Hash = *(monoString**)((uint64_t) obj + 0x60);
       // LOGE("hash %s", Hash->getString().c_str());
    }
}

int framerate, maxframerate = 0;
void FPSMeter(void* obj){
    if(obj != nullptr){
        framerate = *(int*)((uint64_t) obj + string2Offset(OBFUSCATE("0x20")));
        if(framerate > maxframerate){
            maxframerate = framerate;
        }
    }
    oldFPSMeter(obj);
}

HOOKAF(void, Input, void *thiz, void *ex_ab, void *ex_ac) {
    origInput(thiz, ex_ab, ex_ac);
    ImGui_ImplAndroid_HandleInputEvent((AInputEvent *) thiz);
    return;
}

const char *combo_items[3] = {OBFUSCATE("head"), OBFUSCATE("chest"), OBFUSCATE("stomach")};

// Initilizers with patterns <3 NIGGAS GAYYYYYYYYYYYYY
void Hooks() {
    HOOK("0x13238B4", set_Spread, oldset_Spread);// OverlayScope.set_spread
    HOOK("0x1323594", RenderOverlayFlashbang, oldRenderOverlayFlashbang); // overlayflashbang.render
    HOOK("0x1322D50", RenderOverlaySmoke, oldRenderOverlaySmoke); // overlaysmoke.render
    HOOK("0x14E8E80", GameSystemUpdate, oldGameSystemUpdate); // GameSystem.Update
    HOOK("0x118896C", UpdateWeapon, oldUpdateWeapon); // character.UpdateWeapon
    HOOK("0x14D5B30", FovViewModel, oldFovViewModel); // CameraSettings.get_FovViewModel
    HOOK("0x14E4FAC", GameSystemDestroy, oGameSystemDestroy); // GameSystem.Destroy
    HOOK("0x14D5AF0", FovWorld, oldFovWorld); // CameraSettings.FovWorld
    HOOK("0x1185148", setRotation, oSetRotation);//Character.setRotation
    HOOK("0x16D42F8", get_Heightt, oldget_Height);//Transform.get_Height
    HOOK("0x19622F4", TouchControlsUpdate, oTouchControlsUpdate); // touchcontrols.update
    HOOK("0x1961C0C", TouchControlsDestroy, oTouchControlsDestroy); // touchcontrols.ondestroy
    HOOK("0x118693C", CheckCharacterVisiblity, oldCheckCharacterVisibility);
    HOOK("0x129E1AC", GenerateHash, oldGenerateHash);
    HOOK("0x14EB5F4", MeleeHit, oldMeeleHit);
    HOOK("0x182FC88", FPSMeter, oldFPSMeter);//FPSMeter.Update
    //   HOOK("0x16EFBC0", FindTheNiggasThatShouldBeSentToTheField, ReturnToFindingTheNiggasThatShouldBeSentToTheField);
//    HOOK("0x1687EC4", CreateEMessage, oldCM);
    //   HOOK("0x199430C", InGameChatMenu_Awake, oInGameChatMenu_Awake);
//    HOOK("0x19945A4", InGameChatMenuDestroy, oInGameChatMenuDestroy);
}


void Pointers() {
//RequestPurchaseSkin = (void(*)(void*, int, int, bool)) get_absolute_address(string2Offset(OBFUSCATE("0x1B80760")));
    get_Width = (int (*)()) get_absolute_address(
            string2Offset(OBFUSCATE("0x1669CDC"))); // screen.get_Width
    get_Height = (int (*)()) get_absolute_address(
            string2Offset(OBFUSCATE("0x1669D04"))); // screen.get_Height
    getAllCharacters = (monoList<void **> *(*)(void *)) get_absolute_address(
            string2Offset(OBFUSCATE("0x14E4010"))); // get_AllCharacters
    getLocalId = (int (*)(void *)) get_absolute_address(
            string2Offset(OBFUSCATE("0x14D7FF0"))); // get_LocalId
    getPlayer = (void *(*)(void *, int)) get_absolute_address(
            string2Offset(OBFUSCATE("0x14E7300"))); // GameSystem.GetPlayer
    getLocalPlayer = (void *(*)(void *)) get_absolute_address(
            string2Offset(OBFUSCATE("0x14D79D4"))); // GameSystem.get_LocalPlayer
    getCharacterCount = (int (*)(void *)) get_absolute_address(
            string2Offset(OBFUSCATE("0x14E4020"))); // GameSystem.get_CharacterCount
    get_Health = (int (*)(void *)) get_absolute_address(
            string2Offset(OBFUSCATE("0x11846B8"))); // get_Health
    get_Player = (void *(*)(void *)) get_absolute_address(
            string2Offset(OBFUSCATE("0x11846A0"))); // Gameplay.Character.get_Player
    get_IsInitialized = (bool (*)(void *)) get_absolute_address(
            string2Offset(OBFUSCATE("0x1184528"))); // Gameplay.Character.get_IsInitialized
    get_Position = (Vector3(*)(void *)) get_absolute_address(
            string2Offset(OBFUSCATE("0x1972FE8"))); // Transform.get_position
    get_camera = (void *(*)()) get_absolute_address(
            string2Offset(OBFUSCATE("0x16719A0"))); // camera.get_main
    WorldToScreen = (Vector3(*)(void *, Vector3, int)) get_absolute_address(
            string2Offset(OBFUSCATE("0x1670DA4"))); // WorldToScreenPoint
    //set_targetFrameRate = (void(*)(int)) get_absolute_address(""); NOT USED
    get_CharacterBodyPart = (void *(*)(void *, int)) get_absolute_address(
            string2Offset(OBFUSCATE("0x1184714"))); // Character.GetBodyPart
    get_LocalCharacter = (void *(*)(void *)) get_absolute_address(
            string2Offset(OBFUSCATE("0x14DA66C"))); // get_LocalCharacter
    isHeadBehindWall = (bool (*)(void *, void *)) get_absolute_address(
            string2Offset(OBFUSCATE("0x1332334"))); // IsHeadBehindWall
    get_FovWorld = (float (*)(void *)) get_absolute_address(
            string2Offset(OBFUSCATE("0x14D5AF0"))); // get_FovWorld
    getIsCrouched = (bool (*)(void *)) get_absolute_address(
            string2Offset(OBFUSCATE("0x1184540"))); // ISCrouched
    ScreenPointToRay = (Ray(*)(void *, Vector2, int)) get_absolute_address(
            string2Offset(OBFUSCATE("0x167147C"))); // ScreenPointToRay
    onInputButtons = (void (*)(void *, int, bool)) get_absolute_address(
            string2Offset(OBFUSCATE("0x1965114"))); // OnInputButton
    TraceShot = (void *(*)(void *, void *, Ray)) get_absolute_address(
            string2Offset(OBFUSCATE("0x14EB3C0"))); // TraceShot
    getDeviceUniqueIdentifier = (monoString *(*)()) get_absolute_address(
            string2Offset(OBFUSCATE("0x166D28C")));
    OpenURL = (void (*)(monoString *)) get_absolute_address(
            string2Offset(OBFUSCATE("0x19721C8")));//Application.OpenURL
    getTeamIndex = (int (*)(PlayerAdapter *))
            get_absolute_address(string2Offset(OBFUSCATE("0x12D79F4")));//PlayerAdapter.teamindex
    UpdateCharacterHitBuffer = (void (*)(void *, void *, Ray, int *))
            get_absolute_address(string2Offset(OBFUSCATE("0x14ED9D4")));//UpdateCharacterHitBuffer
    get_Username = (monoString *(*)(PlayerAdapter *))
            get_absolute_address(string2Offset(OBFUSCATE("0x12D7B3C")));//PlayerAdapter.get_Username
    get_ClanTag = (monoString *(*)(PlayerAdapter *))
            get_absolute_address(string2Offset(OBFUSCATE("0x12D7E8C")));//PlayerAdapter.get_ClanTag
    CreateMessage = (void *(*)(monoString *, ChatMessageType, bool))
            get_absolute_address(string2Offset(OBFUSCATE("0x1639768")));//RequestSendIngameChatMessage.Create
    SendMessage = (void (*)(void*))
            get_absolute_address(string2Offset(OBFUSCATE("0x183C114")));//InGameChatMenu.SendMessage
    get_Invulnerable = (bool (*)(void*))
            get_absolute_address(string2Offset(OBFUSCATE("0x124E9AC")));//Character.Invulnerable
}

void Patches() {
    PATCH_SWITCH("0x1250F18", "1F2003D5C0035FD6", spread);//UpdateSpread
    PATCH_SWITCH("0x1250EB4", "000080D2C0035FD6", aimpunch);//AimPunchRecover
    PATCH_SWITCH("0x14EBED4", "000080D2C0035FD6", ggod);//GrenadeHitCharacter
    PATCH_SWITCH("0xD98BD4", "200080D2C0035FD6", crosshair);//get_Crosshair
    PATCH_SWITCH("0x15803E8", "1F2003D5C0035FD6", smoke);//SmokeGrenadeEffect
    PATCH_SWITCH("0x14ED104", "1F2003D5", wallbang);//ProcessHitBuffers + 0xB8
    PATCH_SWITCH("0x14EDBF0", "01F0271E", headhitbox);//UpdateCahracterHitBuffer + 0x21C
    PATCH_SWITCH("0x14D8150", "E0031FAA",radar);//FetchFollowedCharacterTeamIndex + 0x84
    PATCH_SWITCH("0x14EBDFC", "01F0271E", silentknife);//MeleeHit + 0x808
    PATCH_SWITCH("0x11869C0", "1F01086B", tradar);//CheckCharacterVisibility + 0x84
    PATCH_SWITCH("0x16D13D0", "000080D2C0035FD6",tradar);//Linecast(Vector3 start, Vector3 end, int layerMask)
    PATCH_SWITCH("0x18435C8", "000080D2C0035FD6",killnotes);//KillNotification.Init(IPlayer killer, IPlayer victim, Sprite killerWeaponIcon, bool headshot)
    PATCH("0x12CAC38", "000080D2C0035FD6");//IsFirstMatchStageCompleted
    PATCH("0x12CAAC0", "000080D2C0035FD6");//t
    PATCH("0x12CB8D4", "000080D2C0035FD6");//t
    PATCH("0x12CADB0", "000080D2C0035FD6");//t
    PATCH("0x12CB6C8", "000080D2C0035FD6");//t
    PATCH("0x12CA9E0", "000080D2C0035FD6");//t

    //  PATCH("0x10D7444", "1F2003D5");
    // PATCH("0x1A71B50", "1F2003D5C0035FD6");
    //  PATCH("0x1A5C210", "1F2003D5C0035FD6");
    // PATCH("0x1A5D454", "1F2003D5C0035FD6");
    //  PATCH("0x1A71A84", "1F2003D5C0035FD6");
}

float get_cameraFov(void *pSys) {
    if(pSys != nullptr){
        void *GameplayModule = *(void **) ((uint64_t) pSys + 0x80);
        void *CameraSystem = *(void **) ((uint64_t) GameplayModule + 0x30);
        return *(float *) ((uint64_t) CameraSystem + 0x8C); // m_horizontalFieldOfView
    }
}

void ESP() {
    AimbotCfg cfg;
    std::lock_guard<std::mutex> guard(esp_mtx);
    if (pSys == nullptr || !(esp || pistolCfg.aimbot || shotgunCfg.aimbot || smgCfg.aimbot || arCFg.aimbot ||
          sniperCfg.aimbot)) {
        return;
    }

    int id = getLocalId(pSys);
    if (id == 0) {
        return;
    }

    if (pSys == nullptr) {
        return;
    }

    void *localPlayer = getPlayer(pSys, id);
    if (localPlayer == nullptr) {
        return;
    }

    int localTeam = get_PlayerTeam(localPlayer);
    monoList<void **> *characterList = getAllCharacters(pSys);
    if (characterList == nullptr) {
        return;
    }

    auto cam = get_camera();
    if (cam == nullptr) {
        return;
    }

    for (int i = 0; i < characterList->getSize(); i++) {
        void *currentCharacter = reinterpret_cast<monoList<void **> *>(characterList)->getItems()[i];
        if (currentCharacter == nullptr) {
            continue;
        }

        if (get_Player(currentCharacter) == localPlayer) {
            esp_localCharacter = currentCharacter;
        }

        if (esp_localCharacter == nullptr || currentCharacter == nullptr) {
            continue;
        }

        int curTeam = get_CharacterTeam(currentCharacter);
        int health = get_Health(currentCharacter);
        if (health <= 0 || localTeam == curTeam || curTeam == std::stoi(OBFUSCATE("-1"))) {
            continue;
        }

        void *transform = getTransform(currentCharacter);
        void *localTransform = getTransform(esp_localCharacter);
        if (transform == nullptr || localTransform == nullptr) {
            continue;
        }

        Vector3 position = get_Position(transform);
        Vector3 transformPos = WorldToScreen(cam, position, 2);
        transformPos.Y = glHeight - transformPos.Y;
        Vector3 headPos = getBonePosition(currentCharacter, HEAD);
        Vector3 chestPos = getBonePosition(currentCharacter, CHEST);
        Vector3 wschestPos = WorldToScreen(cam, chestPos, 2);
        Vector3 wsheadPos = WorldToScreen(cam, headPos, 2);
        Vector3 aboveHead = headPos + Vector3(0, 0.2, 0);
        Vector3 headEstimate = position + Vector3(0, 1.48, 0);


        Vector3 wsAboveHead = WorldToScreen(cam, aboveHead, 2);
        Vector3 wsheadEstimate = WorldToScreen(cam, headEstimate, 2);

        wsAboveHead.Y = glHeight - wsAboveHead.Y;
        wsheadEstimate.Y = glHeight - wsheadEstimate.Y;

        float height = transformPos.Y - wsAboveHead.Y;
        float width = (transformPos.Y - wsheadEstimate.Y) / 2;

        Vector3 localPosition = get_Position(localTransform);
        Vector3 currentCharacterPosition = get_Position(transform);
        float currentEntDist = Vector3::Distance(localPosition, currentCharacterPosition);

        espcfg = invisibleCfg;

        if (esp) {
            if (espcfg.snapline && transformPos.Z > 0) {
                DrawLine(ImVec2(glWidth / 2, glHeight),
                         ImVec2(transformPos.X, transformPos.Y),
                         ImColor(espcfg.snaplineColor.x,
                                 espcfg.snaplineColor.y,
                                 espcfg.snaplineColor.z,
                                 (255 - currentEntDist * 5.0)),
                         3);
            }

            if (espcfg.bone && transformPos.Z > 0) {
                DrawBones(currentCharacter, LOWERLEG_LEFT, UPPERLEG_LEFT,
                          espcfg);
                DrawBones(currentCharacter, LOWERLEG_RIGHT, UPPERLEG_RIGHT,
                          espcfg);
                DrawBones(currentCharacter, UPPERLEG_LEFT, STOMACH, espcfg);
                DrawBones(currentCharacter, UPPERLEG_RIGHT, STOMACH,
                          espcfg);
                DrawBones(currentCharacter, STOMACH, CHEST, espcfg);
                DrawBones(currentCharacter, LOWERARM_LEFT, UPPERARM_LEFT,
                          espcfg);
                DrawBones(currentCharacter, LOWERARM_RIGHT, UPPERARM_RIGHT,
                          espcfg);
                DrawBones(currentCharacter, UPPERARM_LEFT, CHEST, espcfg);
                DrawBones(currentCharacter, UPPERARM_RIGHT, CHEST, espcfg);
                Vector3 diff = wschestPos - wsheadPos;
                Vector3 neck = (chestPos + headPos) / 2;
                Vector3 wsneck = WorldToScreen(cam, neck, 2);
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

                    background->AddCircle(ImVec2(wsheadPos.X, wsheadPos.Y),
                                          radius / 2,
                                          IM_COL32(espcfg.boneColor.x * 255,
                                                   espcfg.boneColor.y * 255,
                                                   espcfg.boneColor.z * 255,
                                                   255),
                                          0, 3.0f);
                }
            }

            if (espcfg.box && transformPos.Z > 0 && wsAboveHead.Z > 0) {
                DrawOutlinedBox2(wsAboveHead.X - width / 2, wsAboveHead.Y, width,
                                 height,
                                 ImVec4(espcfg.boxColor.x,
                                        espcfg.boxColor.y,
                                        espcfg.boxColor.z, 255), 3);
            }

            if (espcfg.healthesp && transformPos.Z > 0 && wsAboveHead.Z > 0) {
                DrawOutlinedFilledRect(wsAboveHead.X - width / 2 - 12,
                                       wsAboveHead.Y + height * (1 -
                                                                 (static_cast<float>(health) /
                                                                  100.0f)),
                                       3,
                                       height *
                                       (static_cast<float>(health) /
                                        100.0f),
                                       HealthToColor(health));
            }

            if (espcfg.healthNumber && transformPos.Z > 0 && wsAboveHead.Z > 0) {
                if (health > 100) {
                    DrawText(ImVec2(wsAboveHead.X - width / 2 - 17,
                                    wsAboveHead.Y +
                                    height * (1 -
                                              static_cast<float>(health) /
                                              100.0f) -
                                    3),
                             ImVec4(1, 1, 1, 255),
                             std::to_string(health), espFont);
                }
            }

            if (espcfg.distance && transformPos.Z > 0) {
                DrawText(ImVec2(transformPos.X + width / 2, transformPos.Y - 12),
                         ImVec4(1, 1, 1, 255),
                         std::to_string(static_cast<int>(currentEntDist)) + "m", espFont);
            }

            if (espcfg.name && transformPos.Z > 0 && currentCharacter != nullptr) {
                void *player = get_Player(currentCharacter);
                if (player == nullptr)
                    continue;
                std::string username = get_PlayerUsername(player);
                float compensation = strlen(username.c_str()) * 4.0f;
                DrawText(ImVec2(wsheadPos.X - compensation, wsAboveHead.Y - 20),
                         ImVec4(espcfg.nameColor.x, espcfg.nameColor.y, espcfg.nameColor.z, 255),
                         username, espFont);
            }
        }
    }
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

            if (cfg.drawFov) {
                float worldFov = get_cameraFov(pSys);
                float radius =
                        tan(cfg.fovValue * (PI / 180) / 2) / tan(90 / 2) *
                        glWidth /
                        2;
                auto background = ImGui::GetBackgroundDrawList();
                float correction = 0;
                if ((worldFov - 90) < 0) {
                    correction = worldFov - 90;
                }

                background->AddCircle(ImVec2(glWidth / 2, glHeight / 2),
                                      (radius) * PI - correction / PI * 2,
                                      IM_COL32(255, 255, 255, 255), 0, 3.0f);
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

static std::string keyStatus = OBFUSCATE("Checking in: ");
size_t writeFunction(void* ptr, size_t size, size_t nmemb, std::string* data) {
    data->append(static_cast<char*>(ptr), size * nmemb);
    return size * nmemb;
}

std::optional<std::string> get_for_thread(std::string url) {
    std::string result;


    std::string response;
    auto* handle = curl_easy_init();
    curl_easy_setopt(handle, CURLOPT_USE_SSL, CURLUSESSL_ALL);
    curl_easy_setopt(handle, CURLOPT_SSL_VERIFYPEER, 0);
    curl_easy_setopt(handle, CURLOPT_URL, (url).c_str());
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, writeFunction);
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt(handle, CURLOPT_MAXREDIRS, 10);

    auto success = curl_easy_perform(handle);

    curl_easy_cleanup(handle);

    if (success == CURLE_OK)
        result = response;
    else
        return std::nullopt;

    return result;
}
static bool authenticated = false, is_key_found = false;
std::string notauth = OBFUSCATE("false");
void loop_authenticate() {
    std::thread([] {
        std::string primetools_key = (*get_for_thread(std::string(OBFUSCATE("https://spdmteam.com/api/primetools/isauth?hwid=")) + getDeviceUniqueIdentifier()->getString()));
        std::vector<std::string> lines;
        std::istringstream iss(primetools_key);
        if (primetools_key.size()) {
            std::string line;
            while (std::getline(iss, line)) {
                if (line != std::string(OBFUSCATE("false"))) {
                    trollage = string2Offset(OBFUSCATE("0x124AC5"));
                    auto json = nlohmann::json::parse(line);
                    authenticated = json[std::string(OBFUSCATE("authenticated"))];

                    is_key_found = true;
                }
            }
        }
        while (!authenticated) {
            static int countdown_timer = 15;
            if (!countdown_timer) {
                countdown_timer = 15;
                std::string primetools_key = (*get_for_thread(std::string(OBFUSCATE("https://spdmteam.com/api/primetools/isauth?hwid=")) +
                        getDeviceUniqueIdentifier()->getString()));

                std::vector<std::string> lines;
                std::istringstream iss(primetools_key);
                if (primetools_key.size()) {
                    std::string line;
                    while (std::getline(iss, line)) {
                        if (line != notauth) {
                            trollage = string2Offset(OBFUSCATE("0x124AC5"));
                            auto json = nlohmann::json::parse(line);
                            authenticated = json[std::string(OBFUSCATE("authenticated"))];
                            is_key_found = true;
                        }
                    }
                }
            }
            for (int i = countdown_timer; i > 0; i--) {
                if (is_key_found) break;
                std::this_thread::sleep_for(std::chrono::seconds(1));
                countdown_timer--;
                keyStatus = std::string(std::string (OBFUSCATE("Checking in: ")) + std::to_string(countdown_timer) + std::string(OBFUSCATE("s")));
            }

        }
    }).detach();
}

bool isAuth(){
    if(trollage == 0x124AC5){
        return true;
    }
    return false;
}


void DrawKeySystemMenu() {

    static std::once_flag loop_thread;
    std::call_once(loop_thread, loop_authenticate);
    ImGui::Begin(OBFUSCATE("keysystemMenu"), nullptr, ImGuiWindowFlags_NoDecoration);
    auto draw = ImGui::GetWindowDrawList();
    ImVec2 pos = ImGui::GetWindowPos();
    ImGui::SetWindowSize(ImVec2(600, 100));
    ImVec2 size = ImGui::GetWindowSize();

    ImGui::SetCursorPos(ImVec2(20, 20));
    ImGui::TextUnformatted(OBFUSCATE("Key System"));
    ImGui::Separator();
    ImGui::SetCursorPos(ImVec2(20, 70));
    ImGui::TextUnformatted(OBFUSCATE("Complete the Key System to access PrimeTools."));
    ImGui::SetCursorPos(ImVec2(20, 120));
    ImGui::TextUnformatted(OBFUSCATE("Key Status: "));
    ImGui::SetCursorPos(ImVec2(100, 120));
    ImGui::TextUnformatted(keyStatus.c_str());

    ImGui::SetCursorPos(ImVec2(15, 170));
    if (ImGui::Button(OBFUSCATE("Get Key"), ImVec2(200, 70))) {
        openurls = true;
    }

    ImGui::SetCursorPos(ImVec2(20, 300));
    if(uptodate)
        ImGui::TextUnformatted(OBFUSCATE("HWID : "));
        ImGui::SetCursorPos(ImVec2(20, 350));
        ImGui::TextUnformatted(getDeviceUniqueIdentifier()->getString().c_str());

    ImGui::End();
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
        if (custom_interface::tab(OBFUSCATE("A"), 0 == selected)) {
            selected = 0;
            sub_selected = 10;
        }
        if (custom_interface::tab(OBFUSCATE("V"), 1 == selected)) {
            selected = 1;
            sub_selected = 1;
        }
        if (custom_interface::tab(OBFUSCATE("M"), 2 == selected)) {
            selected = 2;
            sub_selected = 2;
        }
        if (custom_interface::tab(OBFUSCATE(" -"), 3 == selected)) {
            hidden = true;
        }
        ImGui::EndGroup();

        if (selected == 0) {
            ImGui::BeginGroup();
            ImGui::SetCursorPos(ImVec2(65, 14));
            if (custom_interface::subtab(OBFUSCATE("Rifle"), 10 == sub_selected)) {
                sub_selected = 10;
            }
            ImGui::SameLine();
            if (custom_interface::subtab(OBFUSCATE("SMG"), 11 == sub_selected)) {
                sub_selected = 11;
            }
            ImGui::SameLine();
            if (custom_interface::subtab(OBFUSCATE("Pistol"), 12 == sub_selected)) {
                sub_selected = 12;
            }
            ImGui::SameLine();
            if (custom_interface::subtab(OBFUSCATE("Shotgun"), 13 == sub_selected)) {
                sub_selected = 13;
            }
            ImGui::SameLine();
            if (custom_interface::subtab(OBFUSCATE("Sniper"), 14 == sub_selected)) {
                sub_selected = 14;
            }
            ImGui::EndGroup();
        }

        if (sub_selected == 10) {
            ImGui::SetCursorPos(ImVec2(67, 74));
            ImGui::BeginChild(OBFUSCATE("Aim Assistance"), ImVec2(405.5, 406));
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
            ImGui::BeginChild(OBFUSCATE("Triggerbot"), ImVec2(405.5, 406));
            {
                ImGui::Checkbox(OBFUSCATE("Triggerbot"), &arCFg.triggerbot);
            }
            ImGui::EndChild();
        }

        if (sub_selected == 11) {
            ImGui::SetCursorPos(ImVec2(67, 74));
            ImGui::BeginChild(OBFUSCATE("Aim Assistance"), ImVec2(405.5, 406));
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
            ImGui::BeginChild(OBFUSCATE("Triggerbot"), ImVec2(405.5, 406));
            {
                ImGui::Checkbox(OBFUSCATE("Triggerbot"), &smgCfg.triggerbot);
            }
            ImGui::EndChild();
        }
        if (sub_selected == 12) {
            ImGui::SetCursorPos(ImVec2(67, 74));
            ImGui::BeginChild(OBFUSCATE("Aim Assistance"), ImVec2(405.5, 406));
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
            ImGui::BeginChild(OBFUSCATE("Triggerbot"), ImVec2(405.5, 406));
            {
                ImGui::Checkbox(OBFUSCATE("Triggerbot"), &pistolCfg.triggerbot);
            }
            ImGui::EndChild();
        }

        if (sub_selected == 13) {
            ImGui::SetCursorPos(ImVec2(67, 74));
            ImGui::BeginChild(OBFUSCATE("Aim Assistance"), ImVec2(405.5, 406));
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
            ImGui::BeginChild(OBFUSCATE("Triggerbot"), ImVec2(405.5, 406));
            {
                ImGui::Checkbox(OBFUSCATE("Triggerbot"), &shotgunCfg.triggerbot);
            }
            ImGui::EndChild();
        }

        if (sub_selected == 14) {
            ImGui::SetCursorPos(ImVec2(67, 74));
            ImGui::BeginChild(OBFUSCATE("Aim Assistance"), ImVec2(405.5, 406));
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
            ImGui::BeginChild(OBFUSCATE("Triggerbot"), ImVec2(405.5, 406));
            {
                ImGui::Checkbox(OBFUSCATE("Triggerbot"), &sniperCfg.triggerbot);
            }
            ImGui::EndChild();
        }

        if (selected == 1) {
            ImGui::BeginGroup();
            ImGui::SetCursorPos(ImVec2(65, 14));
            if (custom_interface::subtab(OBFUSCATE("Visuals"), 1 == sub_selected)) {
                sub_selected = 1;
            }
            ImGui::SameLine();
            if (custom_interface::subtab(OBFUSCATE("Configuration"), 8 == sub_selected)) {
                sub_selected = 8;
            }
            ImGui::EndGroup();
        }


        if (sub_selected == 1) {
            ImGui::SetCursorPos(ImVec2(67, 74));
            ImGui::BeginChild(OBFUSCATE("ESP"), ImVec2(405.5, 406));
            {
                ImGui::Checkbox(OBFUSCATE("ESP Main Toggle"), &esp);
                ImGui::Checkbox(OBFUSCATE("Chams"), &chams);
                ImGui::Checkbox(OBFUSCATE("Bones"), &invisibleCfg.bone);
                ImGui::Checkbox(OBFUSCATE("Boxes"), &invisibleCfg.box);
                ImGui::Checkbox(OBFUSCATE("Snaplines"), &invisibleCfg.snapline);
                ImGui::Checkbox(OBFUSCATE("Names"), &invisibleCfg.name);
                ImGui::Checkbox(OBFUSCATE("Distance"), &invisibleCfg.distance);
                //   ImGui::Checkbox(OBFUSCATE("Weapons"), &invisibleCfg.weapon);
                ImGui::Checkbox(OBFUSCATE("Health"), &invisibleCfg.healthesp);
                if (&invisibleCfg.healthesp) {
                    ImGui::Checkbox(OBFUSCATE("Health Numbers"), &invisibleCfg.healthNumber);
                }
                //ImGui::Checkbox(OBFUSCATE(" Show Weapon"), &weaponEsp);
                ImGui::EndChild();

                ImGui::SetCursorPos(ImVec2(481.5, 74));
                ImGui::BeginChild(OBFUSCATE("Other"), ImVec2(405.5, 406));
                {
                    if (ImGui::Checkbox(OBFUSCATE("Radar"), &radar)) { Patches(); }
                    if (ImGui::Checkbox(OBFUSCATE("Team Radar"), &tradar)) { Patches(); }
                    ImGui::Checkbox(OBFUSCATE("No Flash"), &flash);
                    ImGui::Checkbox(OBFUSCATE("No Smoke"), &smoke);
                    ImGui::Checkbox(OBFUSCATE("View Model FOV"), &viewmodelfov);
                    if (viewmodelfov) {
                        ImGui::SliderFloat(OBFUSCATE(" · Viewmodel Value"), &viewmodelfovval, 1.0,
                                           220.0);
                    }
                    ImGui::Checkbox(OBFUSCATE("Field Of View"), &fov);
                    if (fov) {
                        ImGui::SliderFloat(OBFUSCATE(" · FOV Value"), &fovModifier, 1.0, 220.0);
                    }
                    if (ImGui::Checkbox(OBFUSCATE("Force Crosshair"), &crosshair)) { Patches(); }
                    if (ImGui::Checkbox(OBFUSCATE("No Aimpunch"), &aimpunch)) { Patches(); }
                    ImGui::Checkbox(OBFUSCATE("Nazi Crosshair"), &p100Crosshair);
                    if (ImGui::Checkbox(OBFUSCATE("Spoof Kill Notifications"), &killnotes)) { Patches(); }
                    ImGui::Checkbox(OBFUSCATE("No Camera Shake"), &shake);

                    ImGui::Checkbox(OBFUSCATE("No Scope Overlay"), &scope);
                }
                ImGui::EndChild();
            }
        }

        if (sub_selected == 8) {
            ImGui::SetCursorPos(ImVec2(67, 74));
            ImGui::BeginChild(OBFUSCATE("ESP Configuration"), ImVec2(811, 406));
            ImGui::ColorEdit4(OBFUSCATE("Snapline Color"), &invisibleCfg.snaplineColor.x);
            ImGui::ColorEdit4(OBFUSCATE("Bone Color"), &invisibleCfg.boneColor.x);
            ImGui::ColorEdit4(OBFUSCATE("Box Color"), &invisibleCfg.boxColor.x);
            ImGui::ColorEdit4(OBFUSCATE("Name Color"), &invisibleCfg.nameColor.x);
            ImGui::ColorEdit4(OBFUSCATE("Weapon Color"), &invisibleCfg.weaponColor.x);
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
                ImGui::BeginChild(OBFUSCATE("Legit"), ImVec2(405.5, 406));
                ImGui::Checkbox(OBFUSCATE("Recoil"), &recoil);
                if (recoil) {
                    ImGui::SliderFloat(OBFUSCATE(" Recoil"), &recoilval, 1.0, 10.0);
                }
                if (ImGui::Checkbox(OBFUSCATE("Spread"), &spread)) { Patches(); }
                ImGui::Checkbox(OBFUSCATE("Force Scope"), &fscope);
                ImGui::Checkbox(OBFUSCATE("Burst Shots"), &burstfire);
                ImGui::EndChild();

                ImGui::SetCursorPos(ImVec2(481.5, 74));
                ImGui::BeginChild(OBFUSCATE("Rage"), ImVec2(405.5, 406));
                if (ImGui::Checkbox(OBFUSCATE("Expand Hitbox"), &headhitbox)) { Patches(); };
                if (ImGui::Checkbox(OBFUSCATE("Wallbang"), &wallbang)) { Patches(); }
                ImGui::Checkbox(OBFUSCATE("Firerate"), &firerate);
                ImGui::Checkbox(OBFUSCATE("Instant Pick-up"), &pickup);
                if (ImGui::Checkbox(OBFUSCATE("Always Backstab"), &silentknife)) { Patches(); };
                if (ImGui::Checkbox(OBFUSCATE("Long Knife Range"), &silentknife1)) { Patches(); };
                ImGui::EndChild();
            }

            if (sub_selected == 3) {
                ImGui::SetCursorPos(ImVec2(67, 74));
                ImGui::BeginChild(OBFUSCATE("Player Modifications"), ImVec2(813, 406));
                ImGui::Checkbox(OBFUSCATE("Speed"), &speed);
                if (speed) {
                    ImGui::SliderFloat(OBFUSCATE(" · Speed"), &speedval, 0.0, 20.0);
                }
                ImGui::Checkbox(OBFUSCATE("Jump Force"), &jumpheight);
                if (jumpheight) {
                    ImGui::SliderFloat(OBFUSCATE(" · Force"), &jumpval, 0.0, 7.0);
                }
                ImGui::Checkbox(OBFUSCATE("Player Height"), &fly);
                if (fly) {
                    ImGui::SliderFloat(OBFUSCATE(" · Height"), &flyval, 0.0, 6.0);
                }
                ImGui::Checkbox(OBFUSCATE("No Slow-Down"), &noslow);
                ImGui::EndChild();
            }
        }
    }
    ImGui::End();
}

void HiddenCircle(){
    auto draw = ImGui::GetBackgroundDrawList();
    ImVec2 pos = ImVec2(49.99999, 49.99999);
    ImVec2 cursorPos = ImGui::GetCursorScreenPos();
    ImVec2 center = ImVec2(49.99999, 49.99999);
    float radius = 50;
    ImGui::SetNextWindowSize(ImVec2(50, 90));
    draw->AddCircleFilled(ImVec2(50, 50), radius, ImColor(255, 255, 255, 70), 32);

    DrawText(ImVec2(23, 34), ImVec4(255, 255, 255, 70), OBFUSCATE("Show"), espFont);
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        ImVec2 mouse_pos = ImGui::GetMousePos();
        float distance = sqrtf((mouse_pos.x - center.x) * (mouse_pos.x - center.x) + (mouse_pos.y - center.y) * (mouse_pos.y - center.y));
        if (distance <= radius) {
            hidden = !hidden;
        }
    }
}

void SetupImgui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    if(uptodate){
        glWidth = get_Width();
        glHeight = get_Height();
    }
    else{
        glWidth = std::stoi(OBFUSCATE("1920"));
        glHeight = std::stoi(OBFUSCATE("1080"));
    }

    io.DisplaySize = ImVec2((float) glWidth, (float) glHeight);
    ImGui_ImplOpenGL3_Init(OBFUSCATE("#version 100"));
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


    if(isAuth() && !ready && uptodate && authenticated){
        Hooks();
        Patches();
        ready = true;
    }



    if(uptodate && authenticated){
            ESP();
            if(!hidden) {
                DrawMenu();
            }
            else{
                HiddenCircle();
            }
    }
    else if(uptodate){
        DrawKeySystemMenu();
    }

    if(maxframerate > 62 && uptodate){
        DrawText(ImVec2(10, 40), ImVec4(220, 20, 60, 255), OBFUSCATE("You're playing over 60 fps which may cause crashes, Lower the FPS in settings and restart"), espFont);
    }

    if(uptodate) {
        if (!hidden)
            DrawText(ImVec2(10, 10), ImVec4(255, 255, 255, 255), OBFUSCATE("PrimeTools BETA"), espFont);
    }
    else{
        DrawText(ImVec2(100, 200), ImVec4(255, 255, 255, 255), OBFUSCATE("Your module version is outdated, Update here : https://discord.gg/8uxxYSn7") ,espFont);
    }

    ImGui::EndFrame();
    ImGui::Render();
    glViewport(0, 0, (int) glWidth, (int) glHeight);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    return old_eglSwapBuffers(dpy, surface);
}


GLint (*old_glGetUniformLocation)(GLuint program, const GLchar *name);
GLint glGetUniformLocation(GLuint program, const GLchar *name) // returns location of a shader/uniform.
{
    return old_glGetUniformLocation(program, name);
}

bool Shaders() {
    GLint program;
    glGetIntegerv(GL_CURRENT_PROGRAM, &program);
    return old_glGetUniformLocation(program, Shader) != -1;
}

void (*old_glDrawElements)(GLenum mode, GLsizei count, GLenum type, const void *indices);

void glDrawElements(GLenum mode, GLsizei count, GLenum type, const void *indices) {
    old_glDrawElements(mode, count, type, indices);

    if (Shaders() > 0 && chams) {
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
        glBlendColor(0, 0, 0, 0);
        old_glDrawElements(mode, count, type, indices);
        glDepthRangef(0.5, 1);
        glColorMask(1, 1, 1, 1);
        glDisable(GL_BLEND);
    }
}

void *hack_thread(void *arg) {
    do {
        sleep(1);
        auto maps = KittyMemory::getMapsByName(OBFUSCATE("split_config.arm64_v8a.apk"));
        for (std::vector<ProcMap>::iterator it = maps.begin(); it != maps.end(); ++it) {
            auto address = KittyScanner::findHexFirst(it->startAddress, it->endAddress,
                                                      OBFUSCATE("7F 45 4C 46 02 01 01 00 00 00 00 00 00 00 00 00 03 00 B7 00 01 00 00 00 B0 05 67 00 00 00 00 00 40 00 00 00 00 00 00 00 00 51 3B 02 00 00 00 00"),
                                                      OBFUSCATE("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"));
            if (address != 0) {
                libBaseAddress = address;
                libBaseEndAddress = it->endAddress;
                libSize = it->length;
            }
        }
    } while (libBaseAddress == 0);
    auto get_version = (*get_for_thread(std::string(OBFUSCATE("https://cdn.discordapp.com/attachments/1102979543200956538/1103007574137241731/v.txt"))));
    std::vector<std::string> lines;
    std::istringstream iss(get_version);
    if (get_version.size()) {
        std::string line;
        while (std::getline(iss, line)) {
            if (line != std::string(OBFUSCATE("1.38.0f2184"))) {
                uptodate = false;
            }
        }
    }
    if(uptodate){
        Pointers();
        HOOK("0x12360B0", LoadSettings, oldLoadSettings);
        HOOK("0x1758DBC", AppManager, oldAppManager);
    }

    auto eglhandle = dlopen(OBFUSCATE("libunity.so"), RTLD_LAZY);
    auto eglSwapBuffers = dlsym(eglhandle, OBFUSCATE("eglSwapBuffers"));
    auto renderHandle = dlopen(OBFUSCATE("/system/lib64/libGLESv2.so"), RTLD_LAZY);
    auto glDrawElement = dlsym(renderHandle, OBFUSCATE("glDrawElements"));
    auto glGetUniformLocations = dlsym(renderHandle, OBFUSCATE("glGetUniformLocation"));
    DobbyHook((void *) eglSwapBuffers, (void *) hook_eglSwapBuffers, (void **) &old_eglSwapBuffers);
    DobbyHook((void *) glDrawElement, (void *) glDrawElements, (void **) &old_glDrawElements);
    DobbyHook((void *) glGetUniformLocations, (void *) glGetUniformLocation, (void **) &old_glGetUniformLocation);
    void *sym_input = DobbySymbolResolver((OBFUSCATE("/system/lib/libinput.so")),
                                          (OBFUSCATE("_ZN7android13InputConsumer21initializeMotionEventEPNS_11MotionEventEPKNS_12InputMessageE")));

    if (NULL != sym_input) {
        DobbyHook(sym_input, (void *) myInput, (void **) &origInput);
    }

    while (true) {
        if (p100Crosshair) {
            if (crosshairRotation > 89) {
                crosshairRotation = 0;
            }
            if (crosshairRotation1 > 134) {
                crosshairRotation1 = 45;
            }
            crosshairRotation++;
            crosshairRotation1++;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    return nullptr;
}

void *triggerbot_thread(void *arg) {
    while (true) {
        if (TouchControls && shootControl && esp_localCharacter && get_Health(esp_localCharacter)) {
            onInputButtons(TouchControls, 7, 1);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            onInputButtons(TouchControls, 7, 0);
            shootControl = 0;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return nullptr;
}

