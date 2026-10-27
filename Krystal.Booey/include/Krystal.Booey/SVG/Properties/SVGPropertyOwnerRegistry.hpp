#pragma once

#include "Krystal.Booey/SVG/Properties/SVGAnimatedPropertyAccessorImpl.hpp"
#include "Krystal.Booey/SVG/Properties/SVGAnimatedPropertyPairAccessorImpl.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyAccessorImpl.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyRegistry.hpp"
#include "Krystal.Booey/SVG/SVGNames.hpp"
#include "Krystal.Core/Concepts.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include "Krystal.Core/Utils/NeverDestroyed.hpp"
#include <algorithm>

namespace krys::boo::svg
{
  class SVGConditionalProcessingAttributes;

  template <typename T>
  concept HasFastPropertyForAttribute = requires(const T &element, const dom::QualifiedName &name) {
    { element.PropertyForAttribute(name) } -> SameType<SVGAnimatedProperty *>;
  };

  /// @brief The property registry of @p TOwner: the attributes its own properties reflect, each with the
  /// accessor that reaches the property, then those of @p TBaseTypes, the owner's bases that have properties.
  template <typename TOwner, typename... TBaseTypes>
  class SVGPropertyOwnerRegistry : public SVGPropertyRegistry
  {
    // Every owner type has one list, in the order its properties are registered.
    using AttributeNameToAccessorMap = List<Pair<dom::QualifiedName, const SVGMemberAccessor<TOwner> *>>;

  private:
    TOwner &_owner;

  public:
    explicit SVGPropertyOwnerRegistry(TOwner &owner) noexcept : _owner(owner)
    {
    }

    template <Ref<SVGStringList> SVGConditionalProcessingAttributes::*property>
    static void
      RegisterConditionalProcessingAttributeProperty(const dom::QualifiedName &attributeName) noexcept
    {
      RegisterProperty(attributeName,
                       SVGConditionalProcessingAttributeAccessor<TOwner>::template Singleton<property>());
    }

    template <Ref<SVGTransformList> TOwner::*property>
    static void RegisterProperty(const dom::QualifiedName &attributeName) noexcept
    {
      RegisterProperty(attributeName, SVGTransformListAccessor<TOwner>::template Singleton<property>());
    }

    template <Ref<SVGAnimatedBoolean> TOwner::*property>
    static void RegisterProperty(const dom::QualifiedName &attributeName) noexcept
    {
      RegisterProperty(attributeName, SVGAnimatedBooleanAccessor<TOwner>::template Singleton<property>());
    }

    template <typename TEnum, Ref<SVGAnimatedEnumeration> TOwner::*property>
    static void RegisterProperty(const dom::QualifiedName &attributeName) noexcept
    {
      RegisterProperty(attributeName,
                       SVGAnimatedEnumerationAccessor<TOwner, TEnum>::template Singleton<property>());
    }

    template <Ref<SVGAnimatedInteger> TOwner::*property>
    static void RegisterProperty(const dom::QualifiedName &attributeName) noexcept
    {
      RegisterProperty(attributeName, SVGAnimatedIntegerAccessor<TOwner>::template Singleton<property>());
    }

    template <Ref<SVGAnimatedLength> TOwner::*property>
    static void RegisterProperty(const dom::QualifiedName &attributeName) noexcept
    {
      RegisterProperty(attributeName, SVGAnimatedLengthAccessor<TOwner>::template Singleton<property>());
    }

    template <Ref<SVGAnimatedLengthList> TOwner::*property>
    static void RegisterProperty(const dom::QualifiedName &attributeName) noexcept
    {
      RegisterProperty(attributeName, SVGAnimatedLengthListAccessor<TOwner>::template Singleton<property>());
    }

    template <Ref<SVGAnimatedNumber> TOwner::*property>
    static void RegisterProperty(const dom::QualifiedName &attributeName) noexcept
    {
      RegisterProperty(attributeName, SVGAnimatedNumberAccessor<TOwner>::template Singleton<property>());
    }

    template <Ref<SVGAnimatedNumberList> TOwner::*property>
    static void RegisterProperty(const dom::QualifiedName &attributeName) noexcept
    {
      RegisterProperty(attributeName, SVGAnimatedNumberListAccessor<TOwner>::template Singleton<property>());
    }

    template <Ref<SVGAnimatedAngle> TOwner::*property1, Ref<SVGAnimatedOrientType> TOwner::*property2>
    static void RegisterProperty(const dom::QualifiedName &attributeName) noexcept
    {
      RegisterProperty(attributeName,
                       SVGAnimatedAngleOrientAccessor<TOwner>::template Singleton<property1, property2>());
    }

