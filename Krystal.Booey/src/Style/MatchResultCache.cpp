#include "Krystal.Booey/Style/MatchResultCache.hpp"
#include "Krystal.Booey/CSS/Properties/MutableStyleProperties.hpp"
#include "Krystal.Booey/DOM/StyledElement.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+SettersInlines.hpp"
#include "Krystal.Booey/Style/MatchResult.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::style
{
  // Cache entries contain a MatchResult object that references element's mutable inline style.
  // As a result the cache entry mutates when element's inline style mutates.
  // We save the original properties and values so we can check which properties have changed.
  // If a property changes we null the value and assume it will change in the future too.
  //
  // It would be nicer if the cache entries were immutable but doing that in a sufficiently
  // performant way is tricky.

  namespace
  {
    struct OriginalInlineProperty
    {
      css::PropertyId PropertyId;
      RefPtr<const css::Value> ValueIfUnchanged;
    };

    KRYS_NODISCARD UnadjustedStyle Copy(const UnadjustedStyle &other) noexcept
    {
      return {.Style = RenderStyle::ClonePtr(*other.Style),
              .Relations =
                other.Relations ? CreateUnique<Relations>(*other.Relations) : UniquePtr<Relations> {},
              .MatchResult = other.MatchResult};
    }
  }

  struct MatchResultCache::Entry
  {
    // Entries are keyed by address: the element they were made for, so a new element at the same address
    // doesn't use them.
    WeakPtr<const dom::Element> Element;
    style::UnadjustedStyle UnadjustedStyle;
    Ref<const css::MutableStyleProperties> InlineStyle;
    List<OriginalInlineProperty> OriginalInlineProperties;

    Entry(const dom::Element &element, style::UnadjustedStyle &&unadjustedStyle,
          const css::MutableStyleProperties &inlineStyle) noexcept
        : Element(WeakPtr<const dom::Element>::WithRef(element)),
          UnadjustedStyle(krys::move(unadjustedStyle)), InlineStyle(ShareRef(inlineStyle))
    {
      OriginalInlineProperties.reserve(inlineStyle.PropertyCount());
      for (auto property : inlineStyle)
      {
        OriginalInlineProperties.push_back(
          {.PropertyId = property.Id(), .ValueIfUnchanged = ShareRefPtr(property.Value())});
      }
    }
  };

  MatchResultCache::MatchResultCache() noexcept = default;
  MatchResultCache::~MatchResultCache() noexcept = default;

  bool
    MatchResultCache::IsUsableAfterInlineStyleChange(const MatchResultCache::Entry &entry,
                                                     const css::StyleProperties &currentInlineStyle) noexcept
  {
    if (entry.InlineStyle.get() != &currentInlineStyle)
    {
      return false;
    }

    auto &inlineStyle = *entry.InlineStyle;

    // Only allow the same exact properties after a change. This way the previous values in RenderStyle are
    // guranteed to get overwritten. Adding properties could be allowed without other changes. Removal would
    // require resetting the removed property to initial value in the style builder.

    auto size = entry.OriginalInlineProperties.size();
    if (size != inlineStyle.PropertyCount())
    {
      return false;
    }

    for (size_t index = 0; index < size; ++index)
    {
      if (entry.OriginalInlineProperties[index].PropertyId
          != inlineStyle.PropertyAt(static_cast<uint32>(index)).Id())
      {
        return false;
      }
    }
    return true;
  }

  PropertyCascade::IncludedProperties
    MatchResultCache::ComputeAndUpdateChangedProperties(MatchResultCache::Entry &entry) noexcept
  {
    auto &originalProperties = entry.OriginalInlineProperties;
    auto &inlineStyle = *entry.InlineStyle;

    PropertyCascade::IncludedProperties result;

    auto size = originalProperties.size();
    for (size_t index = 0; index < size; ++index)
    {
      auto currentProperty = inlineStyle.PropertyAt(static_cast<uint32>(index));
      auto propertyId = currentProperty.Id();

      krys_debug_assert(originalProperties[index].PropertyId == propertyId);

      if (originalProperties[index].ValueIfUnchanged.get() == currentProperty.Value())
      {
        continue;
      }

      // Assume that if a value changes ones then it will change more. Don't track changes anymore.
      originalProperties[index].ValueIfUnchanged = nullptr;

      // FIXME: Support custom properties.
      if (propertyId == css::PropertyId::Custom)
      {
        return PropertyCascade::NormalProperties();
      }

      // Only use partial applying with low-priority properties since we know changes to them can't
      // affect values of other properties.
      // FIXME: CSSPropertyLineHeight shouldn't be a low-priority property as others can depend on it via `lh`
      // unit.
      if (propertyId < css::FirstLowPriorityProperty || propertyId == css::PropertyId::LineHeight)
      {
        return PropertyCascade::NormalProperties();
      }

      result.Ids.push_back(propertyId);
    }

    return result;
  }

  auto MatchResultCache::Find(const dom::Element &element) noexcept -> Entry *
  {
    auto it = _entries.find(&element);
    if (it == _entries.end())
    {
      return nullptr;
    }
    if (it->second->Element.get() != &element)
    {
      _entries.erase(it);
      return nullptr;
    }
    return it->second.get();
  }

  Maybe<CachedMatchResult>
    MatchResultCache::ResultWithCurrentInlineStyle(const dom::Element &element) noexcept
  {
    auto *entry = Find(element);
    if (!entry)
    {
      return {};
    }

    auto *styledElement = DynamicDowncast<dom::StyledElement>(element);
    auto *inlineStyle = styledElement ? styledElement->InlineStyle() : nullptr;

    if (!inlineStyle || !IsUsableAfterInlineStyleChange(*entry, *inlineStyle))
    {
      _entries.erase(&element);
      return {};
    }

    auto changedProperties = ComputeAndUpdateChangedProperties(*entry);

    return CachedMatchResult {.UnadjustedStyle = Copy(entry->UnadjustedStyle),
                              .ChangedProperties = krys::move(changedProperties),
                              .StyleToUpdate = entry->UnadjustedStyle.Style.get()};
  }

  void MatchResultCache::Update(CachedMatchResult &result, const RenderStyle &style) noexcept
  {
    *result.StyleToUpdate = RenderStyle::Clone(style);
  }

  void MatchResultCache::UpdateForFastPathInherit(const dom::Element &element,
                                                  const RenderStyle &parentStyle) noexcept
  {
    auto *entry = Find(element);
    if (!entry)
    {
      return;
    }
    entry->UnadjustedStyle.Style->FastPathInheritFrom(parentStyle);
  }

  void MatchResultCache::Set(const dom::Element &element, const UnadjustedStyle &unadjustedStyle) noexcept
  {
    // For now we cache match results if there is mutable inline style. This way we can avoid
    // selector matching when it gets mutated again.
    auto *styledElement = DynamicDowncast<dom::StyledElement>(element);
    auto *inlineStyle =
      styledElement ? DynamicDowncast<css::MutableStyleProperties>(styledElement->InlineStyle()) : nullptr;

    if (inlineStyle)
    {
      _entries.insert_or_assign(&element, CreateUnique<Entry>(element, Copy(unadjustedStyle), *inlineStyle));
      constexpr uint32 setsBetweenPrunes = 64;
      if (++_setsSinceLastPrune >= setsBetweenPrunes)
      {
        PruneDeadEntries();
      }
    }
    else
    {
      _entries.erase(&element);
    }
  }

  void MatchResultCache::PruneDeadEntries() noexcept
  {
    std::erase_if(_entries, [](auto &keyValue) { return keyValue.second->Element.get() != keyValue.first; });
    _setsSinceLastPrune = 0;
  }
}
