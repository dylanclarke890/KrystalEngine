#include "Krystal.Booey/Rendering/Updating/RenderTreeBuilderFirstLetter.hpp"
#include "Krystal.Booey/Core/TextBreakIterator.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/Graphics/FontMetrics.hpp"
#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Booey/Rendering/RenderInline.hpp"
#include "Krystal.Booey/Rendering/RenderTextFragment.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+SettersInlines.hpp"
#include "Krystal.Booey/Rendering/SVG/RenderSVGText.hpp"
#include "Krystal.Booey/Style/Change.hpp"
#include "Krystal.Booey/Style/PseudoElementIdentifier.hpp"
#include "Krystal.Core/Text/CodePointIterator.hpp"
#include "Krystal.Core/Text/Unicode.hpp"
#include <unicode/uchar.h>

namespace krys::boo
{
  namespace
  {
    KRYS_NODISCARD Maybe<RenderStyle> StyleForFirstLetter(const RenderElement &firstLetterContainer) noexcept
    {
      const auto &styleContainer = firstLetterContainer.IsAnonymous()
                                     ? *firstLetterContainer.FirstNonAnonymousAncestor()
                                     : firstLetterContainer;
      auto *style = styleContainer.Style().GetCachedPseudoStyle({PseudoElementType::FirstLetter});
      if (!style)
      {
        return null;
      }

      auto firstLetterStyle = RenderStyle::Clone(*style);

      // If we have an initial letter drop that is >= 1, then we need to force floating to be on.
      if (firstLetterStyle.InitialLetter().Drop() >= 1 && !firstLetterStyle.IsFloating())
      {
        firstLetterStyle.SetFloating(firstLetterStyle.GetWritingMode().IsBidiLTR() ? Float::Left
                                                                                   : Float::Right);
      }

      // We have to compute the correct font-size for the first-letter if it has an initial letter height set.
      const auto *paragraph = firstLetterContainer.IsRenderBlockFlow()
                                ? &firstLetterContainer
                                : static_cast<const RenderElement *>(firstLetterContainer.ContainingBlock());
      if (firstLetterStyle.InitialLetter().Height() >= 1
          && firstLetterStyle.MetricsOfPrimaryFont().CapHeight()
          && paragraph->Style().MetricsOfPrimaryFont().CapHeight())
      {
        // FIXME: For ideographic baselines, we want to go from line edge to line edge. This is equivalent to
        // (N-1)*line-height + the font height. We don't yet support ideographic baselines.
        // For an N-line first-letter and for alphabetic baselines, the cap-height of the first letter needs
        // to equal (N-1)*line-height of paragraph lines + cap-height of the paragraph. Mathematically we
        // can't rely on font-size, since font().height() doesn't necessarily match. For reliability, the best
        // approach is simply to compare the final measured cap-heights of the two fonts in order to get to
        // the closest possible value.
        firstLetterStyle.SetLineBoxContain({style::WebkitLineBoxContainValue::InitialLetter});
        auto lineHeight = static_cast<int32>(paragraph->Style().ComputedLineHeight());

        // Set the font to be one line too big and then ratchet back to get to a precise fit. We can't just
        // set the desired font size based off font height metrics because many fonts bake ascent into the
        // font metrics. Therefore we have to look at actual measured cap height values in order to know when
        // we have a good fit.
        auto newFontDescription = firstLetterStyle.GetFontDescription();
        float capRatio =
          *firstLetterStyle.MetricsOfPrimaryFont().CapHeight() / firstLetterStyle.ComputedFontSize();
        float startingFontSize =
          ((firstLetterStyle.InitialLetter().Height() - 1) * static_cast<float>(lineHeight)
           + static_cast<float>(paragraph->Style().MetricsOfPrimaryFont().IntCapHeight()))
          / capRatio;
        newFontDescription.SetSpecifiedSize(startingFontSize);
        newFontDescription.SetComputedSize(startingFontSize);
        firstLetterStyle.SetFontDescription(krys::move(newFontDescription));

        auto desiredCapHeight =
          static_cast<int32>((firstLetterStyle.InitialLetter().Height() - 1) * static_cast<float>(lineHeight))
          + paragraph->Style().MetricsOfPrimaryFont().IntCapHeight();
        auto actualCapHeight = firstLetterStyle.MetricsOfPrimaryFont().IntCapHeight();
        while (actualCapHeight > desiredCapHeight)
        {
          auto smallerFontDescription = firstLetterStyle.GetFontDescription();
          smallerFontDescription.SetSpecifiedSize(smallerFontDescription.SpecifiedSize() - 1);
          smallerFontDescription.SetComputedSize(smallerFontDescription.ComputedSize() - 1);
          firstLetterStyle.SetFontDescription(krys::move(smallerFontDescription));
          actualCapHeight = firstLetterStyle.MetricsOfPrimaryFont().IntCapHeight();
        }
      }

      firstLetterStyle.SetPseudoElementIdentifier(
        style::PseudoElementIdentifier {PseudoElementType::FirstLetter});
      // Force inline display (except for floating first-letters).
      firstLetterStyle.SetDisplay(firstLetterStyle.IsFloating() ? DisplayType::Block : DisplayType::Inline);
      // CSS2 says first-letter can't be positioned.
      firstLetterStyle.SetPosition(PositionType::Static);

      return firstLetterStyle;
    }