    // FIXME: <path>'s `d` as an animated property (SVGAnimatedPathSegList, without the segment list SVG 2
    // removed), with SMIL (B11i).

    template <Ref<SVGAnimatedPointList> TOwner::*property>
    static void RegisterProperty(const dom::QualifiedName &attributeName) noexcept
    {
      RegisterProperty(attributeName, SVGAnimatedPointListAccessor<TOwner>::template Singleton<property>());
    }

    template <Ref<SVGAnimatedPreserveAspectRatio> TOwner::*property>
    static void RegisterProperty(const dom::QualifiedName &attributeName) noexcept
    {
      RegisterProperty(attributeName,
                       SVGAnimatedPreserveAspectRatioAccessor<TOwner>::template Singleton<property>());
    }

    template <Ref<SVGAnimatedRect> TOwner::*property>
    static void RegisterProperty(const dom::QualifiedName &attributeName) noexcept
    {
      RegisterProperty(attributeName, SVGAnimatedRectAccessor<TOwner>::template Singleton<property>());
    }

    template <Ref<SVGAnimatedString> TOwner::*property>
    static void RegisterProperty(const dom::QualifiedName &attributeName) noexcept
    {
      RegisterProperty(attributeName, SVGAnimatedStringAccessor<TOwner>::template Singleton<property>());
    }

    template <Ref<SVGAnimatedTransformList> TOwner::*property>
    static void RegisterProperty(const dom::QualifiedName &attributeName) noexcept
    {
      RegisterProperty(attributeName,
                       SVGAnimatedTransformListAccessor<TOwner>::template Singleton<property>());
    }

    template <Ref<SVGAnimatedInteger> TOwner::*property1, Ref<SVGAnimatedInteger> TOwner::*property2>
    static void RegisterProperty(const dom::QualifiedName &attributeName) noexcept
    {
      RegisterProperty(attributeName,
                       SVGAnimatedIntegerPairAccessor<TOwner>::template Singleton<property1, property2>());
    }

    template <Ref<SVGAnimatedNumber> TOwner::*property1, Ref<SVGAnimatedNumber> TOwner::*property2>
    static void RegisterProperty(const dom::QualifiedName &attributeName) noexcept
    {
      RegisterProperty(attributeName,
                       SVGAnimatedNumberPairAccessor<TOwner>::template Singleton<property1, property2>());
    }

    // Enumerate all the SVGMemberAccessors recursively. The functor will be called and will
    // be given the pair<QualifiedName, SVGMemberAccessor> till the functor returns false.
    template <typename TFunctor>
    static bool EnumerateRecursively(const TFunctor &functor) noexcept
    {
      for (const auto &entry : AttributeNameToAccessors())
      {
        if (!functor(entry))
        {
          return false;
        }
      }
      return (TBaseTypes::PropertyRegistry::EnumerateRecursively(functor) && ...);
    }

    template <typename TFunctor>
    static bool LookupRecursivelyAndApply(const dom::QualifiedName &attributeName,
                                          const TFunctor &functor) noexcept
    {
      if (auto *accessor = FindAccessor(attributeName))
      {
        functor(*accessor);
        return true;
      }
      return (TBaseTypes::PropertyRegistry::LookupRecursivelyAndApply(attributeName, functor) || ...);
    }

    // Returns true if OwnerType owns a property whose name is attributeName.
    KRYS_NODISCARD static bool IsKnownAttribute(const dom::QualifiedName &attributeName) noexcept
    {
      return FindAccessor(attributeName);
    }

    // Returns true if OwnerType owns a property whose name is attributeName
    // and its type is SVGAnimatedLength.
    KRYS_NODISCARD static bool IsAnimatedLengthAttribute(const dom::QualifiedName &attributeName) noexcept
    {
      if (const auto *accessor = FindAccessor(attributeName))
      {
        return accessor->IsAnimatedLength();
      }
      return false;
    }

    KRYS_NODISCARD dom::QualifiedName
      PropertyAttributeName(const SVGProperty &property) const noexcept override
    {
      auto attributeName = dom::NullQualifiedName();
      EnumerateRecursively(
        [&](const auto &entry) -> bool
        {
          if (!entry.second->Matches(_owner, property))
          {
            return true;
          }
          attributeName = entry.first;
          return false;
        });
      return attributeName;
    }

    KRYS_NODISCARD dom::QualifiedName
      AnimatedPropertyAttributeName(const SVGAnimatedProperty &animatedProperty) const noexcept override
    {
      auto attributeName = dom::NullQualifiedName();
      EnumerateRecursively(
        [&](const auto &entry) -> bool
        {
          if (!entry.second->Matches(_owner, animatedProperty))
          {
            return true;
          }
          attributeName = entry.first;
          return false;
        });
      return attributeName;
    }

