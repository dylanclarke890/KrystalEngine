#include "Krystal.Booey/HTML/Sanitizer/Sanitizer.hpp"
#include "Krystal.Booey/HTML/Sanitizer/SanitizerAlgorithms.hpp"
#include "Krystal.Booey/HTML/Sanitizer/SanitizerConstants.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"
#include <algorithm>

namespace krys::boo::html
{
  namespace
  {
    template <typename T>
    KRYS_NODISCARD bool SanitizerContains(const List<T> &list, const T &item) noexcept
    {
      return std::ranges::find(list, item) != list.end();
    }

    KRYS_NODISCARD bool SanitizerContainsElement(const List<SanitizerElementNamespace> &list,
                                                 const SanitizerElementNamespace &element) noexcept
    {
      return std::ranges::any_of(list, [&](const auto &entry) { return entry.HasSameName(element); });
    }

    // Removes @p element from @p list (by name), returning whether it was there; a missing list holds
    // nothing.
    template <typename T>
    bool SanitizerRemoveElement(Maybe<List<T>> &list, const SanitizerElementNamespace &element) noexcept
    {
      return list && std::erase_if(*list, [&](const T &entry) { return entry.HasSameName(element); }) > 0;
    }

    template <typename T>
    bool SanitizerRemove(List<T> &list, const T &item) noexcept
    {
      return std::erase(list, item) > 0;
    }

    // https://infra.spec.whatwg.org/#list-iterate creating a set: drops later duplicates.
    void SanitizerMakeSet(List<SanitizerAttributeNamespace> &list) noexcept
    {
      List<SanitizerAttributeNamespace> set;
      for (auto &attribute : list)
      {
        if (!SanitizerContains(set, attribute))
        {
          set.push_back(krys::move(attribute));
        }
      }
      list = krys::move(set);
    }

    void SanitizerDifference(List<SanitizerAttributeNamespace> &list,
                             const List<SanitizerAttributeNamespace> &other) noexcept
    {
      std::erase_if(list, [&](const auto &attribute) { return SanitizerContains(other, attribute); });
    }

    void SanitizerIntersection(List<SanitizerAttributeNamespace> &list,
                               const List<SanitizerAttributeNamespace> &other) noexcept
    {
      std::erase_if(list, [&](const auto &attribute) { return !SanitizerContains(other, attribute); });
    }

    template <typename T>
    void SanitizerSortItems(List<T> &list) noexcept
    {
      std::ranges::stable_sort(
        list, [](const auto &a, const auto &b)
        { return SanitizerAlgorithms::CompareItems(a.Name, a.Namespace, b.Name, b.Namespace); });
    }

    void SanitizerSortItems(List<SanitizerProcessingInstruction> &list) noexcept
    {
      std::ranges::stable_sort(list, [](const auto &a, const auto &b)
                               { return infra::StringAlgorithms::CodeUnitLessThan(a.Target, b.Target); });
    }
  }

  Ref<Sanitizer> Sanitizer::Create(KRYS_MAYBE_UNUSED SanitizerPresets configuration) noexcept
  {
    // 1.-1b. If configuration is a SanitizerPresets string: assert: configuration is "default"; set
    //        configuration to the built-in safe default configuration.
    krys_debug_assert(configuration == SanitizerPresets::Default);
    auto sanitizer = Create(SanitizerConstants::BuiltInSafeDefaultConfiguration());
    krys_assert_msg(!sanitizer.HasException(), "the built-in safe default configuration is valid");
    return sanitizer.ReleaseValue();
  }

  dom::ExceptionOr<Ref<Sanitizer>> Sanitizer::Create(const SanitizerConfig &configuration) noexcept
  {
    // 2. Configure this given configuration and true.
    auto configured = SanitizerAlgorithms::Configure(configuration, true);
    if (configured.HasException())
    {
      return configured.ReleaseException();
    }

    Ref<Sanitizer> sanitizer = AdoptRef(*new Sanitizer());
    sanitizer->_configuration = configured.ReleaseValue();
    return sanitizer;
  }

