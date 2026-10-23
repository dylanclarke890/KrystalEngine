#pragma once

#include "Krystal.Booey/HTML/HTMLDivElement.hpp"

namespace krys::boo
{
  class RenderTextControlInnerBlock;
}

namespace krys::boo::html
{
  /// @brief The flex container a text field's inner text and decorations (search buttons, the spin button)
  /// are laid out in, when it has decorations.
  class TextControlInnerContainer final : public HTMLDivElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(TextControlInnerContainer);

  public:
    explicit TextControlInnerContainer(dom::Document &document) noexcept;

    KRYS_NODISCARD static Ref<TextControlInnerContainer> Create(dom::Document &document) noexcept;

  private:
    KRYS_NODISCARD RenderPtr<RenderElement> CreateElementRenderer(RenderStyle &&style) noexcept override;
    KRYS_NODISCARD Maybe<style::UnadjustedStyle>
      ResolveCustomStyle(const style::ResolutionContext &resolutionContext,
                         const RenderStyle *shadowHostStyle) noexcept override;
  };

  /// @brief The block in a text field's inner container that holds the inner text, and grows to fill it.
  class TextControlInnerElement final : public HTMLDivElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(TextControlInnerElement);

  public:
    explicit TextControlInnerElement(dom::Document &document) noexcept;

    KRYS_NODISCARD static Ref<TextControlInnerElement> Create(dom::Document &document) noexcept;

  private:
    KRYS_NODISCARD Maybe<style::UnadjustedStyle>
      ResolveCustomStyle(const style::ResolutionContext &resolutionContext,
                         const RenderStyle *shadowHostStyle) noexcept override;
  };

  /// @brief The editing host that holds a text control's text: text nodes, and a `<br>` after a trailing line
  /// break. It's plain-text editable while the control is mutable.
  class TextControlInnerTextElement final : public HTMLDivElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(TextControlInnerTextElement);

  public:
    explicit TextControlInnerTextElement(dom::Document &document) noexcept;

    KRYS_NODISCARD static Ref<TextControlInnerTextElement> Create(dom::Document &document,
                                                                  bool isEditable) noexcept;

    void DefaultEventHandler(dom::Event &event) noexcept override;

    KRYS_NODISCARD RenderTextControlInnerBlock *Renderer() const noexcept;

    void UpdateInnerTextElementEditability(bool isEditable) noexcept
    {
      UpdateInnerTextElementEditabilityImpl(isEditable);
    }

    KRYS_NODISCARD bool IsTextControlInnerTextElement() const noexcept override
    {
      return true;
    }

  private:
    void UpdateInnerTextElementEditabilityImpl(bool isEditable) noexcept;

    KRYS_NODISCARD RenderPtr<RenderElement> CreateElementRenderer(RenderStyle &&style) noexcept override;
    KRYS_NODISCARD Maybe<style::UnadjustedStyle>
      ResolveCustomStyle(const style::ResolutionContext &resolutionContext,
                         const RenderStyle *shadowHostStyle) noexcept override;
  };

  /// @brief A text control's placeholder text (the `::placeholder` part), shown while the control is empty.
  class TextControlPlaceholderElement final : public HTMLDivElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(TextControlPlaceholderElement);

  public:
    explicit TextControlPlaceholderElement(dom::Document &document) noexcept;

    KRYS_NODISCARD static Ref<TextControlPlaceholderElement> Create(dom::Document &document) noexcept;

  private:
    KRYS_NODISCARD Maybe<style::UnadjustedStyle>
      ResolveCustomStyle(const style::ResolutionContext &resolutionContext,
                         const RenderStyle *shadowHostStyle) noexcept override;
  };

  /// @brief The decoration at the start of a search field (the `::-webkit-search-results-decoration` part).
  // FIXME: The recent searches menu it opens (the `results` attribute), which isn't standard.
  class SearchFieldResultsButtonElement final : public HTMLDivElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SearchFieldResultsButtonElement);

  private:
    bool _canAdjustStyleForAppearance {true};

  public:
    explicit SearchFieldResultsButtonElement(dom::Document &document) noexcept;

    KRYS_NODISCARD static Ref<SearchFieldResultsButtonElement> Create(dom::Document &document) noexcept;

    void DefaultEventHandler(dom::Event &event) noexcept override;

    KRYS_NODISCARD bool CanAdjustStyleForAppearance() const noexcept
    {
      return _canAdjustStyleForAppearance;
    }

    KRYS_NODISCARD bool IsSearchFieldResultsButtonElement() const noexcept override
    {
      return true;
    }

  private:
    KRYS_NODISCARD Maybe<style::UnadjustedStyle>
      ResolveCustomStyle(const style::ResolutionContext &resolutionContext,
                         const RenderStyle *shadowHostStyle) noexcept override;
  };

  /// @brief The button at the end of a search field that clears it (the `::-webkit-search-cancel-button`
  /// part), hidden while the field is empty.
  class SearchFieldCancelButtonElement final : public HTMLDivElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SearchFieldCancelButtonElement);

  public:
    explicit SearchFieldCancelButtonElement(dom::Document &document) noexcept;

    KRYS_NODISCARD static Ref<SearchFieldCancelButtonElement> Create(dom::Document &document) noexcept;

    void DefaultEventHandler(dom::Event &event) noexcept override;

  private:
    KRYS_NODISCARD Maybe<style::UnadjustedStyle>
      ResolveCustomStyle(const style::ResolutionContext &resolutionContext,
                         const RenderStyle *shadowHostStyle) noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::TextControlInnerTextElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &element) noexcept
  {
    return element.IsTextControlInnerTextElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &node) noexcept
  {
    return node.IsHTMLElement() && IsType(Downcast<krys::boo::html::HTMLElement>(node));
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::SearchFieldResultsButtonElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &element) noexcept
  {
    return element.IsSearchFieldResultsButtonElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &node) noexcept
  {
    return node.IsHTMLElement() && IsType(Downcast<krys::boo::html::HTMLElement>(node));
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
