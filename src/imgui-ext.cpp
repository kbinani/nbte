#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"
#include <optional>
#include <string>
#include "texture.hpp"
#include "texture-set.hpp"

using namespace ImGui;

namespace nbte {

bool TreeNode(std::string const &label, ImGuiTreeNodeFlags flags, std::optional<Texture> icon) {
  ImGuiContext &g = *GImGui;
  ImGuiWindow *window = g.CurrentWindow;
  ImGuiStyle const &style = g.Style;

  ImGuiID id = window->GetID(label.c_str());
  ImVec2 pos = window->DC.CursorPos;
  float frameHeight = GetFrameHeight();
  ImVec2 padding = style.FramePadding;

  ImRect bb(pos, ImVec2(pos.x + GetContentRegionAvail().x, pos.y + frameHeight));
  bool opened = TreeNodeBehaviorIsOpen(id, flags);
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

  ImU32 const textCol = GetColorU32(ImGuiCol_Text);
  RenderArrow(window->DrawList, ImVec2(pos.x + padding.x, pos.y + padding.y), textCol, opened ? ImGuiDir_Down : ImGuiDir_Right, 1.0f);
  if (icon) {
    ImVec2 iconSize(icon->fWidth, icon->fHeight);
    ImVec2 p(pos.x + GetTreeNodeToLabelSpacing(), pos.y + frameHeight * 0.5f - iconSize.y * 0.5f);
    window->DrawList->AddImage((ImTextureID)(intptr_t)icon->fTexture, p, p + iconSize);
    window->DC.CursorPos = ImVec2(pos.x + iconSize.x + style.FramePadding.x, pos.y);
    RenderText(ImVec2(pos.x + frameHeight + frameHeight + 1.0f, pos.y + padding.y), label.c_str());
  } else {
    RenderText(ImVec2(pos.x + frameHeight + style.ItemInnerSpacing.x, pos.y + padding.y), label.c_str());
  }

  ItemSize(bb, 0);
  ItemAdd(bb, id);

  if (opened) {
    TreePush(label.c_str());
  }
  return opened;
}

bool IconButton(std::string const &label, std::optional<Texture> icon) {
  using namespace ImGui;

  ImGuiWindow *window = GetCurrentWindow();
  if (window->SkipItems) {
    return false;
  }

  ImVec2 const pos = window->DC.CursorPos;
  ImGuiStyle const &style = GetStyle();
  float const frameHeight = GetFrameHeight();
  ImVec2 const frameSize(frameHeight, frameHeight);
  ImVec2 const padding = style.FramePadding;
  ImVec2 const availableSize = GetContentRegionAvail();
  ImRect bounds(pos, pos + ImVec2(availableSize.x, frameHeight));
  ImGuiID const id = window->GetID(label.c_str());

  if (icon) {
    ImVec2 iconSize(icon->fWidth, icon->fHeight);
    ImVec2 p(pos.x, pos.y + frameHeight * 0.5f - iconSize.y * 0.5f);
    window->DrawList->AddImage((ImTextureID)(intptr_t)icon->fTexture, p, p + iconSize);
    window->DC.CursorPos = ImVec2(pos.x + iconSize.x + style.FramePadding.x, pos.y);
  }
  PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, style.FramePadding.y));
  PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
  RenderText(window->DC.CursorPos + ImVec2(0, padding.y), label.c_str());

  bool hovered, held;
  bool pressed = ButtonBehavior(bounds, id, &hovered, &held);
  PopStyleVar();
  PopStyleColor();
  ItemSize(bounds.GetSize());
  ItemAdd(bounds, id);
  return pressed;
}

void InlineImage(Texture const &image) {
  using namespace ImGui;
  ImGuiWindow *window = GetCurrentWindow();
  if (window->SkipItems) {
    return;
  }
  ImVec2 const pos = window->DC.CursorPos;
  ImGuiStyle const &style = GetStyle();
  float const frameHeight = GetFrameHeight();
  ImVec2 imagePos(pos.x, pos.y + frameHeight * 0.5f - image.fHeight * 0.5f);
  ImVec2 imageSize(image.fWidth, image.fHeight);
  window->DrawList->AddImage((ImTextureID)(intptr_t)image.fTexture, imagePos, imagePos + imageSize);
  window->DC.CursorPos = ImVec2(pos.x + image.fWidth, pos.y);
}

} // namespace nbte
