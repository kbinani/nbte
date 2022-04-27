#pragma once

namespace nbte {

constexpr float kIndent = 8.0f;
constexpr float kArrowWidth = 21.0f;

static void VisitCompoundTag(State &s, mcfile::nbt::CompoundTag const &tag, unsigned int &line);
static void Visit(State &s, std::string const &name, std::shared_ptr<mcfile::nbt::Tag> const &tag, unsigned int &line);

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
      EndMenu();
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

static void PushScalarInput(std::string const &name, unsigned int &line) {
  using namespace ImGui;
  PushItemWidth(-FLT_EPSILON);
  Indent(kArrowWidth);
  PushID(line);
  line++;
  TextUnformatted(name.c_str());
  SameLine();
}

static void PopScalarInput() {
  using namespace ImGui;
  PopID();
  Unindent(kArrowWidth);
  PopItemWidth();
}

static void VisitScalar(State &s, std::string const &name, std::shared_ptr<mcfile::nbt::Tag> const &tag, unsigned int &line) {
  using namespace std;
  using namespace ImGui;
  using namespace mcfile::nbt;

  PushScalarInput(name, line);

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

static void VisitNonScalar(State &s, std::string const &name, std::shared_ptr<mcfile::nbt::Tag> const &tag, unsigned int &line) {
  using namespace std;
  using namespace ImGui;
  using namespace mcfile::nbt;

  PushID(line);
  line++;
  if (TreeNodeEx(name.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
    Indent(kIndent);

    switch (tag->type()) {
    case Tag::Type::Compound:
      if (auto v = dynamic_pointer_cast<CompoundTag>(tag); v) {
        VisitCompoundTag(s, *v, line);
      }
      break;
    case Tag::Type::List:
      if (auto v = dynamic_pointer_cast<ListTag>(tag); v) {
        for (size_t i = 0; i < v->fValue.size(); i++) {
          auto const &it = v->fValue[i];
          Visit(s, "#" + to_string(i), it, line);
        }
      }
      break;
    case Tag::Type::ByteArray:
      if (auto v = dynamic_pointer_cast<ByteArrayTag>(tag); v) {
        for (size_t i = 0; i < v->fValue.size(); i++) {
          PushScalarInput("#" + to_string(i), line);
          InputScalar<uint8_t>(v->fValue[i]);
          PopScalarInput();
        }
      }
      break;
    case Tag::Type::IntArray:
      if (auto v = dynamic_pointer_cast<IntArrayTag>(tag); v) {
        for (size_t i = 0; i < v->fValue.size(); i++) {
          PushScalarInput("#" + to_string(i), line);
          InputScalar<int>(v->fValue[i]);
          PopScalarInput();
        }
      }
      break;
    case Tag::Type::LongArray:
      if (auto v = dynamic_pointer_cast<LongArrayTag>(tag); v) {
        for (size_t i = 0; i < v->fValue.size(); i++) {
          PushScalarInput("#" + to_string(i), line);
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

static void Visit(State &s, std::string const &name, std::shared_ptr<mcfile::nbt::Tag> const &tag, unsigned int &line) {
  using namespace mcfile::nbt;

  switch (tag->type()) {
  case Tag::Type::Compound:
  case Tag::Type::List:
  case Tag::Type::ByteArray:
  case Tag::Type::IntArray:
  case Tag::Type::LongArray:
    VisitNonScalar(s, name, tag, line);
    break;
  default:
    VisitScalar(s, name, tag, line);
    break;
  }
}

static void VisitCompoundTag(State &s, mcfile::nbt::CompoundTag const &tag, unsigned int &line) {
  for (auto &it : tag) {
    auto const &name = it.first;
    if (!it.second) {
      continue;
    }
    Visit(s, name, it.second, line);
  }
}

static void RenderCompoundTag(State &s) {
  using namespace std;
  using namespace ImGui;
  using namespace mcfile::nbt;

  if (s.fOpened.index() != 0) {
    return;
  }
  shared_ptr<CompoundTag> const &tag = get<0>(s.fOpened);
  if (!tag) {
    return;
  }

  float height = GetFrameHeight();
  unsigned int line = 0;
  VisitCompoundTag(s, *tag, line);
}

static void Render(State &s) {
  using namespace ImGui;

  ImGuiWindowFlags flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove;
  Begin("main", nullptr, flags);
  SetWindowPos(ImVec2(0, 0));
  SetWindowSize(s.fDisplaySize);

  RenderMainMenu(s);
  RenderErrorPopup(s);
  RenderCompoundTag(s);

  End();

  ImGui::Render();
}

} // namespace nbte
