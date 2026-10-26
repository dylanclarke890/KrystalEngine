#pragma once

#include "Krystal.Booey/Graphics/FontCascade.hpp"
#include "Krystal.Booey/Graphics/IntRect.hpp"
#include "Krystal.Booey/Rendering/RenderObject.hpp"
#include "Krystal.Booey/Style/Difference.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/HashSet.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::boo::dom
{
  class Text;
}

namespace krys::boo::layout
{
  class InlineTextBox;
}

namespace krys::boo
{
  class RenderInline;
  struct WordTrailingSpace;
  class RenderStyle;

  /// @brief Renders a text node (or anonymous text, such as generated content): its text, after the style's
  /// text transform and text security.
  // FIXME: The secure text timer that reveals the last typed character comes with editing.
  class RenderText : public RenderObject
  {
    KRYS_NON_COPY_MOVABLE(RenderText);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderText);

  private:
    utf8_string _text;
    // The text before the text transform and text security, when they changed it.
    Maybe<utf8_string> _originalText;
    RenderInline *_inlineWrapperForDisplayContents {nullptr};
    Maybe<bool> _canUseSimplifiedTextMeasuring;
    Maybe<bool> _hasPositionDependentContentWidth;
    Maybe<bool> _hasStrongDirectionalityContent;
    bool _useBackslashAsYenSymbol {false};
    bool _containsOnlyASCII {false};
    bool _needsVisualReordering {false};

    Maybe<float> _minWidth;
    Maybe<float> _maxWidth;
    float _beginMinWidth {0};
    float _endMinWidth {0};
    bool _hasBreakableChar {false}; // Whether or not we can be broken into multiple lines.
    // Whether or not we have a hard break (e.g., <pre> with '\n').
    bool _hasBreak {false};
    // Whether or not we have a variable width tab character (e.g., <pre> with '\t').
    bool _hasTab {false};
    bool _hasBeginWS {false}; // Whether or not we begin with WS (only true if we aren't pre)
    bool _hasEndWS {false};   // Whether or not we end with WS (only true if we aren't pre)
    mutable bool _knownToHaveNoOverflowAndNoFallbackFonts {false};
    FontCascade::CodePath _fontCodePath {FontCascade::CodePath::Auto};

  public:
    RenderText(Type type, dom::Text &textNode, utf8_string text) noexcept;
    RenderText(Type type, dom::Document &document, utf8_string text) noexcept;
    ~RenderText() noexcept override;

    KRYS_NODISCARD utf8_stringview RenderName() const noexcept override;

    KRYS_NODISCARD dom::Text *TextNode() const noexcept;

    KRYS_NODISCARD layout::InlineTextBox *LayoutBox() noexcept;
    KRYS_NODISCARD const layout::InlineTextBox *LayoutBox() const noexcept;

    /// @brief The bounding box of the text's line boxes.
    KRYS_NODISCARD IntRect LinesBoundingBox() const noexcept;
    /// @brief The location of the text's first text box.
    KRYS_NODISCARD IntPoint FirstRunLocation() const noexcept;

    /// @brief The preferred widths of the text, and how it can break, for the legacy inline preferred widths.
    struct Widths
    {
      float Min {0};
      float Max {0};
      float BeginMin {0};
      float EndMin {0};
      float BeginMax {0};
      float EndMax {0};
      bool BeginWS {false};
      bool EndWS {false};
      bool EndZeroSpace {false};
      bool HasBreakableChar {false};
      bool HasBreak {false};
      bool EndsWithBreak {false};
    };
    KRYS_NODISCARD Widths TrimmedPreferredWidths(float leadWidth, bool &stripFrontSpaces) noexcept;

    KRYS_NODISCARD float HangablePunctuationStartWidth(size_t index) const noexcept;
    KRYS_NODISCARD float HangablePunctuationEndWidth(size_t index) const noexcept;
    KRYS_NODISCARD size_t FirstCharacterIndexStrippingSpaces() const noexcept;
    KRYS_NODISCARD size_t LastCharacterIndexStrippingSpaces() const noexcept;

    KRYS_NODISCARD float MinLogicalWidth() const noexcept;
    KRYS_NODISCARD float MaxLogicalWidth() const noexcept;
    void ResetMinMaxWidth() noexcept
    {
      _minWidth = null;
      _maxWidth = null;
    }

