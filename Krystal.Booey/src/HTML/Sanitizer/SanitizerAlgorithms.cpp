#include "Krystal.Booey/HTML/Sanitizer/SanitizerAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/NameValidation.hpp"
#include "Krystal.Booey/HTML/Sanitizer/Sanitizer.hpp"
#include "Krystal.Booey/HTML/Sanitizer/SanitizerConstants.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"
#include "Krystal.Booey/URL/URL.hpp"
#include <algorithm>

namespace krys::boo::html
{
  namespace
  {
    // Whether two items of a sanitizer list are the same item: elements and attributes by name and namespace,
    // processing instructions by target.
    KRYS_NODISCARD bool IsSameSanitizerItem(const SanitizerElementNamespace &a,
                                            const SanitizerElementNamespace &b) noexcept
    {
      return a.HasSameName(b);
    }

    KRYS_NODISCARD bool IsSameSanitizerItem(const SanitizerAttributeNamespace &a,
                                            const SanitizerAttributeNamespace &b) noexcept
    {
      return a == b;
    }

    KRYS_NODISCARD bool IsSameSanitizerItem(const SanitizerProcessingInstruction &a,
                                            const SanitizerProcessingInstruction &b) noexcept
    {
      return a == b;
    }

    template <typename T, typename TItem>
    KRYS_NODISCARD bool SanitizerListContains(const List<T> &list, const TItem &item) noexcept
    {
      return std::ranges::any_of(list, [&](const T &entry) { return IsSameSanitizerItem(entry, item); });
    }

    // A list that doesn't exist contains nothing.
    template <typename T, typename TItem>
    KRYS_NODISCARD bool SanitizerListContains(const Maybe<List<T>> &list, const TItem &item) noexcept
    {
      return list && SanitizerListContains(*list, item);
    }

    // Removes @p item from @p list, returning whether it was there.
    template <typename T, typename TItem>
    bool RemoveFromSanitizerList(List<T> &list, const TItem &item) noexcept
    {
      return std::erase_if(list, [&](const T &entry) { return IsSameSanitizerItem(entry, item); }) > 0;
    }

    template <typename T, typename TItem>
    bool RemoveFromSanitizerList(Maybe<List<T>> &list, const TItem &item) noexcept
    {
      return list && RemoveFromSanitizerList(*list, item);
    }

    template <typename T>
    KRYS_NODISCARD bool SanitizerListHasDuplicates(const List<T> &list) noexcept
    {
      for (auto it = list.begin(); it != list.end(); ++it)
      {
        if (std::any_of(std::next(it), list.end(),
                        [&](const T &entry) { return IsSameSanitizerItem(*it, entry); }))
        {
          return true;
        }
      }
      return false;
    }

    template <typename T, typename U>
    KRYS_NODISCARD bool SanitizerListsIntersect(const List<T> &a, const List<U> &b) noexcept
    {
      return std::ranges::any_of(a, [&](const T &entry) { return SanitizerListContains(b, entry); });
    }

    template <typename T>
    void CanonicalizeSanitizerList(Maybe<List<T>> &list) noexcept
    {
      if (list)
      {
        for (auto &entry : *list)
        {
          entry = SanitizerAlgorithms::Canonicalize(krys::move(entry));
        }
      }
    }

    void CanonicalizeSanitizerList(Maybe<List<SanitizerProcessingInstruction>> &) noexcept
    {
      // SPEC: To canonicalize a processing instruction given a SanitizerPI pi: if pi is a DOMString, then
      // return «[ "target" → pi ]»; [...] return «[ "target" → pi["target"] ]». NOTE: A processing
      // instruction is always already canonical.
    }

    KRYS_NODISCARD dom::DOMStringAtom CanonicalSanitizerNamespace(dom::DOMStringAtom namespaceURI) noexcept
    {
      // SPEC: If name["namespace"] is the empty string, then set it to null.
      if (namespaceURI != dom::DOMStringAtom::Null() && namespaceURI.View().empty())
      {
        return dom::DOMStringAtom::Null();
      }
      return namespaceURI;
    }
  }

