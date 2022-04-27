#pragma once

namespace nbte {

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
  using namespace std;
  using namespace ImGui;

  ImGuiWindowFlags flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground;
  Begin("main_menu", nullptr, flags);
  SetWindowPos(ImVec2(0, 0));
  SetWindowSize(s.fDisplaySize);

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
  End();
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
    if (it.second->type() == Tag::Type::Compound) {
      if (auto v = dynamic_pointer_cast<CompoundTag>(it.second); v) {
        SetNextItemOpen(true, ImGuiCond_Once);
        PushID(line);
        line++;
        if (TreeNode(name.c_str())) {
          Indent(kIndent);
          VisitCompoundTag(s, *v, depth + 1, line);
          TreePop();
          Unindent(kIndent);
        }
        PopID();
      }
      continue;
    }

    auto textSize = CalcTextSize(name.c_str());
    Indent(kArrowWidth);
    TextUnformatted(name.c_str());
    SameLine();
    PushItemWidth(s.fDisplaySize.x * 0.5 - textSize.x - kArrowWidth - 30 * depth - 32);
    PushID(line);

    switch (it.second->type()) {
    case Tag::Type::Int:
      if (auto v = dynamic_pointer_cast<IntTag>(it.second); v) {
        int t = v->fValue;
        if (InputInt("", &t)) {
          v->fValue = t;
        }
      }
      break;
    case Tag::Type::Byte:
      if (auto v = dynamic_pointer_cast<ByteTag>(it.second); v) {
        int t = v->fValue;
        if (InputInt("", &t)) {
          v->fValue = Clamp<uint8_t, int>(t);
        }
      }
      break;
    case Tag::Type::Short:
      if (auto v = dynamic_pointer_cast<ShortTag>(it.second); v) {
        int t = v->fValue;
        if (InputInt("", &t)) {
          v->fValue = Clamp<int16_t, int>(t);
        }
      }
      break;
    case Tag::Type::Long:
      if (auto v = dynamic_pointer_cast<LongTag>(it.second); v) {
        string t = to_string(v->fValue);
        if (InputText("", &t, ImGuiInputTextFlags_CharsDecimal)) {
          v->fValue = atoll(t.c_str());
        }
      }
      break;
    case Tag::Type::String:
      if (auto v = dynamic_pointer_cast<StringTag>(it.second); v) {
        InputText("", &v->fValue);
      }
      break;
    case mcfile::nbt::Tag::Type::Float:
      //TODO:
      break;
    case mcfile::nbt::Tag::Type::Double:
      //TODO:
      break;
    case mcfile::nbt::Tag::Type::ByteArray:
      //TODO:
      break;
    case mcfile::nbt::Tag::Type::List:
      //TODO:
      break;
    case mcfile::nbt::Tag::Type::IntArray:
      //TODO:
      break;
    case mcfile::nbt::Tag::Type::LongArray:
      //TODO:
      break;
    default:
      break;
    }

    line++;
    PopID();
    Unindent(kArrowWidth);
    PopItemWidth();
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

  ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove;
  Begin("compound_tag", nullptr, flags);
  float height = GetFrameHeight();
  SetWindowPos(ImVec2(0, height));
  SetWindowSize(ImVec2(s.fDisplaySize.x, s.fDisplaySize.y - height));
  unsigned int line = 0;
  VisitCompoundTag(s, *tag, 0, line);
  End();
}

static void Render(State &s) {
  RenderMainMenu(s);
  RenderErrorPopup(s);
  RenderCompoundTag(s);
  ImGui::Render();
}

} // namespace nbte