    void SetAnimatedPropertyDirty(const dom::QualifiedName &attributeName,
                                  SVGAnimatedProperty &animatedProperty) const noexcept override
    {
      if (auto *property = FastAnimatedPropertyLookup(_owner, attributeName))
      {
        property->SetDirty();
        return;
      }

      LookupRecursivelyAndApply(attributeName,
                                [&](auto &accessor) { accessor.SetDirty(_owner, animatedProperty); });
    }

    // Detach all the properties recursively from their OwnerTypes.
    void DetachAllProperties() const noexcept override
    {
      EnumerateRecursively(
        [&](const auto &entry) -> bool
        {
          entry.second->Detach(_owner);
          return true;
        });
    }

    KRYS_NODISCARD static SVGAnimatedProperty *
      FastAnimatedPropertyLookup(TOwner &owner, const dom::QualifiedName &attributeName) noexcept
    {
      if constexpr (HasFastPropertyForAttribute<TOwner>)
      {
        return owner.PropertyForAttribute(attributeName);
      }
      else
      {
        (void)owner;
        (void)attributeName;
        return nullptr;
      }
    }

    // Finds the property whose name is attributeName and returns the synchronize
    // string through the associated SVGMemberAccessor.
    KRYS_NODISCARD Maybe<dom::DOMString>
      Synchronize(const dom::QualifiedName &attributeName) const noexcept override
    {
      if (auto *property = FastAnimatedPropertyLookup(_owner, attributeName))
      {
        return property->Synchronize();
      }

      Maybe<dom::DOMString> value;
      LookupRecursivelyAndApply(attributeName, [&](auto &accessor) { value = accessor.Synchronize(_owner); });
      return value;
    }

    // Enumerate recursively the SVGMemberAccessors of the OwnerType and all its BaseTypes.
    // Collect all the pairs <AttributeName, String> only for the dirty properties.
    KRYS_NODISCARD List<Pair<dom::QualifiedName, dom::DOMString>>
      SynchronizeAllAttributes() const noexcept override
    {
      List<Pair<dom::QualifiedName, dom::DOMString>> map;
      EnumerateRecursively(
        [&](const auto &entry) -> bool
        {
          if (auto string = entry.second->Synchronize(_owner))
          {
            map.emplace_back(entry.first, krys::move(*string));
          }
          return true;
        });
      return map;
    }

    KRYS_NODISCARD bool
      IsAnimatedPropertyAttribute(const dom::QualifiedName &attributeName) const noexcept override
    {
      if (FastAnimatedPropertyLookup(_owner, attributeName))
      {
        return true;
      }

      bool isAnimatedPropertyAttribute = false;
      LookupRecursivelyAndApply(attributeName, [&](auto &accessor)
                                { isAnimatedPropertyAttribute = accessor.IsAnimatedProperty(); });
      return isAnimatedPropertyAttribute;
    }

    KRYS_NODISCARD bool
      IsAnimatedStylePropertyAttribute(const dom::QualifiedName &attributeName) const noexcept override
    {
      static const Array<const dom::QualifiedName *, 9> animatedStyleAttributes {
        &SVGNames::CxAttr(),    &SVGNames::CyAttr(), &SVGNames::RAttr(),
        &SVGNames::RxAttr(),    &SVGNames::RyAttr(), &SVGNames::HeightAttr(),
        &SVGNames::WidthAttr(), &SVGNames::XAttr(),  &SVGNames::YAttr(),
      };
      return IsAnimatedLengthAttribute(attributeName)
             && std::ranges::any_of(animatedStyleAttributes,
                                    [&](const auto *name) { return *name == attributeName; });
    }

    // FIXME: createAnimator and appendAnimatedInstance come with SMIL (B11i).

  private:
    KRYS_NODISCARD static AttributeNameToAccessorMap &AttributeNameToAccessors() noexcept
    {
      static NeverDestroyed<AttributeNameToAccessorMap> attributeNameToAccessorMap;
      return attributeNameToAccessorMap;
    }

    static void RegisterProperty(const dom::QualifiedName &attributeName,
                                 const SVGMemberAccessor<TOwner> &propertyAccessor) noexcept
    {
      AttributeNameToAccessors().emplace_back(attributeName, &propertyAccessor);
    }

    KRYS_NODISCARD static const SVGMemberAccessor<TOwner> *
      FindAccessor(const dom::QualifiedName &attributeName) noexcept
    {
      for (const auto &[name, accessor] : AttributeNameToAccessors())
      {
        if (name.Matches(attributeName))
        {
          return accessor;
        }
      }
      return nullptr;
    }
  };
}
