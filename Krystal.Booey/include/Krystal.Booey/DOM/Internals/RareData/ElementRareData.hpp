#pragma once

#include "Krystal.Booey/CSS/Calc/RandomCachingKeyMap.hpp"
#include "Krystal.Booey/DOM/Internals/RareData/ParentNodeRareData.hpp"
#include "Krystal.Booey/Style/PseudoElementIdentifier.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo
{
  class RenderStyle;
}

namespace krys::boo::dom
{
  class ElementRareData : public ParentNodeRareData
  {
    struct PseudoElementIdentifierHash
    {
      KRYS_NODISCARD size_t operator()(const Maybe<style::PseudoElementIdentifier> &identifier) const noexcept
      {
        return identifier ? DefaultHash<style::PseudoElementIdentifier>::Hash(*identifier) + 1 : 0;
      }
    };

  private:
    uint16 _childIndex {0};
    UniquePtr<RenderStyle> _computedStyle;
    UniquePtr<RenderStyle> _displayContentsOrNoneStyle;
    HashMap<Maybe<style::PseudoElementIdentifier>, Ref<css::calc::RandomCachingKeyMap>,
            PseudoElementIdentifierHash>
      _randomCachingKeyMaps;

  public:
    ElementRareData() noexcept;
    ~ElementRareData() noexcept;

    /// @brief The element's index among its parent's element children, plus one, cached by selector matching
    /// (zero if unknown).
    KRYS_NODISCARD uint32 ChildIndex() const noexcept
    {
      return _childIndex;
    }

    void SetChildIndex(uint32 index) noexcept
    {
      _childIndex = static_cast<uint16>(index);
    }

    /// @brief The element's style resolved outside style resolution (`Element::ComputedStyle`).
    KRYS_NODISCARD RenderStyle *ComputedStyle() const noexcept
    {
      return _computedStyle.get();
    }

    void SetComputedStyle(UniquePtr<RenderStyle> style) noexcept;

    /// @brief The style of an element with `display: contents` or `display: none`, which has no renderer to
    /// hold it.
    KRYS_NODISCARD RenderStyle *DisplayContentsOrNoneStyle() const noexcept
    {
      return _displayContentsOrNoneStyle.get();
    }

    void SetDisplayContentsOrNoneStyle(UniquePtr<RenderStyle> style) noexcept;

    /// @brief The `random()` base values of the element (or one of its pseudo-elements).
    KRYS_NODISCARD Ref<css::calc::RandomCachingKeyMap>
      EnsureRandomCachingKeyMap(const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier) noexcept
    {
      auto it = _randomCachingKeyMaps.find(pseudoElementIdentifier);
      if (it == _randomCachingKeyMaps.end())
      {
        it = _randomCachingKeyMaps.emplace(pseudoElementIdentifier, css::calc::RandomCachingKeyMap::Create())
               .first;
      }
      return it->second;
    }

    KRYS_NODISCARD bool HasRandomCachingKeyMap() const noexcept
    {
      return !_randomCachingKeyMaps.empty();
    }
  };
}