  dom::ExceptionOr<SanitizerConfig> SanitizerAlgorithms::Configure(const SanitizerConfig &configuration,
                                                                   bool permissiveDefaults) noexcept
  {
    // SPEC: Let freshConfiguration be a clone of configuration.
    auto freshConfiguration = configuration;

    // SPEC: Canonicalize the configuration freshConfiguration with permissiveDefaults.
    Canonicalize(freshConfiguration, permissiveDefaults);

    // SPEC: If freshConfiguration is not valid, then throw a TypeError.
    if (!IsValid(freshConfiguration))
    {
      return dom::Exception {dom::ExceptionCode::TypeError, u8"The sanitizer configuration isn't valid."};
    }

    // SPEC: Set sanitizer's configuration to freshConfiguration.
    return freshConfiguration;
  }

  void SanitizerAlgorithms::Canonicalize(SanitizerConfig &configuration, bool permissiveDefaults) noexcept
  {
    // SPEC: If neither configuration["elements"] nor configuration["removeElements"] exists, then set
    // configuration["removeElements"] to an empty list.
    if (!configuration.Elements && !configuration.RemoveElements)
    {
      configuration.RemoveElements.emplace();
    }

    // SPEC: If neither configuration["attributes"] nor configuration["removeAttributes"] exists, then set
    // configuration["removeAttributes"] to an empty list.
    if (!configuration.Attributes && !configuration.RemoveAttributes)
    {
      configuration.RemoveAttributes.emplace();
    }

    // SPEC: If neither configuration["processingInstructions"] nor
    // configuration["removeProcessingInstructions"] exists: if permissiveDefaults is true, then set
    // configuration["removeProcessingInstructions"] to an empty list; otherwise, set
    // configuration["processingInstructions"] to an empty list.
    if (!configuration.ProcessingInstructions && !configuration.RemoveProcessingInstructions)
    {
      if (permissiveDefaults)
      {
        configuration.RemoveProcessingInstructions.emplace();
      }
      else
      {
        configuration.ProcessingInstructions.emplace();
      }
    }

    // SPEC: If configuration["elements"] exists: [...] append the result of canonicalizing element to
    // newElements. Set configuration["elements"] to newElements.
    CanonicalizeSanitizerList(configuration.Elements);

    // SPEC: If configuration["removeElements"] exists, then set configuration["removeElements"] to the result
    // of canonicalizing configuration["removeElements"].
    CanonicalizeSanitizerList(configuration.RemoveElements);

    // SPEC: If configuration["attributes"] exists, then set configuration["attributes"] to the result of
    // canonicalizing configuration["attributes"].
    CanonicalizeSanitizerList(configuration.Attributes);

    // SPEC: If configuration["removeAttributes"] exists, then set configuration["removeAttributes"] to the
    // result of canonicalizing configuration["removeAttributes"].
    CanonicalizeSanitizerList(configuration.RemoveAttributes);

    // SPEC: If configuration["replaceWithChildrenElements"] exists, then set
    // configuration["replaceWithChildrenElements"] to the result of canonicalizing
    // configuration["replaceWithChildrenElements"].
    CanonicalizeSanitizerList(configuration.ReplaceWithChildrenElements);

    // SPEC: If configuration["processingInstructions"] exists, then [...] canonicalizing it. If
    // configuration["removeProcessingInstructions"] exists, then [...] canonicalizing it.
    CanonicalizeSanitizerList(configuration.ProcessingInstructions);
    CanonicalizeSanitizerList(configuration.RemoveProcessingInstructions);

    // SPEC: If configuration["comments"] does not exist, then set it to permissiveDefaults.
    if (!configuration.Comments)
    {
      configuration.Comments = permissiveDefaults;
    }

    // SPEC: If configuration["attributes"] exists and configuration["dataAttributes"] does not exist, then
    // set it to permissiveDefaults.
    if (configuration.Attributes && !configuration.DataAttributes)
    {
      configuration.DataAttributes = permissiveDefaults;
    }

    // SPEC: If configuration["javascriptURLs"] does not exist, then set it to permissiveDefaults.
    if (!configuration.JavascriptURLs)
    {
      configuration.JavascriptURLs = permissiveDefaults;
    }
  }

