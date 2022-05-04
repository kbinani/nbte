#include "imgui_internal.h"
#include <optional>
#include "texture.hpp"
#include "texture-set.hpp"

using namespace ImGui;

namespace nbte {

bool MyTreeNode(const char *label, ImGuiTreeNodeFlags flags, std::optional<Texture> icon) {
  ImGuiContext &g = *GImGui;
  ImGuiWindow *window = g.CurrentWindow;
  ImGuiStyle const &style = g.Style;

  ImGuiID id = window->GetID(label);
  ImVec2 pos = window->DC.CursorPos;
  float frameHeight = GetFrameHeight();
  ImVec2 padding = style.FramePadding;

  ImRect bb(pos, ImVec2(pos.x + GetContentRegionAvail().x, pos.y + frameHeight));
  bool opened = TreeNodeBehaviorIsOpen(id);
  bool hovered, held;

  if (opened && !g.NavIdIsAlive && (flags & ImGuiTreeNodeFlags_NavLeftJumpsBackHere) && !(flags & ImGuiTreeNodeFlags_NoTreePushOnOpen)) {
    window->DC.TreeJumpToParentOnPopMask |= (1 << window->DC.TreeDepth);
  }

  bool pressed = ButtonBehavior(bb, id, &hovered, &held, true);
  if (pressed) {
    window->DC.StateStorage->SetInt(id, opened ? 0 : 1);
  }

  if (hovered || held) {
    ImGuiCol col = ImGuiCol_Header;
    if (held && hovered) {
      col = ImGuiCol_HeaderActive;
    } else if (hovered) {
      col = ImGuiCol_HeaderHovered;
    }
    ImU32 bg_col = GetColorU32(col);
    window->DrawList->AddRectFilled(bb.Min, bb.Max, bg_col);
  }

  // Icon, text
  const ImU32 text_col = GetColorU32(ImGuiCol_Text);
  RenderArrow(window->DrawList, ImVec2(pos.x + padding.x, pos.y + padding.y), text_col, opened ? ImGuiDir_Down : ImGuiDir_Right, 1.0f);
  if (icon) {
    ImVec2 p = GetCursorPos();
    SetCursorPos(ImVec2(p.x + frameHeight + frameHeight * 0.5 - icon->fWidth * 0.5, p.y + frameHeight * 0.5f - icon->fHeight * 0.5f));
    Image(icon->fTexture, ImVec2(icon->fWidth, icon->fHeight));
    SameLine();
    RenderText(ImVec2(pos.x + frameHeight + frameHeight + 1.0f /*what is this "1.0f" ?*/, pos.y + padding.y), label);
  } else {
    RenderText(ImVec2(pos.x + frameHeight + style.ItemInnerSpacing.x, pos.y + padding.y), label);
  }

  ItemSize(bb, padding.y);
  ItemAdd(bb, id);

  if (opened) {
    TreePush(label);
  }
  return opened;
}

} // namespace nbte
