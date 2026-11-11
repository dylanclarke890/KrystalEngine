/*
 * Copyright (C) 2016 Igalia, S.L.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/Accessibility/AccessibilityMathMLElement.hpp"
#include "Krystal.Booey/Accessibility/AXObjectCache.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/MathML/Elements/MathMLElement.hpp"
#include "Krystal.Booey/Rendering/MathML/RenderMathMLBlock.hpp"
#include "Krystal.Booey/Rendering/MathML/RenderMathMLOperator.hpp"

namespace krys::boo::ax
{
  AccessibilityMathMLElement::AccessibilityMathMLElement(AXID id, RenderObject &renderer,
                                                         AXObjectCache &cache) noexcept
      : AccessibilityRenderObject(id, &renderer, renderer.GetNode(), cache)
  {
  }

  AccessibilityMathMLElement::~AccessibilityMathMLElement() noexcept = default;

  Ref<AccessibilityMathMLElement> AccessibilityMathMLElement::Create(AXID id, RenderObject &renderer,
                                                                     AXObjectCache &cache) noexcept
  {
    return AdoptRef(*new AccessibilityMathMLElement(id, renderer, cache));
  }

  bool AccessibilityMathMLElement::HasLocalName(dom::DOMStringView localName) const noexcept
  {
    auto *element = GetElement();
    return element && Is<mathml::MathMLElement>(*element) && element->LocalName() == localName;
  }

  AccessibilityRole AccessibilityMathMLElement::DetermineAccessibilityRole() noexcept
  {
    if (!Renderer())
    {
      return AccessibilityRole::Unknown;
    }
    if (_ariaRole != AccessibilityRole::Unknown)
    {
      return _ariaRole;
    }
    if (HasLocalName(u8"math"))
    {
      return AccessibilityRole::DocumentMath;
    }
    // Which role a math element has isn't clear: a role of its own leaves it to the platform.
    return AccessibilityRole::MathElement;
  }

  bool AccessibilityMathMLElement::IsIgnoredElementWithinMathTree() const noexcept
  {
    // Only the elements recognised here are in the tree: not mstyle and the like.
    return !(IsMathFraction() || IsMathSubscriptSuperscript() || IsMathRow() || IsMathUnderOver()
             || IsMathRoot() || IsMathText() || IsMathNumber() || IsMathOperator() || IsMathFenceOperator()
             || IsMathSeparatorOperator() || IsMathIdentifier() || IsMathTable() || IsMathTableRow()
             || IsMathTableCell() || IsMathMultiscript());
  }

  bool AccessibilityMathMLElement::IsMathFraction() const noexcept
  {
    auto *renderer = Renderer();
    return renderer && renderer->IsRenderMathMLFraction();
  }

  bool AccessibilityMathMLElement::IsMathSubscriptSuperscript() const noexcept
  {
    auto *renderer = Renderer();
    return renderer && renderer->IsRenderMathMLScripts() && !IsMathMultiscript();
  }

  bool AccessibilityMathMLElement::IsMathRow() const noexcept
  {
    auto *renderer = Renderer();
    return renderer && renderer->IsRenderMathMLRow() && !IsMathRoot() && !IsMathUnderOver()
           && !IsMathMultiscript() && !IsMathFraction();
  }

  bool AccessibilityMathMLElement::IsMathUnderOver() const noexcept
  {
    auto *renderer = Renderer();
    return renderer && renderer->IsRenderMathMLUnderOver();
  }

  bool AccessibilityMathMLElement::IsMathRoot() const noexcept
  {
    auto *renderer = Renderer();
    return renderer && renderer->IsRenderMathMLRoot();
  }

  bool AccessibilityMathMLElement::IsMathSquareRoot() const noexcept
  {
    return IsMathRoot() && HasLocalName(u8"msqrt");
  }

  bool AccessibilityMathMLElement::IsMathToken() const noexcept
  {
    auto *renderer = Renderer();
    return renderer && renderer->IsRenderMathMLToken();
  }

  bool AccessibilityMathMLElement::IsMathOperator() const noexcept
  {
    auto *renderer = Renderer();
    return renderer && renderer->IsRenderMathMLOperator();
  }

  bool AccessibilityMathMLElement::IsMathFenceOperator() const noexcept
  {
    auto *mathOperator = DynamicDowncast<RenderMathMLOperator>(Renderer());
    return mathOperator && mathOperator->HasOperatorFlag(mathml::MathMLOperatorDictionary::Fence);
  }

  bool AccessibilityMathMLElement::IsMathSeparatorOperator() const noexcept
  {
    auto *mathOperator = DynamicDowncast<RenderMathMLOperator>(Renderer());
    return mathOperator && mathOperator->HasOperatorFlag(mathml::MathMLOperatorDictionary::Separator);
  }

  bool AccessibilityMathMLElement::IsMathText() const noexcept
  {
    return HasLocalName(u8"mtext") || HasLocalName(u8"ms");
  }

  bool AccessibilityMathMLElement::IsMathNumber() const noexcept
  {
    return HasLocalName(u8"mn");
  }

  bool AccessibilityMathMLElement::IsMathIdentifier() const noexcept
  {
    return HasLocalName(u8"mi");
  }

  bool AccessibilityMathMLElement::IsMathMultiscript() const noexcept
  {
    return HasLocalName(u8"mmultiscripts");
  }

  bool AccessibilityMathMLElement::IsMathTable() const noexcept
  {
    return HasLocalName(u8"mtable");
  }

  bool AccessibilityMathMLElement::IsMathTableRow() const noexcept
  {
    return HasLocalName(u8"mtr");
  }

  bool AccessibilityMathMLElement::IsMathTableCell() const noexcept
  {
    return HasLocalName(u8"mtd");
  }

  Maybe<AccessibilityObject::AccessibilityChildrenVector> AccessibilityMathMLElement::MathRadicand() noexcept
  {
    if (!IsMathRoot())
    {
      return null;
    }
    const auto &children = Children();
    if (children.empty())
    {
      return null;
    }
    // A square root's radicand is all its children (an inferred row); a root's is its first.
    if (IsMathSquareRoot())
    {
      return children;
    }
    return AccessibilityChildrenVector {children[0]};
  }

  AccessibilityObject *AccessibilityMathMLElement::MathRootIndexObject() noexcept
  {
    if (!IsMathRoot() || IsMathSquareRoot())
    {
      return nullptr;
    }
    const auto &children = Children();
    return children.size() < 2 ? nullptr : children[1].get();
  }

  AccessibilityObject *AccessibilityMathMLElement::MathNumeratorObject() noexcept
  {
    if (!IsMathFraction())
    {
      return nullptr;
    }
    const auto &children = Children();
    return children.size() != 2 ? nullptr : children[0].get();
  }

  AccessibilityObject *AccessibilityMathMLElement::MathDenominatorObject() noexcept
  {
    if (!IsMathFraction())
    {
      return nullptr;
    }
    const auto &children = Children();
    return children.size() != 2 ? nullptr : children[1].get();
  }

  AccessibilityObject *AccessibilityMathMLElement::MathUnderObject() noexcept
  {
    if (!IsMathUnderOver())
    {
      return nullptr;
    }
    const auto &children = Children();
    if (children.size() >= 2 && (HasLocalName(u8"munder") || HasLocalName(u8"munderover")))
    {
      return children[1].get();
    }
    return nullptr;
  }

  AccessibilityObject *AccessibilityMathMLElement::MathOverObject() noexcept
  {
    if (!IsMathUnderOver())
    {
      return nullptr;
    }
    const auto &children = Children();
    if (children.size() >= 2 && HasLocalName(u8"mover"))
    {
      return children[1].get();
    }
    if (children.size() >= 3 && HasLocalName(u8"munderover"))
    {
      return children[2].get();
    }
    return nullptr;
  }

  AccessibilityObject *AccessibilityMathMLElement::MathBaseObject() noexcept
  {
    if (!IsMathSubscriptSuperscript() && !IsMathUnderOver() && !IsMathMultiscript())
    {
      return nullptr;
    }
    // The base is always the first child.
    const auto &children = Children();
    return children.empty() ? nullptr : children[0].get();
  }

  AccessibilityObject *AccessibilityMathMLElement::MathSubscriptObject() noexcept
  {
    if (!IsMathSubscriptSuperscript())
    {
      return nullptr;
    }
    const auto &children = Children();
    if (children.size() >= 2 && (HasLocalName(u8"msub") || HasLocalName(u8"msubsup")))
    {
      return children[1].get();
    }
    return nullptr;
  }

  AccessibilityObject *AccessibilityMathMLElement::MathSuperscriptObject() noexcept
  {
    if (!IsMathSubscriptSuperscript())
    {
      return nullptr;
    }
    const auto &children = Children();
    if (children.size() >= 2 && HasLocalName(u8"msup"))
    {
      return children[1].get();
    }
    if (children.size() >= 3 && HasLocalName(u8"msubsup"))
    {
      return children[2].get();
    }
    return nullptr;
  }

  AccessibilityMathMultiscriptPairs AccessibilityMathMLElement::MathPrescripts() noexcept
  {
    AccessibilityMathMultiscriptPairs prescripts;
    auto *node = GetNode();
    auto *cache = AxObjectCache();
    if (!IsMathMultiscript() || !node || !cache)
    {
      return prescripts;
    }
    // The prescripts are the subscript, superscript pairs after mprescripts.
    bool foundPrescript = false;
    Pair<AccessibilityObject *, AccessibilityObject *> pair {};
    for (auto *child = node->FirstChild(); child; child = child->NextSibling())
    {
      if (!foundPrescript)
      {
        auto *element = DynamicDowncast<mathml::MathMLElement>(*child);
        foundPrescript = element && element->LocalName() == u8"mprescripts";
        continue;
      }
      auto *object = cache->GetOrCreate(*child);
      if (!object || !object->IsMathElement())
      {
        continue;
      }
      if (!pair.first)
      {
        pair.first = object;
      }
      else
      {
        pair.second = object;
        prescripts.push_back(pair);
        pair = {};
      }
    }
    // An odd number of prescripts.
    if (pair.first)
    {
      prescripts.push_back(pair);
    }
    return prescripts;
  }

  AccessibilityMathMultiscriptPairs AccessibilityMathMLElement::MathPostscripts() noexcept
  {
    AccessibilityMathMultiscriptPairs postscripts;
    auto *node = GetNode();
    auto *cache = AxObjectCache();
    if (!IsMathMultiscript() || !node || !cache)
    {
      return postscripts;
    }
    // The postscripts are the subscript, superscript pairs after the base and before mprescripts.
    Pair<AccessibilityObject *, AccessibilityObject *> pair {};
    bool foundBaseElement = false;
    for (auto *child = node->FirstChild(); child; child = child->NextSibling())
    {
      auto *element = DynamicDowncast<mathml::MathMLElement>(*child);
      if (element && element->LocalName() == u8"mprescripts")
      {
        break;
      }
      auto *object = cache->GetOrCreate(*child);
      if (!object || !object->IsMathElement())
      {
        continue;
      }
      if (!foundBaseElement)
      {
        foundBaseElement = true;
      }
      else if (!pair.first)
      {
        pair.first = object;
      }
      else
      {
        pair.second = object;
        postscripts.push_back(pair);
        pair = {};
      }
    }
    // An odd number of postscripts.
    if (pair.first)
    {
      postscripts.push_back(pair);
    }
    return postscripts;
  }
}