  SanitizerConfig Sanitizer::Get() const noexcept
  {
    // 1. Let config be this's configuration.
    auto config = _configuration;

    // 2. Assert: config is valid.
    krys_debug_assert(SanitizerAlgorithms::IsValid(config));

    // 3. If config["elements"] exists:
    if (config.Elements)
    {
      // 3a. For each element of config["elements"]: if element["attributes"] exists, then set
      //     element["attributes"] to the result of sorting element["attributes"], with compare sanitizer
      //     items; if element["removeAttributes"] exists, then set it to the result of sorting it, with
      //     compare sanitizer items.
      for (auto &element : *config.Elements)
      {
        if (element.Attributes)
        {
          SanitizerSortItems(*element.Attributes);
        }
        if (element.RemoveAttributes)
        {
          SanitizerSortItems(*element.RemoveAttributes);
        }
      }

      // 3b. Set config["elements"] to the result of sorting config["elements"], with compare sanitizer
      //     items.
      SanitizerSortItems(*config.Elements);
    }
    // 4.-4a. Otherwise: set config["removeElements"] to the result of sorting config["removeElements"], with
    //        compare sanitizer items.
    else
    {
      SanitizerSortItems(*config.RemoveElements);
    }

    // 5. If config["replaceWithChildrenElements"] exists, then set it to the result of sorting it, with
    //    compare sanitizer items.
    if (config.ReplaceWithChildrenElements)
    {
      SanitizerSortItems(*config.ReplaceWithChildrenElements);
    }

    // 6.-7a. If config["processingInstructions"] exists, then set it to the result of sorting it, with
    //        piA["target"] being code unit less than piB["target"]. Otherwise: set
    //        config["removeProcessingInstructions"] to the result of sorting it, in the same way.
    SanitizerSortItems(config.ProcessingInstructions ? *config.ProcessingInstructions
                                                     : *config.RemoveProcessingInstructions);

    // 8.-9a. If config["attributes"] exists, then set config["attributes"] to the result of sorting it given
    //        compare sanitizer items. Otherwise: set config["removeAttributes"] to the result of sorting it
    //        given compare sanitizer items.
    SanitizerSortItems(config.Attributes ? *config.Attributes : *config.RemoveAttributes);

    // 10. Return config.
    return config;
  }

