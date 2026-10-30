#include "Krystal.Booey/CSS/Values/Color/KeywordColor.hpp"
#include "Krystal.Booey/CSS/Values/Color/PlatformColorResolutionState.hpp"
#include "Krystal.Booey/Graphics/Color/NamedColors.hpp"
#include "Krystal.Booey/Rendering/RenderTheme.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::css
{
  namespace
  {
    KRYS_NODISCARD constexpr bool IsVGAPaletteColor(ValueId id) noexcept
    {
      // https://drafts.csswg.org/css-color-4/#named-colors
      // "16 of CSS's named colors come from the VGA palette originally, and were then adopted into HTML"
      return id >= ValueId::Aqua && id <= ValueId::Grey;
    }

    KRYS_NODISCARD constexpr bool IsNonVGANamedColor(ValueId id) noexcept
    {
      // https://drafts.csswg.org/css-color-4/#named-colors
      return id >= ValueId::Aliceblue && id <= ValueId::Yellowgreen;
    }
  }

  bool IsAbsoluteColorKeyword(ValueId id) noexcept
  {
    // https://drafts.csswg.org/css-color-4/#typedef-absolute-color
    return IsVGAPaletteColor(id) || IsNonVGANamedColor(id) || id == ValueId::Alpha
           || id == ValueId::Transparent;
  }

  bool IsCurrentColorKeyword(ValueId id) noexcept
  {
    return id == ValueId::Currentcolor;
  }

  bool IsSystemColorKeyword(ValueId id) noexcept
  {
    // https://drafts.csswg.org/css-color-4/#css-system-colors
    return (id >= ValueId::Canvas && id <= ValueId::InternalDocumentTextColor) || id == ValueId::Text
           || IsDeprecatedSystemColorKeyword(id);
  }

  bool IsDeprecatedSystemColorKeyword(ValueId id) noexcept
  {
    // https://drafts.csswg.org/css-color-4/#deprecated-system-colors
    return (id >= ValueId::Activeborder && id <= ValueId::Windowtext) || id == ValueId::Menu;
  }

  bool IsColorKeyword(ValueId id, ColorType allowedColorTypes) noexcept
  {
    return (HasFlag(allowedColorTypes, ColorType::Absolute) && IsAbsoluteColorKeyword(id))
           || (HasFlag(allowedColorTypes, ColorType::Current) && IsCurrentColorKeyword(id))
           || (HasFlag(allowedColorTypes, ColorType::System) && IsSystemColorKeyword(id));
  }

  bool IsColorKeyword(ValueId id) noexcept
  {
    return IsAbsoluteColorKeyword(id) || IsCurrentColorKeyword(id) || IsSystemColorKeyword(id);
  }

  boo::Color ColorFromAbsoluteKeyword(ValueId keyword) noexcept
  {
    krys_debug_assert(IsAbsoluteColorKeyword(keyword));

    // FIXME: Investigate if this should be a constexpr map for performance.

    if (auto namedColor = FindColor(ToString(keyword)))
    {
      return AsSRGBA(PackedColour::ARGB {namedColor->ARGBValue});
    }
    krys_debug_assert_msg(false, "Every absolute colour keyword is a named colour.");
    return {};
  }

  boo::Color ColorFromKeyword(ValueId keyword, style::ColorOptions options) noexcept
  {
    if (IsAbsoluteColorKeyword(keyword))
    {
      return ColorFromAbsoluteKeyword(keyword);
    }

    return RenderTheme::Singleton().SystemColor(keyword, options);
  }

  boo::Color CreateColor(const KeywordColor &unresolved, PlatformColorResolutionState &state) noexcept
  {
    switch (unresolved.Value)
    {
      case ValueId::InternalDocumentTextColor: return state.InternalDocumentTextColor();
      case ValueId::WebkitLink:
        return state.ForVisitedLink == style::ForVisitedLink::Yes ? state.WebkitLinkVisited()
                                                                  : state.WebkitLink();
      case ValueId::WebkitActivelink:     return state.WebkitActiveLink();
      case ValueId::WebkitFocusRingColor: return state.WebkitFocusRingColor();
      case ValueId::Currentcolor:         return state.CurrentColor();
      default:                            return ColorFromKeyword(unresolved.Value, state.KeywordOptions);
    }
  }

  bool ContainsCurrentColor(const KeywordColor &unresolved) noexcept
  {
    return IsCurrentColorKeyword(unresolved.Value);
  }

  bool ContainsColorSchemeDependentColor(const KeywordColor &unresolved) noexcept
  {
    return IsSystemColorKeyword(unresolved.Value);
  }

  void Serialiser<KeywordColor>::operator()(StringBuilder &builder, const SerialisationContext &,
                                            const KeywordColor &value) noexcept
  {
    builder.Append(ToLower(value.Value));
  }
}
