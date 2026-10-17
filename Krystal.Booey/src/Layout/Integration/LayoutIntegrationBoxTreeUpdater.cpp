#include "Krystal.Booey/Layout/Integration/LayoutIntegrationBoxTreeUpdater.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/Text/TextUtil.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutElementBox.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutInitialContainingBlock.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutInlineTextBox.hpp"
#include "Krystal.Booey/Rendering/InlineWalker.hpp"
#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Booey/Rendering/RenderChildIterator.hpp"
#include "Krystal.Booey/Rendering/RenderFlexibleBox.hpp"
#include "Krystal.Booey/Rendering/RenderGrid.hpp"
#include "Krystal.Booey/Rendering/RenderInline.hpp"
#include "Krystal.Booey/Rendering/RenderLineBreak.hpp"
#include "Krystal.Booey/Rendering/RenderListMarker.hpp"
#include "Krystal.Booey/Rendering/RenderReplaced.hpp"
#include "Krystal.Booey/Rendering/RenderTable.hpp"
#include "Krystal.Booey/Rendering/RenderText.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+SettersInlines.hpp"

namespace krys::boo::layout_integration
{
  namespace
  {
    KRYS_NODISCARD UniquePtr<RenderStyle> FirstLineStyleFor(const RenderObject &renderer) noexcept
    {
      const auto &firstLineStyle = renderer.FirstLineStyle();
      if (&renderer.Style() == &firstLineStyle)
      {
        return nullptr;
      }
      return RenderStyle::ClonePtr(firstLineStyle);
    }

    KRYS_NODISCARD layout::Box::AnonymousBox IsAnonymous(const RenderObject &renderer) noexcept
    {
      return renderer.IsAnonymous() ? layout::Box::AnonymousBox::Yes : layout::Box::AnonymousBox::No;
    }

    KRYS_NODISCARD layout::Box::ElementAttributes ElementAttributes(const RenderElement &renderer) noexcept
    {
      auto nodeType = [&]
      {
        if (Is<RenderListMarker>(renderer))
        {
          return layout::Box::NodeType::ListMarker;
        }

        // FIXME: Images (NodeType::Image), once there's RenderImage.
        if (Is<RenderReplaced>(renderer))
        {
          return layout::Box::NodeType::ReplacedElement;
        }

        // FIXME: Buttons, menu lists, text control inner containers, sliders and slider containers
        // (NodeType::ImplicitFlexBox), once there are form controls.

        if (auto *renderLineBreak = DynamicDowncast<RenderLineBreak>(renderer))
        {
          return renderLineBreak->IsWBR() ? layout::Box::NodeType::WordBreakOpportunity
                                          : layout::Box::NodeType::LineBreak;
        }

        if (Is<RenderTable>(renderer))
        {
          return layout::Box::NodeType::TableBox;
        }

        return layout::Box::NodeType::GenericElement;
      }();
      return {nodeType, IsAnonymous(renderer)};
    }
  }

  BoxTreeUpdater::BoxTreeUpdater(RenderBlock &rootRenderer) noexcept
      : _rootRenderer(rootRenderer), _document(ShareCheckedPtr(&rootRenderer.GetDocument()))
  {
  }

  BoxTreeUpdater::BoxTreeUpdater(RenderBlock &rootRenderer, const dom::Document &document) noexcept
      : _rootRenderer(rootRenderer), _document(ShareCheckedPtr(&document))
  {
  }

  BoxTreeUpdater::~BoxTreeUpdater() noexcept = default;

