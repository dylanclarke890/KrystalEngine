#pragma once

#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/IterationStatus.hpp"
#include "Krystal.Web/Css/Types/CSSOMString.hpp"
#include "Krystal.Web/Css/Values/Units.hpp"
#include "Krystal.Web/Css/Values/ValueSeparator.hpp"
#include "Krystal.Web/Css/Values/ValueType.hpp"
#include <new>

namespace krys::web
{
  class CachedResource;
}

namespace krys::web::css
{
  class StaticValuePool;
  class ToLengthConversionData;

  struct ComputedStyleDependencies;
  struct SerialisationContext;

  enum class ValueId : uint16;

  namespace detail
  {
    /// @brief What a `PrimitiveValue` holds. A number's unit is kept beside it; the other kinds have none.
    enum class PrimitiveKind : uint8
    {
      Number,
      String,
      CustomIdent,
      FontFamily,
      PropertyId,
      ValueId,
      Calc,
    };

    consteval PrimitiveKind MaxEnumValue(PrimitiveKind) noexcept
    {
      return PrimitiveKind::Calc;
    }
  }

  /// @brief A CSS value: specified, computed or declared, as parsed from a style sheet or built by code.
  /// @details Values are reference counted and immutable once built. There are no virtual functions: each
  /// class has a `ValueType`, and the operations below switch on it to call the class's own version
  /// (`CustomCSSText`, `Equals`, and the optional `AddDerivedHash`, `CustomVisitChildren`,
  /// `CustomMayDependOnBaseUrl` and `CustomTraverseSubresources`). Destruction dispatches the same way.
  /// @see https://www.w3.org/TR/css-values-4/
  class Value
  {
    friend class StaticValuePool;

    KRYS_NON_COPYABLE(Value);

  protected:
    /// @brief Packed state: the class's tag and flags, and room that some classes use for their own
    /// state so that it costs them no space.
    struct Header
    {
      // Ordered to pack into three bytes.
      ValueType Type : EnumBitWidth<ValueType>;
      /// @brief Used by the lists, `ValuePair` and `QuadValue`.
      ValueSeparator Separator : EnumBitWidth<ValueSeparator> {ValueSeparator::Space};
      /// @brief Used by `PrimitiveValue`.
      detail::PrimitiveKind PrimitiveKind
        : EnumBitWidth<detail::PrimitiveKind> {detail::PrimitiveKind::Number};
      /// @brief The `initial` a declaration block fills in for a longhand that a shorthand left out.
      bool IsImplicitInitialValue : 1 {false};
      /// @brief Used by `ValuePair` and `QuadValue`: whether equal values serialise as one.
      bool CoalesceIdenticalValues : 1 {true};
      /// @brief Used by `PrimitiveValue`.
      UnitType Unit : EnumBitWidth<UnitType> {UnitType::Unknown};
    };

  private:
    mutable uint32 _refCount {1u};

  protected:
    Header _header;

  private:
    // A value of the static pool: never destroyed and never counted, so threads can share it without writing
    // to it.
    bool _isStatic {false};

  public:
    // Reference counting, for Ref and RefPtr.

    void AddRef() const noexcept
    {
      if (!_isStatic)
      {
        ++_refCount;
      }
    }

    void SubRef() const noexcept
    {
      if (_isStatic)
      {
        return;
      }

      krys_debug_assert(_refCount > 0u);
      if (--_refCount == 0u)
      {
        delete const_cast<Value *>(this);
      }
    }

    KRYS_NODISCARD bool HasOneRef() const noexcept
    {
      return _refCount == 1u;
    }

    KRYS_NODISCARD uint32 GetRefCount() const noexcept
    {
      return _refCount;
    }

    KRYS_NODISCARD bool IsStatic() const noexcept
    {
      return _isStatic;
    }

    // The class and its flags.

    KRYS_NODISCARD ValueType Type() const noexcept
    {
      return _header.Type;
    }

    KRYS_NODISCARD bool IsImplicitInitialValue() const noexcept
    {
      return _header.IsImplicitInitialValue;
    }

    /// @brief Whether this is an `<image>`: a URL image, an `image-set()` or a generated image.
    /// @see https://www.w3.org/TR/css-images-4/#image-values
    KRYS_NODISCARD bool IsImage() const noexcept
    {
      return _header.Type == ValueType::Image || _header.Type == ValueType::ImageSet
             || IsImageGeneratorValue();
    }

    /// @brief Whether this is an image drawn from a description rather than loaded: a gradient, `paint()`,
    /// `cross-fade()` or `filter()`.
    KRYS_NODISCARD bool IsImageGeneratorValue() const noexcept
    {
      switch (_header.Type)
      {
        case ValueType::CrossFade:
        case ValueType::FilterImage:
        case ValueType::Gradient:
        case ValueType::PaintImage:  return true;
        default:                     return false;
      }
    }

