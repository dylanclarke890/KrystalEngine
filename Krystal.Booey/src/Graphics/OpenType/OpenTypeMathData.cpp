#include "Krystal.Booey/Graphics/OpenType/OpenTypeMathData.hpp"
#include "Krystal.Booey/Graphics/Font.hpp"
#include "Krystal.Booey/Graphics/FontPlatformData.hpp"
#include <array>
#include <hb-ot.h>
#include <hb.h>

namespace krys::boo
{
  static_assert(static_cast<int>(OpenTypeMathData::MathConstant::ScriptPercentScaleDown)
                == HB_OT_MATH_CONSTANT_SCRIPT_PERCENT_SCALE_DOWN);
  static_assert(static_cast<int>(OpenTypeMathData::MathConstant::FractionRuleThickness)
                == HB_OT_MATH_CONSTANT_FRACTION_RULE_THICKNESS);
  static_assert(static_cast<int>(OpenTypeMathData::MathConstant::RadicalDegreeBottomRaisePercent)
                == HB_OT_MATH_CONSTANT_RADICAL_DEGREE_BOTTOM_RAISE_PERCENT);

  OpenTypeMathData::OpenTypeMathData(const FontPlatformData &font) noexcept
      : _mathFont(font.CreateOpenTypeMathHarfBuzzFont())
  {
  }

  OpenTypeMathData::~OpenTypeMathData() noexcept = default;

  float OpenTypeMathData::GetMathConstant(const Font &font, MathConstant constant) const noexcept
  {
    hb_position_t value =
      hb_ot_math_get_constant(_mathFont.get(), static_cast<hb_ot_math_constant_t>(constant));
    if (constant == MathConstant::ScriptPercentScaleDown
        || constant == MathConstant::ScriptScriptPercentScaleDown
        || constant == MathConstant::RadicalDegreeBottomRaisePercent)
    {
      return static_cast<float>(value / 100.0);
    }

    return static_cast<float>(value) * font.SizePerUnit();
  }

  float OpenTypeMathData::GetItalicCorrection(const Font &font, Glyph glyph) const noexcept
  {
    return static_cast<float>(hb_ot_math_get_glyph_italics_correction(_mathFont.get(), glyph))
           * font.SizePerUnit();
  }

  void OpenTypeMathData::GetMathVariants(Glyph glyph, bool isVertical, List<Glyph> &sizeVariants,
                                         List<AssemblyPart> &assemblyParts) const noexcept
  {
    hb_direction_t direction = isVertical ? HB_DIRECTION_BTT : HB_DIRECTION_LTR;

    sizeVariants.clear();
    std::array<hb_ot_math_glyph_variant_t, 10> variants {};
    uint32 variantsSize = static_cast<uint32>(variants.size());
    uint32 count;
    uint32 offset = 0;
    do
    {
      count = variantsSize;
      hb_ot_math_get_glyph_variants(_mathFont.get(), glyph, direction, offset, &count, variants.data());
      offset += count;
      for (uint32 i = 0; i < count; i++)
      {
        sizeVariants.push_back(static_cast<Glyph>(variants[i].glyph));
      }
    } while (count == variantsSize);

    assemblyParts.clear();
    std::array<hb_ot_math_glyph_part_t, 10> parts {};
    uint32 partsSize = static_cast<uint32>(parts.size());
    offset = 0;
    do
    {
      count = partsSize;
      hb_ot_math_get_glyph_assembly(_mathFont.get(), glyph, direction, offset, &count, parts.data(), nullptr);
      offset += count;
      for (uint32 i = 0; i < count; i++)
      {
        assemblyParts.push_back(
          AssemblyPart {.Glyph = static_cast<Glyph>(parts[i].glyph),
                        .IsExtender = (parts[i].flags & HB_OT_MATH_GLYPH_PART_FLAG_EXTENDER) != 0});
      }
    } while (count == partsSize);
  }
}
