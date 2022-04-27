#pragma once

namespace nbte {

static void VisitCompoundTag(State &s, mcfile::nbt::CompoundTag const &tag, unsigned int depth, unsigned int &line);

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

static void VisitScalar(State &s, std::shared_ptr<mcfile::nbt::Tag> const &tag) {
  using namespace std;
  using namespace ImGui;
  using namespace mcfile::nbt;

  switch (tag->type()) {
  case Tag::Type::Int:
    if (auto v = dynamic_pointer_cast<IntTag>(tag); v) {
      int t = v->fValue;
      if (InputInt("", &t)) {
        v->fValue = t;
      }
    }
    break;
  case Tag::Type::Byte:
    if (auto v = dynamic_pointer_cast<ByteTag>(tag); v) {
      int t = v->fValue;
      if (InputInt("", &t)) {
        v->fValue = Clamp<uint8_t, int>(t);
      }
    }
    break;
  case Tag::Type::Short:
    if (auto v = dynamic_pointer_cast<ShortTag>(tag); v) {
      int t = v->fValue;
      if (InputInt("", &t)) {
        v->fValue = Clamp<int16_t, int>(t);
      }
    }
    break;
  case Tag::Type::Long:
    if (auto v = dynamic_pointer_cast<LongTag>(tag); v) {
      string t = to_string(v->fValue);
      if (InputText("", &t, ImGuiInputTextFlags_CharsDecimal)) {
        v->fValue = atoll(t.c_str());
      }
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
}

static void VisitNonScalar(State &s, std::shared_ptr<mcfile::nbt::Tag> const &tag, unsigned int depth, unsigned int &line) {
  using namespace std;
  using namespace ImGui;
  using namespace mcfile::nbt;

  switch (tag->type()) {
  case Tag::Type::Compound:
    if (auto v = dynamic_pointer_cast<CompoundTag>(tag); v) {
      VisitCompoundTag(s, *v, depth + 1, line);
    }
    break;
  case Tag::Type::List:
    if (auto v = dynamic_pointer_cast<ListTag>(tag); v) {
    }
    break;
  }
}

static void VisitCompoundTag(State &s, mcfile::nbt::CompoundTag const &tag, unsigned int depth, unsigned int &line) {
  using namespace std;
  using namespace ImGui;
  using namespace mcfile::nbt;

  constexpr float kIndent = 8.0f;
  constexpr float kArrowWidth = 21.0f;

  auto const &style = GetStyle();
  float currentIndent = depth * kIndent;

  for (auto &it : tag) {
    string const &name = it.first;
    if (!it.second) {
      continue;
    }
    switch (it.second->type()) {
    case Tag::Type::Compound:
    case Tag::Type::List:
    case Tag::Type::ByteArray:
    case Tag::Type::IntArray:
    case Tag::Type::LongArray:
      SetNextItemOpen(true, ImGuiCond_Once);
      PushID(line);
      line++;
      if (TreeNode(name.c_str())) {
        Indent(kIndent);

        VisitNonScalar(s, it.second, depth, line);

        TreePop();
        Unindent(kIndent);
      }
      PopID();
      break;
    default: {
      PushItemWidth(-FLT_EPSILON);
      Indent(kArrowWidth);
      TextUnformatted(name.c_str());
      SameLine();
      PushID(line);

      VisitScalar(s, it.second);

      line++;
      PopID();
      Unindent(kArrowWidth);
      PopItemWidth();

      break;
    }
    }
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
  VisitCompoundTag(s, *tag, 0, line);
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