    /// @brief Whether the class derives from `ValueContainingList`.
    KRYS_NODISCARD bool ContainsValueList() const noexcept
    {
      switch (_header.Type)
      {
        case ValueType::Function:
        case ValueType::GridAutoRepeat:
        case ValueType::GridIntegerRepeat:
        case ValueType::ImageSet:
        case ValueType::Subgrid:
        case ValueType::TransformList:
        case ValueType::ValueList:         return true;
        default:                           return false;
      }
    }

    /// @brief Whether this is a `var()` reference, or a longhand waiting for its shorthand's.
    KRYS_NODISCARD bool HasVariableReferences() const noexcept
    {
      return _header.Type == ValueType::VariableReference || _header.Type == ValueType::PendingSubstitution;
    }

    // Serialisation, comparison and hashing.

    /// @see https://www.w3.org/TR/cssom-1/#serialize-a-css-value
    KRYS_NODISCARD CSSOMString CssText(const SerialisationContext &context) const noexcept;

    /// @brief Whether the values are of the same class and equal, or one is a list holding just the other.
    KRYS_NODISCARD bool Equals(const Value &other) const noexcept;

    KRYS_NODISCARD bool operator==(const Value &other) const noexcept
    {
      return Equals(other);
    }

    /// @brief Adds the value's contents to @p seed, for deduplicating declaration blocks.
    /// @return False if the value's class has no `AddDerivedHash`, so it can't be hashed.
    KRYS_NODISCARD bool AddHash(size_t &seed) const noexcept;

    // What computing the value needs.

    /// @brief The properties and environment the value needs to be computed, such as `font-size` for `em`.
    KRYS_NODISCARD css::ComputedStyleDependencies ComputedStyleDependencies() const noexcept;

    void CollectComputedStyleDependencies(css::ComputedStyleDependencies &dependencies) const noexcept;

    /// @brief Whether @p conversionData has everything the value's dependencies need.
    KRYS_NODISCARD bool
      CanResolveDependenciesWith(const ToLengthConversionData &conversionData) const noexcept;

    /// @brief Whether the value holds a URL that would resolve differently against another base URL.
    KRYS_NODISCARD bool MayDependOnBaseUrl() const noexcept;

    // Walking.

    /// @brief Calls @p func with each value this one is made of, until it returns `Done`.
    KRYS_NODISCARD IterationStatus VisitChildren(const Func<IterationStatus(Value &)> &func) const noexcept;

    /// @brief Calls @p handler with each loaded resource the value uses, until it returns true.
    /// @return True if @p handler did.
    KRYS_NODISCARD bool
      TraverseSubresources(const Func<bool(const CachedResource &)> &handler) const noexcept;

    // Shortcuts into PrimitiveValue and ValuePair, defined with them.

    /// @brief Whether this is a `PrimitiveValue` holding a keyword.
    KRYS_NODISCARD inline bool IsValueId() const noexcept;

    /// @brief The keyword of a `PrimitiveValue` holding one, otherwise `ValueId::Invalid`.
    KRYS_NODISCARD inline css::ValueId ValueId() const noexcept;

    /// @brief Whether this is a `PrimitiveValue` holding a custom identifier.
    KRYS_NODISCARD inline bool IsCustomIdent() const noexcept;

    /// @pre This is a `ValuePair`.
    KRYS_NODISCARD inline const Value &First() const noexcept;

    /// @pre This is a `ValuePair`.
    KRYS_NODISCARD inline const Value &Second() const noexcept;

  protected:
    explicit Value(ValueType type) noexcept : _header {.Type = type}
    {
    }

    /// @brief Selects the constructors of the static pool's values.
    struct StaticValueTag
    {
    };

    constexpr static StaticValueTag StaticValue {};

    /// @brief Marks a value of the static pool, so that it is never destroyed.
    void MakeStatic() noexcept
    {
      _isStatic = true;
    }

    /// @brief Destroys the value as its own class, found from its `ValueType`.
    void operator delete(Value *value, std::destroying_delete_t) noexcept;

    KRYS_NODISCARD ValueSeparator Separator() const noexcept
    {
      return _header.Separator;
    }

  private:
    /// @brief Calls @p visitor with this value as its own class.
    template <typename Visitor>
    constexpr decltype(auto) VisitDerived(Visitor &&visitor) noexcept;

    template <typename Visitor>
    constexpr decltype(auto) VisitDerived(Visitor &&visitor) const noexcept;
  };
}
