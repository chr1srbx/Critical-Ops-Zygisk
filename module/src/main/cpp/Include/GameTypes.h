//
// Created by 0x7is on 09/03/2023.
//

#ifndef COPS_MENU_RAY_H
#define COPS_MENU_RAY_H

struct Ray
{
    Vector3 origin;
    Vector3 direction;
};

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

enum WeaponCategory
{
    Pistol,
    AssaultRifle,
    SubmachineGun,
    Shotgun,
    SniperRifle,
    Melee,
    Grenade,
    Utility
};

struct ESPCfg
{
    bool snapline = 0;
    ImVec4 snaplineColor = ImColor(255,255,255);
    bool bone = 0;
    ImVec4 boneColor = ImColor(255,255,255);
    bool box = 0;
    ImVec4 boxColor = ImColor(255,255,255);
    bool healthesp = 0;
    bool healthNumber = 0;
    bool name = 0;
    ImVec4 nameColor = ImColor(255,255,255);
    bool weapon = 0;
    ImVec4 weaponColor = ImColor(255,255,255);
};

struct AimbotCfg
{
    bool aimbot = 0;
    bool visCheck = 0;
    BodyPart aimBone = STOMACH;
    bool aimbotSmooth = 0;
    float smoothAmount = 1.0f;
    bool fovCheck = 0;
    float fovValue = 0.0f;
    bool drawFov = 0;
    bool onShoot = 0;
    bool triggerbot = 0;
};

struct CustomWeapon
{
    void* EGID = 0;
    int liveId = -1;
    int weaponDefId = -1;
};


struct Color
{
    float r;
    float g;
    float b;
    float a;
};

#endif //COPS_MENU_RAY_H
