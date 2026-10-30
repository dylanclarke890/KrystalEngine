#include "Krystal.Booey/XML/Parser/XMLDocumentParserScope.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceLoader.hpp"
#include "Krystal.Booey/XML/Parser/XMLDocumentParser.hpp"
#include <libxml/parser.h>

namespace krys::boo::xml
{
  WeakPtr<CachedResourceLoader> &XMLDocumentParserScope::CurrentCachedResourceLoader() noexcept
  {
    static WeakPtr<CachedResourceLoader> currentCachedResourceLoader;
    return currentCachedResourceLoader;
  }

  XMLDocumentParserScope::XMLDocumentParserScope(CachedResourceLoader *cachedResourceLoader) noexcept
      : _oldCachedResourceLoader(CurrentCachedResourceLoader())
  {
    InitializeXMLParser();
    _oldEntityLoader = xmlGetExternalEntityLoader();
    CurrentCachedResourceLoader() = CreateWeakPtr(cachedResourceLoader);
    xmlSetExternalEntityLoader(ExternalEntityLoader);
  }

  XMLDocumentParserScope::~XMLDocumentParserScope() noexcept
  {
    CurrentCachedResourceLoader() = _oldCachedResourceLoader;
    xmlSetExternalEntityLoader(_oldEntityLoader);
  }
}
