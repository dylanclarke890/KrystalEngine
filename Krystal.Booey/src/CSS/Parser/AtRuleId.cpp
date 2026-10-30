#include "Krystal.Booey/CSS/Parser/AtRuleId.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include "Krystal.Core/Types/Pair.hpp"

namespace krys::boo::css
{
  using namespace std::string_view_literals;

  AtRuleId AtRuleIdFromName(CSSOMStringView name) noexcept
  {
    constexpr auto mappings = Array<Pair<CSSOMStringView, AtRuleId>, 25> {{
      {u8"annotation"sv, AtRuleId::Annotation},
      {u8"character-variant"sv, AtRuleId::CharacterVariant},
      {u8"charset"sv, AtRuleId::Charset},
      {u8"container"sv, AtRuleId::Container},
      {u8"counter-style"sv, AtRuleId::CounterStyle},
      {u8"font-face"sv, AtRuleId::FontFace},
      {u8"font-feature-values"sv, AtRuleId::FontFeatureValues},
      {u8"font-palette-values"sv, AtRuleId::FontPaletteValues},
      {u8"function"sv, AtRuleId::Function},
      {u8"import"sv, AtRuleId::Import},
      {u8"keyframes"sv, AtRuleId::Keyframes},
      {u8"layer"sv, AtRuleId::Layer},
      {u8"media"sv, AtRuleId::Media},
      {u8"namespace"sv, AtRuleId::Namespace},
      {u8"ornaments"sv, AtRuleId::Ornaments},
      {u8"page"sv, AtRuleId::Page},
      {u8"position-try"sv, AtRuleId::PositionTry},
      {u8"property"sv, AtRuleId::Property},
      {u8"scope"sv, AtRuleId::Scope},
      {u8"starting-style"sv, AtRuleId::StartingStyle},
      {u8"styleset"sv, AtRuleId::Styleset},
      {u8"stylistic"sv, AtRuleId::Stylistic},
      {u8"supports"sv, AtRuleId::Supports},
      {u8"swash"sv, AtRuleId::Swash},
      {u8"view-transition"sv, AtRuleId::ViewTransition},
    }};

    for (const auto &[ruleName, id] : mappings)
    {
      if (text::IsASCIICaselessEqual(name, ruleName))
      {
        return id;
      }
    }
    return AtRuleId::Invalid;
  }
}