  CheckedRef<layout::ElementBox> BoxTreeUpdater::Build() noexcept
  {
    auto *rootBox = _rootRenderer.LayoutBox();
    if (!rootBox)
    {
      auto newRootBox = CreateLayoutBox(_rootRenderer);
      rootBox = Downcast<layout::ElementBox>(newRootBox.get());
      _rootRenderer.SetLayoutBox(*rootBox);
      GetInitialContainingBlock().AppendChild(krys::move(newRootBox));
    }

    if (Is<RenderBlockFlow>(_rootRenderer))
    {
      rootBox->SetIsInlineIntegrationRoot();
    }

    auto isAnonymousTextIndentCandidateForIntegration = [&]
    {
      if (!_rootRenderer.IsAnonymousBlock())
      {
        return false;
      }

      if (_rootRenderer.IsBlockBox())
      {
        return !_rootRenderer.Parent() || _rootRenderer.Parent()->FirstChild() == &_rootRenderer;
      }

      // E.g. flex and grid items are not block boxes (they are block containers though).
      return _rootRenderer.IsBlockContainer();
    };
    rootBox->SetIsAnonymousTextIndentCandidateForIntegration(isAnonymousTextIndentCandidateForIntegration());

    if (Is<RenderBlockFlow>(_rootRenderer))
    {
      BuildTreeForInlineContent();
    }
    else if (Is<RenderFlexibleBox>(_rootRenderer))
    {
      BuildTreeForFlexContent();
    }
    else if (Is<RenderGrid>(_rootRenderer))
    {
      BuildTreeForGridContent();
    }
    else
    {
      krys_debug_assert_msg(false, "Not implemented yet");
    }

    return ShareCheckedRef(RootLayoutBox());
  }

  void BoxTreeUpdater::TearDown() noexcept
  {
    if (_document->RenderTreeBeingDestroyed())
    {
      RootLayoutBox().DestroyChildren();
      return;
    }

    // FIXME: Detach the boxes of the inline blocks that have their own line layout (a RenderBlockFlow with an
    // inlineLayout()) and keep them under the initial containing block, once there's LineLayout.

    if (&RootLayoutBox().Parent() == &GetInitialContainingBlock())
    {
      (void)RootLayoutBox().RemoveFromParent();
    }
    else
    {
      RootLayoutBox().DestroyChildren();
    }
  }

  void BoxTreeUpdater::AdjustStyleIfNeeded(const RenderElement &renderer, RenderStyle &style,
                                           RenderStyle *firstLineStyle) noexcept
  {
    auto adjustStyle = [&](RenderStyle &styleToAdjust)
    {
      if (Is<RenderBlock>(renderer))
      {
        if (styleToAdjust.Display() == DisplayType::Inline)
        {
          styleToAdjust.SetDisplay(DisplayType::InlineBlock);
        }

        if (renderer.IsAnonymousBlock())
        {
          const auto &anonBlockParentStyle = renderer.Parent()->Style();
          // overflow and text-overflow property values don't get forwarded to anonymous block boxes.
          // e.g. <div style="overflow: hidden; text-overflow: ellipsis; width: 100px; white-space: pre;">this
          // text should have ellipsis<div></div></div>
          styleToAdjust.SetTextOverflow(anonBlockParentStyle.GetTextOverflow());
          styleToAdjust.SetOverflowX(anonBlockParentStyle.OverflowX());
          styleToAdjust.SetOverflowY(anonBlockParentStyle.OverflowY());
        }

        // FIXME: Text controls and MathML math boxes with an internal table display (which confuses IFC)
        // become blocks, once there are text controls and MathML rendering.
        return;
      }

      if (auto *renderInline = DynamicDowncast<RenderInline>(renderer))
      {
        auto isSupportedInlineDisplay = [&]
        {
          auto display = styleToAdjust.Display();
          if (display == DisplayType::RubyBase || display == DisplayType::RubyAnnotation)
          {
            return renderInline->Parent()->Style().Display() == DisplayType::Ruby;
          }
          // FIXME: Only inline for SVG inlines (RenderSVGInline), once there's SVG rendering.
          return styleToAdjust.IsDisplayInlineType();
        };

        if (!isSupportedInlineDisplay())
        {
          styleToAdjust.SetDisplay(DisplayType::Inline);
        }
        return;
      }

      if (auto *renderLineBreak = DynamicDowncast<RenderLineBreak>(renderer))
      {
        if (!styleToAdjust.HasOutOfFlowPosition())
        {
          // Force in-flow display value to inline (see webkit.org/b/223151).
          styleToAdjust.SetDisplay(DisplayType::Inline);
        }

        styleToAdjust.SetFloating(Float::None);
        // Clear property should only apply on block elements, however,
        // it appears that browsers seem to ignore it on <br> inline elements.
        // https://drafts.csswg.org/css2/#propdef-clear
        if (renderLineBreak->IsWBR())
        {
          styleToAdjust.SetClear(Clear::None);
        }
        return;
      }
    };

    adjustStyle(style);
    if (firstLineStyle)
    {
      adjustStyle(*firstLineStyle);
    }
  }

