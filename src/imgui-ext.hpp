#pragma once

namespace nbte {

bool TreeNode(std::string const &label, ImGuiTreeNodeFlags flags, std::optional<Texture> icon, std::string const &filter, bool caseSensitive);

bool IconButton(std::string const &label, std::optional<Texture> icon);

void InlineImage(Texture const &image);

void IconLabel(std::string const &label, std::optional<Texture> icon);

} // namespace nbte
