#include "Krystal.Booey/Style/Values/Color/KeywordColor.hpp"
#include "Krystal.Booey/CSS/Values/Color/KeywordColor.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Style/Seam.hpp"
#include "Krystal.Booey/Style/Values/Color/Color.hpp"
#include "Krystal.Booey/Style/Values/Color/ColorResolutionState.hpp"
#include "Krystal.Booey/Style/Values/Color/CurrentColor.hpp"
#include "Krystal.Booey/Style/Values/Color/SystemColor.hpp"

namespace krys::boo::style
{
  Color ToStyleColor(const css::KeywordColor &unresolved, ColorResolutionState &state) noexcept
  {
    Ref protectedDocument = state.Document;
    switch (unresolved.Value)
    {
      case css::ValueId::InternalDocumentTextColor: return {DocumentTextColor(*protectedDocument)};
      case css::ValueId::WebkitLink:
        return {state.ForVisitedLink == ForVisitedLink::Yes
                  ? DocumentVisitedLinkColor(*protectedDocument, state.Style)
                  : DocumentLinkColor(*protectedDocument, state.Style)};
      case css::ValueId::WebkitActivelink: return {DocumentActiveLinkColor(*protectedDocument, state.Style)};
      case css::ValueId::WebkitFocusRingColor:
        return {FocusRingColor(DocumentStyleColorOptions(*protectedDocument, &state.Style))};
      case css::ValueId::Currentcolor: return {CurrentColor()};
      default:
        return {css::ColorFromKeyword(unresolved.Value,
                                      DocumentStyleColorOptions(*protectedDocument, &state.Style))};
    }
  }
}