  SanitizerElementNamespaceWithAttributes
    SanitizerAlgorithms::Canonicalize(SanitizerElementNamespaceWithAttributes element) noexcept
  {
    // SPEC: Let result be the result of canonicalizing element.
    element.Namespace = CanonicalSanitizerNamespace(element.Namespace);

    // SPEC: If element is a dictionary: if element["attributes"] exists, then set result["attributes"] to the
    // result of canonicalizing element["attributes"]; if element["removeAttributes"] exists, then set
    // result["removeAttributes"] to the result of canonicalizing element["removeAttributes"].
    CanonicalizeSanitizerList(element.Attributes);
    CanonicalizeSanitizerList(element.RemoveAttributes);

    // SPEC: If neither result["attributes"] nor result["removeAttributes"] exists, then set
    // result["removeAttributes"] to an empty list.
    if (!element.Attributes && !element.RemoveAttributes)
    {
      element.RemoveAttributes.emplace();
    }

    // SPEC: Return result.
    return element;
  }

  SanitizerElementNamespace SanitizerAlgorithms::Canonicalize(SanitizerElementNamespace element) noexcept
  {
    // SPEC: Return the result of canonicalizing element with the HTML namespace as the default namespace.
    // NOTE: A string element already has the HTML namespace.
    element.Namespace = CanonicalSanitizerNamespace(element.Namespace);
    return element;
  }

  SanitizerAttributeNamespace
    SanitizerAlgorithms::Canonicalize(SanitizerAttributeNamespace attribute) noexcept
  {
    attribute.Namespace = CanonicalSanitizerNamespace(attribute.Namespace);
    return attribute;
  }