  UniquePtr<layout::Box> BoxTreeUpdater::CreateLayoutBox(RenderObject &renderer) noexcept
  {
    UniquePtr<RenderStyle> firstLineStyle = FirstLineStyleFor(renderer);

    if (auto *textRenderer = DynamicDowncast<RenderText>(renderer))
    {
      auto style = RenderStyle::CreateAnonymousStyleWithDisplay(textRenderer->Style(), DisplayType::Inline);

      // FIXME: Combined text (RenderCombineText) uses its original text, once there's text-combine-upright.
      auto isCombinedText = false;
      utf8_string text = textRenderer->Text();

      auto canUseSimpleFontCodePath = textRenderer->CanUseSimpleFontCodePath();
      auto canUseSimplifiedTextMeasuring = textRenderer->CanUseSimplifiedTextMeasuring();
      if (!canUseSimplifiedTextMeasuring)
      {
        canUseSimplifiedTextMeasuring =
          canUseSimpleFontCodePath
          && layout::TextUtil::CanUseSimplifiedTextMeasuring(
            text, style.GetFontCascade(), style.CollapseWhiteSpace(), firstLineStyle.get());
        textRenderer->SetCanUseSimplifiedTextMeasuring(*canUseSimplifiedTextMeasuring);
      }

      auto hasPositionDependentContentWidth = textRenderer->HasPositionDependentContentWidth();
      if (!hasPositionDependentContentWidth)
      {
        hasPositionDependentContentWidth = layout::TextUtil::HasPositionDependentContentWidth(text);
        textRenderer->SetHasPositionDependentContentWidth(*hasPositionDependentContentWidth);
      }

      auto hasStrongDirectionalityContent = textRenderer->HasStrongDirectionalityContent();
      if (!hasStrongDirectionalityContent)
      {
        hasStrongDirectionalityContent = layout::TextUtil::ContainsStrongDirectionalityText(text);
        textRenderer->SetHasStrongDirectionalityContent(*hasStrongDirectionalityContent);
      }

      using ContentCharacteristic = layout::InlineTextBox::ContentCharacteristic;
      auto contentCharacteristic = EnumSet<ContentCharacteristic> {};
      if (canUseSimpleFontCodePath)
      {
        contentCharacteristic.Add(ContentCharacteristic::CanUseSimpleFontCodepath);
      }
      if (textRenderer->ShouldUseSimpleGlyphOverflowCodePath())
      {
        contentCharacteristic.Add(ContentCharacteristic::ShouldUseSimpleGlyphOverflowCodePath);
      }
      if (*canUseSimplifiedTextMeasuring)
      {
        contentCharacteristic.Add(ContentCharacteristic::CanUseSimplifiedContentMeasuring);
      }
      if (*hasPositionDependentContentWidth)
      {
        contentCharacteristic.Add(ContentCharacteristic::HasPositionDependentContentWidth);
      }
      if (*hasStrongDirectionalityContent)
      {
        contentCharacteristic.Add(ContentCharacteristic::HasStrongDirectionalityContent);
      }
      return CreateUnique<layout::InlineTextBox>(krys::move(text), isCombinedText, contentCharacteristic,
                                                 krys::move(style), krys::move(firstLineStyle));
    }

    auto &renderElement = Downcast<RenderElement>(renderer);
    auto style = RenderStyle::Clone(renderElement.Style());
    AdjustStyleIfNeeded(renderElement, style, firstLineStyle.get());

    if (auto *listMarkerRenderer = DynamicDowncast<RenderListMarker>(renderElement))
    {
      EnumSet<layout::ElementBox::ListMarkerAttribute> listMarkerAttributes;
      if (listMarkerRenderer->IsImage())
      {
        listMarkerAttributes.Add(layout::ElementBox::ListMarkerAttribute::Image);
      }
      if (!listMarkerRenderer->IsInside())
      {
        listMarkerAttributes.Add(layout::ElementBox::ListMarkerAttribute::Outside);
      }
      return CreateUnique<layout::ElementBox>(ElementAttributes(renderElement), listMarkerAttributes,
                                              krys::move(style), krys::move(firstLineStyle));
    }

    return CreateUnique<layout::ElementBox>(ElementAttributes(renderElement), krys::move(style),
                                            krys::move(firstLineStyle));
  }