    // CSS 2.1 http://www.w3.org/TR/CSS21/selector.html#first-letter
    // "Punctuation (i.e, characters defined in Unicode [UNICODE] in the "open" (Ps), "close" (Pe),
    // "initial" (Pi). "final" (Pf) and "other" (Po) punctuation classes), that precedes or follows the first
    // letter should be included"
    KRYS_NODISCARD bool IsPunctuationForFirstLetter(char32 c) noexcept
    {
      return U_GET_GC_MASK(static_cast<UChar32>(c))
             & (U_GC_PS_MASK | U_GC_PE_MASK | U_GC_PI_MASK | U_GC_PF_MASK | U_GC_PO_MASK);
    }

    KRYS_NODISCARD bool ShouldSkipForFirstLetter(char32 c) noexcept
    {
      return c == ' ' || c == '\n' || c == '\t' || c == '\r' || c == '\f' || c == text::NoBreakSpace
             || IsPunctuationForFirstLetter(c);
    }

    KRYS_NODISCARD char32 CharacterStartingAt(utf8_stringview text, size_t offset) noexcept
    {
      return *text::CodePointIterator(text.substr(offset));
    }

    KRYS_NODISCARD bool SupportsFirstLetter(RenderBlock &block) noexcept
    {
      if (block.IsRenderButton())
      {
        return true;
      }
      if (!Is<RenderBlockFlow>(block))
      {
        return false;
      }
      if (Is<RenderSVGText>(block))
      {
        return false;
      }
      return block.CanHaveGeneratedChildren();
    }
  }

  RenderTreeBuilder::FirstLetter::FirstLetter(RenderTreeBuilder &builder) noexcept : _builder(builder)
  {
  }

  void RenderTreeBuilder::FirstLetter::UpdateAfterDescendants(RenderBlock &block) noexcept
  {
    if (!block.Style().HasPseudoStyle(PseudoElementType::FirstLetter))
    {
      return;
    }

    if (!SupportsFirstLetter(block))
    {
      return;
    }

    // FIXME: This should be refactored, firstLetterContainer is not needed.
    auto [firstLetter, firstLetterContainer] = block.FirstLetterAndContainer();
    if (!firstLetter)
    {
      return;
    }

    // Other containers are handled when updating their renderers.
    if (&block != firstLetterContainer)
    {
      return;
    }

    // If the child already has style, then it has already been created, so we just want
    // to update it.
    if (firstLetter->Parent()->Style().GetPseudoElementType() == PseudoElementType::FirstLetter)
    {
      UpdateStyle(block, *firstLetter);
      return;
    }

    if (!Is<RenderText>(firstLetter))
    {
      return;
    }

    CreateRenderers(Downcast<RenderText>(*firstLetter));
  }

