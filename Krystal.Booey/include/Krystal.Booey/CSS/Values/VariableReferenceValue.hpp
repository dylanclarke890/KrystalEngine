#pragma once

#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyId.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Booey/CSS/Values/VariableData.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/Pair.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::style
{
  class Builder;
}

namespace krys::boo::css
{
  /// @brief A value containing `var()` or `env()` references, kept as tokens until it's resolved against a
  /// style.
  class VariableReferenceValue final : public Value
  {
  private:
    /// @brief A value that is only `var(--name)` or `env(name)`, without a fallback.
    struct SimpleReference
    {
      CSSOMStringAtom Name;
      css::ValueId FunctionId;
    };

    enum class FallbackResult : uint8
    {
      None,
      Valid,
      Invalid
    };

    const Ref<VariableData> _data;
    mutable CSSOMString _stringValue;

    // For quickly resolving simple var(--foo) values.
    Maybe<SimpleReference> _simpleReference;

    mutable RefPtr<VariableData> _cacheDependencyData;
    mutable RefPtr<Value> _cachedValue;
#if KRYS_DEBUG
    mutable css::PropertyId _cachePropertyId {css::PropertyId::Invalid};
#endif

    explicit VariableReferenceValue(Ref<VariableData> &&data) noexcept;

  public:
    /// @brief The maximum number of tokens that may be produced by a var() reference or var() fallback value.
    /// @note https://drafts.csswg.org/css-variables/#long-variables
    constexpr static size_t MaxSubstitutionTokens = 65'536uz;

    KRYS_NODISCARD static Ref<VariableReferenceValue> Create(const css::TokenRange &range,
                                                             const ParserContext &context) noexcept;
    KRYS_NODISCARD static Ref<VariableReferenceValue> Create(Ref<VariableData> &&data) noexcept;

    KRYS_NODISCARD bool Equals(const VariableReferenceValue &other) const noexcept;
    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept;

    KRYS_NODISCARD RefPtr<VariableData> ResolveVariableReferences(style::Builder &builder) const noexcept;

    KRYS_NODISCARD const ParserContext &Context() const noexcept;

    KRYS_NODISCARD RefPtr<Value> ResolveSingleValue(style::Builder &builder,
                                                    css::PropertyId propertyId) const noexcept;

    KRYS_NODISCARD const VariableData &Data() const noexcept
    {
      return *_data;
    }

    template <typename TCacheFunction>
    KRYS_NODISCARD bool
      ResolveAndCacheValue(style::Builder &builder,
                           KRYS_NOESCAPE const TCacheFunction &cacheFunction) const noexcept;

  private:
    KRYS_NODISCARD Maybe<List<Token>> ResolveTokenRange(css::TokenRange range,
                                                        style::Builder &builder) const noexcept;
    KRYS_NODISCARD bool ResolveVariableReference(css::TokenRange range, css::ValueId functionId,
                                                 List<Token> &tokens, style::Builder &builder) const noexcept;
    KRYS_NODISCARD Pair<FallbackResult, List<Token>>
      ResolveVariableFallback(const CSSOMStringAtom &variableName, css::TokenRange range,
                              css::ValueId functionId, style::Builder &builder) const noexcept;

    void CacheSimpleReference() noexcept;
    KRYS_NODISCARD RefPtr<VariableData> TryResolveSimpleReference(style::Builder &builder) const noexcept;
  };

  template <typename TCacheFunction>
  bool VariableReferenceValue::ResolveAndCacheValue(
    style::Builder &builder, KRYS_NOESCAPE const TCacheFunction &cacheFunction) const noexcept
  {
    if (auto data = TryResolveSimpleReference(builder))
    {
      bool equalData =
        _cacheDependencyData.get() == data.get() || (_cacheDependencyData && *_cacheDependencyData == *data);
      if (!equalData)
      {
        cacheFunction(data);
      }
      _cacheDependencyData = krys::move(data);
      return true;
    }

    auto resolvedTokens = ResolveTokenRange(_data->TokenRange(), builder);
    if (!resolvedTokens)
    {
      return false;
    }

    if (!_cacheDependencyData || _cacheDependencyData->Tokens() != *resolvedTokens)
    {
      _cacheDependencyData = VariableData::Create(css::TokenRange(*resolvedTokens), Context());
      cacheFunction(_cacheDependencyData);
    }
    return true;
  }
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::VariableReferenceValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsVariableReferenceValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
