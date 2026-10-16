#include "Krystal.Booey/Rendering/RenderListMarker.hpp"
#include "Krystal.Booey/CSS/Counter/CounterStyle.hpp"
#include "Krystal.Booey/CSS/Counter/CounterStyleRegistry.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Rendering/RenderListItem.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/StyleImage.hpp"
#include "Krystal.Booey/Style/Scope.hpp"
#include "Krystal.Booey/Style/Values/Lists/ListStyleType.hpp"
#include "Krystal.Core/Text/CodePointIterator.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"
#include <unicode/uchar.h>

namespace krys::boo
{
  namespace
  {
    KRYS_NODISCARD style::Difference AdjustedStyleDifference(style::Difference diff,
                                                             const RenderStyle &oldStyle,
                                                             const RenderStyle &newStyle) noexcept
    {
      if (diff >= style::DifferenceResult::Layout)
      {
        return diff;
      }
      // FIXME: Preferably we do this at RenderStyle::changeRequiresLayout but checking against
      // pseudo(::marker) is not sufficient.
      auto needsLayout = oldStyle.GetListStylePosition() != newStyle.GetListStylePosition()
                         || oldStyle.GetListStyleType() != newStyle.GetListStyleType()
                         || oldStyle.IsDisplayInlineType() != newStyle.IsDisplayInlineType();
      return needsLayout ? style::Difference {style::DifferenceResult::Layout} : diff;
    }

    KRYS_NODISCARD TextDirection ContentTextDirection(utf8_stringview content) noexcept
    {
      if (content.empty())
      {
        return TextDirection::LTR;
      }
      // FIXME: Depending on the string value, we may need the real bidi algorithm. (rdar://106139180)
      // Also we may need to start checking for the entire content for directionality (and whether we need to
      // check for additional directionality characters like U_RIGHT_TO_LEFT_EMBEDDING).
      auto bidiCategory = u_charDirection(static_cast<UChar32>(*text::CodePointIterator(content)));
      if (bidiCategory != U_RIGHT_TO_LEFT && bidiCategory != U_RIGHT_TO_LEFT_ARABIC)
      {
        return TextDirection::LTR;
      }
      return TextDirection::RTL;
    }
  }

  RenderListMarker::RenderListMarker(RenderListItem &listItem, RenderStyle &&style) noexcept
      : RenderBox(Type::ListMarker, listItem.GetDocument(), krys::move(style)),
        _listItem(CreateWeakPtr(&listItem))
  {
    SetInline(true);
    SetBlockLevelReplacedOrAtomicInline(true); // pretend to be replaced
    krys_debug_assert(IsRenderListMarker());
  }

  // Do not add any code in below destructor. Add it to willBeDestroyed() instead.
  RenderListMarker::~RenderListMarker() noexcept = default;

  void RenderListMarker::WillBeDestroyed() noexcept
  {
    // FIXME: Stop being the image's client, once there's image loading.
    RenderBox::WillBeDestroyed();
  }

  void RenderListMarker::StyleWillChange(style::Difference diff, const RenderStyle &newStyle) noexcept
  {
    RenderBox::StyleWillChange(AdjustedStyleDifference(diff, Style(), newStyle), newStyle);
  }

  void RenderListMarker::StyleDidChange(style::Difference diff, const RenderStyle *oldStyle) noexcept
  {
    if (oldStyle)
    {
      diff = AdjustedStyleDifference(diff, *oldStyle, Style());
    }
    RenderBox::StyleDidChange(diff, oldStyle);

    if (auto newImage = Style().ListStyleImage().TryStyleImage(); _image != newImage)
    {
      // FIXME: Stop being the old image's client and become the new one's, once there's image loading.
      _image = krys::move(newImage);
    }
  }

  bool RenderListMarker::IsImage() const noexcept
  {
    return _image && !_image->ErrorOccurred();
  }

  void RenderListMarker::UpdateInlineMarginsAndContent() noexcept
  {
    // FIXME: Only update the content when the preferred widths need updating, and update the inline margins,
    // once there's layout.
    UpdateContent();
  }

  void RenderListMarker::UpdateContent() noexcept
  {
    if (IsImage())
    {
      // FIXME: The image's container size, once there's image loading and layout.
      _textContent = {
        .TextWithSuffix = {},
        .TextWithoutSuffixLength = 0,
        .TextDirection = TextDirection::LTR,
      };
      return;
    }

    Style().GetListStyleType().SwitchOn(
      [&](const css::keywords::None &)
      {
        _textContent = {
          .TextWithSuffix = u8" ",
          .TextWithoutSuffixLength = 0,
          .TextDirection = TextDirection::LTR,
        };
      },
      [&](const css::CSSOMStringAtom &identifier)
      {
        _textContent = {
          .TextWithSuffix = utf8_string(identifier.View()),
          .TextWithoutSuffixLength = identifier.View().size(),
          .TextDirection = ContentTextDirection(identifier.View()),
        };
      },
      [&](const style::CounterStyle &)
      {
        auto counter = GetCounterStyle();
        krys_debug_assert(counter);

        StringBuilder text;
        text.Append(counter->Prefix().Text, counter->Text(_listItem->Value(), Style().GetWritingMode()));
        auto textWithoutSuffix = krys::move(text).ToString();
        StringBuilder textWithSuffix;
        textWithSuffix.Append(textWithoutSuffix, counter->Suffix().Text);
        _textContent = {
          .TextWithSuffix = krys::move(textWithSuffix).ToString(),
          .TextWithoutSuffixLength = textWithoutSuffix.size(),
          .TextDirection = ContentTextDirection(textWithoutSuffix),
        };
      });
  }

  bool RenderListMarker::IsInside() const noexcept
  {
    return Style().GetListStylePosition() == ListStylePosition::Inside;
  }

  bool RenderListMarker::IsDisclosureMarker() const noexcept
  {
    const auto &listStyleType = Style().GetListStyleType();
    if (auto counterStyle = listStyleType.TryCounterStyle())
    {
      const auto &identifier = counterStyle->Identifier.Value;
      return identifier == u8"disclosure-open" || identifier == u8"disclosure-closed";
    }
    return false;
  }

  const RenderListItem *RenderListMarker::ListItem() const noexcept
  {
    return _listItem.get();
  }

  RefPtr<css::CounterStyle> RenderListMarker::GetCounterStyle() const noexcept
  {
    auto counterStyle = Style().GetListStyleType().TryCounterStyle();
    if (!counterStyle)
    {
      return nullptr;
    }
    return GetDocument().StyleScope().GetCounterStyleRegistry().ResolvedCounterStyle(*counterStyle);
  }
}
