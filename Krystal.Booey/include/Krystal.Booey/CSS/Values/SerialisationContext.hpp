#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class CSSStyleSheet;

  struct SerialisationContext
  {
    SerialisationContext() noexcept;

    SerialisationContext(HashMap<CSSOMString, CSSOMString> &&replacementUrlStrings,
                         HashMap<Ref<CSSStyleSheet>, CSSOMString> &&replacementUrlStringsForStyleSheet,
                         bool shouldUseResolvedURLInCSSText) noexcept;

    ~SerialisationContext() noexcept;

    HashMap<CSSOMString, CSSOMString> ReplacementUrlStrings;
    HashMap<Ref<CSSStyleSheet>, CSSOMString> ReplacementUrlStringsForStyleSheet;
    bool ShouldUseResolvedUrlInCSSText = false;
  };

  KRYS_NODISCARD const SerialisationContext &DefaultSerialisationContext() noexcept;
}