    /// @brief The width of the text's bytes [from, from + length), in the first line's style if @p firstLine.
    KRYS_NODISCARD float Width(size_t from, size_t length, float xPos, bool firstLine = false,
                               HashSet<RefPtr<const Font>> *fallbackFonts = nullptr,
                               GlyphOverflow *glyphOverflow = nullptr) const noexcept;
    KRYS_NODISCARD float Width(size_t from, size_t length, const FontCascade &fontCascade, float xPos,
                               HashSet<RefPtr<const Font>> *fallbackFonts = nullptr,
                               GlyphOverflow *glyphOverflow = nullptr) const noexcept;

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

    /// @brief Whether any of the text is laid out in a text box.
    KRYS_NODISCARD bool HasRenderedText() const noexcept;

    KRYS_NODISCARD dom::PositionWithAffinity PositionForPoint(const LayoutPoint &point,
                                                              HitTestSource source) noexcept override;

    KRYS_NODISCARD int32 CaretMinOffset() const noexcept final;
    KRYS_NODISCARD int32 CaretMaxOffset() const noexcept final;
    /// @brief The number of rendered (not collapsed) bytes before @p offset.
    KRYS_NODISCARD uint32 CountRenderedCharacterOffsetsUntil(uint32 offset) const noexcept;
    KRYS_NODISCARD bool ContainsRenderedCharacterOffset(uint32 offset) const noexcept;
    /// @brief Whether a caret can be at @p offset: in a text box, and not inside a character.
    KRYS_NODISCARD bool ContainsCaretOffset(uint32 offset) const noexcept;

    // Return before/after offset for the next grapheme cluster (character), which is not always one byte.
    KRYS_NODISCARD int32 PreviousOffset(int32 current) const noexcept final;
    KRYS_NODISCARD int32 PreviousOffsetForBackwardDeletion(int32 current) const noexcept final;
    KRYS_NODISCARD int32 NextOffset(int32 current) const noexcept final;

    KRYS_NODISCARD Color SelectionBackgroundColor() const noexcept;
    KRYS_NODISCARD Color SelectionForegroundColor() const noexcept;
    KRYS_NODISCARD Color SelectionEmphasisMarkColor() const noexcept;
    KRYS_NODISCARD UniquePtr<RenderStyle> SelectionPseudoStyle() const noexcept;
    /// @brief The `::target-text` style of the text's nearest element.
    KRYS_NODISCARD const RenderStyle *TargetTextPseudoStyle() const noexcept;

    KRYS_NODISCARD bool CanBeSelectionLeaf() const noexcept override
    {
      return true;
    }
    void SetSelectionState(HighlightState state) noexcept final;
    KRYS_NODISCARD LayoutRect SelectionRectForRepaint(const RenderLayerModelObject *repaintContainer,
                                                      bool clipToVisibleContent = true) noexcept final;
    /// @brief The selected part of the text's boxes (with their ellipses), in @p repaintContainer's
    /// coordinates, adding each box's quad to @p quads.
    LayoutRect CollectSelectionGeometriesForLineBoxes(const RenderLayerModelObject *repaintContainer,
                                                      bool clipToVisibleContent,
                                                      List<FloatQuad> &quads) noexcept;

  private:
    LayoutRect CollectSelectionGeometriesForLineBoxes(const RenderLayerModelObject *repaintContainer,
                                                      bool clipToVisibleContent,
                                                      List<FloatQuad> *quads) noexcept;

  public:
    KRYS_NODISCARD FontCascade::CodePath FontCodePath() const noexcept
    {
      return _fontCodePath;
    }
    KRYS_NODISCARD bool CanUseSimpleFontCodePath() const noexcept
    {
      return FontCodePath() == FontCascade::CodePath::Simple;
    }
    KRYS_NODISCARD bool ShouldUseSimpleGlyphOverflowCodePath() const noexcept
    {
      return FontCodePath() == FontCascade::CodePath::SimpleWithGlyphOverflow;
    }

    KRYS_NODISCARD bool NeedsVisualReordering() const noexcept
    {
      return _needsVisualReordering;
    }
    void SetNeedsVisualReordering() noexcept
    {
      _needsVisualReordering = true;
    }

