#pragma once

#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorTextBox.hpp"
#include "Krystal.Booey/Rendering/SVG/RenderSVGInlineText.hpp"
#include "Krystal.Core/Hash.hpp"
#include "Krystal.Core/Types/List.hpp"
#include <utility>

namespace krys::boo
{
  class RenderSVGText;
  class SVGInlineTextBox;
  struct SVGTextFragment;
}

namespace krys::boo::inline_iterator
{
  /// @brief A text box of an SVG text renderer, placed by the SVG text layout as text fragments.
  class SVGTextBox : public TextBox
  {
  public:
    using Key = std::pair<const RenderSVGInlineText *, uint32>;

    explicit SVGTextBox(PathVariant &&path) noexcept;

    KRYS_NODISCARD FloatRect CalculateBoundariesIncludingSVGTransform() const noexcept;
    KRYS_NODISCARD LayoutRect LocalSelectionRect(uint32 start, uint32 end) const noexcept;
    KRYS_NODISCARD const List<SVGTextFragment> &TextFragments() const noexcept;

    KRYS_NODISCARD const RenderSVGInlineText &Renderer() const noexcept
    {
      return Downcast<RenderSVGInlineText>(TextBox::Renderer());
    }

    KRYS_NODISCARD const SVGInlineTextBox *GetLegacyInlineBox() const noexcept;
  };

  /// @brief Hashes an SVGTextBox::Key.
  struct SVGTextBoxKeyHash
  {
    KRYS_NODISCARD size_t operator()(const SVGTextBox::Key &key) const noexcept
    {
      return UnsignedIntegerPairHash(DefaultHash<const RenderSVGInlineText *>::Hash(key.first), key.second);
    }
  };

  /// @brief Iterates the text boxes of an SVG text renderer.
  class SVGTextBoxIterator : public TextBoxIterator
  {
  public:
    SVGTextBoxIterator() noexcept = default;
    explicit SVGTextBoxIterator(Box::PathVariant &&path) noexcept;
    explicit SVGTextBoxIterator(const Box &box) noexcept;

    SVGTextBoxIterator &operator++() noexcept
    {
      return static_cast<SVGTextBoxIterator &>(TraverseNextTextBox());
    }

    KRYS_NODISCARD const SVGTextBox &operator*() const noexcept
    {
      return Get();
    }
    KRYS_NODISCARD const SVGTextBox *operator->() const noexcept
    {
      return &Get();
    }

  private:
    KRYS_NODISCARD const SVGTextBox &Get() const noexcept
    {
      return static_cast<const SVGTextBox &>(_box);
    }
  };

  KRYS_NODISCARD SVGTextBoxIterator FirstSVGTextBoxFor(const RenderSVGInlineText &text) noexcept;
  KRYS_NODISCARD BoxRange<SVGTextBoxIterator> SVGTextBoxesFor(const RenderSVGInlineText &text) noexcept;
  KRYS_NODISCARD SVGTextBoxIterator SVGTextBoxFor(const SVGInlineTextBox *box) noexcept;
  KRYS_NODISCARD SVGTextBoxIterator SVGTextBoxFor(const layout_integration::InlineContent &inlineContent,
                                                  size_t boxIndex) noexcept;

  KRYS_NODISCARD BoxRange<BoxIterator> BoxesFor(const RenderSVGText &svgText) noexcept;

  KRYS_NODISCARD SVGTextBox::Key MakeKey(const SVGTextBox &textBox) noexcept;
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::inline_iterator::SVGTextBox)
  KRYS_NODISCARD static bool IsType(const krys::boo::inline_iterator::Box &box) noexcept
  {
    return box.IsSVGText();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