  void BoxTreeUpdater::BuildTreeForInlineContent() noexcept
  {
    for (auto walker = InlineWalker(Downcast<RenderBlockFlow>(_rootRenderer)); !walker.AtEnd();
         walker.Advance())
    {
      auto &childRenderer = *walker.Current();
      auto childLayoutBox = [&]
      {
        if (auto *existingChildBox = childRenderer.LayoutBox())
        {
          return existingChildBox->RemoveFromParent();
        }
        return CreateLayoutBox(childRenderer);
      };
      InsertChild(childLayoutBox(), childRenderer, childRenderer.PreviousSibling());
    }
  }

  void BoxTreeUpdater::BuildTreeForFlexContent() noexcept
  {
    for (auto &flexItemRenderer : ChildrenOfType<RenderElement>(_rootRenderer))
    {
      if (auto *existingChildBox = flexItemRenderer.LayoutBox())
      {
        InsertChild(existingChildBox->RemoveFromParent(), flexItemRenderer,
                    flexItemRenderer.PreviousSibling());
        continue;
      }

      auto style = RenderStyle::Clone(flexItemRenderer.Style());
      auto flexItemBox =
        CreateUnique<layout::ElementBox>(ElementAttributes(flexItemRenderer), krys::move(style));
      InsertChild(krys::move(flexItemBox), flexItemRenderer, flexItemRenderer.PreviousSibling());
    }
  }

  void BoxTreeUpdater::BuildTreeForGridContent() noexcept
  {
    for (auto &gridItemRenderer : ChildrenOfType<RenderElement>(_rootRenderer))
    {
      if (auto *existingChildBox = gridItemRenderer.LayoutBox())
      {
        InsertChild(existingChildBox->RemoveFromParent(), gridItemRenderer,
                    gridItemRenderer.PreviousSibling());
        continue;
      }

      auto style = RenderStyle::Clone(gridItemRenderer.Style());
      auto gridItemBox =
        CreateUnique<layout::ElementBox>(ElementAttributes(gridItemRenderer), krys::move(style));
      InsertChild(krys::move(gridItemBox), gridItemRenderer, gridItemRenderer.PreviousSibling());
    }
  }

  void BoxTreeUpdater::InsertChild(UniquePtr<layout::Box> childBox, RenderObject &childRenderer,
                                   const RenderObject *beforeChild) noexcept
  {
    auto &parentBox = *childRenderer.Parent()->LayoutBox();
    auto *beforeChildBox = beforeChild ? beforeChild->LayoutBox() : nullptr;
    childRenderer.SetLayoutBox(*childBox);
    parentBox.InsertChild(krys::move(childBox), const_cast<layout::Box *>(beforeChildBox));
  }

