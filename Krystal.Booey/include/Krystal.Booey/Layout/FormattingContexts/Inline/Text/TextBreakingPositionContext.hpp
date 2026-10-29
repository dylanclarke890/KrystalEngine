#pragma once

#include "Krystal.Booey/Core/TextFlags.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Hash.hpp"
#include "Krystal.Core/Types/StringAtom.hpp"

namespace krys::boo
{
  class RenderStyle;
}

namespace krys::boo::layout
{
  enum class WhiteSpaceCollapseBehavior : uint8
  {
    CollapseAll,
    CollapseWhitespaceSequence,
    Preserve
  };

  KRYS_NODISCARD constexpr WhiteSpaceCollapseBehavior
    BehaviorForWhiteSpaceCollapse(WhiteSpaceCollapse collapse) noexcept
  {
    switch (collapse)
    {
      case WhiteSpaceCollapse::Collapse:       return WhiteSpaceCollapseBehavior::CollapseAll;
      case WhiteSpaceCollapse::PreserveBreaks: return WhiteSpaceCollapseBehavior::CollapseWhitespaceSequence;
      case WhiteSpaceCollapse::Preserve:
      case WhiteSpaceCollapse::BreakSpaces:
        // FIXME: WhiteSpaceCollapse::PreserveSpaces should also be handled this way.
        // WhiteSpaceCollapse::Preserve and WhiteSpaceCollapse::BreakSpaces
        // have the same text breaking positions so we don't need to recalculate
        // breaking points when switching between these behaviors.
        return WhiteSpaceCollapseBehavior::Preserve;
    }
    krys_unreachable();
  }

  /// @brief The style that decides where text can break, which breaking positions are cached under.
  struct TextBreakingPositionContext
  {
    WhiteSpaceCollapseBehavior WhitespaceCollapseBehavior {WhiteSpaceCollapseBehavior::CollapseAll};
    boo::OverflowWrap OverflowWrap {boo::OverflowWrap::Normal};
    boo::LineBreak LineBreak {boo::LineBreak::Normal};
    boo::WordBreak WordBreak {boo::WordBreak::Normal};
    StringAtom Locale {StringAtom::Null()};

    explicit TextBreakingPositionContext(const RenderStyle &style) noexcept;
    TextBreakingPositionContext() noexcept = default;

    KRYS_NODISCARD friend bool operator==(const TextBreakingPositionContext &,
                                          const TextBreakingPositionContext &) noexcept = default;
  };
}

namespace krys
{
  template <>
  struct DefaultHash<boo::layout::TextBreakingPositionContext>
  {
    KRYS_NODISCARD static size_t Hash(const boo::layout::TextBreakingPositionContext &context) noexcept
    {
      return Hash::Combine(context.WhitespaceCollapseBehavior, context.OverflowWrap, context.LineBreak,
                           context.WordBreak, context.Locale);
    }
  };
}
