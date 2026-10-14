#include "Krystal.Booey/CSS/Rules/KeyframeRule.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Animations.hpp"
#include "Krystal.Booey/CSS/Properties/MutableStyleProperties.hpp"
#include "Krystal.Booey/CSS/Properties/StyleProperties.hpp"
#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"
#include "Krystal.Core/Assert.hpp"
#include <charconv>

namespace krys::boo::css
{
  using namespace std::string_view_literals;

  namespace
  {
    KRYS_NODISCARD List<KeyframeRule::Key> ToKeys(Span<const Pair<ValueId, double>> keys) noexcept
    {
      List<KeyframeRule::Key> result;
      result.reserve(keys.size());
      for (const auto &[rangeName, offset] : keys)
      {
        result.push_back({rangeName, offset});
      }
      return result;
    }
  }

  void KeyframeRule::Key::WriteToString(CSSOMString &builder) const noexcept
  {
    switch (RangeName)
    {
      case ValueId::Contain:
      {
        builder += u8"contain "sv;
        break;
      }
      case ValueId::Cover:
      {
        builder += u8"cover "sv;
        break;
      }
      case ValueId::Entry:
      {
        builder += u8"entry "sv;
        break;
      }
      case ValueId::EntryCrossing:
      {
        builder += u8"entry-crossing "sv;
        break;
      }
      case ValueId::Exit:
      {
        builder += u8"exit "sv;
        break;
      }
      case ValueId::ExitCrossing:
      {
        builder += u8"exit-crossing "sv;
        break;
      }
      default:
      {
        break;
      }
    }

    // The shortest representation that round-trips.
    char buffer[64];
    auto [end, error] =
      std::to_chars(buffer, buffer + sizeof(buffer), Offset * 100, std::chars_format::fixed);
    krys_debug_assert(error == std::errc {});
    builder.append(reinterpret_cast<const char8_t *>(buffer), reinterpret_cast<const char8_t *>(end));
    builder += u8'%';
  }

  KeyframeRule::KeyframeRule(Ref<StyleProperties> &&properties) noexcept
      : Rule(RuleType::Keyframe), _properties(krys::move(properties))
  {
  }

  KeyframeRule::KeyframeRule(List<Key> &&keys, Ref<StyleProperties> &&properties) noexcept
      : Rule(RuleType::Keyframe), _properties(krys::move(properties)), _keys(krys::move(keys))
  {
  }

  KeyframeRule::~KeyframeRule() noexcept = default;

  Ref<KeyframeRule> KeyframeRule::Create(Ref<StyleProperties> &&properties) noexcept
  {
    return AdoptRef(*new KeyframeRule(krys::move(properties)));
  }

  Ref<KeyframeRule> KeyframeRule::Create(Span<const Pair<ValueId, double>> keys,
                                         Ref<StyleProperties> &&properties) noexcept
  {
    return AdoptRef(*new KeyframeRule(ToKeys(keys), krys::move(properties)));
  }

  Ref<KeyframeRule> KeyframeRule::Copy() const noexcept
  {
    krys_unreachable();
  }

  MutableStyleProperties &KeyframeRule::MutableProperties() noexcept
  {
    if (!Is<MutableStyleProperties>(*_properties))
    {
      _properties = _properties->MutableCopy();
    }
    return Downcast<MutableStyleProperties>(*_properties);
  }

  CSSOMString KeyframeRule::KeyText() const noexcept
  {
    CSSOMString keyText;
    for (size_t i = 0uz; i < _keys.size(); ++i)
    {
      if (i)
      {
        keyText += u8',';
      }
      _keys[i].WriteToString(keyText);
    }
    return keyText;
  }

  bool KeyframeRule::SetKeyText(const CSSOMString &keyText) noexcept
  {
    auto keys = PropertyParserHelpers::ParseKeyframeKeyList(keyText, StrictParserContext());
    if (keys.empty())
    {
      return false;
    }
    _keys = ToKeys(keys);
    return true;
  }

  CSSOMString KeyframeRule::CssText() const noexcept
  {
    if (auto declarations = _properties->AsText(DefaultSerialisationContext()); !declarations.empty())
    {
      return KeyText() + u8" { " + declarations + u8" }";
    }
    return KeyText() + u8" { }";
  }
}
