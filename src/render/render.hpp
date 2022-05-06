#pragma once

namespace nbte {

constexpr float kIndent = 6.0f;

static void VisitNbtCompound(State &s,
                             Compound &root,
                             mcfile::nbt::CompoundTag const &tag,
                             String const &path,
                             String const &filter);
static void VisitNbt(State &s,
                     Compound &root,
                     String const &name,
                     std::shared_ptr<mcfile::nbt::Tag> const &tag,
                     String const &path,
                     String const &filter);
static void Visit(State &s,
                  std::shared_ptr<Node> const &node,
                  String const &path,
                  String const &filter);

static void RenderSavingModal(State &s) {
  OpenPopup(u8"Info");
  im::SetNextWindowSize(ImVec2(512, 0));
  if (BeginPopupModal(u8"Info", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize)) {
    TextUnformatted(u8"Saving edited files...");
    im::EndPopup();
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
  if (im::BeginMenuBar()) {
    if (BeginMenu(u8"File", &s.fMainMenuBarFileSelected)) {
      if (MenuItem(u8"Open", DecorateModCtrl(u8"O"), nullptr)) {
        if (auto selected = OpenFileDialog(); selected) {
          s.open(*selected);
        }
      }
      if (MenuItem(u8"Open Folder", DecorateModCtrl(u8"Shift+O"), nullptr)) {
        if (auto selected = OpenDirectoryDialog(); selected) {
          s.openDirectory(*selected);
        }
      }
      if (s.fMinecraftSaveDirectory) {
        if (MenuItem(u8"Open Minecraft Save Directory", {}, nullptr)) {
          s.openDirectory(*s.fMinecraftSaveDirectory);
        }
      }
      if (MenuItem(u8"Save", DecorateModCtrl(u8"S"), nullptr, s.canSave())) {
        Save(s);
      }
      im::Separator();
      if (MenuItem(u8"Quit", QuitMenuShortcut(), nullptr)) {
        s.fMainMenuBarQuitSelected = true;
      }
      im::EndMenu();
    }
    if (BeginMenu(u8"Find", &s.fMainMenuBarFindSelected)) {
      if (MenuItem(u8"Filter", DecorateModCtrl(u8"F"), nullptr)) {
        s.fFilterBarOpened = true;
      }
      im::EndMenu();
    }
    if (BeginMenu(u8"Help", &s.fMainMenuBarHelpSelected)) {
      if (MenuItem(u8"About nbte", {}, nullptr)) {
        s.fMainMenuBarHelpAboutOpened = true;
      }
      if (MenuItem(u8"Legal", {}, nullptr)) {
        s.fMainMenuBarHelpOpenSourceLicensesOpened = true;
      }
      im::EndMenu();
    }
    im::EndMenuBar();
  }
}

static void RenderErrorPopup(State &s) {
  if (s.fError.empty()) {
    return;
  }
  OpenPopup(u8"Error");
  if (BeginPopupModal(u8"Error", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize)) {
    TextUnformatted(s.fError);
    if (Button(u8"OK")) {
      s.fError.clear();
      im::CloseCurrentPopup();
    }
    im::EndPopup();
  }
}

static void RenderAboutDialog(State &s) {
  if (!s.fMainMenuBarHelpAboutOpened) {
    return;
  }
  OpenPopup(u8"About");
  im::SetNextWindowSize(ImVec2(512, 0), ImGuiCond_Once);
  if (BeginPopupModal(u8"About", &s.fMainMenuBarHelpAboutOpened)) {
    TextUnformatted(u8"nbte: https://github.com/kbinani/nbte");
    TextUnformatted(String(u8"Version ") + kAppVersion);
    TextUnformatted(u8"");
    TextUnformatted(u8"Copyright © 2022 kbinani");
    TextUnformatted(u8"");
    TextWrapped(u8R"(This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.)");
    if (im::IsKeyDown(im::GetKeyIndex(ImGuiKey_Escape))) {
      s.fMainMenuBarHelpAboutOpened = false;
      im::CloseCurrentPopup();
    }
    im::EndPopup();
  }
}

template <class T, class U>
static T Clamp(U u) {
  return (T)std::min<U>(std::max<U>(u, (U)std::numeric_limits<T>::lowest()), (U)std::numeric_limits<T>::max());
}

static bool ContainsTerm(std::shared_ptr<mcfile::nbt::Tag> const &tag,
                         String const &filter,
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
          String key = ReinterpretAsU8String(it.first);
          if ((caseSensitive ? key : ToLower(key)).find(filter) != String::npos) {
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
        String value = ReinterpretAsU8String(v->fValue);
        return value.find(filter) != String::npos;
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
static ImGuiDataType DataType() {
  if (std::is_same_v<T, uint8_t>) {
    return ImGuiDataType_U8;
  } else if (std::is_same_v<T, int8_t>) {
    return ImGuiDataType_S8;
  } else if (std::is_same_v<T, uint16_t>) {
    return ImGuiDataType_S16;
  } else if (std::is_same_v<T, int16_t>) {
    return ImGuiDataType_U16;
  } else if (std::is_same_v<T, uint32_t>) {
    return ImGuiDataType_U32;
  } else if (std::is_same_v<T, int32_t>) {
    return ImGuiDataType_S32;
  } else if (std::is_same_v<T, uint64_t>) {
    return ImGuiDataType_U64;
  } else if (std::is_same_v<T, int64_t>) {
    return ImGuiDataType_S64;
  } else {
    assert(false);
    return ImGuiDataType_S32;
  }
}

template <std::integral T>
static void InputScalar(T &v, Compound &root) {
  ImGuiDataType type = DataType<T>();
  T step = 1;
  if (im::InputScalar("", type, &v, &step)) {
    root.fEdited = true;
  }
}

static void PushScalarInput(String const &name,
                            String const &path,
                            String const &filter,
                            bool filterCaseSensitive,
                            std::optional<Texture> const &icon) {
  using namespace std;
  auto const &style = im::GetStyle();
  im::PushItemWidth(-FLT_EPSILON);
  im::Indent(im::GetTreeNodeToLabelSpacing());
  if (icon) {
    InlineImage(*icon);
    auto cursor = im::GetCursorPos();
    auto style = im::GetStyle();
    im::SetCursorPos(ImVec2(cursor.x + style.FramePadding.x, cursor.y));
  }
  PushID(path + u8"/" + name);
  if (!filter.empty()) {
    ImDrawList *list = im::GetWindowDrawList();
    auto cursor = im::GetCursorScreenPos();
    auto color = im::GetColorU32(ImGuiCol_Button);
    size_t pos = 0;
    while (true) {
      size_t found = (filterCaseSensitive ? name : ToLower(name)).find(filter, pos);
      if (found == String::npos) {
        break;
      } else {
        auto leading = CalcTextSize(name.substr(0, found));
        auto trailing = CalcTextSize(name.substr(0, found + filter.size()));
        list->AddRectFilled(ImVec2(cursor.x + leading.x, cursor.y + style.FramePadding.y), ImVec2(cursor.x + trailing.x, cursor.y + style.FramePadding.y + trailing.y), color, 2.0f);
        pos = found + filter.size();
      }
    }
  }
  {
    auto cursor = im::GetCursorPos();
    im::SetCursorPos(ImVec2(cursor.x, cursor.y + style.FramePadding.y));
    TextUnformatted(name);
  }
  im::SameLine();
}

static void PopScalarInput() {
  im::PopID();
  im::Unindent(im::GetTreeNodeToLabelSpacing());
  im::PopItemWidth();
}

static void VisitNbtScalar(State &s,
                           Compound &root,
                           String const &name,
                           std::shared_ptr<mcfile::nbt::Tag> const &tag,
                           String const &path,
                           String const &filter) {
  using namespace std;
  using namespace mcfile::nbt;

  switch (tag->type()) {
  case Tag::Type::Int:
    if (auto v = dynamic_pointer_cast<IntTag>(tag); v) {
      PushScalarInput(name, path, filter, s.fFilterCaseSensitive, s.fTextures.fIconDocumentAttributeI);
      InputScalar<int>(v->fValue, root);
    }
    break;
  case Tag::Type::Byte:
    if (auto v = dynamic_pointer_cast<ByteTag>(tag); v) {
      PushScalarInput(name, path, filter, s.fFilterCaseSensitive, s.fTextures.fIconDocumentAttributeB);
      InputScalar<uint8_t>(v->fValue, root);
    }
    break;
  case Tag::Type::Short:
    if (auto v = dynamic_pointer_cast<ShortTag>(tag); v) {
      PushScalarInput(name, path, filter, s.fFilterCaseSensitive, s.fTextures.fIconDocumentAttributeS);
      InputScalar<int16_t>(v->fValue, root);
    }
    break;
  case Tag::Type::Long:
    if (auto v = dynamic_pointer_cast<LongTag>(tag); v) {
      PushScalarInput(name, path, filter, s.fFilterCaseSensitive, s.fTextures.fIconDocumentAttributeL);
      InputScalar(v->fValue, root);
    }
    break;
  case Tag::Type::String:
    if (auto v = dynamic_pointer_cast<StringTag>(tag); v) {
      PushScalarInput(name, path, filter, s.fFilterCaseSensitive, s.fTextures.fIconEditSmallCaps);
      String value = ReinterpretAsU8String(v->fValue);
      if (InputText(u8"", &value)) {
        v->fValue = ReinterpretAsStdString(value);
        root.fEdited = true;
      }
    }
    break;
  case mcfile::nbt::Tag::Type::Float:
    if (auto v = dynamic_pointer_cast<FloatTag>(tag); v) {
      PushScalarInput(name, path, filter, s.fFilterCaseSensitive, s.fTextures.fIconDocumentAttributeF);
      if (InputFloat(u8"", &v->fValue)) {
        root.fEdited = true;
      }
    }
    break;
  case mcfile::nbt::Tag::Type::Double:
    if (auto v = dynamic_pointer_cast<DoubleTag>(tag); v) {
      PushScalarInput(name, path, filter, s.fFilterCaseSensitive, s.fTextures.fIconDocumentAttributeD);
      if (InputDouble(u8"", &v->fValue)) {
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
                              String const &name,
                              std::shared_ptr<mcfile::nbt::Tag> const &tag,
                              String const &path,
                              String const &filterTerm) {
  using namespace std;
  using namespace mcfile::nbt;

  String filter = filterTerm;
  bool matchedNode = !filter.empty() && (s.fFilterCaseSensitive ? name : ToLower(name)).find(filter) != String::npos;
  if (matchedNode) {
    filter.clear();
  }

  optional<Texture> icon = nullopt;
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
    icon = s.fTextures.fIconBox;
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
    icon = s.fTextures.fIconEditList;
    break;
  case Tag::Type::ByteArray:
    if (auto v = dynamic_pointer_cast<ByteArrayTag>(tag); v) {
      size = v->fValue.size();
    }
    icon = s.fTextures.fIconEditCode;
    break;
  case Tag::Type::IntArray:
    if (auto v = dynamic_pointer_cast<IntArrayTag>(tag); v) {
      size = v->fValue.size();
    }
    icon = s.fTextures.fIconEditCode;
    break;
  case Tag::Type::LongArray:
    if (auto v = dynamic_pointer_cast<LongArrayTag>(tag); v) {
      size = v->fValue.size();
    }
    icon = s.fTextures.fIconEditCode;
    break;
  }
  String label = name + u8": ";
  if (size < 2) {
    label += ToString(size) + u8" entry";
  } else {
    label += ToString(size) + u8" entries";
  }

  auto nextPath = path + u8"/" + name;
  PushID(nextPath);

  TreeNodeOptions opt;
  if (!filter.empty()) {
    opt.openIgnoringStorage = true;
  }
  if (size == 0) {
    opt.disable = true;
  }
  ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_NavLeftJumpsBackHere;
  if (matchedNode) {
    flags = flags | ImGuiTreeNodeFlags_Selected;
  }
  if (TreeNode(label, flags, icon, s.filterTerm(), s.fFilterCaseSensitive, opt)) {
    im::Indent(kIndent);

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
          auto label = u8"#" + ToString(i);
          VisitNbt(s, root, label, it, nextPath, filter);
        }
      }
      break;
    case Tag::Type::ByteArray:
      if (auto v = dynamic_pointer_cast<ByteArrayTag>(tag); v) {
        for (size_t i = 0; i < v->fValue.size(); i++) {
          auto label = u8"#" + ToString(i);
          PushScalarInput(label, nextPath, filter, s.fFilterCaseSensitive, s.fTextures.fIconDocumentAttributeB);
          InputScalar<uint8_t>(v->fValue[i], root);
          PopScalarInput();
        }
      }
      break;
    case Tag::Type::IntArray:
      if (auto v = dynamic_pointer_cast<IntArrayTag>(tag); v) {
        for (size_t i = 0; i < v->fValue.size(); i++) {
          auto label = u8"#" + ToString(i);
          PushScalarInput(label, nextPath, filter, s.fFilterCaseSensitive, s.fTextures.fIconDocumentAttributeI);
          InputScalar<int>(v->fValue[i], root);
          PopScalarInput();
        }
      }
      break;
    case Tag::Type::LongArray:
      if (auto v = dynamic_pointer_cast<LongArrayTag>(tag); v) {
        for (size_t i = 0; i < v->fValue.size(); i++) {
          auto label = u8"#" + ToString(i);
          PushScalarInput(label, nextPath, filter, s.fFilterCaseSensitive, s.fTextures.fIconDocumentAttributeL);
          InputScalar<int64_t>(v->fValue[i], root);
          PopScalarInput();
        }
      }
      break;
    }

    im::TreePop();
    im::Unindent(kIndent);
  }
  im::PopID();
}

static void VisitNbt(State &s,
                     Compound &root,
                     String const &name,
                     std::shared_ptr<mcfile::nbt::Tag> const &tag,
                     String const &path,
                     String const &filter) {
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
                             String const &path,
                             String const &filter) {
  using namespace std;
  using namespace mcfile::nbt;

  for (auto &it : tag) {
    String name = ReinterpretAsU8String(it.first);
    if (!it.second) {
      continue;
    }
    if (!filter.empty()) {
      if (s.fFilterMode == FilterMode::Key) {
        if ((s.fFilterCaseSensitive ? name : ToLower(name)).find(filter) == String::npos && !ContainsTerm(it.second, filter, s.fFilterMode, s.fFilterCaseSensitive)) {
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
                  String const &path,
                  String const &filter) {
  using namespace std;

  auto const &style = im::GetStyle();
  float frameHeight = im::GetFrameHeight();

  if (auto compound = node->compound(); compound) {
    PushID(path + u8"/" + compound->name());
    if (node->hasParent()) {
      TreeNodeOptions opt;
      if (!filter.empty() && ContainsTerm(compound->fTag, filter, s.fFilterMode, s.fFilterCaseSensitive)) {
        opt.openIgnoringStorage = true;
      }
      ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_NavLeftJumpsBackHere;
      if (TreeNode(compound->name(), flags, s.fTextures.fIconBox, s.filterTerm(), s.fFilterCaseSensitive, opt)) {
        VisitNbtCompound(s, *compound, *compound->fTag, path, filter);
        im::TreePop();
      }
    } else {
      VisitNbtCompound(s, *compound, *compound->fTag, path, filter);
    }
    im::PopID();
  } else if (auto contents = node->directoryContents(); contents) {
    String name = contents->fDir.filename().u8string();
    String label = name + u8": " + ToString(contents->fValue.size());
    if (contents->fValue.size() < 2) {
      label += u8" entry";
    } else {
      label += u8" entries";
    }
    TreeNodeOptions opt;
    if (contents->fValue.empty()) {
      opt.disable = true;
    }
    PushID(path + u8"/" + name);
    if (node->hasParent()) {
      if (TreeNode(label, ImGuiTreeNodeFlags_NavLeftJumpsBackHere, s.fTextures.fIconFolder, s.filterTerm(), s.fFilterCaseSensitive, opt)) {
        for (auto const &it : contents->fValue) {
          Visit(s, it, path + u8"/" + name, filter);
        }
        im::TreePop();
      }
    } else {
      for (auto const &it : contents->fValue) {
        Visit(s, it, path + u8"/" + name, filter);
      }
    }
    im::PopID();
  } else if (auto region = node->region(); region) {
    std::string rawName = mcfile::je::Region::GetDefaultRegionFileName(region->fX, region->fZ);
    String name = ReinterpretAsU8String(rawName);
    PushID(path + u8"/" + name);
    if (node->hasParent()) {
      bool ready = region->wait();
      if (TreeNode(name, ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_NavLeftJumpsBackHere, s.fTextures.fIconBlock, s.filterTerm(), s.fFilterCaseSensitive)) {
        if (ready) {
          for (auto const &it : std::get<0>(region->fValue)) {
            Visit(s, it, path + u8"/" + name, filter);
          }
        } else {
          im::Indent(im::GetTreeNodeToLabelSpacing());
          TextUnformatted(u8"loading...");
          im::Unindent(im::GetTreeNodeToLabelSpacing());
        }
        im::TreePop();
      }
    } else {
      if (region->wait()) {
        for (auto const &it : std::get<0>(region->fValue)) {
          Visit(s, it, path + u8"/" + name, filter);
        }
      } else {
        im::Indent(im::GetTreeNodeToLabelSpacing());
        TextUnformatted(u8"loading...");
        im::Unindent(im::GetTreeNodeToLabelSpacing());
      }
    }
    im::PopID();
  } else if (auto unopenedFile = node->fileUnopened(); unopenedFile) {
    String name = unopenedFile->filename().u8string();
    PushID(path + u8"/" + name);
    if (auto pos = mcfile::je::Region::RegionXZFromFile(*unopenedFile); pos) {
      if (TreeNode(name, 0, s.fTextures.fIconBlock, s.filterTerm(), s.fFilterCaseSensitive)) {
        node->load(*s.fPool);
        im::TreePop();
      }
    } else {
      im::Indent(im::GetTreeNodeToLabelSpacing());
      if (IconButton(name, s.fTextures.fIconDocument)) {
        node->load(*s.fPool);
      }
      im::Unindent(im::GetTreeNodeToLabelSpacing());
    }
    im::PopID();
  } else if (auto unopenedChunk = node->unopenedChunk(); unopenedChunk) {
    String name = unopenedChunk->name();
    PushID(path + u8"/" + name);
    auto icon = s.fTextures.fIconBox;
    if (TreeNode(name, 0, icon, s.filterTerm(), s.fFilterCaseSensitive)) {
      node->load(*s.fPool);
      im::TreePop();
    }
    im::PopID();
  } else if (auto unopenedDirectory = node->directoryUnopened(); unopenedDirectory) {
    PushID(path + u8"/" + unopenedDirectory->filename().u8string());
    if (TreeNode(unopenedDirectory->filename().u8string(), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_NavLeftJumpsBackHere, s.fTextures.fIconFolder, s.filterTerm(), s.fFilterCaseSensitive)) {
      node->load(*s.fPool);
      im::Indent(im::GetTreeNodeToLabelSpacing());
      TextUnformatted(u8"loading...");
      im::Unindent(im::GetTreeNodeToLabelSpacing());
      im::TreePop();
    }
    im::PopID();
  } else if (auto unsupported = node->unsupportedFile(); unsupported) {
    im::Indent(im::GetTreeNodeToLabelSpacing());
    PushID(path + u8"/" + unsupported->filename().u8string());
    im::PushStyleColor(ImGuiCol_Text, style.Colors[ImGuiCol_TextDisabled]);
    IconLabel(unsupported->filename().u8string(), s.fTextures.fIconDocumentExclamation);
    im::PopStyleColor();
    auto pos = im::GetItemRectMin();
    auto size = im::GetItemRectSize();
    auto mouse = im::GetMousePos();
    if (im::IsMouseHoveringRect(im::GetItemRectMin(), im::GetItemRectMax())) {
      SetTooltip(u8"Unsupported format");
    }
    im::PopID();
    im::Unindent(im::GetTreeNodeToLabelSpacing());
  }
}

static void RenderNode(State &s) {
  if (!s.fOpened) {
    return;
  }
  Visit(s, s.fOpened, u8"", s.filterTerm());
}

static void RenderFooter(State &s) {
  float const frameHeight = im::GetFrameHeightWithSpacing();

  Begin(u8"footer", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration);
  im::SetWindowPos(ImVec2(0, s.fDisplaySize.y - frameHeight));
  im::SetWindowSize(ImVec2(s.fDisplaySize.x, frameHeight));

  if (s.fOpened && !s.fOpenedPath.empty()) {
    im::PushItemWidth(-FLT_EPSILON);
    auto formatDescription = s.fOpened->description();
    if (formatDescription.empty()) {
      TextUnformatted(u8"Path: " + s.fOpenedPath.u8string());
    } else {
      TextUnformatted(u8"Path: " + s.fOpenedPath.u8string() + u8", Format: " + formatDescription);
    }
    im::PopItemWidth();
  }

  im::End();
}

static void RenderFilterBar(State &s) {
  using namespace std;

  auto const &style = im::GetStyle();

  if (s.fFilterBarOpened) {
    BeginChild(u8"filter_panel", ImVec2(s.fDisplaySize.x, im::GetFrameHeightWithSpacing()));

    TextUnformatted(u8"Filter: ");

    im::SameLine();
    PushID(u8"filter_panel#button_case_sensitive");
    im::PushStyleColor(ImGuiCol_Text, s.fFilterCaseSensitive ? style.Colors[ImGuiCol_ButtonActive] : style.Colors[ImGuiCol_TextDisabled]);
    im::PushStyleColor(ImGuiCol_Button, s.fFilterCaseSensitive ? style.Colors[ImGuiCol_Button] : style.Colors[ImGuiCol_ChildBg]);
    if (Button(u8"Aa")) {
      s.fFilterCaseSensitive = !s.fFilterCaseSensitive;
    }
    im::PopStyleColor(2);
    im::PopID();

    im::SameLine();
    PushID(u8"filter_panel#text");
    if (!s.fFilterBarGotFocus || (im::IsKeyDown(GetModCtrlKeyIndex()) && im::IsKeyDown(im::GetKeyIndex(ImGuiKey_F)))) {
      im::SetKeyboardFocusHere();
      s.fFilterBarGotFocus = true;
    }
    InputText(u8"", &s.fFilter);
    if (im::IsItemDeactivated() && im::IsKeyPressed(im::GetKeyIndex(ImGuiKey_Escape))) {
      s.fFilterBarOpened = false;
    }
    im::PopID();

    im::SameLine();
    PushID(u8"filter_panel#close");
    if (Button(u8"x", ImVec2(im::GetFrameHeight(), im::GetFrameHeight()))) {
      s.fFilterBarOpened = false;
    }
    im::PopID();

    im::EndChild();
    im::Separator();
  } else {
    s.fFilterBarGotFocus = false;
  }
}

static void CaptureShortcutKey(State &s) {
  if (im::IsKeyDown(GetModCtrlKeyIndex())) {
    if (im::IsKeyDown(im::GetKeyIndex(ImGuiKey_F))) {
      s.fFilterBarOpened = true;
    } else if (im::IsKeyDown(im::GetKeyIndex(ImGuiKey_S)) && s.fOpened) {
      Save(s);
    } else if (im::IsKeyDown(im::GetKeyIndex(ImGuiKey_O))) {
      if (im::IsKeyDown(im::GetKeyIndex(ImGuiKey_ModShift))) {
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
  ImGuiStyle const &style = im::GetStyle();
  ImVec4 bg = style.Colors[ImGuiCol_WindowBg];
  if (s.fFilterBarOpened && !s.fFilter.empty()) {
    float v = 230.0f / 255.0f;
    bg = ImVec4(v, v, v, 1.0f);
  }
  im::PushStyleColor(ImGuiCol_WindowBg, im::GetColorU32(bg));

  ImGuiWindowFlags flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus;
  Begin(u8"main", nullptr, flags);
  im::SetWindowPos(ImVec2(0, 0));
  im::SetWindowSize(ImVec2(s.fDisplaySize.x, s.fDisplaySize.y - im::GetFrameHeightWithSpacing()));

  if (s.fSaveTask) {
    RenderSavingModal(s);
  }

  RenderMainMenu(s);
  RenderErrorPopup(s);
  RenderFilterBar(s);

  BeginChild(u8"editor", ImVec2(0, 0), false, ImGuiWindowFlags_NavFlattened);
  RenderNode(s);
  im::EndChild();

  RenderAboutDialog(s);
  RenderLegal(s);

  im::PopStyleColor();
  im::End();

  RenderFooter(s);

  if (!s.fSaveTask) {
    CaptureShortcutKey(s);
  }

#if !defined(NDEBUG)
  ImGui::ShowMetricsWindow();
#endif

  ImGui::Render();

  s.retrieveSaveTask();
}

} // namespace nbte
