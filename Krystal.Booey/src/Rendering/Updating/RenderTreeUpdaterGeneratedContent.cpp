#include "Krystal.Booey/Rendering/Updating/RenderTreeUpdaterGeneratedContent.hpp"
#include "Krystal.Booey/Animation/KeyframeEffectStack.hpp"
#include "Krystal.Booey/DOM/PseudoElement.hpp"
#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Booey/Rendering/RenderCounter.hpp"
#include "Krystal.Booey/Rendering/RenderDescendantIterator.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/RenderImage.hpp"
#include "Krystal.Booey/Rendering/RenderPtr.hpp"
#include "Krystal.Booey/Rendering/RenderQuote.hpp"
#include "Krystal.Booey/Rendering/RenderTextFragment.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+SettersInlines.hpp"
#include "Krystal.Booey/Rendering/Updating/RenderTreeBuilder.hpp"
#include "Krystal.Booey/Style/Change.hpp"
#include "Krystal.Booey/Style/Styleable.hpp"
#include "Krystal.Booey/Style/Update.hpp"

namespace krys::boo
{
  RenderTreeUpdater::GeneratedContent::GeneratedContent(RenderTreeUpdater &updater) noexcept
      : _updater(updater)
  {
  }

  void RenderTreeUpdater::GeneratedContent::UpdateRemainingQuotes() noexcept
  {
    if (!_updater.GetRenderView().HasQuotesNeedingUpdate())
    {
      return;
    }
    UpdateQuotesUpTo(nullptr);
    _previousUpdatedQuote = nullptr;
    _updater.GetRenderView().SetHasQuotesNeedingUpdate(false);
  }

  void RenderTreeUpdater::GeneratedContent::UpdateQuotesUpTo(RenderQuote *lastQuote) noexcept
  {
    auto quoteRenderers = DescendantsOfType<RenderQuote>(_updater.GetRenderView());
    auto it = _previousUpdatedQuote ? ++quoteRenderers.at(*_previousUpdatedQuote) : quoteRenderers.begin();
    auto end = quoteRenderers.end();
    for (; it != end; ++it)
    {
      auto &quote = *it;
      // Quote character depends on quote depth so we chain the updates.
      quote.UpdateRenderer(*_updater._builder, _previousUpdatedQuote.get());
      _previousUpdatedQuote = CreateWeakPtr(&quote);
      if (&quote == lastQuote)
      {
        return;
      }
    }
    krys_debug_assert(!lastQuote);
  }

  void RenderTreeUpdater::GeneratedContent::UpdateCounters() noexcept
  {
    auto update = [&]
    {
      auto counters = _updater.GetRenderView().TakeCountersNeedingUpdate();
      for (auto &[address, counter] : counters)
      {
        if (counter)
        {
          counter->UpdateCounter();
        }
      }
    };
    // Update twice and hope it stabilizes.
    update();
    update();
  }

  static RenderPtr<RenderObject> CreateContentRenderer(const style::Content::Text &value,
                                                       const utf8_string &altText, dom::Document &document,
                                                       const RenderStyle &) noexcept
  {
    if (value.Value.empty() && altText.empty())
    {
      return {};
    }

    auto contentRenderer = CreateRenderer<RenderTextFragment>(document, value.Value);
    contentRenderer->SetAltText(altText);
    return contentRenderer;
  }

  static RenderPtr<RenderObject> CreateContentRenderer(const style::Content::Image &value,
                                                       const utf8_string &altText, dom::Document &document,
                                                       const RenderStyle &pseudoStyle) noexcept
  {
    auto contentRenderer = CreateRenderer<RenderImage>(
      RenderObject::Type::Image, document, RenderStyle::CreateStyleInheritingFromPseudoStyle(pseudoStyle),
      value.Value.Value.get());
    contentRenderer->InitializeStyle();
    contentRenderer->SetAltText(altText);
    return contentRenderer;
  }

  static RenderPtr<RenderObject> CreateContentRenderer(const style::Content::Counter &value,
                                                       const utf8_string &, dom::Document &document,
                                                       const RenderStyle &) noexcept
  {
    return CreateRenderer<RenderCounter>(document, value);
  }

  static RenderPtr<RenderObject> CreateContentRenderer(const style::Content::Quote &value,
                                                       const utf8_string &, dom::Document &document,
                                                       const RenderStyle &pseudoStyle) noexcept
  {
    auto contentRenderer = CreateRenderer<RenderQuote>(
      document, RenderStyle::CreateStyleInheritingFromPseudoStyle(pseudoStyle), value.Value);
    contentRenderer->InitializeStyle();
    return contentRenderer;
  }

  static KeyframeEffectStack *
    KeyframeEffectStackForPseudoElement(const dom::Element &element,
                                        PseudoElementType pseudoElementType) noexcept
  {
    if (!element.MayHaveKeyframeEffects())
    {
      return nullptr;
    }

    return element.GetKeyframeEffectStack(style::PseudoElementIdentifier {pseudoElementType});
  }

