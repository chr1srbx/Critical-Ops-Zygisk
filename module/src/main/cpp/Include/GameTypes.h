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

#endif //COPS_MENU_RAY_H
