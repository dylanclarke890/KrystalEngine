#include "Krystal.Booey/Style/CustomPropertyRegistry.hpp"
#include "Krystal.Booey/CSS/Parser/KeywordLookup.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParser.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+SettersInlines.hpp"
#include "Krystal.Booey/Style/BuilderState.hpp"
#include "Krystal.Booey/Style/CustomProperty.hpp"
#include "Krystal.Booey/Style/Scope.hpp"
#include "Krystal.Core/Assert.hpp"

namespace krys::boo::style
{
  CustomPropertyRegistry::CustomPropertyRegistry(Scope &scope) noexcept
      : _scope(scope), _initialValuePrototypeStyle(RenderStyle::CreatePtr())
  {
  }

  CustomPropertyRegistry::~CustomPropertyRegistry() noexcept = default;

  const css::RegisteredCustomProperty *
    CustomPropertyRegistry::Get(const css::CSSOMStringAtom &name) const noexcept
  {
    krys_debug_assert(css::IsCustomPropertyName(name.View()));

    // API wins.
    // https://drafts.css-houdini.org/css-properties-values-api/#determining-registration
    if (auto it = _propertiesFromAPI.find(name); it != _propertiesFromAPI.end())
    {
      return it->second.get();
    }

    if (auto it = _propertiesFromStylesheet.find(name); it != _propertiesFromStylesheet.end())
    {
      return it->second.get();
    }
    return nullptr;
  }

  bool CustomPropertyRegistry::IsInherited(const css::CSSOMStringAtom &name) const noexcept
  {
    const auto *registered = Get(name);
    return registered ? registered->Inherits : true;
  }

  bool CustomPropertyRegistry::RegisterFromAPI(css::RegisteredCustomProperty &&property) noexcept
  {
    // First registration wins.
    // https://drafts.css-houdini.org/css-properties-values-api/#determining-registration
    auto name = property.Name;
    auto success =
      _propertiesFromAPI.try_emplace(name, CreateUnique<css::RegisteredCustomProperty>(krys::move(property)))
        .second;

    if (success)
    {
      Invalidate(name);
      _scope.DidChangeStyleSheetEnvironment();
    }

    return success;
  }

  void
    CustomPropertyRegistry::RegisterFromStylesheet(const css::PropertyRule::Descriptor &descriptor) noexcept
  {
    auto syntax = css::CustomPropertySyntax::Parse(descriptor.Syntax);
    krys_debug_assert(syntax);
    if (!syntax)
    {
      return;
    }

    auto &document = _scope.Document();

    RefPtr<const CustomProperty> initialValue;
    RefPtr<const css::VariableData> initialValueTokensForViewportUnits;

    if (descriptor.InitialValue)
    {
      auto tokenRange = descriptor.InitialValue->TokenRange();

      auto parsedInitialValue = ParseInitialValue(document, descriptor.Name, *syntax, tokenRange);
      if (!parsedInitialValue)
      {
        return;
      }

      initialValue = parsedInitialValue->first;
      if (parsedInitialValue->second == ViewportUnitDependency::Yes)
      {
        initialValueTokensForViewportUnits = css::VariableData::Create(tokenRange);
        // FIXME: `Document::setHasStyleWithViewportUnits()` comes with viewport unit invalidation.
      }
    }
    else if (syntax->IsUniversal())
    {
      // "If the value of the syntax descriptor is the universal syntax definition, then the initial-value
      // descriptor is optional. If omitted, the initial value of the property is the guaranteed-invalid
      // value." https://drafts.css-houdini.org/css-properties-values-api/#initial-value-descriptor
      initialValue = CustomProperty::CreateForGuaranteedInvalid(descriptor.Name);
    }
    else
    {
      // Should have been discarded at parse-time.
      krys_debug_assert_msg(false, "a non-universal @property without an initial value");
      return;
    }

    auto property = css::RegisteredCustomProperty {
      .Name = descriptor.Name,
      .Syntax = krys::move(*syntax),
      .Inherits = *descriptor.Inherits,
      .InitialValue = krys::move(initialValue),
      .InitialValueTokensForViewportUnits = krys::move(initialValueTokensForViewportUnits),
    };

    // Last rule wins.
    // https://drafts.css-houdini.org/css-properties-values-api/#determining-registration
    auto name = property.Name;
    _propertiesFromStylesheet.insert_or_assign(
      name, CreateUnique<css::RegisteredCustomProperty>(krys::move(property)));

    Invalidate(name);
  }