  bool SanitizerAlgorithms::IsValid(const SanitizerConfig &config) noexcept
  {
    // SPEC: Assert: config["elements"] exists or config["removeElements"] exists.
    krys_debug_assert(config.Elements || config.RemoveElements);

    // SPEC: If config["elements"] exists and config["removeElements"] exists, then return false.
    if (config.Elements && config.RemoveElements)
    {
      return false;
    }

    // SPEC: Assert: Either config["processingInstructions"] exists or config["removeProcessingInstructions"]
    // exists.
    krys_debug_assert(config.ProcessingInstructions || config.RemoveProcessingInstructions);

    // SPEC: If config["processingInstructions"] exists and config["removeProcessingInstructions"] exists,
    // then return false.
    if (config.ProcessingInstructions && config.RemoveProcessingInstructions)
    {
      return false;
    }

    // SPEC: Assert: Either config["attributes"] exists or config["removeAttributes"] exists.
    krys_debug_assert(config.Attributes || config.RemoveAttributes);

    // SPEC: If config["attributes"] exists and config["removeAttributes"] exists, then return false.
    if (config.Attributes && config.RemoveAttributes)
    {
      return false;
    }

    // SPEC: If config["elements"] exists: if config["elements"] has duplicates, then return false. Otherwise:
    // if config["removeElements"] has duplicates, then return false.
    if (config.Elements ? SanitizerListHasDuplicates(*config.Elements)
                        : SanitizerListHasDuplicates(*config.RemoveElements))
    {
      return false;
    }

    // SPEC: If config["replaceWithChildrenElements"] exists and has duplicates, then return false.
    if (config.ReplaceWithChildrenElements && SanitizerListHasDuplicates(*config.ReplaceWithChildrenElements))
    {
      return false;
    }

    // SPEC: If config["processingInstructions"] exists: if it has duplicates, then return false. Otherwise:
    // if config["removeProcessingInstructions"] has duplicates, then return false.
    if (config.ProcessingInstructions ? SanitizerListHasDuplicates(*config.ProcessingInstructions)
                                      : SanitizerListHasDuplicates(*config.RemoveProcessingInstructions))
    {
      return false;
    }

    // SPEC: If config["attributes"] exists: if config["attributes"] has duplicates, then return false.
    // Otherwise: if config["removeAttributes"] has duplicates, then return false.
    if (config.Attributes ? SanitizerListHasDuplicates(*config.Attributes)
                          : SanitizerListHasDuplicates(*config.RemoveAttributes))
    {
      return false;
    }

    // SPEC: If config["replaceWithChildrenElements"] exists:
    if (config.ReplaceWithChildrenElements)
    {
      // SPEC: For each element of config["replaceWithChildrenElements"]: if the built-in non-replaceable
      // elements list contains element, then return false.
      for (auto &element : *config.ReplaceWithChildrenElements)
      {
        if (std::ranges::any_of(SanitizerConstants::BuiltInNonReplaceableElements(),
                                [&](const SanitizerElementNamespace &entry)
                                { return entry.HasSameName(element); }))
        {
          return false;
        }
      }

      // SPEC: If config["elements"] exists: if the intersection of config["elements"] and
      // config["replaceWithChildrenElements"] is not empty, then return false. Otherwise: if the intersection
      // of config["removeElements"] and config["replaceWithChildrenElements"] is not empty, then return
      // false.
      if (config.Elements
            ? SanitizerListsIntersect(*config.Elements, *config.ReplaceWithChildrenElements)
            : SanitizerListsIntersect(*config.RemoveElements, *config.ReplaceWithChildrenElements))
      {
        return false;
      }
    }

    // SPEC: If config["attributes"] exists:
    if (config.Attributes)
    {
      // SPEC: Assert: config["dataAttributes"] exists.
      krys_debug_assert(config.DataAttributes);

      // SPEC: If config["elements"] exists: for each element of config["elements"]:
      if (config.Elements)
      {
        for (auto &element : *config.Elements)
        {
          // SPEC: If element["attributes"] exists and element["attributes"] has duplicates, then return
          // false.
          if (element.Attributes && SanitizerListHasDuplicates(*element.Attributes))
          {
            return false;
          }

          // SPEC: If element["removeAttributes"] exists and element["removeAttributes"] has duplicates, then
          // return false.
          if (element.RemoveAttributes && SanitizerListHasDuplicates(*element.RemoveAttributes))
          {
            return false;
          }

          // SPEC: If the intersection of config["attributes"] and element["attributes"] with default « » is
          // not empty, then return false.
          if (element.Attributes && SanitizerListsIntersect(*config.Attributes, *element.Attributes))
          {
            return false;
          }

          // SPEC: If element["removeAttributes"] with default « » is not a subset of config["attributes"],
          // then return false.
          if (element.RemoveAttributes
              && !std::ranges::all_of(*element.RemoveAttributes,
                                      [&](const SanitizerAttributeNamespace &attribute)
                                      { return SanitizerListContains(*config.Attributes, attribute); }))
          {
            return false;
          }

          // SPEC: If config["dataAttributes"] is true and element["attributes"] contains a custom data
          // attribute, then return false.
          if (*config.DataAttributes && element.Attributes
              && std::ranges::any_of(*element.Attributes, IsCustomDataAttribute))
          {
            return false;
          }
        }
      }

      // SPEC: If config["dataAttributes"] is true and config["attributes"] contains a custom data attribute,
      // then return false.
      if (*config.DataAttributes && std::ranges::any_of(*config.Attributes, IsCustomDataAttribute))
      {
        return false;
      }
    }
    // SPEC: Otherwise:
    else
    {
      // SPEC: If config["elements"] exists: for each element of config["elements"]:
      if (config.Elements)
      {
        for (auto &element : *config.Elements)
        {
          // SPEC: If element["attributes"] exists and element["removeAttributes"] exists, then return false.
          if (element.Attributes && element.RemoveAttributes)
          {
            return false;
          }

          // SPEC: If element["attributes"] exists and element["attributes"] has duplicates, then return
          // false.
          if (element.Attributes && SanitizerListHasDuplicates(*element.Attributes))
          {
            return false;
          }

          // SPEC: If element["removeAttributes"] exists and element["removeAttributes"] has duplicates, then
          // return false.
          if (element.RemoveAttributes && SanitizerListHasDuplicates(*element.RemoveAttributes))
          {
            return false;
          }

          // SPEC: If the intersection of config["removeAttributes"] and element["attributes"] with default «
          // » is not empty, then return false.
          if (element.Attributes && SanitizerListsIntersect(*config.RemoveAttributes, *element.Attributes))
          {
            return false;
          }

          // SPEC: If the intersection of config["removeAttributes"] and element["removeAttributes"] with
          // default « » is not empty, then return false.
          if (element.RemoveAttributes
              && SanitizerListsIntersect(*config.RemoveAttributes, *element.RemoveAttributes))
          {
            return false;
          }
        }
      }

      // SPEC: If config["dataAttributes"] exists, then return false.
      if (config.DataAttributes)
      {
        return false;
      }
    }

    // SPEC: Return true.
    return true;
  }