  void RenderTreeBuilder::FirstLetter::CleanupOnDestroy(RenderTextFragment &textFragment) noexcept
  {
    if (!textFragment.FirstLetter())
    {
      return;
    }
    _builder.Destroy(*textFragment.FirstLetter(), CanCollapseAnonymousBlock::No);
  }

  void RenderTreeBuilder::FirstLetter::UpdateStyle(RenderBlock &firstLetterBlock,
                                                   RenderObject &currentChild) noexcept
  {
    RenderElement *firstLetter = currentChild.Parent();
    krys_debug_assert(firstLetter->IsFirstLetter());
    krys_debug_assert(firstLetter->IsFloating() || firstLetter->IsInline());
    if (!firstLetter || !firstLetter->Parent())
    {
      return;
    }

    auto &firstLetterContainer = *firstLetter->Parent();
    auto pseudoStyle = StyleForFirstLetter(firstLetterContainer);
    if (!pseudoStyle)
    {
      krys_debug_assert(false);
      return;
    }

    if (!style::DetermineChanges(firstLetter->Style(), *pseudoStyle).Contains(style::Change::Renderer))
    {
      firstLetter->SetStyle(krys::move(*pseudoStyle));
      return;
    }

    // The first-letter renderer needs to be replaced. Create a new renderer of the right type.
    RenderPtr<RenderBoxModelObject> newFirstLetter;
    if (pseudoStyle->Display() == DisplayType::Inline)
    {
      newFirstLetter = CreateRenderer<RenderInline>(RenderObject::Type::Inline,
                                                    firstLetterBlock.GetDocument(), krys::move(*pseudoStyle));
    }
    else
    {
      newFirstLetter = CreateRenderer<RenderBlockFlow>(
        RenderObject::Type::BlockFlow, firstLetterBlock.GetDocument(), krys::move(*pseudoStyle));
    }
    newFirstLetter->InitializeStyle();
    newFirstLetter->SetIsFirstLetter();

    // Move the first letter into the new renderer.
    while (RenderObject *child = firstLetter->FirstChild())
    {
      auto toMove = _builder.Detach(*firstLetter, *child, WillBeDestroyed::No);
      _builder.Attach(*newFirstLetter, krys::move(toMove));
    }

    auto remainingText =
      CreateWeakPtr(Downcast<RenderBoxModelObject>(*firstLetter).FirstLetterRemainingText());
    krys_debug_assert(!remainingText || remainingText->IsAnonymous()
                      || remainingText->TextNode()->Renderer() == remainingText.get());
    auto nextSibling = CreateWeakPtr(firstLetter->NextSibling());
    _builder.Destroy(*firstLetter);
    if (remainingText)
    {
      // Replace the old renderer with the new one.
      remainingText->SetFirstLetter(*newFirstLetter);
      newFirstLetter->SetFirstLetterRemainingText(*remainingText);
    }
    _builder.Attach(firstLetterContainer, krys::move(newFirstLetter), nextSibling.get());
  }

