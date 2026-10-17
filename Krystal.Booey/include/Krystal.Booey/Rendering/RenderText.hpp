#pragma once

#include "Krystal.Booey/Graphics/IntRect.hpp"
#include "Krystal.Booey/Rendering/RenderObject.hpp"
#include "Krystal.Booey/Style/Difference.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::boo::dom
{
  class Text;
}

namespace krys::boo
{
  class RenderInline;
  class RenderStyle;

  /// @brief Renders a text node (or anonymous text, such as generated content): its text, after the style's
  /// text transform and text security.
  // FIXME: Measurement (widths, preferred widths, the font code path and simplified measuring), line boxes,
  // selection, caret offsets, hit testing, painting and the secure text timer that reveals the last typed
  // character come with layout, painting and editing.
  class RenderText : public RenderObject
  {
    KRYS_NON_COPY_MOVABLE(RenderText);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderText);

  private:
    utf8_string _text;
    // The text before the text transform and text security, when they changed it.
    Maybe<utf8_string> _originalText;
    RenderInline *_inlineWrapperForDisplayContents {nullptr};
    bool _useBackslashAsYenSymbol {false};

  public:
    RenderText(Type type, dom::Text &textNode, utf8_string text) noexcept;
    RenderText(Type type, dom::Document &document, utf8_string text) noexcept;
    ~RenderText() noexcept override;

    KRYS_NODISCARD utf8_stringview RenderName() const noexcept override;

    KRYS_NODISCARD dom::Text *TextNode() const noexcept;

    /// @brief The bounding box of the text's line boxes.
    // FIXME: The union of the text boxes, once there's inline layout.
    KRYS_NODISCARD IntRect LinesBoundingBox() const noexcept
    {
      return {};
    }

    /// @brief The text before the text transform and text security.
    KRYS_NODISCARD virtual utf8_string OriginalText() const noexcept;

    /// @brief The rendered text.
    KRYS_NODISCARD const utf8_string &Text() const noexcept
    {
      return _text;
    }

    KRYS_NODISCARD bool HasEmptyText() const noexcept
    {
      return _text.empty();
    }

    /// @brief The length of the rendered text, in bytes.
    KRYS_NODISCARD size_t Length() const noexcept
    {
      return _text.size();
    }

    void SetText(utf8_string newContent, bool force = false) noexcept;
    void SetTextWithOffset(utf8_string newText, uint32 offset) noexcept;

    KRYS_NODISCARD bool ContainsOnlyCollapsibleWhitespace() const noexcept;
    KRYS_NODISCARD bool ContainsOnlyCSSWhitespace(size_t from, size_t length) const noexcept;

    /// @brief The rendered text, but with backslashes rather than yen signs.
    KRYS_NODISCARD utf8_string TextWithoutConvertingBackslashToYenSymbol() const noexcept;

    /// @brief Called with the style of the renderer's parent, which is the text's style, when it changes.
    virtual void StyleDidChange(style::Difference diff, const RenderStyle *oldStyle) noexcept;

    KRYS_NODISCARD RenderInline *InlineWrapperForDisplayContents() const noexcept
    {
      return _inlineWrapperForDisplayContents;
    }
    void SetInlineWrapperForDisplayContents(RenderInline *wrapper) noexcept;

    KRYS_NODISCARD bool CanHaveChildren() const noexcept final
    {
      return false;
    }

    /// @brief The last code point of the in-flow text before this one, which capitalization continues from (a
    /// space if there's none).
    KRYS_NODISCARD virtual utf8_string PreviousCharacter() const noexcept;

  protected:
    RenderText(Type type, dom::Node &node, utf8_string text) noexcept;

    void WillBeDestroyed() noexcept override;

    virtual void SetRenderedText(utf8_string newText) noexcept;
    virtual void SetTextInternal(utf8_string text, bool force) noexcept;

  private:
    KRYS_NODISCARD bool ComputeUseBackslashAsYenSymbol() const noexcept;
    void SecureText(char32 maskingCharacter) noexcept;
  };

  /// @brief Capitalizes the first letter of each word of @p string, which follows @p previousCharacter.
  KRYS_NODISCARD utf8_string Capitalize(utf8_stringview string,
                                        utf8_stringview previousCharacter = u8" ") noexcept;

  /// @brief Applies @p style's text transform to @p text, which follows @p previousCharacter.
  KRYS_NODISCARD utf8_string ApplyTextTransform(const RenderStyle &style, utf8_string text,
                                                utf8_stringview previousCharacter = u8" ") noexcept;
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderText)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderText();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