  bool Sanitizer::AllowElement(SanitizerElementNamespaceWithAttributes element) noexcept
  {
    // 1. Let configuration be this's configuration.
    auto &configuration = _configuration;

    // 2. Assert: configuration is valid.
    krys_debug_assert(SanitizerAlgorithms::IsValid(configuration));

    // 3. Set element to the result of canonicalizing element.
    element = SanitizerAlgorithms::Canonicalize(krys::move(element));

    // 4. If configuration["elements"] exists:
    if (configuration.Elements)
    {
      // 4a. Let modified be the result of removing element from
      //     configuration["replaceWithChildrenElements"].
      bool modified = SanitizerRemoveElement(configuration.ReplaceWithChildrenElements, element);

      // 4b. If configuration["attributes"] exists:
      if (configuration.Attributes)
      {
        // 4b.i. If element["attributes"] exists:
        if (element.Attributes)
        {
          // 4b.i.1. Set element["attributes"] to the result of creating a set from element["attributes"].
          SanitizerMakeSet(*element.Attributes);

          // 4b.i.2. Set element["attributes"] to the difference of element["attributes"] and
          //         configuration["attributes"].
          SanitizerDifference(*element.Attributes, *configuration.Attributes);

          // 4b.i.3. If configuration["dataAttributes"] is true, then remove all items item from
          //         element["attributes"] where item is a custom data attribute.
          if (*configuration.DataAttributes)
          {
            std::erase_if(*element.Attributes, SanitizerAlgorithms::IsCustomDataAttribute);
          }
        }

        // 4b.ii. If element["removeAttributes"] exists:
        if (element.RemoveAttributes)
        {
          // 4b.ii.1. Set element["removeAttributes"] to the result of creating a set from
          //          element["removeAttributes"].
          SanitizerMakeSet(*element.RemoveAttributes);

          // 4b.ii.2. Set element["removeAttributes"] to the intersection of element["removeAttributes"] and
          //          configuration["attributes"].
          SanitizerIntersection(*element.RemoveAttributes, *configuration.Attributes);
        }
      }
      // 4c. Otherwise:
      else
      {
        // 4c.i. If element["attributes"] exists:
        if (element.Attributes)
        {
          // 4c.i.1. Set element["attributes"] to the result of creating a set from element["attributes"].
          SanitizerMakeSet(*element.Attributes);

          // 4c.i.2. Set element["attributes"] to the difference of element["attributes"] and
          //         element["removeAttributes"] with default « ».
          if (element.RemoveAttributes)
          {
            SanitizerDifference(*element.Attributes, *element.RemoveAttributes);
          }

          // 4c.i.3. Remove element["removeAttributes"].
          element.RemoveAttributes.reset();

          // 4c.i.4. Set element["attributes"] to the difference of element["attributes"] and
          //         configuration["removeAttributes"].
          SanitizerDifference(*element.Attributes, *configuration.RemoveAttributes);
        }

        // 4c.ii. If element["removeAttributes"] exists:
        if (element.RemoveAttributes)
        {
          // 4c.ii.1. Set element["removeAttributes"] to the result of creating a set from
          //          element["removeAttributes"].
          SanitizerMakeSet(*element.RemoveAttributes);

          // 4c.ii.2. Set element["removeAttributes"] to the difference of element["removeAttributes"] and
          //          configuration["removeAttributes"].
          SanitizerDifference(*element.RemoveAttributes, *configuration.RemoveAttributes);
        }
      }

      // 4d.-4d.ii. If configuration["elements"] does not contain element: append element to
      //            configuration["elements"]; return true.
      auto current = std::ranges::find_if(*configuration.Elements,
                                          [&](const auto &entry) { return entry.HasSameName(element); });
      if (current == configuration.Elements->end())
      {
        configuration.Elements->push_back(krys::move(element));
        return true;
      }

      // 4e. Let currentElement be the item in configuration["elements"] whose name member is element's name
      //     member and whose namespace member is element's namespace member.
      // 4f. If element is equal to currentElement, then return modified.
      if (element == *current)
      {
        return modified;
      }

      // 4g.-4i. Remove element from configuration["elements"]. Append element to configuration["elements"].
      //         Return true.
      configuration.Elements->erase(current);
      configuration.Elements->push_back(krys::move(element));
      return true;
    }

    // 5.-5a. Otherwise: if element["attributes"] exists or element["removeAttributes"] with default « » is
    // not
    //        empty, then return false.
    if (element.Attributes || (element.RemoveAttributes && !element.RemoveAttributes->empty()))
    {
      return false;
    }

    // 5b. Let modified be the result of removing element from configuration["replaceWithChildrenElements"].
    bool modified = SanitizerRemoveElement(configuration.ReplaceWithChildrenElements, element);

    // 5c. If configuration["removeElements"] does not contain element, then return modified.
    // 5d.-5e. Remove element from configuration["removeElements"]. Return true.
    if (!SanitizerRemoveElement(configuration.RemoveElements, element))
    {
      return modified;
    }
    return true;
  }

  bool Sanitizer::RemoveElement(SanitizerElementNamespace element) noexcept
  {
    // Return the result of removing element from this's configuration.
    return SanitizerAlgorithms::RemoveElement(_configuration, krys::move(element));
  }