    void SetCanUseSimplifiedTextMeasuring(bool canUseSimplifiedTextMeasuring) noexcept
    {
      _canUseSimplifiedTextMeasuring = canUseSimplifiedTextMeasuring;
    }
    KRYS_NODISCARD Maybe<bool> CanUseSimplifiedTextMeasuring() const noexcept
    {
      return _canUseSimplifiedTextMeasuring;
    }
    void SetHasPositionDependentContentWidth(bool hasPositionDependentContentWidth) noexcept
    {
      _hasPositionDependentContentWidth = hasPositionDependentContentWidth;
    }
    KRYS_NODISCARD Maybe<bool> HasPositionDependentContentWidth() const noexcept
    {
      return _hasPositionDependentContentWidth;
    }
    void SetHasStrongDirectionalityContent(bool hasStrongDirectionalityContent) noexcept
    {
      _hasStrongDirectionalityContent = hasStrongDirectionalityContent;
    }
    KRYS_NODISCARD Maybe<bool> HasStrongDirectionalityContent() const noexcept
    {
      return _hasStrongDirectionalityContent;
    }
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

    void BoundingRects(List<LayoutRect> &rects, const LayoutPoint &accumulatedOffset) const noexcept override;
    void AbsoluteQuads(List<FloatQuad> &quads, bool *wasFixed = nullptr) const noexcept override;
    /// @brief The quads, in absolute coordinates, of the text from @p start to @p end, one per text box (or
    /// one per character, with `ComputeIndividualCharacterRects`).
    KRYS_NODISCARD List<FloatQuad> AbsoluteQuadsForRange(uint32 start, uint32 end,
                                                         EnumSet<BoundingRectBehavior> behavior = {},
                                                         bool *wasFixed = nullptr) const noexcept;

  protected:
    RenderText(Type type, dom::Node &node, utf8_string text) noexcept;

    KRYS_NODISCARD RepaintRects LocalRectsForRepaint(RepaintOutlineBounds) const noexcept final;

    void WillBeDestroyed() noexcept override;

    virtual void SetRenderedText(utf8_string newText) noexcept;
    virtual void SetTextInternal(utf8_string text, bool force) noexcept;

  private:
    KRYS_NODISCARD bool ComputeUseBackslashAsYenSymbol() const noexcept;
    void ComputeFontCodePath() noexcept;
    void InitiateFontLoadingByAccessingGlyphDataAndComputeCanUseSimplifiedTextMeasuring(
      utf8_stringview textContent) noexcept;
    void SecureText(char32 maskingCharacter) noexcept;

    void ComputePreferredLogicalWidths(float leadWidth, bool forcedMinMaxWidthComputation = false) noexcept;
    void ComputePreferredLogicalWidths(float leadWidth, HashSet<RefPtr<const Font>> &fallbackFonts,
                                       GlyphOverflow &glyphOverflow,
                                       bool forcedMinMaxWidthComputation = false) noexcept;
    KRYS_NODISCARD float MaxWordFragmentWidth(const RenderStyle &style, const FontCascade &font,
                                              utf8_stringview word, size_t minimumPrefixLength,
                                              size_t minimumSuffixLength, bool currentCharacterIsSpace,
                                              size_t characterIndex, float xPos, float entireWordWidth,
                                              WordTrailingSpace &wordTrailingSpace,
                                              HashSet<RefPtr<const Font>> &fallbackFonts,
                                              GlyphOverflow &glyphOverflow) noexcept;
    KRYS_NODISCARD float WidthFromCache(const FontCascade &fontCascade, size_t start, size_t length,
                                        float xPos, HashSet<RefPtr<const Font>> *fallbackFonts,
                                        GlyphOverflow *glyphOverflow,
                                        const RenderStyle &style) const noexcept;
    KRYS_NODISCARD float WidthFromCacheConsideringPossibleTrailingSpace(
      const RenderStyle &style, const FontCascade &font, size_t startIndex, size_t wordLength, float xPos,
      bool currentCharacterIsSpace, WordTrailingSpace &wordTrailingSpace,
      HashSet<RefPtr<const Font>> &fallbackFonts, GlyphOverflow &glyphOverflow) const noexcept;
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