  static bool NeedsPseudoElementForAnimation(const dom::Element &element,
                                             PseudoElementType pseudoElementType) noexcept
  {
    auto *stack = KeyframeEffectStackForPseudoElement(element, pseudoElementType);
    if (!stack)
    {
      return false;
    }

    return stack->RequiresPseudoElement() || stack->ContainsProperty(css::PropertyId::Display);
  }

  static void CreateContentRenderers(RenderTreeBuilder &builder, RenderElement &pseudoRenderer,
                                     const RenderStyle &style) noexcept
  {
    if (auto *contentData = style.GetContent().TryData())
    {
      auto altText = contentData->AltText.value_or(utf8_string {});
      for (auto &contentItem : contentData->List)
      {
        krys::SwitchOn(contentItem,
                       [&](const auto &item)
                       {
                         if (auto child =
                               CreateContentRenderer(item, altText, pseudoRenderer.GetDocument(), style);
                             child && pseudoRenderer.IsChildAllowed(*child, style))
                         {
                           builder.Attach(pseudoRenderer, krys::move(child));
                         }
                       });
      }
    }
    // FIXME: Assert that without content, the pseudo-element is only here for animations that require it,
    // once scripts can create animations.
  }

  static void UpdateStyleForContentRenderers(RenderElement &pseudoRenderer, const RenderStyle &style) noexcept
  {
    for (auto &contentRenderer : DescendantsOfType<RenderElement>(pseudoRenderer))
    {
      // We only manage the style for the generated content which must be images or text.
      if (!Is<RenderImage>(contentRenderer) && !Is<RenderQuote>(contentRenderer))
      {
        continue;
      }
      contentRenderer.SetStyle(RenderStyle::CreateStyleInheritingFromPseudoStyle(style));
    }
  }

  void RenderTreeUpdater::GeneratedContent::UpdateBeforeOrAfterPseudoElement(
    dom::Element &current, const style::ElementUpdate &elementUpdate,
    PseudoElementType pseudoElementType) noexcept
  {
    krys_debug_assert(pseudoElementType == PseudoElementType::Before
                      || pseudoElementType == PseudoElementType::After);

    auto *pseudoElement = pseudoElementType == PseudoElementType::Before ? current.BeforePseudoElement()
                                                                         : current.AfterPseudoElement();

    if (auto *renderer = pseudoElement ? pseudoElement->Renderer() : nullptr)
    {
      _updater.GetRenderTreePosition().InvalidateNextSibling(*renderer);
    }

    auto *updateStyle =
      elementUpdate.Style ? elementUpdate.Style->GetCachedPseudoStyle({pseudoElementType}) : nullptr;

    // If we end up losing a previous pseudo because the style got removed we need to cancel any animations
    // that were on it so we do not end up thinking we need to keep the renderer around. We cannot completely
    // rely on NeedsPseudoElement because we may need to animate a box with display: none.
    if (!updateStyle)
    {
      style::Styleable {current, style::PseudoElementIdentifier {pseudoElementType}}
        .CancelStyleOriginatedAnimations();
    }

    krys_debug_assert(!Is<dom::PseudoElement>(current));
    if (!NeedsPseudoElement(updateStyle) && !NeedsPseudoElementForAnimation(current, pseudoElementType))
    {
      if (pseudoElement)
      {
        if (pseudoElementType == PseudoElementType::Before)
        {
          RemoveBeforePseudoElement(current, *_updater._builder);
        }
        else
        {
          RemoveAfterPseudoElement(current, *_updater._builder);
        }
      }
      return;
    }

    if (!updateStyle)
    {
      return;
    }

    auto *existingStyle = pseudoElement ? pseudoElement->RenderOrDisplayContentsStyle() : nullptr;

    auto styleChanges = existingStyle ? style::DetermineChanges(*updateStyle, *existingStyle)
                                      : EnumSet<style::Change> {style::Change::Renderer};
    if (styleChanges.IsEmpty())
    {
      return;
    }

    pseudoElement = &current.EnsurePseudoElement(pseudoElementType);

    if (updateStyle->Display() == DisplayType::Contents)
    {
      // For display:contents we create an inline wrapper that inherits its
      // style from the display:contents style.
      auto contentsStyle = RenderStyle::CreatePtr();
      contentsStyle->SetPseudoElementIdentifier(style::PseudoElementIdentifier {pseudoElementType});
      contentsStyle->InheritFrom(*updateStyle);
      contentsStyle->CopyContentFrom(*updateStyle);
      contentsStyle->CopyPseudoElementsFrom(*updateStyle);

      style::ElementUpdate contentsUpdate {krys::move(contentsStyle), styleChanges,
                                           elementUpdate.RecompositeLayer};
      _updater.UpdateElementRenderer(*pseudoElement, contentsUpdate);
      auto pseudoElementUpdateStyle = RenderStyle::CloneIncludingPseudoElements(*updateStyle);
      pseudoElement->StoreDisplayContentsOrNoneStyle(
        CreateUnique<RenderStyle>(krys::move(pseudoElementUpdateStyle)));
    }
    else
    {
      auto pseudoElementUpdateStyle = RenderStyle::CloneIncludingPseudoElements(*updateStyle);
      style::ElementUpdate pseudoElementUpdate {
        CreateUnique<RenderStyle>(krys::move(pseudoElementUpdateStyle)), styleChanges,
        elementUpdate.RecompositeLayer};
      _updater.UpdateElementRenderer(*pseudoElement, pseudoElementUpdate);
      if (updateStyle->Display() == DisplayType::None)
      {
        auto displayNoneStyle = RenderStyle::CloneIncludingPseudoElements(*updateStyle);
        pseudoElement->StoreDisplayContentsOrNoneStyle(
          CreateUnique<RenderStyle>(krys::move(displayNoneStyle)));
      }
      else
      {
        pseudoElement->ClearDisplayContentsOrNoneStyle();
      }
    }

    auto *pseudoElementRenderer = pseudoElement->Renderer();
    if (!pseudoElementRenderer)
    {
      return;
    }

    if (styleChanges.Contains(style::Change::Renderer))
    {
      CreateContentRenderers(*_updater._builder, *pseudoElementRenderer, *updateStyle);
    }
    else
    {
      UpdateStyleForContentRenderers(*pseudoElementRenderer, *updateStyle);
    }

    if (_updater.GetRenderView().HasQuotesNeedingUpdate())
    {
      for (auto &child : DescendantsOfType<RenderQuote>(*pseudoElementRenderer))
      {
        UpdateQuotesUpTo(&child);
      }
    }
    _updater._builder->UpdateAfterDescendants(*pseudoElementRenderer);
  }

