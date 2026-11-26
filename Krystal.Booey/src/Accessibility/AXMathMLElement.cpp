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

#include "Krystal.Booey/Accessibility/AXMathMLElement.hpp"
#include "Krystal.Booey/Accessibility/AXObjectCache.hpp"
#include "Krystal.Booey/Dom/Nodes/Element.hpp"
#include "Krystal.Booey/MathML/Elements/MathMLElement.hpp"
#include "Krystal.Booey/Rendering/MathML/RenderMathMLBlock.hpp"
#include "Krystal.Booey/Rendering/MathML/RenderMathMLOperator.hpp"

namespace krys::boo
{
  AXMathMLElement::AXMathMLElement(AXId id, RenderObject &renderer, AXObjectCache &cache) noexcept
      : AXRenderObject(id, &renderer, renderer.GetNode(), cache)
  {
  }

  AXMathMLElement::~AXMathMLElement() noexcept = default;

  Ref<AXMathMLElement> AXMathMLElement::Create(AXId id, RenderObject &renderer, AXObjectCache &cache) noexcept
  {
    return AdoptRef(*new AXMathMLElement(id, renderer, cache));
  }

  bool AXMathMLElement::HasLocalName(DOMStringView localName) const noexcept
  {
    auto *element = GetElement();
    return element && Is<MathMLElement>(*element) && element->LocalName() == localName;
  }

  AXRole AXMathMLElement::DetermineAccessibilityRole() noexcept
  {
    if (!Renderer())
    {
      return AXRole::Unknown;
    }
    if (_ariaRole != AXRole::Unknown)
    {
      return _ariaRole;
    }
    if (HasLocalName(u8"math"))
    {
      return AXRole::DocumentMath;
    }
    // Which role a math element has isn't clear: a role of its own leaves it to the platform.
    return AXRole::MathElement;
  }

  bool AXMathMLElement::IsIgnoredElementWithinMathTree() const noexcept
  {
    // Only the elements recognised here are in the tree: not mstyle and the like.
    return !(IsMathFraction() || IsMathSubscriptSuperscript() || IsMathRow() || IsMathUnderOver()
             || IsMathRoot() || IsMathText() || IsMathNumber() || IsMathOperator() || IsMathFenceOperator()
             || IsMathSeparatorOperator() || IsMathIdentifier() || IsMathTable() || IsMathTableRow()
             || IsMathTableCell() || IsMathMultiscript());
  }

  bool AXMathMLElement::IsMathFraction() const noexcept
  {
    auto *renderer = Renderer();
    return renderer && renderer->IsRenderMathMLFraction();
  }

  bool AXMathMLElement::IsMathSubscriptSuperscript() const noexcept
  {
    auto *renderer = Renderer();
    return renderer && renderer->IsRenderMathMLScripts() && !IsMathMultiscript();
  }

  bool AXMathMLElement::IsMathRow() const noexcept
  {
    auto *renderer = Renderer();
    return renderer && renderer->IsRenderMathMLRow() && !IsMathRoot() && !IsMathUnderOver()
           && !IsMathMultiscript() && !IsMathFraction();
  }

  bool AXMathMLElement::IsMathUnderOver() const noexcept
  {
    auto *renderer = Renderer();
    return renderer && renderer->IsRenderMathMLUnderOver();
  }

  bool AXMathMLElement::IsMathRoot() const noexcept
  {
    auto *renderer = Renderer();
    return renderer && renderer->IsRenderMathMLRoot();
  }

  bool AXMathMLElement::IsMathSquareRoot() const noexcept
  {
    return IsMathRoot() && HasLocalName(u8"msqrt");
  }

  bool AXMathMLElement::IsMathToken() const noexcept
  {
    auto *renderer = Renderer();
    return renderer && renderer->IsRenderMathMLToken();
  }

  bool AXMathMLElement::IsMathOperator() const noexcept
  {
    auto *renderer = Renderer();
    return renderer && renderer->IsRenderMathMLOperator();
  }

  bool AXMathMLElement::IsMathFenceOperator() const noexcept
  {
    auto *mathOperator = DynamicDowncast<RenderMathMLOperator>(Renderer());
    return mathOperator && mathOperator->HasOperatorFlag(MathMLOperatorDictionary::Fence);
  }

