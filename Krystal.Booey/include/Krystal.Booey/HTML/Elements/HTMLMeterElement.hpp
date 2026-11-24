#pragma once

#include "Krystal.Booey/HTML/Elements/HTMLElement.hpp"
#include "Krystal.Core/Memory/WeakPtr.hpp"

namespace krys::boo
{
  class RenderMeter;

  /// @see https://html.spec.whatwg.org/#htmlmeterelement
  class HTMLMeterElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLMeterElement);

  public:
    enum class GaugeRegion : uint8
    {
      Optimum,
      Suboptimal,
      EvenLessGood,
    };

  private:
    WeakPtr<HTMLElement> _valueElement;

  public:
    KRYS_NODISCARD bool CanContainRangeEndPoint() const noexcept final
    {
      return false;
    }

    explicit HTMLMeterElement(Document &document) noexcept;

    ~HTMLMeterElement() noexcept override;

    KRYS_NODISCARD static Ref<HTMLMeterElement> Create(Document &document) noexcept;

#pragma region HTMLMeterElement - https://html.spec.whatwg.org/#htmlmeterelement

    /// @see https://html.spec.whatwg.org/#dom-meter-value
    KRYS_NODISCARD double Value() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-meter-value
    void Value(double value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-meter-min
    KRYS_NODISCARD double Min() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-meter-min
    void Min(double value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-meter-max
    KRYS_NODISCARD double Max() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-meter-max
    void Max(double value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-meter-low
    KRYS_NODISCARD double Low() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-meter-low
    void Low(double value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-meter-high
    KRYS_NODISCARD double High() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-meter-high
    void High(double value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-meter-optimum
    KRYS_NODISCARD double Optimum() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-meter-optimum
    void Optimum(double value) noexcept;

    // readonly attribute NodeList labels;

#pragma endregion

    /// @brief Where the value lies between min and max, from 0 to 1; 0 if max isn't above min.
    KRYS_NODISCARD double ValueRatio() const noexcept;

    /// @brief Whether the value is in the optimum region, a suboptimal one, or an even less good one.
    KRYS_NODISCARD GaugeRegion GetGaugeRegion() const noexcept;

    KRYS_NODISCARD bool ChildShouldCreateRenderer(const Node &child) const noexcept override;

  protected:
    void OnAttributeChanged(const DOMStringAtom &localName, Maybe<DOMStringView> oldValue,
                            Maybe<DOMStringView> value, const DOMStringAtom &namespaceURI) noexcept override;

  private:
    KRYS_NODISCARD RenderPtr<RenderElement> CreateElementRenderer(RenderStyle &&style) noexcept override;

    void DidAddUserAgentShadowRoot(boo::ShadowRoot &root) noexcept override;

    /// @brief Updates the bar for a new value or range: its width and region class, and the renderer.
    void DidElementStateChange() noexcept;

    /// @brief The meter's renderer, if it's a meter one.
    KRYS_NODISCARD RenderMeter *GetRenderMeter() const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::HTMLMeterElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::Node &target) noexcept
  {
    return target.IsHTMLElement() && Downcast<krys::boo::HTMLElement>(target).IsHTMLMeterElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::boo::HTMLElement &target) noexcept
  {
    return target.IsHTMLMeterElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
