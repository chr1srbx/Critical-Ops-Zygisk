//
// Created by 0x7is on 04/03/2023.
//

#ifndef COPS_MENU_ESP_H
#define COPS_MENU_ESP_H

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
        background->AddRect(ImVec2(x - width/2, y), ImVec2(x + width/2, y + height), color,0, ImDrawCornerFlags_All, thickness);
    }
}

void DrawOutlinedBox2(float x, float y, float width, float height, ImVec4 color, int thickness)
{
    DrawBox2(x-1, y-1, width + 2, height + 2, ImVec4(0,0,0,color.w), 1);
    DrawBox2(x, y, width, height, color, thickness);
    DrawBox2(x + thickness, y + thickness, width - thickness*2, height - thickness*2, ImVec4(0,0,0,color.w), 1);
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
    else
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

void DrawFilledRect(ImVec4 rect, ImVec4 color)
{
    auto background = ImGui::GetBackgroundDrawList();
    if(background) {
        // background->AddRectFilled(ImVec2(rect.x,rect.y), ImVec2(rect.z,rect.w), color);
    }
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
void DrawText(ImVec2 position, ImVec4 color, std::string text, ImFont* font) {
    auto background = ImGui::GetBackgroundDrawList();
    if(background) {
        ImGuiContext& g = *GImGui;
        ImGui::PushFont(font);
        background->AddText(g.Font, g.FontSize, ImVec2(position.x -2, position.y -2), IM_COL32(0,0,0,color.w), text.c_str());
        background->AddText(g.Font, g.FontSize, ImVec2(position.x +2, position.y -2), IM_COL32(0,0,0,color.w), text.c_str());
        background->AddText(g.Font, g.FontSize, ImVec2(position.x +2, position.y +2), IM_COL32(0,0,0,color.w), text.c_str());
        background->AddText(g.Font, g.FontSize, ImVec2(position.x -2, position.y +2), IM_COL32(0,0,0,color.w), text.c_str());
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


#endif //COPS_MENU_ESP_H
