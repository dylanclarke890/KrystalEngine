#pragma once

#include "Krystal.Booey/CSS/Properties/MutableStyleProperties.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/Editing/WritingDirection.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/TriState.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class CSSStyleProperties;
  class StyleProperties;
}

namespace krys::boo::dom
{
  class Document;
  class Element;
  class Node;
  class Position;
  class StyledElement;
}

namespace krys::boo
{
  class RenderStyle;
  class VisibleSelection;

  enum class TextDecorationChange : uint8
  {
    None,
    Add,
    Remove
  };

  enum class StandardFontFamilySerializationMode : uint8
  {
    Keep,
    Strip
  };

  /// @brief The style an edit applies, or the style at a position: a set of CSS properties, with a font size
  /// change and underline and line-through changes beside them.
  class EditingStyle : public RefCounted<EditingStyle>
  {
    friend class HTMLElementEquivalent;
    friend class HTMLAttributeEquivalent;
    friend class HTMLTextDecorationEquivalent;
    friend class HTMLFontWeightEquivalent;
    KRYS_NON_COPY_MOVABLE(EditingStyle);

  public:
    enum class PropertiesToInclude : uint8
    {
      AllProperties,
      OnlyEditingInheritableProperties,
      EditingPropertiesInEffect,
      PostLayoutProperties
    };

    enum class ShouldPreserveWritingDirection : bool
    {
      No,
      Yes
    };

    enum class ShouldExtractMatchingStyle : bool
    {
      No,
      Yes
    };

    enum class ShouldIgnoreTextOnlyProperties : bool
    {
      No,
      Yes
    };

    enum class CSSPropertyOverrideMode : bool
    {
      DoNotOverrideValues,
      OverrideValues
    };

    static constexpr float NoFontDelta = 0.0f;

  private:
    RefPtr<css::MutableStyleProperties> _mutableStyle;
    bool _shouldUseFixedDefaultFontSize {false};
    TextDecorationChange _underlineChange {TextDecorationChange::None};
    TextDecorationChange _strikeThroughChange {TextDecorationChange::None};
    float _fontSizeDelta {NoFontDelta};

  public:
    EditingStyle() noexcept;
    EditingStyle(dom::Node *node, PropertiesToInclude propertiesToInclude) noexcept;
    EditingStyle(const dom::Position &position, PropertiesToInclude propertiesToInclude) noexcept;
    explicit EditingStyle(const css::CSSStyleProperties *style) noexcept;
    explicit EditingStyle(const css::StyleProperties *style) noexcept;
    EditingStyle(css::PropertyId propertyID, const dom::DOMString &value) noexcept;
    EditingStyle(css::PropertyId propertyID, css::ValueId value) noexcept;
    ~EditingStyle() noexcept;

    KRYS_NODISCARD static Ref<EditingStyle> Create() noexcept;
    KRYS_NODISCARD static Ref<EditingStyle>
      Create(dom::Node *node, PropertiesToInclude propertiesToInclude =
                                PropertiesToInclude::OnlyEditingInheritableProperties) noexcept;
    KRYS_NODISCARD static Ref<EditingStyle>
      Create(const dom::Position &position, PropertiesToInclude propertiesToInclude =
                                              PropertiesToInclude::OnlyEditingInheritableProperties) noexcept;
    KRYS_NODISCARD static Ref<EditingStyle> Create(const css::StyleProperties *style) noexcept;
    KRYS_NODISCARD static Ref<EditingStyle> Create(const css::CSSStyleProperties *style) noexcept;
    KRYS_NODISCARD static Ref<EditingStyle> Create(css::PropertyId propertyID,
                                                   const dom::DOMString &value) noexcept;
    KRYS_NODISCARD static Ref<EditingStyle> Create(css::PropertyId propertyID, css::ValueId value) noexcept;

    KRYS_NODISCARD css::MutableStyleProperties *Style() const noexcept
    {
      return _mutableStyle.get();
    }
    KRYS_NODISCARD Ref<css::MutableStyleProperties> StyleWithResolvedTextDecorations() const noexcept;
    KRYS_NODISCARD Maybe<WritingDirection> TextDirection() const noexcept;
    KRYS_NODISCARD bool IsEmpty() const noexcept;
    void SetStyle(RefPtr<css::MutableStyleProperties> &&style) noexcept;
    void OverrideWithStyle(const css::StyleProperties &style) noexcept;
    void OverrideTypingStyleAt(const EditingStyle &style, const dom::Position &position) noexcept;
    void Clear() noexcept;
    KRYS_NODISCARD Ref<EditingStyle> Copy() const noexcept;
    KRYS_NODISCARD Ref<EditingStyle> ExtractAndRemoveBlockProperties() noexcept;
    KRYS_NODISCARD Ref<EditingStyle> ExtractAndRemoveTextDirection() noexcept;
    void RemoveBlockProperties() noexcept;
    void RemoveStyleAddedByNode(dom::Node *node) noexcept;
    void RemoveStyleConflictingWithStyleOfNode(dom::Node &node) noexcept;
    template <typename T>
    void RemoveEquivalentProperties(T &style) noexcept;
    void CollapseTextDecorationProperties() noexcept;

