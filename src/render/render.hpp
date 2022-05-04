#pragma once

namespace nbte {

constexpr float kIndent = 6.0f;

static void VisitNbtCompound(State &s,
                             Compound &root,
                             mcfile::nbt::CompoundTag const &tag,
                             std::string const &path,
                             std::string const &filter);
static void VisitNbt(State &s,
                     Compound &root,
                     std::string const &name,
                     std::shared_ptr<mcfile::nbt::Tag> const &tag,
                     std::string const &path,
                     std::string const &filter);
static void Visit(State &s,
                  std::shared_ptr<Node> const &node,
                  std::string const &path,
                  std::string const &filter);

static void PushID(std::string const &id) {
  ImGui::PushID(id.c_str());
}

static void RenderSavingModal(State &s) {
  using namespace ImGui;
  OpenPopup("Info");
  SetNextWindowSize(ImVec2(512, 0));
  if (BeginPopupModal("Info", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize)) {
    TextUnformatted("Saving edited files...");
    EndPopup();
  }
}

static void Save(State &s) {
  if (!s.canSave()) {
    return;
  }
  RenderSavingModal(s);
  s.save();
}

static void RenderMainMenu(State &s) {
  using namespace ImGui;

  if (BeginMenuBar()) {
    if (BeginMenu("File", &s.fMainMenuBarFileSelected)) {
      if (MenuItem("Open", DecorateModCtrl("O").c_str(), nullptr)) {
        if (auto selected = OpenFileDialog(); selected) {
          s.open(*selected);
        }
      }
      if (MenuItem("Open Folder", DecorateModCtrl("Shift+O").c_str(), nullptr)) {
        if (auto selected = OpenDirectoryDialog(); selected) {
          s.openDirectory(*selected);
        }
      }
      if (s.fMinecraftSaveDirectory) {
        if (MenuItem("Open Minecraft Save Directory", nullptr, nullptr)) {
          s.openDirectory(*s.fMinecraftSaveDirectory);
        }
      }
      if (MenuItem("Save", DecorateModCtrl("S").c_str(), nullptr, s.canSave())) {
        Save(s);
      }
      Separator();
      if (MenuItem("Quit", QuitMenuShortcut().c_str())) {
        s.fMainMenuBarQuitSelected = true;
      }
      ImGui::EndMenu();
    }
    if (BeginMenu("Find", &s.fMainMenuBarFindSelected)) {
      if (MenuItem("Filter", DecorateModCtrl("F").c_str(), nullptr)) {
        s.fFilterBarOpened = true;
      }
      ImGui::EndMenu();
    }
    if (BeginMenu("Help", &s.fMainMenuBarHelpSelected)) {
      if (MenuItem("About nbte", nullptr, nullptr)) {
        s.fMainMenuBarHelpAboutOpened = true;
      }
      if (MenuItem("Legal", nullptr, nullptr)) {
        s.fMainMenuBarHelpOpenSourceLicensesOpened = true;
      }
      ImGui::EndMenu();
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
  SetNextWindowSize(ImVec2(512, 0), ImGuiCond_Once);
  if (BeginPopupModal("About", &s.fMainMenuBarHelpAboutOpened)) {
    TextUnformatted("nbte: https://github.com/kbinani/nbte");
    Text("Version %s", kAppVersion);
    TextUnformatted("");
    TextUnformatted("Copyright © 2022 kbinani");
    TextUnformatted("");
    TextWrapped("%s", R"(This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.)");
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
static void InputScalar(T &v, Compound &root) {
  int t = v;
  if (ImGui::InputInt("", &t)) {
    T n = Clamp<T, int>(t);
    if (n != v) {
      v = n;
      root.fEdited = true;
    }
  }
}

template <>
void InputScalar(int64_t &v, Compound &root) {
  std::string t = std::to_string(v);
  if (ImGui::InputText("", &t, ImGuiInputTextFlags_CharsDecimal)) {
    int64_t n = atoll(t.c_str());
    if (v != n) {
      v = n;
      root.fEdited = true;
    }
  }
}

static void PushScalarInput(std::string const &name,
                            std::string const &path,
                            std::string const &filter,
                            bool filterCaseSensitive,
                            std::optional<Texture> const &icon) {
  using namespace std;
  using namespace ImGui;
  PushItemWidth(-FLT_EPSILON);
  Indent(GetTreeNodeToLabelSpacing());
  if (icon) {
    ImGui::Image((ImTextureID)(intptr_t)icon->fTexture, ImVec2(icon->fWidth, icon->fHeight));
    SameLine();
    auto cursor = GetCursorScreenPos();
    auto style = GetStyle();
    SetCursorScreenPos(ImVec2(cursor.x - style.FramePadding.x, cursor.y));
  }
  PushID(path + "/" + name);
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

static void VisitNbtScalar(State &s,
                           Compound &root,
                           std::string const &name,
                           std::shared_ptr<mcfile::nbt::Tag> const &tag,
                           std::string const &path,
                           std::string const &filter) {
  using namespace std;
  using namespace ImGui;
  using namespace mcfile::nbt;

  switch (tag->type()) {
  case Tag::Type::Int:
    if (auto v = dynamic_pointer_cast<IntTag>(tag); v) {
      PushScalarInput(name, path, filter, s.fFilterCaseSensitive, s.fIconDocumentAttributeI);
      InputScalar<int>(v->fValue, root);
    }
    break;
  case Tag::Type::Byte:
    if (auto v = dynamic_pointer_cast<ByteTag>(tag); v) {
      PushScalarInput(name, path, filter, s.fFilterCaseSensitive, s.fIconDocumentAttributeB);
      InputScalar<uint8_t>(v->fValue, root);
    }
    break;
  case Tag::Type::Short:
    if (auto v = dynamic_pointer_cast<ShortTag>(tag); v) {
      PushScalarInput(name, path, filter, s.fFilterCaseSensitive, s.fIconDocumentAttributeS);
      InputScalar<int16_t>(v->fValue, root);
    }
    break;
  case Tag::Type::Long:
    if (auto v = dynamic_pointer_cast<LongTag>(tag); v) {
      PushScalarInput(name, path, filter, s.fFilterCaseSensitive, s.fIconDocumentAttributeL);
      InputScalar(v->fValue, root);
    }
    break;
  case Tag::Type::String:
    if (auto v = dynamic_pointer_cast<StringTag>(tag); v) {
      PushScalarInput(name, path, filter, s.fFilterCaseSensitive, s.fIconEditSmallCaps);
      if (InputText("", &v->fValue)) {
        root.fEdited = true;
      }
    }
    break;
  case mcfile::nbt::Tag::Type::Float:
    if (auto v = dynamic_pointer_cast<FloatTag>(tag); v) {
      PushScalarInput(name, path, filter, s.fFilterCaseSensitive, s.fIconDocumentAttributeF);
      if (InputFloat("", &v->fValue)) {
        root.fEdited = true;
      }
    }
    break;
  case mcfile::nbt::Tag::Type::Double:
    if (auto v = dynamic_pointer_cast<DoubleTag>(tag); v) {
      PushScalarInput(name, path, filter, s.fFilterCaseSensitive, s.fIconDocumentAttributeD);
      if (InputDouble("", &v->fValue)) {
        root.fEdited = true;
      }
    }
    break;
  default:
    break;
  }

  PopScalarInput();
}

static void VisitNbtNonScalar(State &s,
                              Compound &root,
                              std::string const &name,
                              std::shared_ptr<mcfile::nbt::Tag> const &tag,
                              std::string const &path,
                              std::string const &filterTerm) {
  using namespace std;
  using namespace ImGui;
  using namespace mcfile::nbt;

  string filter = filterTerm;
  bool matchedNode = !filter.empty() && (s.fFilterCaseSensitive ? name : ToLower(name)).find(filter) != string::npos;
  if (matchedNode) {
    filter = "";
  }

  int size = 0;
  switch (tag->type()) {
  case Tag::Type::Compound:
    if (!filter.empty()) {
      if (!ContainsTerm(tag, filter, s.fFilterMode, s.fFilterCaseSensitive)) {
        return;
      }
    }
    if (auto v = dynamic_pointer_cast<CompoundTag>(tag); v) {
      size = v->size();
    }
    break;
  case Tag::Type::List:
    if (!filter.empty()) {
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
  PushID(nextPath);

  if (!filter.empty()) {
    SetNextItemOpen(true);
  }
  ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_NavLeftJumpsBackHere;
  if (matchedNode) {
    flags = flags | ImGuiTreeNodeFlags_Selected;
  }
  if (TreeNodeEx(label.c_str(), flags)) {
    Indent(kIndent);

    switch (tag->type()) {
    case Tag::Type::Compound:
      if (auto v = dynamic_pointer_cast<CompoundTag>(tag); v) {
        VisitNbtCompound(s, root, *v, nextPath, filter);
      }
      break;
    case Tag::Type::List:
      if (auto v = dynamic_pointer_cast<ListTag>(tag); v) {
        for (size_t i = 0; i < v->fValue.size(); i++) {
          auto const &it = v->fValue[i];
          auto label = "#" + to_string(i);
          VisitNbt(s, root, label, it, nextPath, filter);
        }
      }
      break;
    case Tag::Type::ByteArray:
      if (auto v = dynamic_pointer_cast<ByteArrayTag>(tag); v) {
        for (size_t i = 0; i < v->fValue.size(); i++) {
          auto label = "#" + to_string(i);
          PushScalarInput(label, nextPath, filter, s.fFilterCaseSensitive, s.fIconDocumentAttributeB);
          InputScalar<uint8_t>(v->fValue[i], root);
          PopScalarInput();
        }
      }
      break;
    case Tag::Type::IntArray:
      if (auto v = dynamic_pointer_cast<IntArrayTag>(tag); v) {
        for (size_t i = 0; i < v->fValue.size(); i++) {
          auto label = "#" + to_string(i);
          PushScalarInput(label, nextPath, filter, s.fFilterCaseSensitive, s.fIconDocumentAttributeI);
          InputScalar<int>(v->fValue[i], root);
          PopScalarInput();
        }
      }
      break;
    case Tag::Type::LongArray:
      if (auto v = dynamic_pointer_cast<LongArrayTag>(tag); v) {
        for (size_t i = 0; i < v->fValue.size(); i++) {
          auto label = "#" + to_string(i);
          PushScalarInput(label, nextPath, filter, s.fFilterCaseSensitive, s.fIconDocumentAttributeL);
          InputScalar<int64_t>(v->fValue[i], root);
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

static void VisitNbt(State &s,
                     Compound &root,
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
    VisitNbtNonScalar(s, root, name, tag, path, filter);
    break;
  default:
    VisitNbtScalar(s, root, name, tag, path, filter);
    break;
  }
}

static void VisitNbtCompound(State &s,
                             Compound &root,
                             mcfile::nbt::CompoundTag const &tag,
                             std::string const &path,
                             std::string const &filter) {
  using namespace std;
  using namespace mcfile::nbt;

  for (auto &it : tag) {
    auto const &name = it.first;
    if (!it.second) {
      continue;
    }
    if (!filter.empty()) {
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
    VisitNbt(s, root, name, it.second, path, filter);
  }
}

static void Visit(State &s,
                  std::shared_ptr<Node> const &node,
                  std::string const &path,
                  std::string const &filter) {
  using namespace std;
  using namespace ImGui;

  if (auto compound = node->compound(); compound) {
    PushID(path + "/" + compound->name());
    if (node->hasParent()) {
      if (!filter.empty() && ContainsTerm(compound->fTag, filter, s.fFilterMode, s.fFilterCaseSensitive)) {
        SetNextItemOpen(true);
      }
      if (TreeNodeEx(compound->name().c_str(), ImGuiTreeNodeFlags_NavLeftJumpsBackHere)) {
        VisitNbtCompound(s, *compound, *compound->fTag, path, filter);
        TreePop();
      }
    } else {
      VisitNbtCompound(s, *compound, *compound->fTag, path, filter);
    }
    PopID();
  } else if (auto contents = node->directoryContents(); contents) {
    string name = contents->fDir.filename().string();
    string label = name + ": " + to_string(contents->fValue.size());
    if (contents->fValue.size() < 2) {
      label += " entry";
    } else {
      label += " entries";
    }
    PushID(path + "/" + name);
    if (node->hasParent()) {
      if (TreeNodeEx(label.c_str(), ImGuiTreeNodeFlags_NavLeftJumpsBackHere)) {
        for (auto const &it : contents->fValue) {
          Visit(s, it, path + "/" + name, filter);
        }
        TreePop();
      }
    } else {
      for (auto const &it : contents->fValue) {
        Visit(s, it, path + "/" + name, filter);
      }
    }
    PopID();
  } else if (auto region = node->region(); region) {
    string name = mcfile::je::Region::GetDefaultRegionFileName(region->fX, region->fZ);
    PushID(path + "/" + name);
    if (node->hasParent()) {
      bool ready = region->wait();
      if (TreeNodeEx(name.c_str(), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_NavLeftJumpsBackHere)) {
        if (ready) {
          for (auto const &it : std::get<0>(region->fValue)) {
            Visit(s, it, path + "/" + name, filter);
          }
        } else {
          Indent(GetTreeNodeToLabelSpacing());
          TextUnformatted("loading...");
          Unindent(GetTreeNodeToLabelSpacing());
        }
        TreePop();
      }
    } else {
      if (region->wait()) {
        for (auto const &it : std::get<0>(region->fValue)) {
          Visit(s, it, path + "/" + name, filter);
        }
      } else {
        Indent(GetTreeNodeToLabelSpacing());
        TextUnformatted("loading...");
        Unindent(GetTreeNodeToLabelSpacing());
      }
    }
    PopID();
  } else if (auto unopenedFile = node->fileUnopened(); unopenedFile) {
    Indent(GetTreeNodeToLabelSpacing());
    PushID(path + "/" + unopenedFile->filename().string());
    PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
    PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
    if (Button(unopenedFile->filename().string().c_str())) {
      node->load(*s.fPool);
    }
    PopStyleVar();
    PopStyleColor();
    PopID();
    Unindent(GetTreeNodeToLabelSpacing());
  } else if (auto unopenedDirectory = node->directoryUnopened(); unopenedDirectory) {
    PushID(path + "/" + unopenedDirectory->filename().string());
    if (TreeNodeEx(unopenedDirectory->filename().string().c_str(), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_NavLeftJumpsBackHere)) {
      node->load(*s.fPool);
      Indent(GetTreeNodeToLabelSpacing());
      TextUnformatted("loading...");
      Unindent(GetTreeNodeToLabelSpacing());
      TreePop();
    }
    PopID();
  } else if (auto unsupported = node->unsupportedFile(); unsupported) {
    Indent(GetTreeNodeToLabelSpacing());
    PushID(path + "/" + unsupported->filename().string());
    TextDisabled("%s", unsupported->filename().string().c_str());
    auto pos = GetItemRectMin();
    auto size = GetItemRectSize();
    auto mouse = GetMousePos();
    if (IsMouseHoveringRect(GetItemRectMin(), GetItemRectMax())) {
      SetTooltip("Unsupported format");
    }
    PopID();
    Unindent(GetTreeNodeToLabelSpacing());
  }
}

static void RenderNode(State &s) {
  if (!s.fOpened) {
    return;
  }
  Visit(s, s.fOpened, "", s.filterTerm());
}

static void RenderFooter(State &s) {
  using namespace ImGui;

  float const frameHeight = GetFrameHeightWithSpacing();

  Begin("footer", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration);
  SetWindowPos(ImVec2(0, s.fDisplaySize.y - frameHeight));
  SetWindowSize(ImVec2(s.fDisplaySize.x, frameHeight));

  if (s.fOpened && !s.fOpenedPath.empty()) {
    PushItemWidth(-FLT_EPSILON);
    auto formatDescription = s.fOpened->description();
    if (formatDescription.empty()) {
      Text("Path: %s", s.fOpenedPath.u8string().c_str());
    } else {
      Text("Path: %s, Format: %s", s.fOpenedPath.u8string().c_str(), formatDescription.c_str());
    }
    PopItemWidth();
  }

  End();
}

static void RenderFilterBar(State &s) {
  using namespace std;
  using namespace ImGui;

  auto const &style = GetStyle();

  if (s.fFilterBarOpened) {
    BeginChild("filter_panel", ImVec2(s.fDisplaySize.x, GetFrameHeightWithSpacing()));

    TextUnformatted("Filter: ");

    SameLine();
    PushID(string("filter_panel#button_case_sensitive"));
    PushStyleColor(ImGuiCol_Text, s.fFilterCaseSensitive ? style.Colors[ImGuiCol_ButtonActive] : style.Colors[ImGuiCol_TextDisabled]);
    PushStyleColor(ImGuiCol_Button, s.fFilterCaseSensitive ? style.Colors[ImGuiCol_Button] : style.Colors[ImGuiCol_ChildBg]);
    if (Button("Aa")) {
      s.fFilterCaseSensitive = !s.fFilterCaseSensitive;
    }
    PopStyleColor(2);
    PopID();

    SameLine();
    PushID(string("filter_panel#text"));
    if (!s.fFilterBarGotFocus || (IsKeyDown(GetModCtrlKeyIndex()) && IsKeyDown(GetKeyIndex(ImGuiKey_F)))) {
      SetKeyboardFocusHere();
      s.fFilterBarGotFocus = true;
    }
    InputText("", &s.fFilter);
    if (IsItemDeactivated() && IsKeyPressed(GetKeyIndex(ImGuiKey_Escape))) {
      s.fFilterBarOpened = false;
    }
    PopID();

    SameLine();
    PushID(string("filter_panel#close"));
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

  if (IsKeyDown(GetModCtrlKeyIndex())) {
    if (IsKeyDown(GetKeyIndex(ImGuiKey_F))) {
      s.fFilterBarOpened = true;
    } else if (IsKeyDown(GetKeyIndex(ImGuiKey_S)) && s.fOpened) {
      Save(s);
    } else if (IsKeyDown(GetKeyIndex(ImGuiKey_O))) {
      if (IsKeyDown(GetKeyIndex(ImGuiKey_ModShift))) {
        if (auto selected = OpenDirectoryDialog(); selected) {
          s.openDirectory(*selected);
        }
      } else {
        if (auto selected = OpenFileDialog(); selected) {
          s.open(*selected);
        }
      }
    }
  }
}

static void Render(State &s) {
  using namespace ImGui;

  ImGuiWindowFlags flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus;
  Begin("main", nullptr, flags);
  SetWindowPos(ImVec2(0, 0));
  SetWindowSize(ImVec2(s.fDisplaySize.x, s.fDisplaySize.y - GetFrameHeightWithSpacing()));

  if (s.fSaveTask) {
    RenderSavingModal(s);
  }

  RenderMainMenu(s);
  RenderErrorPopup(s);
  RenderFilterBar(s);

  BeginChild("editor", ImVec2(0, 0), false, ImGuiWindowFlags_NavFlattened);
  RenderNode(s);
  EndChild();

  RenderAboutDialog(s);
  RenderLegal(s);

  End();

  RenderFooter(s);

  if (!s.fSaveTask) {
    CaptureShortcutKey(s);
  }

  ImGui::Render();

  s.retrieveSaveTask();
}

} // namespace nbte