  void RenderTreeUpdater::GeneratedContent::UpdateBackdropRenderer(
    RenderElement &renderer, style::DifferenceResult minimalStyleDifference) noexcept
  {
    auto destroyBackdropIfNeeded = [&renderer, this]
    {
      if (auto *backdropRenderer = renderer.BackdropRenderer())
      {
        _updater._builder->Destroy(*backdropRenderer);
      }
    };

    // Intentionally bail out early here to avoid computing the style.
    if (!renderer.GetElement() || !renderer.GetElement()->IsInTopLayer())
    {
      destroyBackdropIfNeeded();
      return;
    }

    const auto *style = renderer.GetCachedPseudoStyle({PseudoElementType::Backdrop}, &renderer.Style());
    if (!style || style->Display() == DisplayType::None)
    {
      destroyBackdropIfNeeded();
      return;
    }

    auto newStyle = RenderStyle::Clone(*style);
    if (auto *backdropRenderer = renderer.BackdropRenderer())
    {
      backdropRenderer->SetStyle(krys::move(newStyle), minimalStyleDifference);
    }
    else
    {
      auto newBackdropRenderer = boo::CreateRenderer<RenderBlockFlow>(
        RenderObject::Type::BlockFlow, renderer.GetDocument(), krys::move(newStyle));
      newBackdropRenderer->InitializeStyle();
      renderer.SetBackdropRenderer(*newBackdropRenderer.get());
      _updater._builder->Attach(renderer.View(), krys::move(newBackdropRenderer));
    }
  }

  bool RenderTreeUpdater::GeneratedContent::NeedsPseudoElement(const RenderStyle *style) noexcept
  {
    if (!style)
    {
      return false;
    }
    if (!_updater.GetRenderTreePosition().Parent().CanHaveGeneratedChildren())
    {
      return false;
    }
    if (!PseudoElementRendererIsNeeded(style))
    {
      return false;
    }
    return true;
  }

  void RenderTreeUpdater::GeneratedContent::RemoveBeforePseudoElement(dom::Element &element,
                                                                      RenderTreeBuilder &builder) noexcept
  {
    auto *pseudoElement = element.BeforePseudoElement();
    if (!pseudoElement)
    {
      return;
    }
    TearDownRenderers(*pseudoElement, TeardownType::Full, builder);
    element.ClearBeforePseudoElement();
  }

  void RenderTreeUpdater::GeneratedContent::RemoveAfterPseudoElement(dom::Element &element,
                                                                     RenderTreeBuilder &builder) noexcept
  {
    auto *pseudoElement = element.AfterPseudoElement();
    if (!pseudoElement)
    {
      return;
    }
    TearDownRenderers(*pseudoElement, TeardownType::Full, builder);
    element.ClearAfterPseudoElement();
  }
}
