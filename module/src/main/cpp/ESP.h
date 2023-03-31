//
// Created by 0x7is on 04/03/2023.
//

#ifndef COPS_MENU_ESP_H
#define COPS_MENU_ESP_H

int glHeight;
int glWidth;
float crosshairRotation = 45.0f;
float crosshairRotation1 = 0.0f;

void DrawLine(ImVec2 start, ImVec2 end, ImVec4 color, int thickness) {
    auto background = ImGui::GetBackgroundDrawList();
    if(background) {
        background->AddLine(start, end, ImColor(color.x,color.y,color.z,color.w), thickness);
    }
}
void DrawBox(ImVec4 rect, ImVec4 color, int thickness) {
    ImVec2 v1(rect.x, rect.y);
    ImVec2 v2(rect.x + rect.z, rect.y);
    ImVec2 v3(rect.x + rect.z, rect.y + rect.w);
    ImVec2 v4(rect.x, rect.y + rect.w);

    DrawLine(v1, v2, color, thickness);
    DrawLine(v2, v3, color, thickness);
    DrawLine(v3, v4, color, thickness);
    DrawLine(v4, v1, color, thickness);
}

void DrawBox2(float x, float y, float width, float height, ImColor color, int thickness)
{
    auto background = ImGui::GetBackgroundDrawList();
    if(background) {
        background->AddRect(ImVec2(x, y), ImVec2(x + width ,y + height), color,0, ImDrawCornerFlags_All, thickness);
    }
}

void DrawOutlinedBox2(float x, float y, float width, float height, ImVec4 color, int thickness)
{
    DrawBox2(x-thickness+1, y-thickness+1, width + thickness*2 -2, height + thickness*2 -2, ImVec4(0,0,0,color.w), 1);
    DrawBox2(x, y, width, height, color, thickness);
    DrawBox2(x + thickness -1, y + thickness -1, width - thickness*2 + 2, height - thickness*2 +2, ImVec4(0,0,0,color.w), 1);
}

ImVec4 HealthToColor(int health)
{
    ImVec4 color;
    if (health > 50)
    {
        color.x = (100 - health) * 5.1;
        color.y = 255;
    }
    if (health < 50){
        color.x = 255;
        color.y = (health) * 5.1;
    }
    if (health == 50)
    {
        color.x = 255;
        color.y = 255;
    }
    color.z = 0;
    color.w = 255;

    return color;
}


void DrawOutlinedBox(ImVec4 rect, ImVec4 color, int thickness)
{
    DrawBox(ImVec4(rect.x + 1, rect.y + 1, rect.z - 1, rect.w - 1), ImVec4(0,0,0,color.w), 1);
    DrawBox(ImVec4(rect), color, thickness);
    DrawBox(ImVec4(rect.x - 1, rect.y - 1, rect.z + 1, rect.w + 1), ImVec4(0,0,0,color.w), 1);
}

void DrawFilledRect(float x, float y, float width, float height, ImVec4 color)
{
    auto background = ImGui::GetBackgroundDrawList();
    if(background) {
        background->AddRectFilled(ImVec2(x,y), ImVec2(x+width,y + height), IM_COL32(color.x,color.y,color.z, color.w));
    }
}

void DrawOutlinedFilledRect(float x, float y, float width, float height, ImVec4 color)
{
    DrawFilledRect(x,y,width,height,color);
    DrawBox2(x-1, y-1, width + 2, height + 2, ImVec4(0,0,0,color.w), 1);
}

/*
void DrawCircle(float X, float Y, float radius, bool filled, IM_COL32, int tesselation, float thickness) {
    auto background = ImGui::GetBackgroundDrawList();
    if(background) {
        if(filled) {
            background->AddCircleFilled(ImVec2(X, Y), radius, ImColor(color.x,color.y,color.z,color.w), tesselation);
        } else {
            background->AddCircle(ImVec2(X, Y), radius, IM_COL32(color.x,color.y,color.z,color.w), tesselation, thickness);
        }
    }
}*/

void DrawBones(void* character, int bone1, int bone2, ESPCfg cfg){
    Vector3 bone1Pos = getBonePosition(character, bone1);
    Vector3 bone2Pos = getBonePosition(character, bone2);
    Vector3 wsbone1 = WorldToScreen(get_camera(), bone1Pos, 2);
    Vector3 wsbone2 = WorldToScreen(get_camera(), bone2Pos, 2);
    wsbone1.Y = glHeight - wsbone1.Y;
    wsbone2.Y = glHeight - wsbone2.Y;
    if(wsbone1.Z > 0 && wsbone2.Z > 0){
        DrawLine(ImVec2(wsbone1.X, wsbone1.Y), ImVec2(wsbone2.X, wsbone2.Y), ImColor(cfg.boneColor.x, cfg.boneColor.y, cfg.boneColor.z), 3);
    }
}



