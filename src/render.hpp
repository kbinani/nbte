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

static void RenderLegal(State &s) {
  using namespace ImGui;

  if (!s.fMainMenuBarHelpOpenSourceLicensesOpened) {
    return;
  }
  OpenPopup("Legal");
  SetNextWindowSize(ImVec2(s.fDisplaySize.x * 0.5f, s.fDisplaySize.y * 0.5f));
  if (BeginPopupModal("Legal", &s.fMainMenuBarHelpOpenSourceLicensesOpened, ImGuiWindowFlags_NoSavedSettings)) {
    TextWrapped("%s", R"(Dear ImGui: https://github.com/ocornut/imgui

The MIT License (MIT)

Copyright (c) 2014-2022 Omar Cornut

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.)");
    TextUnformatted("");
    Separator();
    TextWrapped("%s", R"(GLFW: https://github.com/glfw/glfw

Copyright (c) 2002-2006 Marcus Geelnard

Copyright (c) 2006-2019 Camilla Löwy

This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held liable for any damages arising from the use of this software.

Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:

The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.

Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.

This notice may not be removed or altered from any source distribution.)");
    TextUnformatted("");
    Separator();
    TextWrapped("%s", R"(stb: https://github.com/nothings/stb

MIT License
Copyright (c) 2017 Sean Barrett
Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.)");
    TextUnformatted("");
    Separator();

    TextWrapped("%s", R"(UDEV Gothic: https://github.com/yuru7/udev-gothic

Copyright (c) 2022 Yuko OTAWARA. with Reserved Font Name "UDEV Gothic"

This Font Software is licensed under the SIL Open Font License, Version 1.1.
This license is copied below, and is also available with a FAQ at:
https://scripts.sil.org/OFL


-----------------------------------------------------------
SIL OPEN FONT LICENSE Version 1.1 - 26 February 2007
-----------------------------------------------------------

PREAMBLE
The goals of the Open Font License (OFL) are to stimulate worldwide
development of collaborative font projects, to support the font creation
efforts of academic and linguistic communities, and to provide a free and
open framework in which fonts may be shared and improved in partnership
with others.

The OFL allows the licensed fonts to be used, studied, modified and
redistributed freely as long as they are not sold by themselves. The
fonts, including any derivative works, can be bundled, embedded, 
redistributed and/or sold with any software provided that any reserved
names are not used by derivative works. The fonts and derivatives,
however, cannot be released under any other type of license. The
requirement for fonts to remain under this license does not apply
to any document created using the fonts or their derivatives.

DEFINITIONS
"Font Software" refers to the set of files released by the Copyright
Holder(s) under this license and clearly marked as such. This may
include source files, build scripts and documentation.

"Reserved Font Name" refers to any names specified as such after the
copyright statement(s).

"Original Version" refers to the collection of Font Software components as
distributed by the Copyright Holder(s).

"Modified Version" refers to any derivative made by adding to, deleting,
or substituting -- in part or in whole -- any of the components of the
Original Version, by changing formats or by porting the Font Software to a
new environment.

"Author" refers to any designer, engineer, programmer, technical
writer or other person who contributed to the Font Software.

PERMISSION & CONDITIONS
Permission is hereby granted, free of charge, to any person obtaining
a copy of the Font Software, to use, study, copy, merge, embed, modify,
redistribute, and sell modified and unmodified copies of the Font
Software, subject to the following conditions:

1) Neither the Font Software nor any of its individual components,
in Original or Modified Versions, may be sold by itself.

2) Original or Modified Versions of the Font Software may be bundled,
redistributed and/or sold with any software, provided that each copy
contains the above copyright notice and this license. These can be
included either as stand-alone text files, human-readable headers or
in the appropriate machine-readable metadata fields within text or
binary files as long as those fields can be easily viewed by the user.

3) No Modified Version of the Font Software may use the Reserved Font
Name(s) unless explicit written permission is granted by the corresponding
Copyright Holder. This restriction only applies to the primary font name as
presented to the users.

4) The name(s) of the Copyright Holder(s) or the Author(s) of the Font
Software shall not be used to promote, endorse or advertise any
Modified Version, except to acknowledge the contribution(s) of the
Copyright Holder(s) and the Author(s) or with their explicit written
permission.

5) The Font Software, modified or unmodified, in part or in whole,
must be distributed entirely under this license, and must not be
distributed under any other license. The requirement for fonts to
remain under this license does not apply to any document created
using the Font Software.

TERMINATION
This license becomes null and void if any of the above conditions are
not met.

DISCLAIMER
THE FONT SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO ANY WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT
OF COPYRIGHT, PATENT, TRADEMARK, OR OTHER RIGHT. IN NO EVENT SHALL THE
COPYRIGHT HOLDER BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
INCLUDING ANY GENERAL, SPECIAL, INDIRECT, INCIDENTAL, OR CONSEQUENTIAL
DAMAGES, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF THE USE OR INABILITY TO USE THE FONT SOFTWARE OR FROM
OTHER DEALINGS IN THE FONT SOFTWARE.)");
    TextUnformatted("");
    Separator();

    TextWrapped("%s", R"(nativefiledialog: https://github.com/mlabbe/nativefiledialog

This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.)");
    TextUnformatted("");
    Separator();

    TextWrapped("%s", R"(libminecraft-file: https://github.com/kbinani/libminecraft-file

The MIT License (MIT)

Copyright (c) 2020, 2021 kbinani

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.)");
    TextUnformatted("");
    if (IsKeyDown(GetKeyIndex(ImGuiKey_Escape))) {
      s.fMainMenuBarHelpOpenSourceLicensesOpened = false;
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