    KRYS_NODISCARD TriState TriStateOfStyle(EditingStyle *style) const noexcept;
    KRYS_NODISCARD TriState TriStateOfStyle(const VisibleSelection &selection) const noexcept;
    KRYS_NODISCARD bool ConflictsWithInlineStyleOfElement(dom::StyledElement &element) const noexcept
    {
      return ConflictsWithInlineStyleOfElement(element, nullptr, nullptr);
    }
    KRYS_NODISCARD bool ConflictsWithInlineStyleOfElement(dom::StyledElement &element,
                                                          RefPtr<css::MutableStyleProperties> &newInlineStyle,
                                                          EditingStyle *extractedStyle) const noexcept
    {
      return ConflictsWithInlineStyleOfElement(element, &newInlineStyle, extractedStyle);
    }
    KRYS_NODISCARD bool ConflictsWithImplicitStyleOfElement(
      dom::Element &element, EditingStyle *extractedStyle = nullptr,
      ShouldExtractMatchingStyle shouldExtractMatchingStyle = ShouldExtractMatchingStyle::No) const noexcept;
    KRYS_NODISCARD bool ConflictsWithImplicitStyleOfAttributes(dom::Element &element) const noexcept;
    bool ExtractConflictingImplicitStyleOfAttributes(
      dom::Element &element, ShouldPreserveWritingDirection shouldPreserveWritingDirection,
      EditingStyle *extractedStyle, List<dom::DOMStringAtom> &conflictingAttributes,
      ShouldExtractMatchingStyle shouldExtractMatchingStyle) const noexcept;
    KRYS_NODISCARD bool StyleIsPresentInComputedStyleOfNode(dom::Node &node) const noexcept;
    KRYS_NODISCARD static bool ElementIsStyledSpanOrHTMLEquivalent(const dom::Element &element) noexcept;

    void PrepareToApplyAt(const dom::Position &position,
                          ShouldPreserveWritingDirection shouldPreserveWritingDirection =
                            ShouldPreserveWritingDirection::No) noexcept;
    void MergeTypingStyle(dom::Document &document) noexcept;
    void MergeInlineStyleOfElement(
      dom::StyledElement &element, CSSPropertyOverrideMode mode,
      PropertiesToInclude propertiesToInclude = PropertiesToInclude::AllProperties) noexcept;
    KRYS_NODISCARD static Ref<EditingStyle> WrappingStyleForSerialization(
      dom::Node &context, bool shouldAnnotate,
      StandardFontFamilySerializationMode standardFontFamilySerializationMode) noexcept;
    void MergeStyleFromRules(dom::StyledElement &element) noexcept;
    void MergeStyleFromRulesForSerialization(
      dom::StyledElement &element,
      StandardFontFamilySerializationMode standardFontFamilySerializationMode) noexcept;
    void RemoveStyleFromRulesAndContext(dom::StyledElement &element, dom::Node *context) noexcept;
    void RemovePropertiesInElementDefaultStyle(dom::Element &element) noexcept;
    void ForceDisplayInline() noexcept;
    void AddDisplayContents() noexcept;
    bool ConvertPositionStyle() noexcept;
    KRYS_NODISCARD bool IsFloating() noexcept;
    KRYS_NODISCARD int32 LegacyFontSize(dom::Document &document) const noexcept;
    KRYS_NODISCARD float FontSizeDelta() const noexcept
    {
      return _fontSizeDelta;
    }
    KRYS_NODISCARD bool HasFontSizeDelta() const noexcept
    {
      return _fontSizeDelta != NoFontDelta;
    }
    KRYS_NODISCARD bool ShouldUseFixedDefaultFontSize() const noexcept
    {
      return _shouldUseFixedDefaultFontSize;
    }

    void SetUnderlineChange(TextDecorationChange change) noexcept
    {
      _underlineChange = change;
    }
    KRYS_NODISCARD TextDecorationChange UnderlineChange() const noexcept
    {
      return _underlineChange;
    }
    void SetStrikeThroughChange(TextDecorationChange change) noexcept
    {
      _strikeThroughChange = change;
    }
    KRYS_NODISCARD TextDecorationChange StrikeThroughChange() const noexcept
    {
      return _strikeThroughChange;
    }

    KRYS_NODISCARD bool HasStyle(css::PropertyId propertyID, const dom::DOMString &value) noexcept;
    KRYS_NODISCARD bool FontWeightIsBold() noexcept;
    KRYS_NODISCARD bool FontStyleIsItalic() noexcept;
    KRYS_NODISCARD bool WebkitTextDecorationsInEffectIsUnderline() noexcept;
    KRYS_NODISCARD static RefPtr<EditingStyle> StyleAtSelectionStart(
      const VisibleSelection &selection, bool shouldUseBackgroundColorInEffect = false,
      PropertiesToInclude propertiesToInclude = PropertiesToInclude::AllProperties) noexcept;
    KRYS_NODISCARD static WritingDirection
      TextDirectionForSelection(const VisibleSelection &selection, EditingStyle *typingStyle,
                                bool &hasNestedOrMultipleEmbeddings) noexcept;
    KRYS_NODISCARD static bool IsEmbedOrIsolate(css::ValueId unicodeBidi) noexcept
    {
      return unicodeBidi == css::ValueId::Isolate || unicodeBidi == css::ValueId::Embed;
    }

