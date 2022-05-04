#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"
#include <optional>
#include <string>
#include <algorithm>
#include "texture.hpp"
#include "texture-set.hpp"
#include "string.hpp"

using namespace ImGui;

namespace nbte {

bool TreeNode(std::string const &label, ImGuiTreeNodeFlags flags, std::optional<Texture> icon, std::string const &filter, bool caseSensitive) {
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

  float const labelSpacing = GetTreeNodeToLabelSpacing();
  ImVec2 textPos;

  if (icon) {
    ImVec2 iconSize(icon->fWidth, icon->fHeight);
    ImVec2 p(pos.x + labelSpacing, pos.y + frameHeight * 0.5f - iconSize.y * 0.5f);
    window->DrawList->AddImage((ImTextureID)(intptr_t)icon->fTexture, p, p + iconSize);
    window->DC.CursorPos = ImVec2(pos.x + iconSize.x + style.FramePadding.x, pos.y);
    textPos = ImVec2(pos.x + frameHeight + frameHeight, pos.y + padding.y);
  } else {
    textPos = ImVec2(pos.x + frameHeight + style.ItemInnerSpacing.x, pos.y + padding.y);
  }

  if (!filter.empty()) {
    std::string search = caseSensitive ? filter : ToLower(filter);
    auto cursor = window->DC.CursorPos;
    auto color = GetColorU32(ImGuiCol_Button);
    size_t pivot = 0;
    while (true) {
      size_t found = (caseSensitive ? label : ToLower(label)).find(search, pivot);
      if (found == std::string::npos) {
        break;
      } else {
        auto leading = CalcTextSize(label.substr(0, found).c_str());
        auto trailing = CalcTextSize(label.substr(0, found + search.size()).c_str());
        window->DrawList->AddRectFilled(ImVec2(textPos.x + leading.x, textPos.y + style.FramePadding.y), ImVec2(textPos.x + trailing.x, textPos.y + style.FramePadding.y + trailing.y), color, 2.0f);
        pivot = found + search.size();
      }
    }
  }
  RenderText(textPos, label.c_str());

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
  ImGuiID const id = window->GetID(label.c_str());

  ImVec2 textSize = CalcTextSize(label.c_str());
  ImRect bounds;
  if (icon) {
    bounds = ImRect(pos, pos + ImVec2(icon->fWidth + style.FramePadding.x + textSize.x + style.FramePadding.x, frameHeight));
  } else {
    bounds = ImRect(pos, pos + ImVec2(style.FramePadding.x + textSize.x + style.FramePadding.x, frameHeight));
  }

  bool hovered = false;
  bool held = false;
  bool pressed = ButtonBehavior(bounds, id, &hovered, &held);

  if (hovered || held) {
    ImGuiCol col = ImGuiCol_Header;
    if (held && hovered) {
      col = ImGuiCol_HeaderActive;
    } else if (hovered) {
      col = ImGuiCol_HeaderHovered;
    }
    ImU32 background = GetColorU32(col);
    window->DrawList->AddRectFilled(bounds.Min, bounds.Max, background);
  }

  if (icon) {
    ImVec2 iconSize(icon->fWidth, icon->fHeight);
    ImVec2 p(pos.x, pos.y + frameHeight * 0.5f - iconSize.y * 0.5f);
    window->DrawList->AddImage((ImTextureID)(intptr_t)icon->fTexture, p, p + iconSize);
    window->DC.CursorPos = ImVec2(pos.x + iconSize.x + style.FramePadding.x, pos.y);
  }
  RenderText(window->DC.CursorPos + ImVec2(0, padding.y), label.c_str());

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

void IconLabel(std::string const &label, std::optional<Texture> icon) {
  using namespace ImGui;
  ImGuiWindow *window = GetCurrentWindow();
  if (window->SkipItems) {
    return;
  }
  ImVec2 const pos = window->DC.CursorPos;
  ImGuiID const id = window->GetID(label.c_str());
  float const frameHeight = GetFrameHeight();
  ImGuiStyle const &style = GetStyle();

  ImVec2 p = pos;
  if (icon) {
    InlineImage(*icon);
    p += ImVec2(icon->fWidth, 0);
  }
  float const textWidth = CalcTextSize(label.c_str()).x;
  ImRect bounds(pos, p + ImVec2(textWidth + style.FramePadding.x, frameHeight));

  RenderText(p + style.FramePadding, label.c_str());

  ItemSize(bounds);
  ItemAdd(bounds, id);
}

} // namespace nbte
