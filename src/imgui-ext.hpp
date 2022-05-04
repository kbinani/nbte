#pragma once

namespace nbte {

bool TreeNode(std::string const &label, ImGuiTreeNodeFlags, std::optional<Texture> icon);

bool IconButton(std::string const &label, std::optional<Texture> icon);

void InlineImage(Texture const &image);

} // namespace nbte
