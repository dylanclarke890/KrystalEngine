#include "Krystal.Booey/Style/Values/Color/KeywordColor.hpp"
#include "Krystal.Booey/CSS/Values/Color/KeywordColor.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Rendering/RenderTheme.hpp"
#include "Krystal.Booey/Style/Values/Color/Color.hpp"
#include "Krystal.Booey/Style/Values/Color/ColorResolutionState.hpp"
#include "Krystal.Booey/Style/Values/Color/CurrentColor.hpp"

namespace krys::boo::style
{
  Color ToStyleColor(const css::KeywordColor &unresolved, ColorResolutionState &state) noexcept
  {
    Ref protectedDocument = state.Document;
    switch (unresolved.Value)
    {
      case css::ValueId::InternalDocumentTextColor: return {protectedDocument->TextColor()};
      case css::ValueId::WebkitLink:
        return {state.ForVisitedLink == ForVisitedLink::Yes ? protectedDocument->VisitedLinkColor(state.Style)
                                                            : protectedDocument->LinkColor(state.Style)};
      case css::ValueId::WebkitActivelink: return {protectedDocument->ActiveLinkColor(state.Style)};
      case css::ValueId::WebkitFocusRingColor:
        return {RenderTheme::Singleton().FocusRingColor(protectedDocument->StyleColorOptions(&state.Style))};
      case css::ValueId::Currentcolor: return {CurrentColor()};
      default:
        return {css::ColorFromKeyword(unresolved.Value, protectedDocument->StyleColorOptions(&state.Style))};
    }
  }
}
