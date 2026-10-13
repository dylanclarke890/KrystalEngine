#include "Krystal.Booey/CSS/WebKitFontFamilyNames.hpp"
#include "Krystal.Core/Assert.hpp"

namespace krys::boo::css::WebKitFontFamilyNames
{
  const CSSOMStringAtom &CursiveFamily() noexcept
  {
    static const CSSOMStringAtom name {u8"-webkit-cursive"};
    return name;
  }

  const CSSOMStringAtom &FantasyFamily() noexcept
  {
    static const CSSOMStringAtom name {u8"-webkit-fantasy"};
    return name;
  }

  const CSSOMStringAtom &MathFamily() noexcept
  {
    static const CSSOMStringAtom name {u8"math"};
    return name;
  }

  const CSSOMStringAtom &MonospaceFamily() noexcept
  {
    static const CSSOMStringAtom name {u8"-webkit-monospace"};
    return name;
  }

  const CSSOMStringAtom &PictographFamily() noexcept
  {
    static const CSSOMStringAtom name {u8"-webkit-pictograph"};
    return name;
  }

  const CSSOMStringAtom &SansSerifFamily() noexcept
  {
    static const CSSOMStringAtom name {u8"-webkit-sans-serif"};
    return name;
  }

  const CSSOMStringAtom &SerifFamily() noexcept
  {
    static const CSSOMStringAtom name {u8"-webkit-serif"};
    return name;
  }

  const CSSOMStringAtom &StandardFamily() noexcept
  {
    static const CSSOMStringAtom name {u8"-webkit-standard"};
    return name;
  }

  const CSSOMStringAtom &SystemUiFamily() noexcept
  {
    static const CSSOMStringAtom name {u8"-webkit-system-ui"};
    return name;
  }

  const CSSOMStringAtom &FamilyName(FamilyNamesIndex index) noexcept
  {
    switch (index)
    {
      case FamilyNamesIndex::CursiveFamily:
      {
        return CursiveFamily();
      }
      case FamilyNamesIndex::FantasyFamily:
      {
        return FantasyFamily();
      }
      case FamilyNamesIndex::MathFamily:
      {
        return MathFamily();
      }
      case FamilyNamesIndex::MonospaceFamily:
      {
        return MonospaceFamily();
      }
      case FamilyNamesIndex::PictographFamily:
      {
        return PictographFamily();
      }
      case FamilyNamesIndex::SansSerifFamily:
      {
        return SansSerifFamily();
      }
      case FamilyNamesIndex::SerifFamily:
      {
        return SerifFamily();
      }
      case FamilyNamesIndex::StandardFamily:
      {
        return StandardFamily();
      }
      case FamilyNamesIndex::SystemUiFamily:
      {
        return SystemUiFamily();
      }
    }

    krys_unreachable();
  }
}