  bool SanitizerAlgorithms::RemoveElement(SanitizerConfig &configuration,
                                          SanitizerElementNamespace element) noexcept
  {
    // SPEC: Assert: configuration is valid.
    krys_debug_assert(IsValid(configuration));

    // SPEC: Set element to the result of canonicalizing element.
    element = Canonicalize(krys::move(element));

    // SPEC: Let modified be the result of removing element from configuration["replaceWithChildrenElements"].
    bool modified = RemoveFromSanitizerList(configuration.ReplaceWithChildrenElements, element);

    // SPEC: If configuration["elements"] exists:
    if (configuration.Elements)
    {
      // SPEC: If configuration["elements"] contains element: remove element from configuration["elements"];
      // return true.
      if (RemoveFromSanitizerList(*configuration.Elements, element))
      {
        return true;
      }

      // SPEC: Return modified.
      return modified;
    }

    // SPEC: Otherwise: if configuration["removeElements"] contains element, then return modified.
    if (SanitizerListContains(*configuration.RemoveElements, element))
    {
      return modified;
    }

    // SPEC: Append element to configuration["removeElements"]. Return true.
    configuration.RemoveElements->push_back(krys::move(element));
    return true;
  }

  bool SanitizerAlgorithms::RemoveAttribute(SanitizerConfig &configuration,
                                            SanitizerAttributeNamespace attribute) noexcept
  {
    // SPEC: Assert: configuration is valid.
    krys_debug_assert(IsValid(configuration));

    // SPEC: Set attribute to the result of canonicalizing attribute.
    attribute = Canonicalize(krys::move(attribute));

    // SPEC: If configuration["attributes"] exists:
    if (configuration.Attributes)
    {
      // SPEC: Let modified be the result of removing attribute from configuration["attributes"].
      bool modified = RemoveFromSanitizerList(*configuration.Attributes, attribute);

      // SPEC: If configuration["elements"] exists: for each element of configuration["elements"]:
      if (configuration.Elements)
      {
        for (auto &element : *configuration.Elements)
        {
          // SPEC: If element["attributes"] with default « » contains attribute: set modified to true; remove
          // attribute from element["attributes"].
          if (RemoveFromSanitizerList(element.Attributes, attribute))
          {
            modified = true;
          }

          // SPEC: If element["removeAttributes"] with default « » contains attribute: assert: modified is
          // true; remove attribute from element["removeAttributes"].
          if (SanitizerListContains(element.RemoveAttributes, attribute))
          {
            krys_debug_assert(modified);
            RemoveFromSanitizerList(element.RemoveAttributes, attribute);
          }
        }
      }

      // SPEC: Return modified.
      return modified;
    }

    // SPEC: Otherwise: if configuration["removeAttributes"] contains attribute, then return false.
    if (SanitizerListContains(*configuration.RemoveAttributes, attribute))
    {
      return false;
    }

    // SPEC: If configuration["elements"] exists: for each element in configuration["elements"]: if
    // element["attributes"] with default « » contains attribute, then remove attribute from
    // element["attributes"]; if element["removeAttributes"] with default « » contains attribute, then remove
    // attribute from element["removeAttributes"].
    if (configuration.Elements)
    {
      for (auto &element : *configuration.Elements)
      {
        RemoveFromSanitizerList(element.Attributes, attribute);
        RemoveFromSanitizerList(element.RemoveAttributes, attribute);
      }
    }

    // SPEC: Append attribute to configuration["removeAttributes"]. Return true.
    configuration.RemoveAttributes->push_back(krys::move(attribute));
    return true;
  }

