#include "Krystal.Booey/Graphics/Filters/FilterResults.hpp"
#include "Krystal.Booey/Graphics/Filters/FilterEffect.hpp"

namespace krys::boo
{
  FilterResults::FilterResults(AcceleratedApplierCreator &&acceleratedApplierCreator) noexcept
      : _acceleratedApplierCreator(krys::move(acceleratedApplierCreator))
  {
  }

  FilterResults::~FilterResults() noexcept = default;

  UniquePtr<FilterEffectApplier>
    FilterResults::CreateAcceleratedApplier(const FilterEffect &effect) const noexcept
  {
    if (!_acceleratedApplierCreator)
    {
      return nullptr;
    }
    return _acceleratedApplierCreator(effect);
  }

  FilterImage *FilterResults::EffectResult(FilterEffect &effect) const noexcept
  {
    auto it = _results.find(ShareRef(effect));
    return it == _results.end() ? nullptr : it->second.get();
  }

  size_t FilterResults::MemoryCost() const noexcept
  {
    size_t memoryCost = 0u;
    for (const auto &[effect, result] : _results)
    {
      memoryCost += result->MemoryCost();
    }
    return memoryCost;
  }

  bool FilterResults::CanCacheResult(const FilterImage &result) const noexcept
  {
    static constexpr size_t maxAllowedMemoryCost = 100u * 1'024u * 1'024u;
    return MemoryCost() + result.MemoryCost() <= maxAllowedMemoryCost;
  }

  void FilterResults::SetEffectResult(FilterEffect &effect, Span<const Ref<FilterImage>> inputs,
                                      Ref<FilterImage> &&result) noexcept
  {
    if (!CanCacheResult(*result))
    {
      return;
    }

    _results.insert_or_assign(ShareRef(effect), krys::move(result));

    for (const auto &input : inputs)
    {
      _resultReferences[input].insert(ShareRef(effect));
    }
  }

  void FilterResults::ClearEffectResult(FilterEffect &effect) noexcept
  {
    auto it = _results.find(ShareRef(effect));
    if (it == _results.end())
    {
      return;
    }

    auto result = it->second;
    _results.erase(it);

    auto references = _resultReferences.find(result);
    if (references == _resultReferences.end())
    {
      return;
    }
    auto dependents = krys::move(references->second);
    _resultReferences.erase(references);
    for (const auto &reference : dependents)
    {
      ClearEffectResult(*reference);
    }
  }
}
