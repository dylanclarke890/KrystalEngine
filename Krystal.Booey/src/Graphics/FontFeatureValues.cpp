#include "Krystal.Booey/Graphics/FontFeatureValues.hpp"

namespace krys::boo
{
  void FontFeatureValues::UpdateOrInsert(const FontFeatureValues &other) noexcept
  {
    if (this == &other)
    {
      return;
    }

    auto updateOrInsertTags = [](Tags &into, const Tags &tags)
    {
      for (const auto &[name, values] : tags)
      {
        into.insert_or_assign(name, values);
      }
    };
    updateOrInsertTags(_styleset, other.Styleset());
    updateOrInsertTags(_stylistic, other.Stylistic());
    updateOrInsertTags(_characterVariant, other.CharacterVariant());
    updateOrInsertTags(_swash, other.Swash());
    updateOrInsertTags(_ornaments, other.Ornaments());
    updateOrInsertTags(_annotation, other.Annotation());
  }

  void FontFeatureValues::UpdateOrInsertForType(FontFeatureValuesType type,
                                                const List<FontFeatureValuesTag> &tags) noexcept
  {
    auto updateOrInsertTags = [](Tags &into, const List<FontFeatureValuesTag> &tags)
    {
      for (const auto &[name, values] : tags)
      {
        into.insert_or_assign(name, values);
      }
    };
    switch (type)
    {
      case FontFeatureValuesType::Styleset:
      {
        updateOrInsertTags(_styleset, tags);
        break;
      }
      case FontFeatureValuesType::Stylistic:
      {
        updateOrInsertTags(_stylistic, tags);
        break;
      }
      case FontFeatureValuesType::CharacterVariant:
      {
        updateOrInsertTags(_characterVariant, tags);
        break;
      }
      case FontFeatureValuesType::Swash:
      {
        updateOrInsertTags(_swash, tags);
        break;
      }
      case FontFeatureValuesType::Ornaments:
      {
        updateOrInsertTags(_ornaments, tags);
        break;
      }
      case FontFeatureValuesType::Annotation:
      {
        updateOrInsertTags(_annotation, tags);
        break;
      }
    }
  }

  size_t FontFeatureValues::Hash() const noexcept
  {
    // The maps are unordered, so equal maps can iterate in different orders: combine the entries with an
    // order-independent sum.
    auto hashTags = [](const Tags &tags) -> size_t
    {
      size_t sum = 0;
      for (const auto &[name, values] : tags)
      {
        size_t entry = DefaultHash<css::CSSOMString>::Hash(name);
        for (auto value : values)
        {
          entry = krys::Hash::Combine(entry, value);
        }
        sum += entry;
      }
      return krys::Hash::Combine(tags.empty(), sum);
    };
    return krys::Hash::Combine(hashTags(_styleset), hashTags(_stylistic), hashTags(_characterVariant),
                               hashTags(_swash), hashTags(_ornaments), hashTags(_annotation));
  }
}
