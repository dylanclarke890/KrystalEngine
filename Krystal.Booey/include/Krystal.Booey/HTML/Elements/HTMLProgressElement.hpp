#pragma once

#include "Krystal.Booey/HTML/Elements/HTMLElement.hpp"
#include "Krystal.Core/Memory/WeakPtr.hpp"

namespace krys::boo::html
{
  class ProgressValueElement;
}

namespace krys::boo
{
  class RenderProgress;

  /// @see https://html.spec.whatwg.org/multipage/form-elements.html#htmlprogresselement
  class HTMLProgressElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLProgressElement);

  public:
    constexpr static double IndeterminatePosition = -1;
    constexpr static double InvalidPosition = -2;

  private:
    WeakPtr<html::ProgressValueElement> _valueElement;
    bool _isDeterminate {false};

  public:
    KRYS_NODISCARD bool CanContainRangeEndPoint() const noexcept final
    {
      return false;
    }

    explicit HTMLProgressElement(Document &document) noexcept;

    ~HTMLProgressElement() noexcept override;

    KRYS_NODISCARD static Ref<HTMLProgressElement> Create(Document &document) noexcept;

    /// @brief A progress bar without a valid value is indeterminate.
    KRYS_NODISCARD bool MatchesIndeterminatePseudoClass() const noexcept final;

    /// @brief Whether the progress bar has a value attribute (without one, it's indeterminate).
    KRYS_NODISCARD bool IsDeterminate() const noexcept
    {
      return _isDeterminate;
    }

#pragma region HTMLProgressElement - https://html.spec.whatwg.org/multipage/form-elements.html#htmlprogresselement

    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#dom-progress-value
    KRYS_NODISCARD double Value() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#dom-progress-value
    void Value(double value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#dom-progress-max
    KRYS_NODISCARD double Max() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#dom-progress-max
    void Max(double value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#dom-progress-position
    KRYS_NODISCARD double Position() const noexcept;

    // readonly attribute NodeList labels;

#pragma endregion

  protected:
    void OnAttributeChanged(const DOMStringAtom &localName, Maybe<DOMStringView> oldValue,
                            Maybe<DOMStringView> value, const DOMStringAtom &namespaceURI) noexcept override;

  private:
    KRYS_NODISCARD RenderPtr<RenderElement> CreateElementRenderer(RenderStyle &&style) noexcept override;

    void DidAttachRenderers() noexcept override;

    void DidAddUserAgentShadowRoot(boo::ShadowRoot &root) noexcept override;

    /// @brief Updates whether the bar is determinate as its value attribute changes, restyling
    /// :indeterminate.
    void UpdateDeterminateState() noexcept;

    /// @brief Updates the bar for a new value or max: its width, the renderer and the accessibility value.
    void DidElementStateChange() noexcept;

    /// @brief The progress bar's renderer, if it's a progress one.
    KRYS_NODISCARD RenderProgress *GetRenderProgress() const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::HTMLProgressElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::Node &target) noexcept
  {
    return target.IsHTMLElement() && Downcast<krys::boo::HTMLElement>(target).IsHTMLProgressElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::boo::HTMLElement &target) noexcept
  {
    return target.IsHTMLProgressElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