  bool Sanitizer::ReplaceElementWithChildren(SanitizerElementNamespace element) noexcept
  {
    // 1. Let configuration be this's configuration.
    auto &configuration = _configuration;

    // 2. Assert: configuration is valid.
    krys_debug_assert(SanitizerAlgorithms::IsValid(configuration));

    // 3. Set element to the result of canonicalizing element.
    element = SanitizerAlgorithms::Canonicalize(krys::move(element));

    // 4. If the built-in non-replaceable elements list contains element, then return false.
    if (std::ranges::any_of(SanitizerConstants::BuiltInNonReplaceableElements(),
                            [&](const auto &entry) { return entry.HasSameName(element); }))
    {
      return false;
    }

    // 5. Let modified be the result of removing element from configuration["elements"].
    bool modified = SanitizerRemoveElement(configuration.Elements, element);

    // 6. If removing element from configuration["removeElements"] is true, then set modified to true.
    if (SanitizerRemoveElement(configuration.RemoveElements, element))
    {
      modified = true;
    }

    // 7.-7b. If configuration["replaceWithChildrenElements"] does not contain element: append element to
    //        configuration["replaceWithChildrenElements"]; return true.
    if (!configuration.ReplaceWithChildrenElements)
    {
      configuration.ReplaceWithChildrenElements.emplace();
    }
    if (!SanitizerContainsElement(*configuration.ReplaceWithChildrenElements, element))
    {
      configuration.ReplaceWithChildrenElements->push_back(krys::move(element));
      return true;
    }

    // 8. Return modified.
    return modified;
  }

  bool Sanitizer::AllowProcessingInstruction(SanitizerProcessingInstruction pi) noexcept
  {
    // 1. Let configuration be this's configuration.
    auto &configuration = _configuration;

    // 2. Assert: configuration is valid.
    krys_debug_assert(SanitizerAlgorithms::IsValid(configuration));

    // 3. Set pi to the result of canonicalizing pi.
    // 4.-4c. If configuration["processingInstructions"] exists: if it contains pi, then return false; append
    //        pi to configuration["processingInstructions"]; return true.
    if (configuration.ProcessingInstructions)
    {
      if (SanitizerContains(*configuration.ProcessingInstructions, pi))
      {
        return false;
      }
      configuration.ProcessingInstructions->push_back(krys::move(pi));
      return true;
    }

    // 5.-5b. Otherwise: if configuration["removeProcessingInstructions"] contains pi: remove pi from it;
    //        return true. Return false.
    return SanitizerRemove(*configuration.RemoveProcessingInstructions, pi);
  }

  bool Sanitizer::RemoveProcessingInstruction(SanitizerProcessingInstruction pi) noexcept
  {
    // 1. Let configuration be this's configuration.
    auto &configuration = _configuration;

    // 2. Assert: configuration is valid.
    krys_debug_assert(SanitizerAlgorithms::IsValid(configuration));

    // 3. Set pi to the result of canonicalizing pi.
    // 4.-4b. If configuration["processingInstructions"] exists: if it contains pi: remove pi from it; return
    //        true. Return false.
    if (configuration.ProcessingInstructions)
    {
      return SanitizerRemove(*configuration.ProcessingInstructions, pi);
    }

    // 5.-5c. Otherwise: if configuration["removeProcessingInstructions"] contains pi, then return false.
    //        Append pi to configuration["removeProcessingInstructions"]. Return true.
    if (SanitizerContains(*configuration.RemoveProcessingInstructions, pi))
    {
      return false;
    }
    configuration.RemoveProcessingInstructions->push_back(krys::move(pi));
    return true;
  }

