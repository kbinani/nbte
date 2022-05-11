#pragma once

namespace nbte {

constexpr int kFramePaddingX = 4;
constexpr int kFramePaddingY = 3;
constexpr int kWindowPadding = 8;
constexpr int kFontSize = 15;
constexpr int kItemSpacingX = 8;
constexpr int kItemSpacingY = 4;
constexpr int kItemInnerSpacing = 4;

constexpr int kFrameHeight = kFontSize + kFramePaddingY * 2;
constexpr int kFrameHeightWithSpacing = kFrameHeight + kItemSpacingY;
constexpr int kTreeNodeToLabelSpacing = kFontSize + kFramePaddingX * 2;

constexpr int kIconSize = 16;

} // namespace nbte
