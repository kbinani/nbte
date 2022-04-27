#pragma once

namespace nbte {

constexpr float kIndent = 6.0f;

static void VisitCompoundTag(State &s, mcfile::nbt::CompoundTag const &tag, std::string const &path);
static void Visit(State &s, std::string const &name, std::shared_ptr<mcfile::nbt::Tag> const &tag, std::string const &path);

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
      if (MenuItem("Save", nullptr, nullptr, s.fOpened.index() != 0)) {
        s.save();
      }
      ImGui::EndMenu();
    }
    if (BeginMenu("Find", &s.fMainMenuBarFindSelected)) {
      if (MenuItem("Filter", nullptr, nullptr)) {
        s.fFilterBarOpened = true;
      }
      ImGui::EndMenu();
    }
    EndMenuBar();
  }
}

static void RenderErrorPopup(State &s) {
  using namespace std;
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

template <class T, class U>
static T Clamp(U u) {
  return (T)std::min<U>(std::max<U>(u, (U)std::numeric_limits<T>::lowest()), (U)std::numeric_limits<T>::max());
}

template <class T>
static void InputScalar(T &v) {
  int t = v;
  if (ImGui::InputInt("", &t)) {
    v = Clamp<T, int>(t);
  }
}

template <>
static void InputScalar(int64_t &v) {
  std::string t = std::to_string(v);
  if (ImGui::InputText("", &t, ImGuiInputTextFlags_CharsDecimal)) {
    v = atoll(t.c_str());
  }
}

static void PushScalarInput(std::string const &name, std::string const &path) {
  using namespace ImGui;
  PushItemWidth(-FLT_EPSILON);
  Indent(GetTreeNodeToLabelSpacing());
  PushID((path + "/" + name).c_str());
  TextUnformatted(name.c_str());
  SameLine();
}

static void PopScalarInput() {
  using namespace ImGui;
  PopID();
  Unindent(GetTreeNodeToLabelSpacing());
  PopItemWidth();
}

static void VisitScalar(State &s, std::string const &name, std::shared_ptr<mcfile::nbt::Tag> const &tag, std::string const &path) {
  using namespace std;
  using namespace ImGui;
  using namespace mcfile::nbt;

  PushScalarInput(name, path);

  switch (tag->type()) {
  case Tag::Type::Int:
    if (auto v = dynamic_pointer_cast<IntTag>(tag); v) {
      InputScalar<int>(v->fValue);
    }
    break;
  case Tag::Type::Byte:
    if (auto v = dynamic_pointer_cast<ByteTag>(tag); v) {
      InputScalar<uint8_t>(v->fValue);
    }
    break;
  case Tag::Type::Short:
    if (auto v = dynamic_pointer_cast<ShortTag>(tag); v) {
      InputScalar<int16_t>(v->fValue);
    }
    break;
  case Tag::Type::Long:
    if (auto v = dynamic_pointer_cast<LongTag>(tag); v) {
      InputScalar(v->fValue);
    }
    break;
  case Tag::Type::String:
    if (auto v = dynamic_pointer_cast<StringTag>(tag); v) {
      InputText("", &v->fValue);
    }
    break;
  case mcfile::nbt::Tag::Type::Float:
    if (auto v = dynamic_pointer_cast<FloatTag>(tag); v) {
      InputFloat("", &v->fValue);
    }
    break;
  case mcfile::nbt::Tag::Type::Double:
    if (auto v = dynamic_pointer_cast<DoubleTag>(tag); v) {
      InputDouble("", &v->fValue);
    }
    break;
  default:
    break;
  }

  PopScalarInput();
}

static void VisitNonScalar(State &s, std::string const &name, std::shared_ptr<mcfile::nbt::Tag> const &tag, std::string const &path) {
  using namespace std;
  using namespace ImGui;
  using namespace mcfile::nbt;

  int size = 0;
  switch (tag->type()) {
  case Tag::Type::Compound:
    if (auto v = dynamic_pointer_cast<CompoundTag>(tag); v) {
      size = v->size();
    }
    break;
  case Tag::Type::List:
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

  if (TreeNodeEx(label.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
    Indent(kIndent);

    switch (tag->type()) {
    case Tag::Type::Compound:
      if (auto v = dynamic_pointer_cast<CompoundTag>(tag); v) {
        VisitCompoundTag(s, *v, nextPath);
      }
      break;
    case Tag::Type::List:
      if (auto v = dynamic_pointer_cast<ListTag>(tag); v) {
        for (size_t i = 0; i < v->fValue.size(); i++) {
          auto const &it = v->fValue[i];
          auto label = "#" + to_string(i);
          Visit(s, label, it, nextPath);
        }
      }
      break;
    case Tag::Type::ByteArray:
      if (auto v = dynamic_pointer_cast<ByteArrayTag>(tag); v) {
        for (size_t i = 0; i < v->fValue.size(); i++) {
          auto label = "#" + to_string(i);
          PushScalarInput(label, nextPath);
          InputScalar<uint8_t>(v->fValue[i]);
          PopScalarInput();
        }
      }
      break;
    case Tag::Type::IntArray:
      if (auto v = dynamic_pointer_cast<IntArrayTag>(tag); v) {
        for (size_t i = 0; i < v->fValue.size(); i++) {
          auto label = "#" + to_string(i);
          PushScalarInput(label, nextPath);
          InputScalar<int>(v->fValue[i]);
          PopScalarInput();
        }
      }
      break;
    case Tag::Type::LongArray:
      if (auto v = dynamic_pointer_cast<LongArrayTag>(tag); v) {
        for (size_t i = 0; i < v->fValue.size(); i++) {
          auto label = "#" + to_string(i);
          PushScalarInput(label, nextPath);
          InputScalar<int64_t>(v->fValue[i]);
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

static void Visit(State &s, std::string const &name, std::shared_ptr<mcfile::nbt::Tag> const &tag, std::string const &path) {
  using namespace mcfile::nbt;

  switch (tag->type()) {
  case Tag::Type::Compound:
  case Tag::Type::List:
  case Tag::Type::ByteArray:
  case Tag::Type::IntArray:
  case Tag::Type::LongArray:
    VisitNonScalar(s, name, tag, path);
    break;
  default:
    VisitScalar(s, name, tag, path);
    break;
  }
}

static void VisitCompoundTag(State &s, mcfile::nbt::CompoundTag const &tag, std::string const &path) {
  for (auto &it : tag) {
    auto const &name = it.first;
    if (!it.second) {
      continue;
    }
    Visit(s, name, it.second, path);
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

  VisitCompoundTag(s, *tag, "");
}

static void RenderFooter(State &s) {
  using namespace ImGui;

  float const frameHeight = GetFrameHeightWithSpacing();

  Begin("footer", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration);
  SetWindowPos(ImVec2(0, s.fDisplaySize.y - frameHeight));
  SetWindowSize(ImVec2(s.fDisplaySize.x, frameHeight));

  PushItemWidth(-FLT_EPSILON);
  auto formatDescription = TypeDescription(s.fOpenedFormat);
  Text("Path: %s, Format: %s", s.fOpenedPath.u8string().c_str(), formatDescription.c_str());
  PopItemWidth();
  End();
}

static void Render(State &s) {
  using namespace ImGui;

  float const frameHeight = GetFrameHeightWithSpacing();
  auto const &style = GetStyle();

  ImGuiWindowFlags flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove;
  Begin("main", nullptr, flags);
  SetWindowPos(ImVec2(0, 0));
  SetWindowSize(ImVec2(s.fDisplaySize.x, s.fDisplaySize.y - frameHeight));

  RenderMainMenu(s);
  RenderErrorPopup(s);

  if (s.fFilterBarOpened) {
    BeginChild("filter_panel", ImVec2(s.fDisplaySize.x, frameHeight));
    TextUnformatted("Filter: ");
    SameLine();
    PushID("filter_panel#text");
    PushItemWidth(-FLT_EPSILON);
    InputText("", &s.fFilter);
    PopItemWidth();
    PopID();
    EndChild();
    Separator();
  }

  RenderCompoundTag(s);

  End();

  RenderFooter(s);

  ImGui::Render();
}

} // namespace nbte