  bool Sanitizer::AllowAttribute(SanitizerAttributeNamespace attribute) noexcept
  {
    // 1. Let configuration be this's configuration.
    auto &configuration = _configuration;

    // 2. Assert: configuration is valid.
    krys_debug_assert(SanitizerAlgorithms::IsValid(configuration));

    // 3. Set attribute to the result of canonicalizing attribute.
    attribute = SanitizerAlgorithms::Canonicalize(krys::move(attribute));

    // 4. If configuration["attributes"] exists:
    if (configuration.Attributes)
    {
      // 4a. If configuration["dataAttributes"] is true and attribute is a custom data attribute, then
      //     return false.
      if (*configuration.DataAttributes && SanitizerAlgorithms::IsCustomDataAttribute(attribute))
      {
        return false;
      }

      // 4b. If configuration["attributes"] contains attribute, then return false.
      if (SanitizerContains(*configuration.Attributes, attribute))
      {
        return false;
      }

      // 4c.-4c.i.1. If configuration["elements"] exists: for each element in configuration["elements"]: if
      //             element["attributes"] with default « » contains attribute, then remove attribute from
      //             element["attributes"].
      if (configuration.Elements)
      {
        for (auto &element : *configuration.Elements)
        {
          if (element.Attributes)
          {
            SanitizerRemove(*element.Attributes, attribute);
          }
        }
      }

      // 4d.-4e. Append attribute to configuration["attributes"]. Return true.
      configuration.Attributes->push_back(krys::move(attribute));
      return true;
    }

    // 5.-5a. Otherwise: if configuration["removeAttributes"] does not contain attribute, then return false.
    //        Remove attribute from configuration["removeAttributes"]. Return true.
    return SanitizerRemove(*configuration.RemoveAttributes, attribute);
  }

  bool Sanitizer::RemoveAttribute(SanitizerAttributeNamespace attribute) noexcept
  {
    // Return the result of removing attribute from this's configuration.
    return SanitizerAlgorithms::RemoveAttribute(_configuration, krys::move(attribute));
  }

  bool Sanitizer::SetComments(bool allow) noexcept
  {
    // 1. Let configuration be this's configuration.
    // 2. Assert: configuration is valid.
    krys_debug_assert(SanitizerAlgorithms::IsValid(_configuration));

    // 3. If configuration["comments"] is allow, then return false.
    if (_configuration.Comments == allow)
    {
      return false;
    }

    // 4.-5. Set configuration["comments"] to allow. Return true.
    _configuration.Comments = allow;
    return true;
  }

  bool Sanitizer::SetDataAttributes(bool allow) noexcept
  {
    // 1. Let configuration be this's configuration.
    auto &configuration = _configuration;

    // 2. Assert: configuration is valid.
    krys_debug_assert(SanitizerAlgorithms::IsValid(configuration));

    // 3. If configuration["attributes"] does not exist, then return false.
    if (!configuration.Attributes)
    {
      return false;
    }

    // 4. If configuration["dataAttributes"] exists and is equal to allow, then return false.
    if (configuration.DataAttributes == allow)
    {
      return false;
    }

    // 5. If allow is true:
    if (allow)
    {
      // 5a.-5a.i.1. If configuration["elements"] exists: for each element of configuration["elements"]: if
      //             element["attributes"] exists, then remove all items item from element["attributes"] where
      //             item is a custom data attribute.
      if (configuration.Elements)
      {
        for (auto &element : *configuration.Elements)
        {
          if (element.Attributes)
          {
            std::erase_if(*element.Attributes, SanitizerAlgorithms::IsCustomDataAttribute);
          }
        }
      }

      // 5b. Remove all items item from configuration["attributes"] where item is a custom data attribute.
      std::erase_if(*configuration.Attributes, SanitizerAlgorithms::IsCustomDataAttribute);
    }

    // 6.-7. Set configuration["dataAttributes"] to allow. Return true.
    configuration.DataAttributes = allow;
    return true;
  }

  bool Sanitizer::SetJavascriptURLs(bool allow) noexcept
  {
    // 1. Let configuration be this's configuration.
    // 2. Assert: configuration is valid.
    krys_debug_assert(SanitizerAlgorithms::IsValid(_configuration));

    // 3. If configuration["javascriptURLs"] is allow, then return false.
    if (_configuration.JavascriptURLs == allow)
    {
      return false;
    }

    // 4.-5. Set configuration["javascriptURLs"] to allow. Return true.
    _configuration.JavascriptURLs = allow;
    return true;
  }

  bool Sanitizer::RemoveUnsafe() noexcept
  {
    // Return the result of removing unsafe from this's configuration.
    return SanitizerAlgorithms::RemoveUnsafe(_configuration);
  }
}
