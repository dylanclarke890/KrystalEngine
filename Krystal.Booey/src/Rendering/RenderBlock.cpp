#include "Krystal.Booey/Rendering/RenderBlock.hpp"
#include "Krystal.Booey/Rendering/RenderListMarker.hpp"
#include "Krystal.Booey/Rendering/RenderText.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo
{
  RenderBlock::RenderBlock(Type type, dom::Element &element, RenderStyle &&style,
                           EnumSet<TypeFlag> baseTypeFlags, TypeSpecificFlags typeSpecificFlags) noexcept
      : RenderBox(type, element, krys::move(style), baseTypeFlags | TypeFlag::IsRenderBlock,
                  typeSpecificFlags)
  {
    krys_debug_assert(IsRenderBlock());
  }

  RenderBlock::RenderBlock(Type type, dom::Document &document, RenderStyle &&style,
                           EnumSet<TypeFlag> baseTypeFlags, TypeSpecificFlags typeSpecificFlags) noexcept
      : RenderBox(type, document, krys::move(style), baseTypeFlags | TypeFlag::IsRenderBlock,
                  typeSpecificFlags)
  {
    krys_debug_assert(IsRenderBlock());
  }

  // Do not add any code in below destructor. Add it to willBeDestroyed() instead.
  RenderBlock::~RenderBlock() noexcept = default;

  utf8_stringview RenderBlock::RenderName() const noexcept
  {
    if (IsBody())
    {
      return u8"RenderBody"; // FIXME: Temporary hack until we know that the regression tests pass.
    }
    if (IsFieldset())
    {
      return u8"RenderFieldSet"; // FIXME: Remove eventually, but done to keep tests from breaking.
    }
    if (IsFloating())
    {
      return u8"RenderBlock (floating)";
    }
    if (IsOutOfFlowPositioned())
    {
      return u8"RenderBlock (positioned)";
    }
    if (IsAnonymousBlock())
    {
      return u8"RenderBlock (anonymous)";
    }
    // FIXME: Temporary hack while the new generated content system is being implemented.
    if (IsPseudoElement())
    {
      return u8"RenderBlock (generated)";
    }
    if (IsAnonymous())
    {
      return u8"RenderBlock (generated)";
    }
    if (IsRelativelyPositioned())
    {
      return u8"RenderBlock (relative positioned)";
    }
    if (IsStickilyPositioned())
    {
      return u8"RenderBlock (sticky positioned)";
    }
    return u8"RenderBlock";
  }

  LayoutUnit RenderBlock::PaddingBoxWidth() const noexcept
  {
    krys_unreachable();
  }

  LayoutUnit RenderBlock::PaddingBoxHeight() const noexcept
  {
    krys_unreachable();
  }

  namespace
  {
    KRYS_NODISCARD bool IsRenderBlockFlowOrRenderButton(RenderElement &renderElement) noexcept
    {
      // We include isRenderButton in this check because buttons are implemented
      // using flex box but should still support first-line|first-letter.
      // The flex box and specs require that flex box and grid do not support
      // first-line|first-letter, though.
      // FIXME: Remove when buttons are implemented with align-items instead of
      // flex box.
      return renderElement.IsRenderBlockFlow() || renderElement.IsRenderButton();
    }

    KRYS_NODISCARD RenderBlock *FindFirstLetterBlock(RenderBlock *start) noexcept
    {
      RenderBlock *firstLetterBlock = start;
      while (true)
      {
        bool canHaveFirstLetterRenderer =
          firstLetterBlock->Style().HasPseudoStyle(PseudoElementType::FirstLetter)
          && firstLetterBlock->CanHaveGeneratedChildren()
          && IsRenderBlockFlowOrRenderButton(*firstLetterBlock);
        if (canHaveFirstLetterRenderer)
        {
          return firstLetterBlock;
        }

        RenderElement *parentBlock = firstLetterBlock->Parent();
        if (firstLetterBlock->IsBlockLevelReplacedOrAtomicInline() || !parentBlock
            || parentBlock->FirstChild() != firstLetterBlock
            || !IsRenderBlockFlowOrRenderButton(*parentBlock))
        {
          return nullptr;
        }
        firstLetterBlock = Downcast<RenderBlock>(parentBlock);
      }

      return nullptr;
    }
  }

  Pair<RenderObject *, RenderElement *>
    RenderBlock::FirstLetterAndContainer(RenderObject *skipThisAsFirstLetter) noexcept
  {
    // Don't recur
    if (Style().GetPseudoElementType() == PseudoElementType::FirstLetter)
    {
      return {};
    }

    // FIXME: We need to destroy the first-letter object if it is no longer the first child. Need to find
    // an efficient way to check for that situation though before implementing anything.
    RenderElement *firstLetterContainer = FindFirstLetterBlock(this);
    if (!firstLetterContainer)
    {
      return {};
    }

    // Drill into inlines looking for our first text descendant.
    auto *firstLetter = firstLetterContainer->FirstChild();
    while (firstLetter)
    {
      if (Is<RenderText>(*firstLetter))
      {
        if (firstLetter == skipThisAsFirstLetter)
        {
          firstLetter = firstLetter->NextSibling();
          continue;
        }
        break;
      }

      auto &current = Downcast<RenderElement>(*firstLetter);
      if (Is<RenderListMarker>(current))
      {
        firstLetter = current.NextSibling();
      }
      else if (current.IsFloatingOrOutOfFlowPositioned())
      {
        if (current.Style().GetPseudoElementType() == PseudoElementType::FirstLetter)
        {
          firstLetter = current.FirstChild();
          break;
        }
        firstLetter = current.NextSibling();
      }
      // FIXME: Menu lists, once there are form controls.
      else if (current.IsBlockLevelReplacedOrAtomicInline() || current.IsRenderButton())
      {
        break;
      }
      else if (current.IsFlexibleBoxIncludingDeprecated() || current.IsRenderGrid())
      {
        return {};
      }
      else if (current.Style().HasPseudoStyle(PseudoElementType::FirstLetter)
               && current.CanHaveGeneratedChildren())
      {
        // We found a lower-level node with first-letter, which supersedes the higher-level style
        firstLetterContainer = &current;
        firstLetter = current.FirstChild();
      }
      else
      {
        firstLetter = current.FirstChild();
      }
    }

    if (!firstLetter)
    {
      return {};
    }

    return {firstLetter, firstLetterContainer};
  }
}
