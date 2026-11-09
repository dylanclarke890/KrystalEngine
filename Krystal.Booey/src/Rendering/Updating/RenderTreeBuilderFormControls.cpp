/*
 * Copyright (C) 2017-2025 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/Rendering/Updating/RenderTreeBuilderFormControls.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLOptionElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLSelectElement.hpp"
#include "Krystal.Booey/Rendering/FormControls/RenderButton.hpp"
#include "Krystal.Booey/Rendering/FormControls/RenderMenuList.hpp"
#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/Updating/RenderTreeBuilderBlock.hpp"
#include "Krystal.Booey/Rendering/Updating/RenderTreeUpdaterGeneratedContent.hpp"

namespace krys::boo
{
  RenderTreeBuilder::FormControls::FormControls(RenderTreeBuilder &builder) noexcept : _builder(builder)
  {
  }

  void RenderTreeBuilder::FormControls::Attach(RenderButton &parent, RenderPtr<RenderObject> child,
                                               RenderObject *beforeChild) noexcept
  {
    _builder.BlockBuilder().Attach(FindOrCreateParentForChild(parent), krys::move(child), beforeChild);
  }

  void RenderTreeBuilder::FormControls::Attach(RenderMenuList &parent, RenderPtr<RenderObject> child,
                                               RenderObject *beforeChild) noexcept
  {
    auto &newChild = *child.get();
    _builder.BlockBuilder().Attach(FindOrCreateParentForChild(parent), krys::move(child), beforeChild);
    parent.DidAttachChild(newChild, beforeChild);
  }

  RenderPtr<RenderObject> RenderTreeBuilder::FormControls::Detach(RenderMenuList &parent, RenderObject &child,
                                                                  WillBeDestroyed willBeDestroyed) noexcept
  {
    auto *innerRenderer = parent.InnerRenderer();
    if (!innerRenderer || &child == innerRenderer)
    {
      return _builder.BlockBuilder().Detach(parent, child, willBeDestroyed, CanCollapseAnonymousBlock::Yes);
    }
    return _builder.Detach(*innerRenderer, child, willBeDestroyed);
  }

  RenderPtr<RenderObject> RenderTreeBuilder::FormControls::Detach(RenderButton &parent, RenderObject &child,
                                                                  WillBeDestroyed willBeDestroyed) noexcept
  {
    auto *innerRenderer = parent.InnerRenderer();
    if (!innerRenderer || &child == innerRenderer || child.Parent() == &parent)
    {
      krys_debug_assert(&child == innerRenderer || !innerRenderer);
      return _builder.BlockBuilder().Detach(parent, child, willBeDestroyed, CanCollapseAnonymousBlock::Yes);
    }
    return _builder.Detach(*innerRenderer, child, willBeDestroyed);
  }

  RenderBlock &RenderTreeBuilder::FormControls::FindOrCreateParentForChild(RenderButton &parent) noexcept
  {
    auto *innerRenderer = parent.InnerRenderer();
    if (innerRenderer)
    {
      return *innerRenderer;
    }

    auto wrapper = Block::CreateAnonymousBlockWithStyle(parent.GetDocument(), parent.Style());
    innerRenderer = wrapper.get();
    _builder.BlockBuilder().Attach(parent, krys::move(wrapper), nullptr);
    parent.SetInnerRenderer(*innerRenderer);
    return *innerRenderer;
  }

  RenderBlock &RenderTreeBuilder::FormControls::FindOrCreateParentForChild(RenderMenuList &parent) noexcept
  {
    auto *innerRenderer = parent.InnerRenderer();
    if (innerRenderer)
    {
      return *innerRenderer;
    }

    auto wrapper = Block::CreateAnonymousBlockWithStyle(parent.GetDocument(), parent.Style());
    innerRenderer = wrapper.get();
    _builder.BlockBuilder().Attach(parent, krys::move(wrapper), nullptr);
    parent.SetInnerRenderer(*innerRenderer);
    return *innerRenderer;
  }

  void RenderTreeBuilder::FormControls::UpdateAfterDescendants(RenderElement &renderer) noexcept
  {
    auto *element = renderer.GetElement();
    if (!element)
    {
      return;
    }

    auto type = PseudoElementType::Checkmark;
    bool usesBaseAppearance = false;
    if (auto *select = DynamicDowncast<html::HTMLSelectElement>(*element))
    {
      type = PseudoElementType::PickerIcon;
      usesBaseAppearance = select->ContentsUseBaseAppearance();
    }
    else if (auto *option = DynamicDowncast<html::HTMLOptionElement>(*element))
    {
      auto *ownerSelect = option->OwnerSelectElement();
      usesBaseAppearance = ownerSelect && ownerSelect->ContentsUseBaseAppearance();
    }
    else
    {
      return;
    }

    RenderElement *existing = nullptr;
    for (auto *child = renderer.FirstChild(); child; child = child->NextSibling())
    {
      if (auto *childElement = DynamicDowncast<RenderElement>(*child);
          childElement && childElement->IsAnonymous() && childElement->Style().GetPseudoElementType() == type)
      {
        existing = childElement;
        break;
      }
    }

    const auto *style =
      usesBaseAppearance ? renderer.GetCachedPseudoStyle({type}, &renderer.Style()) : nullptr;
    bool isNeeded =
      style && style->Display() != DisplayType::None && style->GetContent().TryData() != nullptr;

    // Kept while its content stays: an animation or transition of it goes on from frame to frame.
    if (existing && isNeeded && existing->Style().GetContent() == style->GetContent())
    {
      existing->SetStyle(RenderStyle::Clone(*style));
      return;
    }
    if (existing)
    {
      _builder.Destroy(*existing);
    }
    if (!isNeeded)
    {
      return;
    }

    auto generated = boo::CreateRenderer<RenderBlockFlow>(RenderObject::Type::BlockFlow,
                                                          renderer.GetDocument(), RenderStyle::Clone(*style));
    generated->InitializeStyle();
    auto &generatedRenderer = *generated;
    // A checkmark comes before the option's contents, and an icon after the select's.
    _builder.Attach(renderer, krys::move(generated),
                    type == PseudoElementType::Checkmark ? renderer.FirstChild() : nullptr);
    RenderTreeUpdater::CreateContentRenderers(_builder, generatedRenderer, generatedRenderer.Style());
  }
}