  namespace
  {
    void UpdateContentCharacteristic(const RenderText &rendererText,
                                     layout::InlineTextBox &inlineTextBox) noexcept
    {
      const auto &rendererStyle = rendererText.Style();
      auto shouldUpdateContentCharacteristic = !rendererStyle.FontCascadeEqual(inlineTextBox.Style());
      if (!shouldUpdateContentCharacteristic)
      {
        return;
      }

      using ContentCharacteristic = layout::InlineTextBox::ContentCharacteristic;
      auto contentCharacteristic = EnumSet<ContentCharacteristic> {};
      // These may only change when content changes.
      if (inlineTextBox.CanUseSimpleFontCodePath())
      {
        contentCharacteristic.Add(ContentCharacteristic::CanUseSimpleFontCodepath);
      }
      if (inlineTextBox.ShouldUseSimpleGlyphOverflowCodePath())
      {
        contentCharacteristic.Add(ContentCharacteristic::ShouldUseSimpleGlyphOverflowCodePath);
      }
      if (inlineTextBox.HasPositionDependentContentWidth())
      {
        contentCharacteristic.Add(ContentCharacteristic::HasPositionDependentContentWidth);
      }
      if (inlineTextBox.HasStrongDirectionalityContent())
      {
        contentCharacteristic.Add(ContentCharacteristic::HasStrongDirectionalityContent);
      }

      if (inlineTextBox.CanUseSimpleFontCodePath()
          && layout::TextUtil::CanUseSimplifiedTextMeasuring(
            inlineTextBox.Content(), rendererStyle.GetFontCascade(), rendererStyle.CollapseWhiteSpace(),
            &rendererText.FirstLineStyle()))
      {
        contentCharacteristic.Add(ContentCharacteristic::CanUseSimplifiedContentMeasuring);
      }

      inlineTextBox.SetContentCharacteristic(contentCharacteristic);
    }

    void UpdateListMarkerAttributes(const RenderListMarker &listMarkerRenderer,
                                    layout::ElementBox &layoutBox) noexcept
    {
      auto listMarkerAttributes = EnumSet<layout::ElementBox::ListMarkerAttribute> {};
      if (listMarkerRenderer.IsImage())
      {
        listMarkerAttributes.Add(layout::ElementBox::ListMarkerAttribute::Image);
      }
      if (!listMarkerRenderer.IsInside())
      {
        listMarkerAttributes.Add(layout::ElementBox::ListMarkerAttribute::Outside);
      }
      layoutBox.SetListMarkerAttributes(listMarkerAttributes);
    }
  }

  void BoxTreeUpdater::UpdateStyle(const RenderObject &renderer) noexcept
  {
    auto *layoutBox = const_cast<layout::Box *>(renderer.LayoutBox());
    if (!layoutBox)
    {
      krys_debug_assert(false);
      return;
    }

    if (auto *renderText = DynamicDowncast<RenderText>(renderer))
    {
      if (auto *inlineTextBox = DynamicDowncast<layout::InlineTextBox>(*layoutBox))
      {
        UpdateContentCharacteristic(*renderText, *inlineTextBox);
        inlineTextBox->UpdateStyle(
          RenderStyle::CreateAnonymousStyleWithDisplay(renderText->Style(), DisplayType::Inline),
          FirstLineStyleFor(*renderText));
        return;
      }
      krys_debug_assert(false);
      return;
    }

    auto firstLineNewStyle = FirstLineStyleFor(renderer);
    auto newStyle = RenderStyle::Clone(Downcast<RenderElement>(renderer).Style());
    AdjustStyleIfNeeded(Downcast<RenderElement>(renderer), newStyle, firstLineNewStyle.get());
    layoutBox->UpdateStyle(krys::move(newStyle), krys::move(firstLineNewStyle));

    if (auto *listMarkerRenderer = DynamicDowncast<RenderListMarker>(renderer);
        listMarkerRenderer && Is<layout::ElementBox>(*layoutBox))
    {
      UpdateListMarkerAttributes(*listMarkerRenderer, Downcast<layout::ElementBox>(*layoutBox));
    }
  }

