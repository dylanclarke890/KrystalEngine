#pragma once

#include "Krystal.Booey/Graphics/Filters/FilterEffectApplier.hpp"
#include "Krystal.Booey/Graphics/Filters/FilterImage.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/HashSet.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/Span.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo
{
  class FilterEffect;

  /// @brief The results of a filter's effects as it's applied, so an effect read by several others is applied
  /// once; and the accelerated backend that applies them.
  class FilterResults
  {
  public:
    /// @brief Makes the applier of an effect for the accelerated backend; null when it can't apply it.
    using AcceleratedApplierCreator = Func<UniquePtr<FilterEffectApplier>(const FilterEffect &)>;

  private:
    // The value is a list of FilterEffects, whose FilterImages depend on the key FilterImage.
    using FilterEffectSet = HashSet<Ref<FilterEffect>>;

    HashMap<Ref<FilterEffect>, Ref<FilterImage>> _results;
    HashMap<Ref<FilterImage>, FilterEffectSet> _resultReferences;
    AcceleratedApplierCreator _acceleratedApplierCreator;

  public:
    explicit FilterResults(AcceleratedApplierCreator &&acceleratedApplierCreator = nullptr) noexcept;
    ~FilterResults() noexcept;

    KRYS_NODISCARD UniquePtr<FilterEffectApplier>
      CreateAcceleratedApplier(const FilterEffect &effect) const noexcept;

    KRYS_NODISCARD FilterImage *EffectResult(FilterEffect &effect) const noexcept;
    void SetEffectResult(FilterEffect &effect, Span<const Ref<FilterImage>> inputs,
                         Ref<FilterImage> &&result) noexcept;
    void ClearEffectResult(FilterEffect &effect) noexcept;

  private:
    KRYS_NODISCARD size_t MemoryCost() const noexcept;
    KRYS_NODISCARD bool CanCacheResult(const FilterImage &result) const noexcept;
  };
}