  bool SanitizerAlgorithms::RemoveUnsafe(SanitizerConfig &configuration) noexcept
  {
    // SPEC: Assert: configuration is valid.
    krys_debug_assert(IsValid(configuration));

    // SPEC: Let result be false.
    bool result = false;

    // SPEC: For each element in built-in safe baseline configuration["removeElements"]: if removing element
    // from configuration is true, then set result to true.
    auto &baseline = SanitizerConstants::BuiltInSafeBaselineConfiguration();
    for (auto &element : *baseline.RemoveElements)
    {
      if (RemoveElement(configuration, element))
      {
        result = true;
      }
    }

    // SPEC: For each attribute in built-in safe baseline configuration["removeAttributes"]: if removing
    // attribute from configuration is true, then set result to true.
    for (auto &attribute : *baseline.RemoveAttributes)
    {
      if (RemoveAttribute(configuration, attribute))
      {
        result = true;
      }
    }

    // SPEC: For each attribute that is an event handler content attribute: if removing attribute from
    // configuration is true, then set result to true.
    for (auto name : SanitizerConstants::EventHandlerContentAttributes())
    {
      if (RemoveAttribute(configuration, dom::DOMString(name)))
      {
        result = true;
      }
    }

    // SPEC: If configuration["javascriptURLs"] is true: set result to true; set
    // configuration["javascriptURLs"] to false.
    if (*configuration.JavascriptURLs)
    {
      result = true;
      configuration.JavascriptURLs = false;
    }

    // SPEC: Return result.
    return result;
  }

  dom::ExceptionOr<Maybe<SanitizerConfig>>
    SanitizerAlgorithms::GetSanitizerConfigFromOptions(const SanitizerInput *sanitizer, bool safe) noexcept
  {
    // SPEC: Let sanitizerInput be null.
    // SPEC: If options is a TrustedHTMLParserOptions: [...]
    // NOTE: There are no Trusted Types.

    // SPEC: Otherwise, if options["sanitizer"] exists, then set sanitizerInput to options["sanitizer"].
    // SPEC: Otherwise, if safe is false, then return null.
    if (!sanitizer && !safe)
    {
      return Maybe<SanitizerConfig> {};
    }

    // SPEC: Otherwise, set sanitizerInput to "default".
    // SPEC: If sanitizerInput is a string: set sanitizerInput to the built-in safe default configuration.
    // SPEC: If sanitizerInput is a Sanitizer, then set sanitizerInput to sanitizerInput's configuration.
    const SanitizerConfig *sanitizerInput = &SanitizerConstants::BuiltInSafeDefaultConfiguration();
    if (sanitizer)
    {
      if (auto *instance = std::get_if<Ref<Sanitizer>>(sanitizer))
      {
        sanitizerInput = &(*instance)->Configuration();
      }
      else if (auto *dictionary = std::get_if<SanitizerConfig>(sanitizer))
      {
        sanitizerInput = dictionary;
      }
    }

    // SPEC: Let sanitizer be a new Sanitizer object. Let permissiveDefaults be true if safe is false; false
    // otherwise. Configure sanitizer given sanitizerInput and permissiveDefaults.
    auto configured = Configure(*sanitizerInput, !safe);
    if (configured.HasException())
    {
      return configured.ReleaseException();
    }

    // SPEC: Let configuration be sanitizer's configuration.
    auto configuration = configured.ReleaseValue();

    // SPEC: If safe is true, then remove unsafe from configuration.
    if (safe)
    {
      RemoveUnsafe(configuration);
    }

    // SPEC: Assert: configuration is valid.
    krys_debug_assert(IsValid(configuration));

    // SPEC: Return configuration.
    return Maybe<SanitizerConfig> {krys::move(configuration)};
  }

  bool SanitizerAlgorithms::ContainsJavaScriptURL(dom::DOMStringView value) noexcept
  {
    // SPEC: Let url be the result of running the basic URL parser on value.
    url::URL url(value);

    // SPEC: If url is failure, then return false.
    if (!url.IsValid())
    {
      return false;
    }

    // SPEC: Return true if url's scheme is "javascript", and false otherwise.
    return url.ProtocolIs(u8"javascript");
  }

  bool SanitizerAlgorithms::AllowsComments(const SanitizerConfig &configuration) noexcept
  {
    return configuration.Comments.value_or(false);
  }

