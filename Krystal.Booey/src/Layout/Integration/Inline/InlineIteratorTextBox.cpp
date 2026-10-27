#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorTextBox.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/LayoutIntegrationLineLayout.hpp"
#include "Krystal.Booey/Rendering/RenderText.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/SVG/RenderSVGInlineText.hpp"

namespace krys::boo::inline_iterator
{
  TextBoxIterator TextBox::NextTextBox() const noexcept
  {
    return TextBoxIterator(*this).TraverseNextTextBox();
  }

  const FontCascade &TextBox::GetFontCascade() const noexcept
  {
    // FIXME: The combined text's font (RenderCombineText::textCombineFont), once there's
    // text-combine-upright.
    return Style().GetFontCascade();
  }

  const RenderText &TextBox::Renderer() const noexcept
  {
    return Downcast<RenderText>(Box::Renderer());
  }

  TextBoxIterator &TextBoxIterator::TraverseNextTextBox() noexcept
  {
    SwitchOn(_box._pathVariant, [](auto &path) { path.TraverseNextTextBox(); });
    return *this;
  }

  TextBoxIterator LineLeftmostTextBoxFor(const RenderText &text) noexcept
  {
    if (auto *lineLayout = layout_integration::LineLayout::Containing(text))
    {
      return lineLayout->TextBoxesFor(text);
    }
    if (auto *svgText = DynamicDowncast<RenderSVGInlineText>(text))
    {
      return TextBoxIterator {BoxLegacyPath {svgText->FirstLegacyTextBox()}};
    }

    // During teardown we may hit this codepath _after_ the display content is destroyed (e.g. calling repaint
    // on RenderText).
    return TextBoxIterator {BoxLegacyPath {nullptr}};
  }

  TextBoxIterator TextBoxFor(const LegacyInlineTextBox *legacyInlineTextBox) noexcept
  {
    return TextBoxIterator {BoxLegacyPath {legacyInlineTextBox}};
  }

  TextBoxIterator TextBoxFor(const layout_integration::InlineContent &content,
                             const inline_display::Box &box) noexcept
  {
    return TextBoxFor(content, content.IndexForBox(box));
  }

  TextBoxIterator TextBoxFor(const layout_integration::InlineContent &content, size_t boxIndex) noexcept
  {
    krys_debug_assert(content.DisplayContent().BoxList[boxIndex].IsTextOrSoftLineBreak());
    return TextBoxIterator {BoxModernPath {content, boxIndex}};
  }

  BoxRange<TextBoxIterator> TextBoxesFor(const RenderText &text) noexcept
  {
    return BoxRange<TextBoxIterator> {LineLeftmostTextBoxFor(text)};
  }
}
