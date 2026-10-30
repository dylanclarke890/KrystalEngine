#pragma once

#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo
{
  class RenderProgress;
}

namespace krys::boo::html
{
  class ProgressValueElement;

  /// @see https://html.spec.whatwg.org/#htmlprogresselement
  class HTMLProgressElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLProgressElement);

  public:
    static constexpr double IndeterminatePosition = -1;
    static constexpr double InvalidPosition = -2;

  private:
    WeakPtr<ProgressValueElement> _valueElement;
    bool _isDeterminate {false};

  public:
    KRYS_NODISCARD bool CanContainRangeEndPoint() const noexcept final
    {
      return false;
    }

    explicit HTMLProgressElement(dom::Document &document) noexcept;
    ~HTMLProgressElement() noexcept override;

    KRYS_NODISCARD static Ref<HTMLProgressElement> Create(dom::Document &document) noexcept;

    /// @brief A progress bar without a valid value is indeterminate.
    KRYS_NODISCARD bool MatchesIndeterminatePseudoClass() const noexcept final;

    KRYS_NODISCARD bool IsDeterminate() const noexcept
    {
      return _isDeterminate;
    }

#pragma region HTMLProgressElement - https://html.spec.whatwg.org/#htmlprogresselement

    /// @see https://html.spec.whatwg.org/#dom-progress-value
    KRYS_NODISCARD double Value() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-progress-value
    void Value(double value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-progress-max
    KRYS_NODISCARD double Max() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-progress-max
    void Max(double value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-progress-position
    KRYS_NODISCARD double Position() const noexcept;

    // readonly attribute NodeList labels;

#pragma endregion

  protected:
    void OnAttributeChanged(dom::DOMStringAtom localName, Maybe<dom::DOMStringView> oldValue,
                            Maybe<dom::DOMStringView> value,
                            dom::DOMStringAtom namespaceURI) noexcept override;

  private:
    KRYS_NODISCARD RenderPtr<RenderElement> CreateElementRenderer(RenderStyle &&style) noexcept override;
    void DidAttachRenderers() noexcept override;
    void DidAddUserAgentShadowRoot(dom::ShadowRoot &root) noexcept override;

    void UpdateDeterminateState() noexcept;
    void DidElementStateChange() noexcept;
    KRYS_NODISCARD RenderProgress *GetRenderProgress() const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLProgressElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &target) noexcept
  {
    return target.IsHTMLElement() && Downcast<krys::boo::html::HTMLElement>(target).IsHTMLProgressElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLProgressElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