  bool SanitizerAlgorithms::AllowsProcessingInstructionTarget(dom::DOMStringView piTarget,
                                                              const SanitizerConfig &configuration) noexcept
  {
    // SPEC: Let pi be «[ "target" → piTarget ]».
    SanitizerProcessingInstruction pi {dom::DOMString(piTarget)};

    // SPEC: If configuration["processingInstructions"] exists, then return whether
    // configuration["processingInstructions"] contains pi.
    if (configuration.ProcessingInstructions)
    {
      return SanitizerListContains(*configuration.ProcessingInstructions, pi);
    }

    // SPEC: If configuration["removeProcessingInstructions"] exists and contains pi, then return false.
    // SPEC: Return true.
    return !SanitizerListContains(configuration.RemoveProcessingInstructions, pi);
  }

  SanitizerAction SanitizerAlgorithms::ActionForElementName(dom::DOMStringView localName,
                                                            dom::DOMStringAtom namespaceURI,
                                                            const SanitizerConfig &configuration) noexcept
  {
    // SPEC: Let elementName be «[ "name" → localName, "namespace" → namespace ]».
    SanitizerElementNamespace elementName {dom::DOMString(localName), namespaceURI};

    // SPEC: If configuration["replaceWithChildrenElements"] exists and contains elementName, then return
    // "Replace with children".
    if (SanitizerListContains(configuration.ReplaceWithChildrenElements, elementName))
    {
      return SanitizerAction::ReplaceWithChildren;
    }

    // SPEC: If configuration["elements"] exists: if there is no item in configuration["elements"] whose name
    // member is localName and whose namespace member is namespace, then return "Remove".
    if (configuration.Elements)
    {
      if (!SanitizerListContains(*configuration.Elements, elementName))
      {
        return SanitizerAction::Remove;
      }
    }
    // SPEC: Otherwise, if configuration["removeElements"] contains elementName, then return "Remove".
    else if (SanitizerListContains(configuration.RemoveElements, elementName))
    {
      return SanitizerAction::Remove;
    }

    // SPEC: Return "Keep".
    return SanitizerAction::Keep;
  }

  bool SanitizerAlgorithms::AllowsAttribute(dom::DOMStringView localName, dom::DOMStringAtom namespaceURI,
                                            dom::DOMStringView value, dom::DOMStringView elementLocalName,
                                            dom::DOMStringAtom elementNamespace,
                                            const SanitizerConfig &configuration) noexcept
  {
    // SPEC: Let attrName be «[ "name" → localName, "namespace" → namespace ]».
    SanitizerAttributeNamespace attrName {dom::DOMString(localName), namespaceURI};

    // SPEC: Let elementName be «[ "name" → elementLocalName, "namespace" → elementNamespace ]».
    SanitizerElementNamespace elementName {dom::DOMString(elementLocalName), elementNamespace};

    // SPEC: Let elementWithLocalAttributes be «[ ]». If configuration["elements"] exists and there is an item
    // element in configuration["elements"] whose name member is elementLocalName and whose namespace member
    // is elementNamespace, then set elementWithLocalAttributes to element.
    const SanitizerElementNamespaceWithAttributes *elementWithLocalAttributes = nullptr;
    if (configuration.Elements)
    {
      auto it = std::ranges::find_if(*configuration.Elements,
                                     [&](const auto &element) { return element.HasSameName(elementName); });
      if (it != configuration.Elements->end())
      {
        elementWithLocalAttributes = &*it;
      }
    }

    auto localAttributes = [&]() -> const Maybe<List<SanitizerAttributeNamespace>> *
    {
      return elementWithLocalAttributes ? &elementWithLocalAttributes->Attributes : nullptr;
    };

    // SPEC: If elementWithLocalAttributes["removeAttributes"] exists and contains attrName, then return
    // false.
    if (elementWithLocalAttributes
        && SanitizerListContains(elementWithLocalAttributes->RemoveAttributes, attrName))
    {
      return false;
    }

    // SPEC: Otherwise, if configuration["attributes"] exists:
    if (configuration.Attributes)
    {
      // SPEC: If configuration["attributes"] does not contain attrName and
      // elementWithLocalAttributes["attributes"] with default « » does not contain attrName, and if "data-"
      // is not a code unit prefix of localName or namespace is not null or configuration["dataAttributes"] is
      // not true, then return false.
      if (!SanitizerListContains(*configuration.Attributes, attrName)
          && !(localAttributes() && SanitizerListContains(*localAttributes(), attrName))
          && (!localName.starts_with(u8"data-") || namespaceURI != dom::DOMStringAtom::Null()
              || !configuration.DataAttributes.value_or(false)))
      {
        return false;
      }
    }
    // SPEC: Otherwise:
    else
    {
      // SPEC: If elementWithLocalAttributes["attributes"] exists and does not contain attrName, then return
      // false.
      if (localAttributes() && *localAttributes() && !SanitizerListContains(**localAttributes(), attrName))
      {
        return false;
      }

      // SPEC: Otherwise, if configuration["removeAttributes"] exists and contains attrName, then return
      // false.
      if (SanitizerListContains(configuration.RemoveAttributes, attrName))
      {
        return false;
      }
    }

    // SPEC: If configuration["javascriptURLs"] is true, then return true.
    if (configuration.JavascriptURLs.value_or(false))
    {
      return true;
    }

    // SPEC: If the pair (elementName, attrName) matches an entry in the built-in navigating URL attributes
    // list, and if value contains a javascript: URL, then return false.
    if (SanitizerConstants::IsNavigatingURLAttribute(elementName, attrName) && ContainsJavaScriptURL(value))
    {
      return false;
    }

    // SPEC: If elementNamespace is the MathML namespace, localName is "href", namespace is null or the XLink
    // namespace, and value contains a javascript: URL, then return false.
    if (elementNamespace == dom::DOMStringAtom(infra::Namespaces::MathML) && localName == u8"href"
        && (namespaceURI == dom::DOMStringAtom::Null()
            || namespaceURI == dom::DOMStringAtom(infra::Namespaces::XLink))
        && ContainsJavaScriptURL(value))
    {
      return false;
    }

    // SPEC: If the built-in animating URL attributes list contains the pair (elementName, attrName), and
    // value is "href" or "xlink:href", then return false.
    if (SanitizerConstants::IsAnimatingURLAttribute(elementName, attrName)
        && (value == u8"href" || value == u8"xlink:href"))
    {
      return false;
    }

    // SPEC: Return true.
    return true;
  }

