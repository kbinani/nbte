#pragma once

namespace nbte {

constexpr float kIndent = 6.0f;

static void VisitCompoundTag(State &s,
                             mcfile::nbt::CompoundTag const &tag,
                             std::string const &path,
                             std::string const &filter);
static void Visit(State &s,
                  std::string const &name,
                  std::shared_ptr<mcfile::nbt::Tag> const &tag,
                  std::string const &path,
                  std::string const &filter);

static std::optional<std::filesystem::path> OpenFileDialog() {
  using namespace std;
  namespace fs = std::filesystem;

  nfdchar_t *outPath = nullptr;
  if (NFD_OpenDialog(nullptr, nullptr, &outPath) == NFD_OKAY) {
    u8string selected;
    selected.assign((char8_t const *)outPath);
    free(outPath);
    return fs::path(selected);
  } else {
    return nullopt;
  }
}

static void RenderMainMenu(State &s) {
  using namespace ImGui;

  if (BeginMenuBar()) {
    if (BeginMenu("File", &s.fMainMenuBarFileSelected)) {
      if (MenuItem("Open", nullptr, nullptr)) {
        if (auto selected = OpenFileDialog(); selected) {
          s.open(*selected);
        }
      }
      if (MenuItem("Save", "Ctrl+S", nullptr, s.fOpened.index() != 0)) {
        s.save();
      }
      EndMenu();
    }
    if (BeginMenu("Find", &s.fMainMenuBarFindSelected)) {
      if (MenuItem("Filter", "Ctrl+F", nullptr)) {
        s.fFilterBarOpened = true;
      }
      EndMenu();
    }
    if (BeginMenu("Help", &s.fMainMenuBarHelpSelected)) {
      if (MenuItem("About nbte", nullptr, nullptr)) {
        s.fMainMenuBarHelpAboutOpened = true;
      }
      if (MenuItem("Legal", nullptr, nullptr)) {
        s.fMainMenuBarHelpOpenSourceLicensesOpened = true;
      }
      EndMenu();
    }
    EndMenuBar();
  }
}

static void RenderErrorPopup(State &s) {
  using namespace ImGui;

  if (s.fError.empty()) {
    return;
  }
  OpenPopup("Error");
  if (BeginPopupModal("Error", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize)) {
    TextUnformatted(s.fError.c_str());
    if (Button("OK")) {
      s.fError.clear();
      CloseCurrentPopup();
    }
    EndPopup();
  }
}

static void RenderAboutDialog(State &s) {
  using namespace ImGui;

  if (!s.fMainMenuBarHelpAboutOpened) {
    return;
  }
  OpenPopup("About");
  SetNextWindowSize(ImVec2(512, 320));
  if (BeginPopupModal("About", &s.fMainMenuBarHelpAboutOpened, ImGuiWindowFlags_NoSavedSettings)) {
    TextUnformatted("nbte");
    Text("Version %s", kAppVersion);
    TextUnformatted("");
    TextUnformatted("Copyright © 2022 kbinani");
    TextUnformatted("");
    TextWrapped("%s", R"(This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.)");
    TextUnformatted("");
    if (IsKeyDown(GetKeyIndex(ImGuiKey_Escape))) {
      s.fMainMenuBarHelpAboutOpened = false;
      CloseCurrentPopup();
    }
    EndPopup();
  }
}

template <class T, class U>
static T Clamp(U u) {
  return (T)std::min<U>(std::max<U>(u, (U)std::numeric_limits<T>::lowest()), (U)std::numeric_limits<T>::max());
}

static bool ContainsTerm(std::shared_ptr<mcfile::nbt::Tag> const &tag,
                         std::string const &filter,
                         FilterMode mode,
                         bool caseSensitive) {
  using namespace std;
  using namespace mcfile::nbt;

  if (filter.empty()) {
    return true;
  }

  switch (tag->type()) {
  case Tag::Type::Byte:
  case Tag::Type::Short:
  case Tag::Type::Int:
  case Tag::Type::Long:
  case Tag::Type::Float:
  case Tag::Type::Double:
  case Tag::Type::ByteArray:
  case Tag::Type::IntArray:
  case Tag::Type::LongArray:
    return false;
  case Tag::Type::Compound:
    if (auto v = dynamic_pointer_cast<CompoundTag>(tag); v) {
      for (auto const &it : *v) {
        if (mode == FilterMode::Key) {
          if ((caseSensitive ? it.first : ToLower(it.first)).find(filter) != string::npos) {
            return true;
          }
        }
        if (ContainsTerm(it.second, filter, mode, caseSensitive)) {
          return true;
        }
      }
    }
    return false;
  case Tag::Type::List:
    if (auto v = dynamic_pointer_cast<ListTag>(tag); v) {
      for (auto const &it : *v) {
        if (ContainsTerm(it, filter, mode, caseSensitive)) {
          return true;
        }
      }
    }
    return false;
  case Tag::Type::String:
    if (mode == FilterMode::Key) {
      return false;
    } else {
      if (auto v = dynamic_pointer_cast<StringTag>(tag); v) {
        return v->fValue.find(filter) != string::npos;
      }
    }
    return false;
  default:
    assert(false);
    return false;
  }
  return false;
}

template <class T>
static void InputScalar(T &v, State &s) {
  int t = v;
  if (ImGui::InputInt("", &t)) {
    T n = Clamp<T, int>(t);
    if (n != v) {
      v = n;
      s.fEdited = true;
    }
  }
}

template <>
static void InputScalar(int64_t &v, State &s) {
  std::string t = std::to_string(v);
  if (ImGui::InputText("", &t, ImGuiInputTextFlags_CharsDecimal)) {
    int64_t n = atoll(t.c_str());
    if (v != n) {
      v = n;
      s.fEdited = true;
    }
  }
}

static void PushScalarInput(std::string const &name,
                            std::string const &path,
                            std::string const &filter,
                            bool filterCaseSensitive) {
  using namespace std;
  using namespace ImGui;
  PushItemWidth(-FLT_EPSILON);
  Indent(GetTreeNodeToLabelSpacing());
  PushID((path + "/" + name).c_str());
  if (!filter.empty()) {
    ImDrawList *list = GetWindowDrawList();
    auto cursor = GetCursorScreenPos();
    auto color = GetColorU32(ImGuiCol_Button);
    size_t pos = 0;
    while (true) {
      size_t found = (filterCaseSensitive ? name : ToLower(name)).find(filter, pos);
      if (found == string::npos) {
        break;
      } else {
        auto leading = CalcTextSize(name.substr(0, found).c_str());
        auto trailing = CalcTextSize(name.substr(0, found + filter.size()).c_str());
        list->AddRectFilled(ImVec2(cursor.x + leading.x, cursor.y), ImVec2(cursor.x + trailing.x, cursor.y + trailing.y), color, 2.0f);
        pos = found + filter.size();
      }
    }
  }
  TextUnformatted(name.c_str());
  SameLine();
}

static void PopScalarInput() {
  using namespace ImGui;
  PopID();
  Unindent(GetTreeNodeToLabelSpacing());
  PopItemWidth();
}

static void VisitScalar(State &s,
                        std::string const &name,
                        std::shared_ptr<mcfile::nbt::Tag> const &tag,
                        std::string const &path,
                        std::string const &filter) {
  using namespace std;
  using namespace ImGui;
  using namespace mcfile::nbt;

  PushScalarInput(name, path, filter, s.fFilterCaseSensitive);

  switch (tag->type()) {
  case Tag::Type::Int:
    if (auto v = dynamic_pointer_cast<IntTag>(tag); v) {
      InputScalar<int>(v->fValue, s);
    }
    break;
  case Tag::Type::Byte:
    if (auto v = dynamic_pointer_cast<ByteTag>(tag); v) {
      InputScalar<uint8_t>(v->fValue, s);
    }
    break;
  case Tag::Type::Short:
    if (auto v = dynamic_pointer_cast<ShortTag>(tag); v) {
      InputScalar<int16_t>(v->fValue, s);
    }
    break;
  case Tag::Type::Long:
    if (auto v = dynamic_pointer_cast<LongTag>(tag); v) {
      InputScalar(v->fValue, s);
    }
    break;
  case Tag::Type::String:
    if (auto v = dynamic_pointer_cast<StringTag>(tag); v) {
      if (InputText("", &v->fValue)) {
        s.fEdited = true;
      }
    }
    break;
  case mcfile::nbt::Tag::Type::Float:
    if (auto v = dynamic_pointer_cast<FloatTag>(tag); v) {
      if (InputFloat("", &v->fValue)) {
        s.fEdited = true;
      }
    }
    break;
  case mcfile::nbt::Tag::Type::Double:
    if (auto v = dynamic_pointer_cast<DoubleTag>(tag); v) {
      if (InputDouble("", &v->fValue)) {
        s.fEdited = true;
      }
    }
    break;
  default:
    break;
  }

  PopScalarInput();
}

static void VisitNonScalar(State &s,
                           std::string const &name,
                           std::shared_ptr<mcfile::nbt::Tag> const &tag,
                           std::string const &path,
                           std::string const &filterTerm) {
  using namespace std;
  using namespace ImGui;
  using namespace mcfile::nbt;

  string filter = filterTerm;
  bool matchedNode = !filter.empty() && (s.fFilterCaseSensitive ? name : ToLower(name)).find(filter) != string::npos;
  if (name == "Player") {
    int a = 0;
  }
  if (matchedNode) {
    filter = "";
  }

  int size = 0;
  switch (tag->type()) {
  case Tag::Type::Compound:
    if (s.fFilterBarOpened) {
      if (!ContainsTerm(tag, filter, s.fFilterMode, s.fFilterCaseSensitive)) {
        return;
      }
    }
    if (auto v = dynamic_pointer_cast<CompoundTag>(tag); v) {
      size = v->size();
    }
    break;
  case Tag::Type::List:
    if (s.fFilterBarOpened) {
      if (!ContainsTerm(tag, filter, s.fFilterMode, s.fFilterCaseSensitive)) {
        return;
      }
    }
    if (auto v = dynamic_pointer_cast<ListTag>(tag); v) {
      size = v->size();
    }
    break;
  case Tag::Type::ByteArray:
    if (auto v = dynamic_pointer_cast<ByteArrayTag>(tag); v) {
      size = v->fValue.size();
    }
    break;
  case Tag::Type::IntArray:
    if (auto v = dynamic_pointer_cast<IntArrayTag>(tag); v) {
      size = v->fValue.size();
    }
    break;
  case Tag::Type::LongArray:
    if (auto v = dynamic_pointer_cast<LongArrayTag>(tag); v) {
      size = v->fValue.size();
    }
    break;
  }
  string label = name + ": ";
  if (size < 2) {
    label += to_string(size) + " entry";
  } else {
    label += to_string(size) + " entries";
  }

  auto nextPath = path + "/" + name;
  PushID(nextPath.c_str());

  if (s.fFilterBarOpened && !filter.empty()) {
    SetNextItemOpen(true);
  }
  ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen;
  if (matchedNode) {
    flags = flags | ImGuiTreeNodeFlags_Selected;
  }
  if (TreeNodeEx(label.c_str(), flags)) {
    Indent(kIndent);

    switch (tag->type()) {
    case Tag::Type::Compound:
      if (auto v = dynamic_pointer_cast<CompoundTag>(tag); v) {
        VisitCompoundTag(s, *v, nextPath, filter);
      }
      break;
    case Tag::Type::List:
      if (auto v = dynamic_pointer_cast<ListTag>(tag); v) {
        for (size_t i = 0; i < v->fValue.size(); i++) {
          auto const &it = v->fValue[i];
          auto label = "#" + to_string(i);
          Visit(s, label, it, nextPath, filter);
        }
      }
      break;
    case Tag::Type::ByteArray:
      if (auto v = dynamic_pointer_cast<ByteArrayTag>(tag); v) {
        for (size_t i = 0; i < v->fValue.size(); i++) {
          auto label = "#" + to_string(i);
          PushScalarInput(label, nextPath, filter, s.fFilterCaseSensitive);
          InputScalar<uint8_t>(v->fValue[i], s);
          PopScalarInput();
        }
      }
      break;
    case Tag::Type::IntArray:
      if (auto v = dynamic_pointer_cast<IntArrayTag>(tag); v) {
        for (size_t i = 0; i < v->fValue.size(); i++) {
          auto label = "#" + to_string(i);
          PushScalarInput(label, nextPath, filter, s.fFilterCaseSensitive);
          InputScalar<int>(v->fValue[i], s);
          PopScalarInput();
        }
      }
      break;
    case Tag::Type::LongArray:
      if (auto v = dynamic_pointer_cast<LongArrayTag>(tag); v) {
        for (size_t i = 0; i < v->fValue.size(); i++) {
          auto label = "#" + to_string(i);
          PushScalarInput(label, nextPath, filter, s.fFilterCaseSensitive);
          InputScalar<int64_t>(v->fValue[i], s);
          PopScalarInput();
        }
      }
      break;
    }

    TreePop();
    Unindent(kIndent);
  }
  PopID();
}

static void Visit(State &s,
                  std::string const &name,
                  std::shared_ptr<mcfile::nbt::Tag> const &tag,
                  std::string const &path,
                  std::string const &filter) {
  using namespace mcfile::nbt;

  switch (tag->type()) {
  case Tag::Type::Compound:
  case Tag::Type::List:
  case Tag::Type::ByteArray:
  case Tag::Type::IntArray:
  case Tag::Type::LongArray:
    VisitNonScalar(s, name, tag, path, filter);
    break;
  default:
    VisitScalar(s, name, tag, path, filter);
    break;
  }
}

static void VisitCompoundTag(State &s,
                             mcfile::nbt::CompoundTag const &tag,
                             std::string const &path,
                             std::string const &filter) {
  using namespace std;

  for (auto &it : tag) {
    auto const &name = it.first;
    if (!it.second) {
      continue;
    }
    if (s.fFilterBarOpened && !filter.empty()) {
      if (s.fFilterMode == FilterMode::Key) {
        if ((s.fFilterCaseSensitive ? name : ToLower(name)).find(filter) == string::npos && !ContainsTerm(it.second, filter, s.fFilterMode, s.fFilterCaseSensitive)) {
          continue;
        }
      } else {
        if (!ContainsTerm(it.second, filter, s.fFilterMode, s.fFilterCaseSensitive)) {
          continue;
        }
      }
    }
    Visit(s, name, it.second, path, filter);
  }
}

static void RenderCompoundTag(State &s) {
  if (s.fOpened.index() != 1) {
    return;
  }
  std::shared_ptr<mcfile::nbt::CompoundTag> const &tag = get<1>(s.fOpened);
  if (!tag) {
    return;
  }

  VisitCompoundTag(s, *tag, "", s.filterTerm());
}

static void RenderFooter(State &s) {
  using namespace ImGui;

  float const frameHeight = GetFrameHeightWithSpacing();

  Begin("footer", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings);
  SetWindowPos(ImVec2(0, s.fDisplaySize.y - frameHeight));
  SetWindowSize(ImVec2(s.fDisplaySize.x, frameHeight));

  PushItemWidth(-FLT_EPSILON);
  auto formatDescription = TypeDescription(s.fOpenedFormat);
  Text("Path: %s, Format: %s", s.fOpenedPath.u8string().c_str(), formatDescription.c_str());
  PopItemWidth();
  End();
}

static void RenderFilterBar(State &s) {
  using namespace ImGui;

  auto const &style = GetStyle();

  if (s.fFilterBarOpened) {
    BeginChild("filter_panel", ImVec2(s.fDisplaySize.x, GetFrameHeightWithSpacing()));

    TextUnformatted("Filter: ");

    SameLine();
    PushID("filter_panel#button_case_sensitive");
    PushStyleColor(ImGuiCol_Text, s.fFilterCaseSensitive ? style.Colors[ImGuiCol_ButtonActive] : style.Colors[ImGuiCol_TextDisabled]);
    PushStyleColor(ImGuiCol_Button, s.fFilterCaseSensitive ? style.Colors[ImGuiCol_Button] : style.Colors[ImGuiCol_ChildBg]);
    if (Button("Aa")) {
      s.fFilterCaseSensitive = !s.fFilterCaseSensitive;
    }
    PopStyleColor(2);
    PopID();

    SameLine();
    PushID("filter_panel#text");
    if (!s.fFilterBarGotFocus || (IsKeyDown(GetKeyIndex(ImGuiKey_ModCtrl)) && IsKeyDown(GetKeyIndex(ImGuiKey_F)))) {
      SetKeyboardFocusHere();
      s.fFilterBarGotFocus = true;
    }
    InputText("", &s.fFilter);
    if (IsItemDeactivated() && IsKeyPressed(GetKeyIndex(ImGuiKey_Escape))) {
      s.fFilterBarOpened = false;
    }
    PopID();

    SameLine();
    PushID("filter_panel#close");
    if (Button("x", ImVec2(GetFrameHeight(), GetFrameHeight()))) {
      s.fFilterBarOpened = false;
    }
    PopID();

    EndChild();
    Separator();
  } else {
    s.fFilterBarGotFocus = false;
  }
}

static void CaptureShortcutKey(State &s) {
  using namespace ImGui;

  if (IsKeyDown(GetKeyIndex(ImGuiKey_ModCtrl))) {
    if (IsKeyDown(GetKeyIndex(ImGuiKey_F))) {
      s.fFilterBarOpened = true;
    } else if (IsKeyDown(GetKeyIndex(ImGuiKey_S)) && s.fOpened.index() != 0) {
      s.save();
    }
  }
}

static void Render(State &s) {
  using namespace ImGui;

  ImGuiWindowFlags flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus;
  Begin("main", nullptr, flags);
  SetWindowPos(ImVec2(0, 0));
  SetWindowSize(ImVec2(s.fDisplaySize.x, s.fDisplaySize.y - GetFrameHeightWithSpacing()));

  RenderMainMenu(s);
  RenderErrorPopup(s);
  RenderFilterBar(s);

  BeginChild("editor");
  RenderCompoundTag(s);
  EndChild();

  RenderAboutDialog(s);
  RenderLegal(s);

  End();

  RenderFooter(s);

  CaptureShortcutKey(s);

  ImGui::Render();
}

} // namespace nbte