  void BoxTreeUpdater::UpdateContent(const RenderText &textRenderer) noexcept
  {
    auto &inlineTextBox = const_cast<layout::InlineTextBox &>(*textRenderer.LayoutBox());
    const auto &style = inlineTextBox.Style();

    // FIXME: Combined text (RenderCombineText) uses its original text, once there's text-combine-upright.
    utf8_string text = textRenderer.Text();

    using ContentCharacteristic = layout::InlineTextBox::ContentCharacteristic;
    auto contentCharacteristic = EnumSet<ContentCharacteristic> {};
    if (textRenderer.CanUseSimpleFontCodePath())
    {
      contentCharacteristic.Add(ContentCharacteristic::CanUseSimpleFontCodepath);
    }
    if (textRenderer.CanUseSimpleFontCodePath()
        && layout::TextUtil::CanUseSimplifiedTextMeasuring(
          text, style.GetFontCascade(), style.CollapseWhiteSpace(), &inlineTextBox.FirstLineStyle()))
    {
      contentCharacteristic.Add(ContentCharacteristic::CanUseSimplifiedContentMeasuring);
    }
    if (textRenderer.ShouldUseSimpleGlyphOverflowCodePath())
    {
      contentCharacteristic.Add(ContentCharacteristic::ShouldUseSimpleGlyphOverflowCodePath);
    }

    auto hasPositionDependentContentWidth = textRenderer.HasPositionDependentContentWidth();
    if (!hasPositionDependentContentWidth)
    {
      hasPositionDependentContentWidth = layout::TextUtil::HasPositionDependentContentWidth(text);
      const_cast<RenderText &>(textRenderer)
        .SetHasPositionDependentContentWidth(*hasPositionDependentContentWidth);
    }
    if (*hasPositionDependentContentWidth)
    {
      contentCharacteristic.Add(ContentCharacteristic::HasPositionDependentContentWidth);
    }

    auto hasStrongDirectionalityContent = textRenderer.HasStrongDirectionalityContent();
    if (!hasStrongDirectionalityContent)
    {
      hasStrongDirectionalityContent = layout::TextUtil::ContainsStrongDirectionalityText(text);
      const_cast<RenderText &>(textRenderer)
        .SetHasStrongDirectionalityContent(*hasStrongDirectionalityContent);
    }
    if (*hasStrongDirectionalityContent)
    {
      contentCharacteristic.Add(ContentCharacteristic::HasStrongDirectionalityContent);
    }

    inlineTextBox.SetContent(krys::move(text), contentCharacteristic);
  }

  const layout::Box &BoxTreeUpdater::Insert(const RenderElement &, RenderObject &child,
                                            const RenderObject *beforeChild) noexcept
  {
    InsertChild(CreateLayoutBox(child), child, beforeChild);
    return *child.LayoutBox();
  }

  UniquePtr<layout::Box> BoxTreeUpdater::Remove(const RenderElement &, RenderObject &child) noexcept
  {
    krys_debug_assert(child.LayoutBox());
    auto *layoutBox = child.LayoutBox();
    child.ClearLayoutBox();
    return layoutBox->RemoveFromParent();
  }

  const layout::ElementBox &BoxTreeUpdater::RootLayoutBox() const noexcept
  {
    return *_rootRenderer.LayoutBox();
  }

  layout::ElementBox &BoxTreeUpdater::RootLayoutBox() noexcept
  {
    return *_rootRenderer.LayoutBox();
  }

  layout::InitialContainingBlock &BoxTreeUpdater::GetInitialContainingBlock() noexcept
  {
    return _rootRenderer.View().GetInitialContainingBlock();
  }
}
