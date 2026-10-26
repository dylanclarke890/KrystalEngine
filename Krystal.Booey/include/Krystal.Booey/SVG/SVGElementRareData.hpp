#pragma once

#include "Krystal.Booey/SVG/SVGElement.hpp"
#include "Krystal.Booey/SVG/SVGTests.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"
#include <algorithm>

namespace krys::boo::svg
{
  class SVGElement;

  /// @brief The state of an SVG element that few elements need: its `<use>` instances, the elements that
  /// refer to it, and its conditional processing attributes.
  class SVGElementRareData
  {
    KRYS_NON_COPYABLE(SVGElementRareData);

  private:
    List<WeakPtr<SVGElement>> _referencingElements;
    WeakPtr<SVGElement> _referenceTarget;

    // FIXME: The CSS clients that refer to the element as a resource (SVGResourceElementClient), with
    // clipping and masking (B11f).

    List<WeakPtr<SVGElement>> _instances;
    WeakPtr<SVGElement> _correspondingElement;
    bool _instancesUpdatesBlocked : 1 {false};

    // FIXME: The animated SMIL style properties and the computed style without them (the base value of a
    // SMIL animation), with SMIL (B11i).

    UniquePtr<SVGConditionalProcessingAttributes> _conditionalProcessingAttributes;

  public:
    SVGElementRareData() noexcept = default;

    void AddInstance(SVGElement &element) noexcept
    {
      Add(_instances, element);
    }

    void RemoveInstance(SVGElement &element) noexcept
    {
      Remove(_instances, element);
    }

    KRYS_NODISCARD const List<WeakPtr<SVGElement>> &Instances() const noexcept
    {
      return _instances;
    }

    KRYS_NODISCARD bool InstanceUpdatesBlocked() const noexcept
    {
      return _instancesUpdatesBlocked;
    }

    void SetInstanceUpdatesBlocked(bool value) noexcept
    {
      _instancesUpdatesBlocked = value;
    }

    void AddReferencingElement(SVGElement &element) noexcept
    {
      Add(_referencingElements, element);
    }

    void RemoveReferencingElement(SVGElement &element) noexcept
    {
      Remove(_referencingElements, element);
    }

    KRYS_NODISCARD const List<WeakPtr<SVGElement>> &ReferencingElements() const noexcept
    {
      return _referencingElements;
    }

    KRYS_NODISCARD List<WeakPtr<SVGElement>> TakeReferencingElements() noexcept
    {
      return std::exchange(_referencingElements, {});
    }

    KRYS_NODISCARD SVGElement *ReferenceTarget() const noexcept
    {
      return _referenceTarget.get();
    }

    void SetReferenceTarget(WeakPtr<SVGElement> &&element) noexcept
    {
      _referenceTarget = krys::move(element);
    }

    KRYS_NODISCARD SVGElement *CorrespondingElement() const noexcept
    {
      return _correspondingElement.get();
    }

    void SetCorrespondingElement(SVGElement *correspondingElement) noexcept
    {
      _correspondingElement = CreateWeakPtr(correspondingElement);
    }

    KRYS_NODISCARD SVGConditionalProcessingAttributes *
      ConditionalProcessingAttributesIfExists() const noexcept
    {
      return _conditionalProcessingAttributes.get();
    }

    KRYS_NODISCARD SVGConditionalProcessingAttributes &
      ConditionalProcessingAttributes(SVGElement &contextElement) noexcept
    {
      if (!_conditionalProcessingAttributes)
      {
        _conditionalProcessingAttributes = CreateUnique<SVGConditionalProcessingAttributes>(contextElement);
      }
      return *_conditionalProcessingAttributes;
    }

  private:
    // The lists are sets: an element is in one at most once, and elements that have gone are dropped.
    static void Add(List<WeakPtr<SVGElement>> &elements, SVGElement &element) noexcept
    {
      std::erase_if(elements, [](const WeakPtr<SVGElement> &weakElement) { return !weakElement; });
      if (std::ranges::none_of(elements, [&](const WeakPtr<SVGElement> &weakElement)
                               { return weakElement.get() == &element; }))
      {
        elements.push_back(CreateWeakPtr(&element));
      }
    }

    static void Remove(List<WeakPtr<SVGElement>> &elements, SVGElement &element) noexcept
    {
      std::erase_if(elements, [&](const WeakPtr<SVGElement> &weakElement)
                    { return !weakElement || weakElement.get() == &element; });
    }
  };
}