  void CustomPropertyRegistry::ClearRegisteredFromStylesheets() noexcept
  {
    if (_propertiesFromStylesheet.empty())
    {
      return;
    }

    _propertiesFromStylesheet.clear();
    Invalidate(css::CSSOMStringAtom::Null());
  }

  const RenderStyle &CustomPropertyRegistry::InitialValuePrototypeStyle() const noexcept
  {
    if (_hasInvalidPrototypeStyle)
    {
      _hasInvalidPrototypeStyle = false;

      auto oldStyle = std::exchange(_initialValuePrototypeStyle, RenderStyle::CreatePtr());

      auto initializeToStyle = [&](auto &map)
      {
        for (auto &[name, property] : map)
        {
          if (property->InitialValue && !property->InitialValue->IsGuaranteedInvalid())
          {
            Ref<const CustomProperty> initialValue = property->InitialValue;
            _initialValuePrototypeStyle->SetCustomPropertyValue(krys::move(initialValue), property->Inherits);
          }
        }
      };

      initializeToStyle(_propertiesFromStylesheet);
      initializeToStyle(_propertiesFromAPI);

      _initialValuePrototypeStyle->DeduplicateCustomProperties(*oldStyle);
    }

    return *_initialValuePrototypeStyle;
  }

  void CustomPropertyRegistry::Invalidate(const css::CSSOMStringAtom &customProperty) noexcept
  {
    _hasInvalidPrototypeStyle = true;
    // Changing property registration may affect computed property values in the cache.
    // FIXME: `Scope::invalidateMatchedDeclarationsCache()` comes with the matched declarations cache.

    // FIXME: Invalidate all?
    if (customProperty != css::CSSOMStringAtom::Null())
    {
      NotifyAnimationsOfCustomPropertyRegistration(customProperty);
    }
  }

  void CustomPropertyRegistry::NotifyAnimationsOfCustomPropertyRegistration(
    const css::CSSOMStringAtom &) noexcept
  {
    // FIXME: Tell the keyframe effects targeting elements in the scope's document
    // (`KeyframeEffect::customPropertyRegistrationDidChange`), once animations are ported.
  }

  auto CustomPropertyRegistry::ParseInitialValue(const dom::Document &document,
                                                 const css::CSSOMStringAtom &propertyName,
                                                 const css::CustomPropertySyntax &syntax,
                                                 css::TokenRange tokenRange) noexcept
    -> Expected<Pair<RefPtr<const CustomProperty>, ViewportUnitDependency>, ParseInitialValueError>
  {
    // FIXME: This parses twice.
    auto dependencies = css::PropertyParser::CollectParsedCustomPropertyValueDependencies(
      syntax, tokenRange, document.CssParserContext());
    if (!dependencies.IsComputationallyIndependent())
    {
      return Unexpected {ParseInitialValueError::NotComputationallyIndependent};
    }

    // We don't need to provide a real context style since only computationally independent values are allowed
    // (no 'em' etc).
    auto placeholderStyle = RenderStyle::Create();
    auto dummyState =
      CreateUnique<BuilderState>(placeholderStyle, BuilderContext {.Document = ShareRef(document)});

    auto initialValue = css::PropertyParser::ParseTypedCustomPropertyInitialValue(
      propertyName, syntax, tokenRange, *dummyState, document.CssParserContext());
    if (!initialValue)
    {
      return Unexpected {ParseInitialValueError::DidNotParse};
    }

    return Pair {krys::move(initialValue),
                 dependencies.ViewportDimensions ? ViewportUnitDependency::Yes : ViewportUnitDependency::No};
  }

  bool CustomPropertyRegistry::InvalidatePropertiesWithViewportUnits(dom::Document &document) noexcept
  {
    bool invalidatedAny = false;

    auto invalidatePropertiesWithViewportUnits = [&](auto &map)
    {
      for (auto &[name, property] : map)
      {
        if (!property->InitialValueTokensForViewportUnits)
        {
          continue;
        }

        auto tokenRange = property->InitialValueTokensForViewportUnits->TokenRange();
        auto parsedInitialValue = ParseInitialValue(document, property->Name, property->Syntax, tokenRange);
        if (!parsedInitialValue)
        {
          krys_debug_assert_msg(false, "a registered initial value no longer parses");
          continue;
        }
        property->InitialValue = krys::move(parsedInitialValue->first);
        krys_debug_assert(parsedInitialValue->second == ViewportUnitDependency::Yes);

        Invalidate(property->Name);
        invalidatedAny = true;
      }
    };

    invalidatePropertiesWithViewportUnits(_propertiesFromAPI);
    invalidatePropertiesWithViewportUnits(_propertiesFromStylesheet);

    return invalidatedAny;
  }
}
