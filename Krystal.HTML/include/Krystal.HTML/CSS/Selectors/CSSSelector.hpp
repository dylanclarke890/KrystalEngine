#pragma once

#include "Krystal.HTML/CSS/Selectors/Enums/CSSSelectorFlag.hpp"
#include "Krystal.HTML/CSS/Selectors/Enums/PseudoClassId.hpp"
#include "Krystal.HTML/CSS/Selectors/Enums/PseudoElementId.hpp"
#include "Krystal.HTML/CSS/Selectors/Enums/SelectorMatch.hpp"
#include "Krystal.HTML/CSS/Selectors/Enums/SelectorRelation.hpp"
#include "Krystal.HTML/CSS/Types/CSSOMString.hpp"
#include "Krystal.HTML/DOM/Types/QualifiedName.hpp"

namespace Krys::HTML
{
  class CSSSelector
  {
  private:
    SelectorMatch _match : BitCount<SelectorMatch>();
    SelectorRelation _relation : BitCount<SelectorRelation>();
    CSSSelectorFlag _flags : BitCount<CSSSelectorFlag>();
    PseudoClassId _pseudoClassId : BitCount<PseudoClassId>();
    PseudoElementId _pseudoElementId : BitCount<PseudoElementId>();
    QualifiedName _tagName;
    CSSOMStringAtom _value;

  public:
    KRYS_NODISCARD SelectorMatch Match() const noexcept
    {
      return _match;
    }

    KRYS_NODISCARD SelectorRelation Relation() const noexcept
    {
      return _relation;
    }
  };
}