void DrawText(ImVec2 position, ImVec4 color, std::string text, ImFont* font) {
    auto background = ImGui::GetBackgroundDrawList();
    if(background) {
        ImGuiContext& g = *GImGui;
        ImGui::PushFont(font);
        background->AddText(g.Font, g.FontSize, ImVec2(position.x -2, position.y -2), IM_COL32(0,0,0,color.w), text.c_str());
        background->AddText(g.Font, g.FontSize, ImVec2(position.x, position.y -2), IM_COL32(0,0,0,color.w), text.c_str());
        background->AddText(g.Font, g.FontSize, ImVec2(position.x +2, position.y -2), IM_COL32(0,0,0,color.w), text.c_str());
        background->AddText(g.Font, g.FontSize, ImVec2(position.x +2, position.y ), IM_COL32(0,0,0,color.w), text.c_str());
        background->AddText(g.Font, g.FontSize, ImVec2(position.x +2, position.y +2), IM_COL32(0,0,0,color.w), text.c_str());
        background->AddText(g.Font, g.FontSize, ImVec2(position.x , position.y +2), IM_COL32(0,0,0,color.w), text.c_str());
        background->AddText(g.Font, g.FontSize, ImVec2(position.x -2, position.y +2), IM_COL32(0,0,0,color.w), text.c_str());
        background->AddText(g.Font, g.FontSize, ImVec2(position.x -2, position.y ), IM_COL32(0,0,0,color.w), text.c_str());
        background->AddText(g.Font, g.FontSize, position, ImColor(color.x,color.y,color.z,color.w), text.c_str());
        ImGui::PopFont();
    }
}

void DrawText(ImVec2 position, ImVec4 color, std::u16string text, ImFont* font) {
    auto background = ImGui::GetBackgroundDrawList();
    if(background) {
        ImGuiContext& g = *GImGui;
        ImGui::PushFont(font);
        background->AddText(g.Font, g.FontSize, ImVec2(position.x -2, position.y -2), IM_COL32(0,0,0,color.w), (const char*)text.c_str());
        background->AddText(g.Font, g.FontSize, ImVec2(position.x +2, position.y -2), IM_COL32(0,0,0,color.w), (const char*) text.c_str());
        background->AddText(g.Font, g.FontSize, ImVec2(position.x +2, position.y +2), IM_COL32(0,0,0,color.w), (const char*)(text.c_str()));
        background->AddText(g.Font, g.FontSize, ImVec2(position.x -2, position.y +2), IM_COL32(0,0,0,color.w), (const char*)text.c_str());
        background->AddText(g.Font, g.FontSize, position, ImColor(color.x,color.y,color.z,color.w), (const char*) text.c_str());
        ImGui::PopFont();
    }
}

float BOG_TO_GRD(float BOG) {
    return (180 / M_PI) * BOG;
}

float GRD_TO_BOG(float GRD) {
    return (M_PI / 180) * GRD;
}

void CoolCrosshair(ImVec2 position, ImVec4 color) {

    int a = ((int)glHeight / 60);

    int Drehungswinkel = crosshairRotation;
    int Drehungswinkel1 = crosshairRotation1;

    int i = 0;
    while (i < 4) {
        std::vector<int> p;
        p.push_back(a * sin(GRD_TO_BOG(Drehungswinkel + (i * 90))));                                    //p[0]		p0_A.x
        p.push_back(a * cos(GRD_TO_BOG(Drehungswinkel + (i * 90))));                                    //p[1]		p0_A.y
        p.push_back(a * sin(GRD_TO_BOG(Drehungswinkel1 + (i * 90))));                                    //p[0]		p0_A.x
        p.push_back(a * cos(GRD_TO_BOG(Drehungswinkel1 + (i * 90))));
        DrawLine(ImVec2(position.x, position.y), ImVec2(position.x + p[0], position.y - p[1]),
                 color, 3);


        DrawLine(ImVec2(position.x + p[0], position.y - p[1]), ImVec2(position.x + sqrt(2)*p[2], position.y - sqrt(2)*p[3]),
                 color, 3);
        i++;
    }
}




#endif //COPS_MENU_ESP_H
