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
void DrawBox(ImVec4 rect, ImVec4 color) {
    ImVec2 v1(rect.x, rect.y);
    ImVec2 v2(rect.x + rect.z, rect.y);
    ImVec2 v3(rect.x + rect.z, rect.y + rect.w);
    ImVec2 v4(rect.x, rect.y + rect.w);

    DrawLine(v1, v2, color, 1);
    DrawLine(v2, v3, color, 1);
    DrawLine(v3, v4, color, 1);
    DrawLine(v4, v1, color, 1);
}

void DrawOutlinedBox(ImVec4 rect, ImVec4 color)
{
    DrawBox(ImVec4(rect.x + 1, rect.y + 1, rect.z - 1, rect.w - 1), ImVec4(0,0,0,color.w));
    DrawBox(ImVec4(rect), color);
    DrawBox(ImVec4(rect.x - 1, rect.y - 1, rect.z + 1, rect.w + 1), ImVec4(0,0,0,color.w));
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
void DrawText(ImVec2 position, ImVec4 color, const char *text) {
    auto background = ImGui::GetBackgroundDrawList();
    if(background) {
        background->AddText(NULL, 25.0f, position, ImColor(color.x,color.y,color.z,color.w), text);
    }
}



#endif //COPS_MENU_ESP_H
