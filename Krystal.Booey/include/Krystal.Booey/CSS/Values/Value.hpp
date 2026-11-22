/*
 * (C) 1999-2003 Lars Knoll (knoll@kde.org)
 * Copyright (C) 2004-2023 Apple Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/CSS/Values/Units.hpp"
#include "Krystal.Booey/CSS/Values/ValueSeparator.hpp"
#include "Krystal.Booey/CSS/Values/ValueType.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Containers/SmallList.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/IterationStatus.hpp"

namespace krys::boo
{
  class CSSStyleDeclaration;
  class CachedResource;
}

namespace krys::boo::css
{
  class PrimitiveValue;
  class StaticValuePool;
  class ToLengthConversionData;

  struct ComputedStyleDependencies;
  struct SerialisationContext;

  enum class PropertyId : uint16;
  enum class ValueId : uint16;

  namespace detail
  {
    // What a PrimitiveValue holds; a number's unit is in `_unit`, which is `Unknown` for the other kinds.
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

  class Value
  {
    friend class StaticValuePool;

    KRYS_NON_COPYABLE(Value);

  public:
    KRYS_TYPE_CAST_TRAITS_ACCESS();

  private:
    // The count goes up and down in steps of two, leaving the lowest bit to mark static values, whose count
    // never reaches zero.
    constexpr static uint32 RefCountFlagIsStatic = 1u;
    constexpr static uint32 RefCountIncrement = 2u;

    mutable uint32 _refCount {RefCountIncrement};

  protected:
    ValueType _type : EnumBitWidth<ValueType>;

    detail::PrimitiveKind _primitiveKind
      : EnumBitWidth<detail::PrimitiveKind> {detail::PrimitiveKind::Number};
    UnitType _unit : EnumBitWidth<UnitType> {UnitType::Unknown};
    ValueSeparator _valueSeparator : EnumBitWidth<ValueSeparator> {ValueSeparator::Space};
    bool _isImplicitInitialValue : 1 {false};
    mutable bool _hasCachedCSSText : 1 {false};
    // Whether a `ValuePair` or `QuadValue` serialises as one value when its values serialise the same. Kept
    // in the bit fields so it never costs them space of their own.
    bool _coalesceIdenticalValues : 1 {true};

    explicit Value(ValueType type) noexcept : _type(type)
    {
    }

    struct StaticValueTag
    {
    };

    constexpr static StaticValueTag StaticValue {};

    /// @brief Marks a value of the static value pool, which is never destroyed.
    void MakeStatic() noexcept
    {
      _refCount |= RefCountFlagIsStatic;
    }

  public:
    void AddRef() const noexcept
    {
      _refCount += RefCountIncrement;
    }

    void SubRef() const noexcept
    {
      krys_debug_assert(_refCount >= RefCountIncrement);
      if (_refCount -= RefCountIncrement; !_refCount)
      {
        delete const_cast<Value *>(this);
      }
    }

    KRYS_NODISCARD bool HasOneRef() const noexcept
    {
      return GetRefCount() == 1u;
    }

    KRYS_NODISCARD uint32 GetRefCount() const noexcept
    {
      return _refCount / RefCountIncrement;
    }

    KRYS_NODISCARD bool IsStatic() const noexcept
    {
      return _refCount & RefCountFlagIsStatic;
    }

    // NOTE: intentionally not virtual for performance reasons.
    // virtual ~Value() noexcept;

    KRYS_NODISCARD ValueType Type() const noexcept
    {
      return _type;
    }

    KRYS_NODISCARD bool IsImplicitInitialValue() const noexcept
    {
      return _isImplicitInitialValue;
    }

    KRYS_NODISCARD IterationStatus VisitChildren(const Func<IterationStatus(Value &)> &) const noexcept;

    KRYS_NODISCARD bool Equals(const Value &other) const noexcept;

    KRYS_NODISCARD bool operator==(const Value &other) const noexcept
    {
      return Equals(other);
    }

    void CollectComputedStyleDependencies(css::ComputedStyleDependencies &dependencies) const noexcept;

    KRYS_NODISCARD css::ComputedStyleDependencies ComputedStyleDependencies() const noexcept;

    KRYS_NODISCARD bool CanResolveDependenciesWith(const ToLengthConversionData &) const noexcept;

    KRYS_NODISCARD CSSOMString CssText(const SerialisationContext &context) const noexcept;

    KRYS_NODISCARD bool
      TraverseSubresources(const Func<bool(const CachedResource &)> &handler) const noexcept;

    KRYS_NODISCARD bool MayDependOnBaseUrl() const noexcept;

    /// @brief Returns false if the hash is computed from the Value pointer instead of the underlying values.
    KRYS_NODISCARD bool AddHash(size_t &seed) const noexcept;

    /// @brief Empty URLs and fragment-only URLs should not be resolved relative to the base URL.
    /// @see https://www.w3.org/TR/css-values-4/#local-urls
    KRYS_NODISCARD static bool IsCSSLocalUrl(CSSOMStringView relativeUrl) noexcept;

    KRYS_NODISCARD static CSSOMStringView SeparatorCSSText(ValueSeparator separator) noexcept;

    KRYS_NODISCARD static bool CustomMayDependOnBaseUrl() noexcept
    {
      return false;
    }

    KRYS_NODISCARD static IterationStatus CustomVisitChildren(const Func<IterationStatus(Value &)> &) noexcept
    {
      return IterationStatus::Continue;
    }

#pragma region Type Checks

    KRYS_NODISCARD bool HasVariableReferences() const noexcept
    {
      return _type == ValueType::VariableReference || _type == ValueType::PendingSubstitution;
    }

    KRYS_NODISCARD bool IsImageGeneratorValue() const noexcept
    {
      return _type >= ValueType::PaintImage && _type <= ValueType::Gradient;
    }

    /// @brief True for all image-like values except CursorImageValues; these are the values that correspond
    /// to the CSS <image> production.
    KRYS_NODISCARD bool IsImage() const noexcept
    {
      return _type == ValueType::Image || _type == ValueType::ImageSet || IsImageGeneratorValue();
    }

    KRYS_NODISCARD bool ContainsValueList() const noexcept
    {
      return _type >= ValueType::ValueList;
    }

#pragma endregion

    KRYS_NODISCARD inline bool IsValueId() const noexcept;

    KRYS_NODISCARD inline css::ValueId ValueId() const noexcept;

    KRYS_NODISCARD inline bool IsCustomIdent() const noexcept;

    KRYS_NODISCARD inline CSSOMString CustomIdent() const noexcept;

    KRYS_NODISCARD inline bool IsString() const noexcept;

    KRYS_NODISCARD inline CSSOMString String() const noexcept;

    KRYS_NODISCARD inline bool IsInteger() const noexcept;

    KRYS_NODISCARD inline int64 Integer(const ToLengthConversionData &) const noexcept;

    KRYS_NODISCARD inline const Value &First() const noexcept; // ValuePair

    KRYS_NODISCARD inline const Value &Second() const noexcept; // ValuePair

  protected:
    void operator delete(Value *, std::destroying_delete_t) noexcept;

    KRYS_NODISCARD ValueSeparator Separator() const noexcept
    {
      return _valueSeparator;
    }

    KRYS_NODISCARD CSSOMStringView SeparatorCSSText() const noexcept
    {
      return SeparatorCSSText(Separator());
    }

  private:
    template <typename Visitor>
    constexpr decltype(auto) VisitDerived(Visitor &&) noexcept;

    template <typename Visitor>
    constexpr decltype(auto) VisitDerived(Visitor &&) const noexcept;

    KRYS_NODISCARD static bool
      CustomTraverseSubresources(const Func<bool(const CachedResource &)> &func) noexcept;

    // FIXME: Add custom hash functions for all derived classes and remove this function.
    KRYS_NODISCARD bool AddDerivedHash(size_t &seed) const noexcept;
  };

  template <typename TValue>
  KRYS_NODISCARD bool CompareValueList(const SmallList<Ref<TValue>> &firstVector,
                                       const SmallList<Ref<TValue>> &secondVector) noexcept
  {
    size_t size = firstVector.size();
    if (size != secondVector.size())
    {
      return false;
    }

    for (size_t i = 0uz; i < size; ++i)
    {
      const Ref<TValue> &firstPtr = firstVector[i];
      const Ref<TValue> &secondPtr = secondVector[i];
      if (firstPtr.get() == secondPtr.get() || firstPtr->Equals(*secondPtr))
      {
        continue;
      }

      return false;
    }

    return true;
  }

  template <typename TValue>
  KRYS_NODISCARD bool CompareValuePtr(const RefPtr<TValue> &first, const RefPtr<TValue> &second) noexcept
  {
    return first ? second && first->Equals(*second) : !second;
  }

  template <typename TValue>
  KRYS_NODISCARD bool CompareValue(const Ref<TValue> &first, const Ref<TValue> &second) noexcept
  {
    return first->Equals(*second);
  }
}