  bool SanitizerAlgorithms::IsCustomDataAttribute(const SanitizerAttributeNamespace &attribute) noexcept
  {
    // A custom data attribute is an attribute in no namespace whose name starts with the string "data-", has
    // at least one character after the hyphen, is XML-compatible, and contains no ASCII upper alphas.
    const auto &name = attribute.Name;
    return attribute.Namespace == dom::DOMStringAtom::Null() && name.starts_with(u8"data-") && name.size() > 5
           && dom::NameValidation::IsValidXMLName(name) && name.find(u8':') == dom::DOMString::npos
           && std::ranges::none_of(name, [](char8_t c) { return c >= u8'A' && c <= u8'Z'; });
  }

  bool SanitizerAlgorithms::CompareItems(dom::DOMStringView nameA, dom::DOMStringAtom namespaceA,
                                         dom::DOMStringView nameB, dom::DOMStringAtom namespaceB) noexcept
  {
    // SPEC: Let namespaceA be itemA["namespace"]. Let namespaceB be itemB["namespace"].
    // SPEC: If namespaceA is null: if namespaceB is not null, then return true.
    if (namespaceA == dom::DOMStringAtom::Null())
    {
      if (namespaceB != dom::DOMStringAtom::Null())
      {
        return true;
      }
    }
    // SPEC: Otherwise: if namespaceB is null, then return false. If namespaceA is code unit less than
    // namespaceB, then return true. If namespaceA is not namespaceB, then return false.
    else
    {
      if (namespaceB == dom::DOMStringAtom::Null())
      {
        return false;
      }
      if (infra::StringAlgorithms::CodeUnitLessThan(namespaceA.View(), namespaceB.View()))
      {
        return true;
      }
      if (namespaceA != namespaceB)
      {
        return false;
      }
    }

    // SPEC: If itemA["name"] is code unit less than itemB["name"], then return true. Return false.
    return infra::StringAlgorithms::CodeUnitLessThan(nameA, nameB);
  }
}
