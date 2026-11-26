#include "Krystal.Web/Html/Elements/HTMLProgressElement.hpp"
#include "Krystal.Web/Accessibility/AXObjectCache.hpp"
#include "Krystal.Web/Dom/Nodes/ShadowRoot.hpp"
#include "Krystal.Web/Html/Attributes/ReflectAttribute.hpp"
#include "Krystal.Web/Html/Parser/ParserIdioms.hpp"
#include "Krystal.Web/Html/Shadow/ProgressShadowElement.hpp"
#include "Krystal.Web/Rendering/FormControls/RenderProgress.hpp"
#include "Krystal.Web/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Web/Style/Invalidation/PseudoClassChangeInvalidation.hpp"
#include <cmath>

namespace krys::web
{
  HTMLProgressElement::HTMLProgressElement(Document &document) noexcept
      : HTMLElement(document, html::HTMLElementInterface::Progress, {},
                    dom::NodeFlag::HasCustomStyleResolveCallbacks)
  {
  }

  HTMLProgressElement::~HTMLProgressElement() noexcept = default;

  Ref<HTMLProgressElement> HTMLProgressElement::Create(Document &document) noexcept
  {
    auto progress = CreateRef<HTMLProgressElement>(document);
    progress->EnsureUserAgentShadowRoot();
    return progress;
  }

  RenderPtr<RenderElement> HTMLProgressElement::CreateElementRenderer(RenderStyle &&style) noexcept
  {
    if (!style.HasUsedAppearance())
    {
      return RenderElement::CreateFor(*this, krys::move(style));
    }

    return CreateRenderer<RenderProgress>(*this, krys::move(style));
  }

  RenderProgress *HTMLProgressElement::GetRenderProgress() const noexcept
  {
    if (auto *renderProgress = DynamicDowncast<RenderProgress>(Renderer()))
    {
      return renderProgress;
    }

    auto *root = UserAgentShadowRoot();
    auto *inner = root ? root->FirstChild() : nullptr;
    auto *renderer = inner ? inner->Renderer() : nullptr;
    return renderer ? DynamicDowncast<RenderProgress>(*renderer) : nullptr;
  }

  void HTMLProgressElement::OnAttributeChanged(const DOMStringAtom &localName, Maybe<DOMStringView> oldValue,
                                               Maybe<DOMStringView> value,
                                               const DOMStringAtom &namespaceURI) noexcept
  {
    HTMLElement::OnAttributeChanged(localName, oldValue, value, namespaceURI);
    if (namespaceURI != null)
    {
      return;
    }

    if (localName == u8"value")
    {
      UpdateDeterminateState();
      DidElementStateChange();
    }
    else if (localName == u8"max")
    {
      DidElementStateChange();
    }
  }

  void HTMLProgressElement::DidAttachRenderers() noexcept
  {
    if (auto *renderer = GetRenderProgress())
    {
      renderer->UpdateFromElement();
    }
  }

#pragma region HTMLProgressElement

  // https://html.spec.whatwg.org/multipage/form-elements.html#dom-progress-value
  // https://html.spec.whatwg.org/multipage/form-elements.html#concept-progress-current-value
  double HTMLProgressElement::Value() const noexcept
  {
    // An indeterminate progress bar has no value attribute, so its value is 0 too.
    double value = html::ParseHTMLFloatingPointNumberValue(GetAttribute(u8"value").value_or(DOMString()));
    return !std::isfinite(value) || value < 0 ? 0 : std::min(value, Max());
  }

  void HTMLProgressElement::Value(double value) noexcept
  {
    html::Reflect<double>(*this, u8"value", krys::move(value));
  }

  // https://html.spec.whatwg.org/multipage/form-elements.html#concept-progress-maximum
  double HTMLProgressElement::Max() const noexcept
  {
    double max = html::ParseHTMLFloatingPointNumberValue(GetAttribute(u8"max").value_or(DOMString()));
    return !std::isfinite(max) || max <= 0 ? 1 : max;
  }

  void HTMLProgressElement::Max(double value) noexcept
  {
    if (value > 0)
    {
      html::Reflect<double>(*this, u8"max", krys::move(value));
    }
  }

  // https://html.spec.whatwg.org/multipage/form-elements.html#dom-progress-position
  double HTMLProgressElement::Position() const noexcept
  {
    if (!IsDeterminate())
    {
      return HTMLProgressElement::IndeterminatePosition;
    }
    return Value() / Max();
  }

#pragma endregion

  void HTMLProgressElement::UpdateDeterminateState() noexcept
  {
    bool newIsDeterminate = HasAttribute(u8"value");
    if (_isDeterminate == newIsDeterminate)
    {
      return;
    }

    style::PseudoClassChangeInvalidation styleInvalidation(*this, css::SelectorPseudoClass::Indeterminate,
                                                           !newIsDeterminate);
    _isDeterminate = newIsDeterminate;
  }

  void HTMLProgressElement::DidElementStateChange() noexcept
  {
    if (auto *valueElement = _valueElement.get())
    {
      valueElement->SetInlineSizePercentage(Position() * 100);
    }
    if (auto *renderer = GetRenderProgress())
    {
      renderer->UpdateFromElement();
    }
    if (auto *cache = NodeDocument().ExistingAXObjectCache())
    {
      cache->ValueChanged(*this);
    }
  }

  void HTMLProgressElement::DidAddUserAgentShadowRoot(web::ShadowRoot &root) noexcept
  {
    krys_debug_assert(!_valueElement);

    auto &document = NodeDocument();
    auto inner = html::ProgressInnerElement::Create(document);
    (void)root.AppendChild(*inner);

    auto bar = html::ProgressBarElement::Create(document);
    auto valueElement = html::ProgressValueElement::Create(document);
    valueElement->SetInlineSizePercentage(HTMLProgressElement::IndeterminatePosition * 100);
    (void)bar->AppendChild(*valueElement);
    _valueElement = CreateWeakPtr(valueElement.get());

    (void)inner->AppendChild(*bar);
  }

  bool HTMLProgressElement::MatchesIndeterminatePseudoClass() const noexcept
  {
    return !IsDeterminate();
  }
}