  void RenderTreeBuilder::FirstLetter::CreateRenderers(RenderText &currentTextChild) noexcept
  {
    RenderElement *textContentParent = currentTextChild.Parent();
    RenderElement *firstLetterContainer = nullptr;
    if (auto *wrapperInlineForDisplayContents = currentTextChild.InlineWrapperForDisplayContents())
    {
      firstLetterContainer = wrapperInlineForDisplayContents->Parent();
    }
    else
    {
      firstLetterContainer = textContentParent;
    }
    if (!firstLetterContainer)
    {
      return;
    }

    auto pseudoStyle = StyleForFirstLetter(*firstLetterContainer);
    if (!pseudoStyle)
    {
      return;
    }

    RenderPtr<RenderBoxModelObject> newFirstLetter;
    if (pseudoStyle->Display() == DisplayType::Inline)
    {
      newFirstLetter = CreateRenderer<RenderInline>(RenderObject::Type::Inline,
                                                    currentTextChild.GetDocument(), krys::move(*pseudoStyle));
    }
    else
    {
      newFirstLetter = CreateRenderer<RenderBlockFlow>(
        RenderObject::Type::BlockFlow, currentTextChild.GetDocument(), krys::move(*pseudoStyle));
    }
    newFirstLetter->InitializeStyle();
    newFirstLetter->SetIsFirstLetter();

    // The original string is going to be either a generated content string or a DOM node's
    // string. We want the original string before it got transformed in case first-letter has
    // no text-transform or a different text-transform applied to it.
    auto oldText = currentTextChild.OriginalText();

    if (!oldText.empty())
    {
      size_t length = 0;

      // Account for leading spaces and punctuation.
      while (length < oldText.size() && ShouldSkipForFirstLetter(CharacterStartingAt(oldText, length)))
      {
        length += NumCodeUnitsInGraphemeClusters(utf8_stringview(oldText).substr(length), 1);
      }

      // Account for first grapheme cluster.
      length += NumCodeUnitsInGraphemeClusters(utf8_stringview(oldText).substr(length), 1);

      // Keep looking for whitespace and allowed punctuation, but avoid
      // accumulating just whitespace into the :first-letter.
      size_t numCodeUnits = 0;
      for (size_t scanLength = length; scanLength < oldText.size(); scanLength += numCodeUnits)
      {
        char32 c = CharacterStartingAt(oldText, scanLength);

        if (!ShouldSkipForFirstLetter(c))
        {
          break;
        }

        numCodeUnits = NumCodeUnitsInGraphemeClusters(utf8_stringview(oldText).substr(scanLength), 1);

        if (IsPunctuationForFirstLetter(c))
        {
          length = scanLength + numCodeUnits;
        }
      }

      auto *textNode = currentTextChild.TextNode();
      auto beforeChild = CreateWeakPtr(currentTextChild.NextSibling());
      auto inlineWrapperForDisplayContents =
        CreateWeakPtr(currentTextChild.InlineWrapperForDisplayContents());
      auto *hasInlineWrapperForDisplayContents = inlineWrapperForDisplayContents.get();
      _builder.Destroy(currentTextChild);

      // Construct a text fragment for the text after the first letter.
      // This text fragment might be empty.
      RenderPtr<RenderTextFragment> newRemainingText;
      if (textNode)
      {
        newRemainingText =
          CreateRenderer<RenderTextFragment>(*textNode, oldText, length, oldText.size() - length);
        textNode->SetRenderer(newRemainingText.get());
      }
      else
      {
        newRemainingText = CreateRenderer<RenderTextFragment>(_builder._view.GetDocument(), oldText, length,
                                                              oldText.size() - length);
      }

      RenderTextFragment &remainingText = *newRemainingText;
      krys_debug_assert(hasInlineWrapperForDisplayContents == inlineWrapperForDisplayContents.get());
      (void)hasInlineWrapperForDisplayContents;
      remainingText.SetInlineWrapperForDisplayContents(inlineWrapperForDisplayContents.get());
      _builder.Attach(*textContentParent, krys::move(newRemainingText), beforeChild.get());

      // FIXME: Make attach the final step so that we don't need to keep firstLetter around.
      auto &firstLetter = *newFirstLetter;
      remainingText.SetFirstLetter(firstLetter);
      firstLetter.SetFirstLetterRemainingText(remainingText);
      _builder.Attach(*firstLetterContainer, krys::move(newFirstLetter), &remainingText);

      // Construct text fragment for the first letter.
      auto letter = CreateRenderer<RenderTextFragment>(_builder._view.GetDocument(), oldText, 0, length);
      _builder.Attach(firstLetter, krys::move(letter));
    }
  }
}
