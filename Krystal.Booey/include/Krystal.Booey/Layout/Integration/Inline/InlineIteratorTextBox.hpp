#pragma once

#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorBox.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo
{
  class FontCascade;
  class RenderText;
}

namespace krys::boo::inline_iterator
{
  /// @brief A text box: a run of a text renderer's text on a line.
  class TextBox : public Box
  {
  public:
    explicit TextBox(PathVariant &&path) noexcept : Box(krys::move(path))
    {
    }

    KRYS_NODISCARD bool HasHyphen() const noexcept
    {
      return SwitchOn(_pathVariant, [&](const auto &path) -> decltype(auto) { return path.HasHyphen(); });
    }
    KRYS_NODISCARD utf8_stringview OriginalText() const noexcept
    {
      return SwitchOn(_pathVariant, [&](const auto &path) -> decltype(auto) { return path.OriginalText(); });
    }

    KRYS_NODISCARD uint32 Start() const noexcept
    {
      return SwitchOn(_pathVariant, [&](const auto &path) -> decltype(auto) { return path.Start(); });
    }
    KRYS_NODISCARD uint32 End() const noexcept
    {
      return SwitchOn(_pathVariant, [&](const auto &path) -> decltype(auto) { return path.End(); });
    }
    KRYS_NODISCARD uint32 Length() const noexcept
    {
      return SwitchOn(_pathVariant, [&](const auto &path) -> decltype(auto) { return path.Length(); });
    }

    /// @brief The part of the renderer's text the box shows, for clamping offsets to it.
    KRYS_NODISCARD TextBoxSelectableRange SelectableRange() const noexcept
    {
      return SwitchOn(_pathVariant,
                      [&](const auto &path) -> decltype(auto) { return path.SelectableRange(); });
    }

    KRYS_NODISCARD const FontCascade &GetFontCascade() const noexcept;
    KRYS_NODISCARD TextRun GetTextRun(TextRunMode mode = TextRunMode::Painting) const noexcept
    {
      return SwitchOn(_pathVariant,
                      [&](const auto &path) -> decltype(auto) { return path.GetTextRun(mode); });
    }

    KRYS_NODISCARD const RenderText &Renderer() const noexcept;

    // FIXME: Remove. For intermediate porting steps only.
    KRYS_NODISCARD const LegacyInlineTextBox *GetLegacyInlineBox() const noexcept
    {
      return Downcast<LegacyInlineTextBox>(Box::GetLegacyInlineBox());
    }

    /// @brief The next text box generated for the same text renderer.
    KRYS_NODISCARD TextBoxIterator NextTextBox() const noexcept;
  };

  /// @brief Iterates the text boxes of a text renderer.
  class TextBoxIterator : public LeafBoxIterator
  {
  public:
    TextBoxIterator() noexcept = default;
    explicit TextBoxIterator(Box::PathVariant &&path) noexcept : LeafBoxIterator(krys::move(path))
    {
    }
    explicit TextBoxIterator(const Box &box) noexcept : LeafBoxIterator(box)
    {
    }

    TextBoxIterator &operator++() noexcept
    {
      return TraverseNextTextBox();
    }

    KRYS_NODISCARD const TextBox &operator*() const noexcept
    {
      return Get();
    }
    KRYS_NODISCARD const TextBox *operator->() const noexcept
    {
      return &Get();
    }

    /// @brief Traverses to the next text box generated for the same text renderer.
    TextBoxIterator &TraverseNextTextBox() noexcept;

  private:
    BoxIterator &TraverseLineRightwardOnLine() = delete;
    BoxIterator &TraverseLineLeftwardOnLine() = delete;
    BoxIterator &TraverseLineRightwardOnLineIgnoringLineBreak() = delete;
    BoxIterator &TraverseLineLeftwardOnLineIgnoringLineBreak() = delete;

    KRYS_NODISCARD const TextBox &Get() const noexcept
    {
      return static_cast<const TextBox &>(_box);
    }
  };

  KRYS_NODISCARD TextBoxIterator LineLeftmostTextBoxFor(const RenderText &text) noexcept;
  KRYS_NODISCARD TextBoxIterator TextBoxFor(const LegacyInlineTextBox *legacyInlineTextBox) noexcept;
  KRYS_NODISCARD TextBoxIterator TextBoxFor(const layout_integration::InlineContent &content,
                                            const inline_display::Box &box) noexcept;
  KRYS_NODISCARD TextBoxIterator TextBoxFor(const layout_integration::InlineContent &content,
                                            size_t boxIndex) noexcept;
  KRYS_NODISCARD BoxRange<TextBoxIterator> TextBoxesFor(const RenderText &text) noexcept;
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::inline_iterator::TextBox)
  KRYS_NODISCARD static bool IsType(const krys::boo::inline_iterator::Box &box) noexcept
  {
    return box.IsText();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