  bool AXMathMLElement::IsMathSeparatorOperator() const noexcept
  {
    auto *mathOperator = DynamicDowncast<RenderMathMLOperator>(Renderer());
    return mathOperator && mathOperator->HasOperatorFlag(MathMLOperatorDictionary::Separator);
  }

  bool AXMathMLElement::IsMathText() const noexcept
  {
    return HasLocalName(u8"mtext") || HasLocalName(u8"ms");
  }

  bool AXMathMLElement::IsMathNumber() const noexcept
  {
    return HasLocalName(u8"mn");
  }

  bool AXMathMLElement::IsMathIdentifier() const noexcept
  {
    return HasLocalName(u8"mi");
  }

  bool AXMathMLElement::IsMathMultiscript() const noexcept
  {
    return HasLocalName(u8"mmultiscripts");
  }

  bool AXMathMLElement::IsMathTable() const noexcept
  {
    return HasLocalName(u8"mtable");
  }

  bool AXMathMLElement::IsMathTableRow() const noexcept
  {
    return HasLocalName(u8"mtr");
  }

  bool AXMathMLElement::IsMathTableCell() const noexcept
  {
    return HasLocalName(u8"mtd");
  }

  Maybe<AXObject::AXChildrenVector> AXMathMLElement::MathRadicand() noexcept
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
    return AXChildrenVector {children[0]};
  }

  AXObject *AXMathMLElement::MathRootIndexObject() noexcept
  {
    if (!IsMathRoot() || IsMathSquareRoot())
    {
      return nullptr;
    }
    const auto &children = Children();
    return children.size() < 2 ? nullptr : children[1].get();
  }

  AXObject *AXMathMLElement::MathNumeratorObject() noexcept
  {
    if (!IsMathFraction())
    {
      return nullptr;
    }
    const auto &children = Children();
    return children.size() != 2 ? nullptr : children[0].get();
  }

  AXObject *AXMathMLElement::MathDenominatorObject() noexcept
  {
    if (!IsMathFraction())
    {
      return nullptr;
    }
    const auto &children = Children();
    return children.size() != 2 ? nullptr : children[1].get();
  }

  AXObject *AXMathMLElement::MathUnderObject() noexcept
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

  AXObject *AXMathMLElement::MathOverObject() noexcept
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

  AXObject *AXMathMLElement::MathBaseObject() noexcept
  {
    if (!IsMathSubscriptSuperscript() && !IsMathUnderOver() && !IsMathMultiscript())
    {
      return nullptr;
    }
    // The base is always the first child.
    const auto &children = Children();
    return children.empty() ? nullptr : children[0].get();
  }

  AXObject *AXMathMLElement::MathSubscriptObject() noexcept
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

  AXObject *AXMathMLElement::MathSuperscriptObject() noexcept
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

  AXMathMultiscriptPairs AXMathMLElement::MathPrescripts() noexcept
  {
    AXMathMultiscriptPairs prescripts;
    auto *node = GetNode();
    auto *cache = AxObjectCache();
    if (!IsMathMultiscript() || !node || !cache)
    {
      return prescripts;
    }
    // The prescripts are the subscript, superscript pairs after mprescripts.
    bool foundPrescript = false;
    Pair<AXObject *, AXObject *> pair {};
    for (auto *child = node->FirstChild(); child; child = child->NextSibling())
    {
      if (!foundPrescript)
      {
        auto *element = DynamicDowncast<MathMLElement>(*child);
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

  AXMathMultiscriptPairs AXMathMLElement::MathPostscripts() noexcept
  {
    AXMathMultiscriptPairs postscripts;
    auto *node = GetNode();
    auto *cache = AxObjectCache();
    if (!IsMathMultiscript() || !node || !cache)
    {
      return postscripts;
    }
    // The postscripts are the subscript, superscript pairs after the base and before mprescripts.
    Pair<AXObject *, AXObject *> pair {};
    bool foundBaseElement = false;
    for (auto *child = node->FirstChild(); child; child = child->NextSibling())
    {
      auto *element = DynamicDowncast<MathMLElement>(*child);
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
