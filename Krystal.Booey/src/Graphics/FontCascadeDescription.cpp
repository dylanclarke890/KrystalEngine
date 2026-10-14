#include "Krystal.Booey/Graphics/FontCascadeDescription.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include <functional>

namespace krys::boo
{
  FontSelectionValue FontCascadeDescription::LighterWeight(FontSelectionValue weight) noexcept
  {
    if (weight < FontSelectionValue(100))
    {
      return weight;
    }
    if (weight < FontSelectionValue(550))
    {
      return FontSelectionValue(100);
    }
    if (weight < FontSelectionValue(750))
    {
      return FontSelectionValue(400);
    }
    return FontSelectionValue(700);
  }

  FontSelectionValue FontCascadeDescription::BolderWeight(FontSelectionValue weight) noexcept
  {
    if (weight < FontSelectionValue(350))
    {
      return FontSelectionValue(400);
    }
    if (weight < FontSelectionValue(550))
    {
      return FontSelectionValue(700);
    }
    if (weight < FontSelectionValue(900))
    {
      return FontSelectionValue(900);
    }
    return weight;
  }

  bool FontCascadeDescription::FamilyNamesAreEqual(const StringAtom &family1,
                                                   const StringAtom &family2) noexcept
  {
    return text::IsASCIICaselessEqual(family1.View(), family2.View());
  }

  size_t FontCascadeDescription::FamilyNameHash(const StringAtom &family) noexcept
  {
    return std::hash<utf8_string> {}(FoldedFamilyName(family.View()));
  }

  utf8_string FontCascadeDescription::FoldedFamilyName(utf8_stringview family) noexcept
  {
    return text::ToASCIILower(family);
  }

  uint32 FontCascadeDescription::EffectiveFamilyCount() const noexcept
  {
    return FamilyCount();
  }

  FontFamilySpecification FontCascadeDescription::EffectiveFamilyAt(uint32 index) const noexcept
  {
    return FamilyAt(index);
  }

  FontSmoothingMode FontCascadeDescription::UsedFontSmoothing() const noexcept
  {
    return FontSmoothing();
  }
}
