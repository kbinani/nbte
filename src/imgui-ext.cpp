#include "imgui_internal.h"

using namespace ImGui;

namespace nbte {

bool MyTreeNode(const char *label, ImGuiTreeNodeFlags) {
  ImGuiContext &g = *GImGui;
  ImGuiWindow *window = g.CurrentWindow;

  ImGuiID id = window->GetID(label);
  ImVec2 pos = window->DC.CursorPos;
  ImRect bb(pos, ImVec2(pos.x + GetContentRegionAvail().x, pos.y + g.FontSize + g.Style.FramePadding.y * 2));
  bool opened = TreeNodeBehaviorIsOpen(id);
  bool hovered, held;
  if (ButtonBehavior(bb, id, &hovered, &held, true)) {
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
  float button_sz = g.FontSize + g.Style.FramePadding.y * 2;
  window->DrawList->AddRectFilled(pos, ImVec2(pos.x + button_sz, pos.y + button_sz), opened ? ImColor(255, 0, 0) : ImColor(0, 255, 0));
  RenderText(ImVec2(pos.x + button_sz + g.Style.ItemInnerSpacing.x, pos.y + g.Style.FramePadding.y), label);

  ItemSize(bb, g.Style.FramePadding.y);
  ItemAdd(bb, id);

  if (opened) {
    TreePush(label);
  }
  return opened;
}

} // namespace nbte