    KRYS_NODISCARD Ref<EditingStyle> InverseTransformColorIfNeeded(dom::Element &element) noexcept;

  private:
    void Init(dom::Node *node, PropertiesToInclude propertiesToInclude) noexcept;
    void RemoveTextFillAndStrokeColorsIfNeeded(const RenderStyle *renderStyle) noexcept;
    Ref<css::MutableStyleProperties>
      RemoveInlineStyleRedundantDueToMatchedRules(dom::StyledElement &element) noexcept;
    void RemoveStyleInContextNotOverridenByMatchedRules(
      dom::StyledElement &element, dom::Node *context,
      css::MutableStyleProperties &styleFromMatchedRules) noexcept;
    void RemoveDisplayPropertyFromSpanStyleIfRedundant(
      dom::StyledElement &element, css::MutableStyleProperties &styleFromMatchedRules) noexcept;
    void SetProperty(css::PropertyId propertyID, const dom::DOMString &value,
                     css::IsImportant important = css::IsImportant(false)) noexcept;
    void ExtractFontSizeDelta() noexcept;
    template <typename T>
    KRYS_NODISCARD TriState TriStateOfStyle(
      T &styleToCompare, ShouldIgnoreTextOnlyProperties shouldIgnoreTextOnlyProperties) const noexcept;
    KRYS_NODISCARD bool ConflictsWithInlineStyleOfElement(dom::StyledElement &element,
                                                          RefPtr<css::MutableStyleProperties> *newInlineStyle,
                                                          EditingStyle *extractedStyle) const noexcept;
    void MergeInlineAndImplicitStyleOfElement(
      dom::StyledElement &element, CSSPropertyOverrideMode mode, PropertiesToInclude propertiesToInclude,
      StandardFontFamilySerializationMode standardFontFamilySerializationMode) noexcept;
    void MergeStyle(const css::StyleProperties *style, CSSPropertyOverrideMode mode) noexcept;
  };

  /// @brief The changes an editing style makes at a position: the CSS left after the HTML elements that
  /// express the rest (bold, italic, underline, line-through, subscripts and superscripts, and `<font>`'s
  /// attributes).
  class StyleChange
  {
  private:
    RefPtr<css::MutableStyleProperties> _cssStyle;
    bool _applyBold {false};
    bool _applyItalic {false};
    bool _applyUnderline {false};
    bool _applyLineThrough {false};
    bool _applySubscript {false};
    bool _applySuperscript {false};
    dom::DOMString _applyFontColor;
    dom::DOMString _applyFontFace;
    dom::DOMString _applyFontSize;

  public:
    StyleChange() noexcept = default;
    StyleChange(EditingStyle *style, const dom::Position &position) noexcept;
    ~StyleChange() noexcept;

    KRYS_NODISCARD const css::MutableStyleProperties *CssStyle() const noexcept
    {
      return _cssStyle.get();
    }
    KRYS_NODISCARD bool ApplyBold() const noexcept
    {
      return _applyBold;
    }
    KRYS_NODISCARD bool ApplyItalic() const noexcept
    {
      return _applyItalic;
    }
    KRYS_NODISCARD bool ApplyUnderline() const noexcept
    {
      return _applyUnderline;
    }
    KRYS_NODISCARD bool ApplyLineThrough() const noexcept
    {
      return _applyLineThrough;
    }
    KRYS_NODISCARD bool ApplySubscript() const noexcept
    {
      return _applySubscript;
    }
    KRYS_NODISCARD bool ApplySuperscript() const noexcept
    {
      return _applySuperscript;
    }
    KRYS_NODISCARD bool ApplyFontColor() const noexcept
    {
      return !_applyFontColor.empty();
    }
    KRYS_NODISCARD bool ApplyFontFace() const noexcept
    {
      return !_applyFontFace.empty();
    }
    KRYS_NODISCARD bool ApplyFontSize() const noexcept
    {
      return !_applyFontSize.empty();
    }
    KRYS_NODISCARD const dom::DOMString &FontColor() const noexcept
    {
      return _applyFontColor;
    }
    KRYS_NODISCARD const dom::DOMString &FontFace() const noexcept
    {
      return _applyFontFace;
    }
    KRYS_NODISCARD const dom::DOMString &FontSize() const noexcept
    {
      return _applyFontSize;
    }

    KRYS_NODISCARD bool operator==(const StyleChange &other) const noexcept;

  private:
    void ExtractTextStyles(dom::Document &document, css::MutableStyleProperties &style,
                           bool shouldUseFixedFontDefaultSize) noexcept;
  };
}
