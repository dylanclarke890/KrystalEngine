#pragma once

#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  /// @brief Two values separated by a space or a slash, e.g. `10px 20px` or `1 / 2`.
  class ValuePair final : public Value
  {
    friend class Value;

    enum class IdenticalValueSerialisation : bool
    {
      DoNotCoalesce,
      Coalesce
    };

  private:
    // FIXME: Store coalesce bit in Value to cut down on object size.
    bool _coalesceIdenticalValues {true};
    const Ref<Value> _first;
    const Ref<Value> _second;

    ValuePair(ValueSeparator separator, Ref<Value> first, Ref<Value> second,
              IdenticalValueSerialisation serialisation) noexcept;

  public:
    /// @brief A space-separated pair that serialises as a single value when both values serialise the same.
    KRYS_NODISCARD static Ref<ValuePair> Create(Ref<Value> first, Ref<Value> second) noexcept;
    /// @brief A slash-separated pair that always serialises both values.
    KRYS_NODISCARD static Ref<ValuePair> CreateSlashSeparated(Ref<Value> first, Ref<Value> second) noexcept;
    /// @brief A space-separated pair that always serialises both values.
    KRYS_NODISCARD static Ref<ValuePair> CreateNonCoalescing(Ref<Value> first, Ref<Value> second) noexcept;

    KRYS_NODISCARD const Value &First() const noexcept
    {
      return *_first;
    }

    KRYS_NODISCARD Value &First() noexcept
    {
      return *_first;
    }

    KRYS_NODISCARD const Value &Second() const noexcept
    {
      return *_second;
    }

    KRYS_NODISCARD Value &Second() noexcept
    {
      return *_second;
    }

    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept;
    KRYS_NODISCARD bool Equals(const ValuePair &other) const noexcept;

    /// @brief Whether the pair coalesces identical values and its values are equal.
    KRYS_NODISCARD bool CanBeCoalesced() const noexcept;

    KRYS_NODISCARD IterationStatus
      CustomVisitChildren(KRYS_NOESCAPE const Func<IterationStatus(css::Value &)> &func) const noexcept
    {
      if (func(*_first) == IterationStatus::Done)
      {
        return IterationStatus::Done;
      }
      if (func(*_second) == IterationStatus::Done)
      {
        return IterationStatus::Done;
      }
      return IterationStatus::Continue;
    }

  private:
    KRYS_NODISCARD bool AddDerivedHash(size_t &seed) const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::ValuePair)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsValuePairValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()

namespace krys::boo::css
{
  inline const Value &Value::First() const noexcept
  {
    return Downcast<ValuePair>(*this).First();
  }

  inline const Value &Value::Second() const noexcept
  {
    return Downcast<ValuePair>(*this).Second();
  }
}
