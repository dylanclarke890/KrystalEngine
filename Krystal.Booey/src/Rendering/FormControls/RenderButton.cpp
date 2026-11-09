/**
 * Copyright (C) 2005-2022 Apple Inc. All rights reserved.
 * Copyright (C) 2015 Google Inc. All rights reserved.
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

#include "Krystal.Booey/Rendering/FormControls/RenderButton.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLFormControlElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLInputElement.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/LayoutIntegrationLineLayout.hpp"
#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Booey/Rendering/RenderTextFragment.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+SettersInlines.hpp"
#include "Krystal.Booey/Rendering/Updating/RenderTreeBuilder.hpp"

namespace krys::boo
{
  RenderButton::RenderButton(html::HTMLFormControlElement &element, RenderStyle &&style) noexcept
      : RenderFlexibleBox(Type::Button, element, krys::move(style))
  {
    krys_debug_assert(IsRenderButton());
  }

  RenderButton::~RenderButton() noexcept = default;

  html::HTMLFormControlElement &RenderButton::FormControlElement() const noexcept
  {
    return Downcast<html::HTMLFormControlElement>(NodeForNonAnonymous());
  }

  bool RenderButton::CanBeSelectionLeaf() const noexcept
  {
    return FormControlElement().HasEditableStyle();
  }

  bool RenderButton::HasLineIfEmpty() const noexcept
  {
    return Is<html::HTMLInputElement>(FormControlElement());
  }

  void RenderButton::SetInnerRenderer(RenderBlock &innerRenderer) noexcept
  {
    krys_debug_assert(!_inner.get());
    _inner = CreateWeakPtr(&innerRenderer);
    UpdateAnonymousChildStyle(_inner->MutableStyle());
    if (_inner && _inner->LayoutBox())
    {
      if (auto *inlineFormattingContextRoot = DynamicDowncast<RenderBlockFlow>(*_inner);
          inlineFormattingContextRoot && inlineFormattingContextRoot->InlineLayout())
      {
        (void)inlineFormattingContextRoot->InlineLayout()->RootStyleWillChange(
          *inlineFormattingContextRoot, inlineFormattingContextRoot->Style());
      }
      if (auto *lineLayout = layout_integration::LineLayout::Containing(*_inner))
      {
        (void)lineLayout->StyleWillChange(*_inner, _inner->Style(), style::DifferenceResult::Layout);
      }
      layout_integration::LineLayout::UpdateStyle(*_inner);
      for (auto *child = _inner->FirstChild(); child; child = child->NextSibling())
      {
        if (auto *text = DynamicDowncast<RenderText>(*child))
        {
          layout_integration::LineLayout::UpdateStyle(*text);
        }
      }
    }
  }

  void RenderButton::UpdateAnonymousChildStyle(RenderStyle &childStyle) const noexcept
  {
    childStyle.SetFlexGrow(style::FlexGrow {1.0f});
    // min-inline-size: 0; is needed for correct shrinking.
    // Use margin-block:auto instead of align-items:center to get safe centering, i.e.
    // when the content overflows, treat it the same as align-items: flex-start.
    if (IsHorizontalWritingMode())
    {
      childStyle.SetMinWidth(style::MinimumSize::Fixed {0});
      childStyle.SetMarginTop(css::keywords::Auto {});
      childStyle.SetMarginBottom(css::keywords::Auto {});
    }
    else
    {
      childStyle.SetMinHeight(style::MinimumSize::Fixed {0});
      childStyle.SetMarginLeft(css::keywords::Auto {});
      childStyle.SetMarginRight(css::keywords::Auto {});
    }
    childStyle.SetTextBoxTrim(Style().GetTextBoxTrim());
  }

  void RenderButton::UpdateFromElement() noexcept
  {
    // If we're an input element, we may need to change our button text.
    if (auto *input = DynamicDowncast<html::HTMLInputElement>(FormControlElement()))
    {
      auto value = input->ValueWithDefault();
      SetText(value);
    }
  }

  void RenderButton::SetText(const utf8_string &str) noexcept
  {
    if (!_buttonText && str.empty())
    {
      return;
    }

    if (!_buttonText)
    {
      auto newButtonText = CreateRenderer<RenderTextFragment>(GetDocument(), str);
      _buttonText = CreateWeakPtr(newButtonText.get());
      // FIXME: This mutation should go through the normal RenderTreeBuilder path.
      if (RenderTreeBuilder::Current())
      {
        RenderTreeBuilder::Current()->Attach(*this, krys::move(newButtonText));
      }
      else
      {
        RenderTreeBuilder(*GetDocument().GetRenderView()).Attach(*this, krys::move(newButtonText));
      }
      return;
    }

    if (!str.empty())
    {
      _buttonText->SetText(str);
      return;
    }

    if (RenderTreeBuilder::Current())
    {
      RenderTreeBuilder::Current()->Destroy(*_buttonText);
    }
    else
    {
      RenderTreeBuilder(*GetDocument().GetRenderView()).Destroy(*_buttonText);
    }
  }

  utf8_string RenderButton::Text() const noexcept
  {
    if (_buttonText)
    {
      return _buttonText->RenderText::Text();
    }
    return {};
  }

  bool RenderButton::CanHaveGeneratedChildren() const noexcept
  {
    // Input elements can't have generated children, but button elements can. We'll
    // write the code assuming any other button types that might emerge in the future
    // can also have children.
    return !Is<html::HTMLInputElement>(FormControlElement());
  }

  LayoutRect RenderButton::ControlClipRect(const LayoutPoint &additionalOffset) const noexcept
  {
    // Clip to the padding box to at least give content the extra padding space.
    return LayoutRect(additionalOffset.X() + BorderLeft(), additionalOffset.Y() + BorderTop(),
                      Width() - BorderLeft() - BorderRight(), Height() - BorderTop() - BorderBottom());
  }

  // Only clip overflow on input elements, to match other browsers.
  bool RenderButton::HasControlClip() const noexcept
  {
    return Is<html::HTMLInputElement>(FormControlElement());
  }
}
