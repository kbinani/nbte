#pragma once

namespace im {
using namespace ImGui;
} // namespace im

namespace nbte {

struct TreeNodeOptions {
  bool openIgnoringStorage = false;
  bool disable = false;
  bool noArrow = false;
};

bool TreeNode(String const &label, ImGuiTreeNodeFlags flags, std::optional<Texture> icon, String const &filter, bool caseSensitive, TreeNodeOptions options = {});

bool IconButton(String const &label, std::optional<Texture> icon);

void InlineImage(Texture const &image);

void IconLabel(String const &label, std::optional<Texture> icon);

void RenderText(ImVec2 pos, String const &text, bool hide_text_after_hash = true);

ImGuiID GetID(String const &label);

bool InputText(String const &label, String *text, ImGuiInputTextFlags flags = 0);

inline ImVec2 CalcTextSize(String const &t) {
  return im::CalcTextSize((char const *)t.c_str());
}

inline void TreePush(String const &label) {
  im::TreePush((char const *)label.c_str());
}

inline void TextUnformatted(String const &text) {
  im::TextUnformatted((char const *)text.c_str());
}

inline void OpenPopup(String const &id) {
  im::OpenPopup((char const *)id.c_str());
}

inline bool BeginPopupModal(String const &id, bool *open, ImGuiWindowFlags flags = 0) {
  return im::BeginPopupModal((char const *)id.c_str(), open, flags);
}

inline void TextWrapped(String const &s) {
  im::TextWrapped("%s", (char const *)s.c_str());
}

inline bool BeginMenu(String const &id, bool enable = true) {
  return im::BeginMenu((char const *)id.c_str(), enable);
}

inline bool MenuItem(String const &label, String const &shortcut, bool *p_selected, bool enabled = true) {
  return im::MenuItem((char const *)label.c_str(), shortcut.empty() ? 0 : (char const *)shortcut.c_str(), p_selected, enabled);
}

inline bool Button(String const &id, ImVec2 size = ImVec2(0, 0)) {
  return im::Button((char const *)id.c_str(), size);
}

inline bool InputInt(String const &label, int *v, ImGuiInputTextFlags flags = 0) {
  return im::InputInt((char const *)label.c_str(), v, 1, 100, flags);
}

inline void PushID(String const &id) {
  im::PushID((char const *)id.c_str());
}

inline bool InputFloat(String const &label, float *v) {
  return im::InputFloat((char const *)label.c_str(), v);
}

inline bool InputDouble(String const &label, double *v) {
  return im::InputDouble((char const *)label.c_str(), v);
}

inline void SetTooltip(String const &label) {
  im::SetTooltip("%s", (char const *)label.c_str());
}

inline bool Begin(String const &label, bool *p_open = 0, ImGuiWindowFlags flags = 0) {
  return im::Begin((char const *)label.c_str(), p_open, flags);
}

inline bool BeginChild(String const &label, ImVec2 size = ImVec2(0, 0), bool border = false, ImGuiWindowFlags flags = 0) {
  return im::BeginChild((char const *)label.c_str(), size, border, flags);
}

} // namespace nbte
