/*
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#include "Krystal.Booey/HTML/Elements/HTMLProgressElement.hpp"
#include "Krystal.Booey/Accessibility/AXObjectCache.hpp"
#include "Krystal.Booey/DOM/Nodes/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/Attributes/ReflectAttribute.hpp"
#include "Krystal.Booey/HTML/Parser/ParserIdioms.hpp"
#include "Krystal.Booey/HTML/Shadow/ProgressShadowElement.hpp"
#include "Krystal.Booey/Rendering/FormControls/RenderProgress.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Style/Invalidation/PseudoClassChangeInvalidation.hpp"
#include <cmath>

namespace krys::boo::html
{
  HTMLProgressElement::HTMLProgressElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Progress, HTMLElementFlags::None,
                    dom::NodeFlags::HasCustomStyleResolveCallbacks)
  {
  }

  HTMLProgressElement::~HTMLProgressElement() noexcept = default;

  Ref<HTMLProgressElement> HTMLProgressElement::Create(dom::Document &document) noexcept
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

  void HTMLProgressElement::OnAttributeChanged(dom::DOMStringAtom localName,
                                               Maybe<dom::DOMStringView> oldValue,
                                               Maybe<dom::DOMStringView> value,
                                               dom::DOMStringAtom namespaceURI) noexcept
  {
    HTMLElement::OnAttributeChanged(localName, oldValue, value, namespaceURI);
    if (namespaceURI != dom::DOMStringAtom::Null())
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

  // https://html.spec.whatwg.org/#dom-progress-value
  // https://html.spec.whatwg.org/#concept-progress-current-value
  double HTMLProgressElement::Value() const noexcept
  {
    // An indeterminate progress bar has no value attribute, so its value is 0 too.
    double value = ParseHTMLFloatingPointNumberValue(GetAttribute(u8"value").value_or(dom::DOMString()));
    return !std::isfinite(value) || value < 0 ? 0 : std::min(value, Max());
  }

  void HTMLProgressElement::Value(double value) noexcept
  {
    Reflect<double>(*this, u8"value", krys::move(value));
  }

  // https://html.spec.whatwg.org/#concept-progress-maximum
  double HTMLProgressElement::Max() const noexcept
  {
    double max = ParseHTMLFloatingPointNumberValue(GetAttribute(u8"max").value_or(dom::DOMString()));
    return !std::isfinite(max) || max <= 0 ? 1 : max;
  }

  void HTMLProgressElement::Max(double max) noexcept
  {
    if (max > 0)
    {
      Reflect<double>(*this, u8"max", krys::move(max));
    }
  }

  // https://html.spec.whatwg.org/#dom-progress-position
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

  void HTMLProgressElement::DidAddUserAgentShadowRoot(dom::ShadowRoot &root) noexcept
  {
    krys_debug_assert(!_valueElement);

    auto &document = NodeDocument();
    auto inner = ProgressInnerElement::Create(document);
    (void)root.AppendChild(*inner);

    auto bar = ProgressBarElement::Create(document);
    auto valueElement = ProgressValueElement::Create(document);
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
