#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"
#include <optional>
#include <string>
#include <algorithm>
#include "texture.hpp"
#include "texture-set.hpp"
#include "string.hpp"
#include "imgui-ext.hpp"

namespace nbte {

bool TreeNode(String const &label, ImGuiTreeNodeFlags flags, std::optional<Texture> icon, String const &filter, bool caseSensitive, TreeNodeOptions opt) {
  ImGuiContext &g = *GImGui;
  ImGuiWindow *window = g.CurrentWindow;
  ImGuiStyle const &style = g.Style;

  ImGuiID id = GetID(label);
  ImVec2 pos = window->DC.CursorPos;
  float frameHeight = im::GetFrameHeight();
  ImVec2 padding = style.FramePadding;

  ImRect bb(pos, ImVec2(pos.x + im::GetContentRegionAvail().x, pos.y + frameHeight));
  bool opened = true;
  if (opt.disable) {
    opened = false;
  } else if (opt.openIgnoringStorage) {
    opened = true;
  } else {
    opened = im::TreeNodeBehaviorIsOpen(id, flags);
  }
  if (opened && !g.NavIdIsAlive && !(flags & ImGuiTreeNodeFlags_NoTreePushOnOpen)) {
    if (flags & ImGuiTreeNodeFlags_NavLeftJumpsBackHere) {
      window->DC.TreeJumpToParentOnPopMask |= (1 << window->DC.TreeDepth);
    }
  }

  bool hovered, held;
  bool toggled = im::ButtonBehavior(bb, id, &hovered, &held, true);
  if (g.NavId == id && g.NavMoveDir == ImGuiDir_Left && opened) {
    toggled = true;
    im::NavMoveRequestCancel();
  }
  if (g.NavId == id && g.NavMoveDir == ImGuiDir_Right && !opened) {
    toggled = true;
    im::NavMoveRequestCancel();
  }
  if (opt.disable) {
    toggled = false;
    hovered = false;
    held = false;
  }
  if (toggled && !opt.openIgnoringStorage) {
    opened = !opened;
    window->DC.StateStorage->SetInt(id, opened ? 1 : 0);
  }

  float const labelSpacing = im::GetTreeNodeToLabelSpacing();

  if (hovered || held) {
    ImGuiCol col = ImGuiCol_Header;
    if (held && hovered) {
      col = ImGuiCol_HeaderActive;
    } else if (hovered) {
      col = ImGuiCol_HeaderHovered;
    }
    ImU32 bgColor = im::GetColorU32(col);
    ImVec2 topLeft = bb.Min;
    ImVec2 bottomRight = bb.Max;
    if (opt.noArrow) {
      topLeft = ImVec2(bb.Min.x + labelSpacing, bb.Min.y);
    }
    window->DrawList->AddRectFilled(topLeft, bottomRight, bgColor);
  }

  if (!opt.noArrow) {
    ImColor arrowColor;
    if (opt.disable) {
      arrowColor = style.Colors[ImGuiCol_TextDisabled];
    } else {
      arrowColor = style.Colors[ImGuiCol_Text];
    }
    ImVec2 defaultSize(frameHeight - 2 * padding.x, frameHeight - 2 * padding.y);
    float const arrowScale = 0.7f;
    ImVec2 arrowOffset = padding + defaultSize * (0.5f - 0.5f * arrowScale);
    im::RenderArrow(window->DrawList, pos + arrowOffset, arrowColor, opened ? ImGuiDir_Down : ImGuiDir_Right, arrowScale);
  }

  ImVec2 textPos;

  if (icon) {
    ImVec2 iconSize(icon->fWidth, icon->fHeight);
    ImVec2 p(pos.x + labelSpacing, pos.y + frameHeight * 0.5f - iconSize.y * 0.5f);
    window->DrawList->AddImage((ImTextureID)(intptr_t)icon->fTexture, p, p + iconSize);
    window->DC.CursorPos = ImVec2(pos.x + iconSize.x + style.FramePadding.x, pos.y);
    textPos = ImVec2(pos.x + labelSpacing + iconSize.x + padding.x, pos.y + padding.y);
  } else {
    textPos = ImVec2(pos.x + labelSpacing + padding.x, pos.y + padding.y);
  }

  if (!filter.empty()) {
    String search = caseSensitive ? filter : ToLower(filter);
    auto cursor = window->DC.CursorPos;
    auto color = im::GetColorU32(ImGuiCol_Button);
    size_t pivot = 0;
    while (true) {
      size_t found = (caseSensitive ? label : ToLower(label)).find(search, pivot);
      if (found == String::npos) {
        break;
      } else {
        auto leading = CalcTextSize(label.substr(0, found));
        auto trailing = CalcTextSize(label.substr(0, found + search.size()));
        window->DrawList->AddRectFilled(ImVec2(textPos.x + leading.x, textPos.y + style.FramePadding.y), ImVec2(textPos.x + trailing.x, textPos.y + style.FramePadding.y + trailing.y), color, 2.0f);
        pivot = found + search.size();
      }
    }
  }
  RenderText(textPos, label);

  im::ItemSize(bb, 0);
  im::ItemAdd(bb, id);

  if (opened) {
    TreePush(label);
  }

  return opened;
}

void InlineImage(Texture const &image) {
  ImGuiWindow *window = im::GetCurrentWindow();
  if (window->SkipItems) {
    return;
  }
  ImVec2 const pos = window->DC.CursorPos;
  ImGuiStyle const &style = im::GetStyle();
  float const frameHeight = im::GetFrameHeight();
  ImVec2 imagePos(pos.x, pos.y + frameHeight * 0.5f - image.fHeight * 0.5f);
  ImVec2 imageSize(image.fWidth, image.fHeight);
  window->DrawList->AddImage((ImTextureID)(intptr_t)image.fTexture, imagePos, imagePos + imageSize);
  window->DC.CursorPos = ImVec2(pos.x + image.fWidth, pos.y);
}

void IconLabel(String const &label, std::optional<Texture> icon) {
  ImGuiWindow *window = im::GetCurrentWindow();
  if (window->SkipItems) {
    return;
  }
  ImVec2 const pos = window->DC.CursorPos;
  ImGuiID const id = GetID(label);
  float const frameHeight = im::GetFrameHeight();
  ImGuiStyle const &style = im::GetStyle();

  ImVec2 p = pos;
  if (icon) {
    InlineImage(*icon);
    p += ImVec2(icon->fWidth, 0);
  }
  float const textWidth = CalcTextSize(label).x;
  ImRect bounds(pos, p + ImVec2(textWidth + style.FramePadding.x, frameHeight));

  RenderText(p + style.FramePadding, label);

  im::ItemSize(bounds);
  im::ItemAdd(bounds, id);
}

void RenderText(ImVec2 pos, String const &text, bool hide_text_after_hash) {
  return im::RenderText(pos, (char const *)text.c_str(), 0, hide_text_after_hash);
}

ImGuiID GetID(String const &label) {
  ImGuiContext &g = *GImGui;
  ImGuiWindow *window = g.CurrentWindow;
  return window->GetID((char const *)label.c_str());
}

static int InputTextCallback(ImGuiInputTextCallbackData *data) {
  String *str = (String *)data->UserData;
  if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
    str->resize(data->BufTextLen);
    data->Buf = (char *)str->c_str();
  }
  return 0;
}

bool InputText(String const &label, String *text, ImGuiInputTextFlags flags) {
  IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
  flags |= ImGuiInputTextFlags_CallbackResize;

  return im::InputText((char const *)label.c_str(), (char *)text->c_str(), text->capacity() + 1, flags, InputTextCallback, text);
}

} // namespace nbte
