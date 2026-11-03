#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

struct _xmlParserInput;
struct _xmlParserCtxt;

namespace krys::boo
{
  class CachedResourceLoader;
}

namespace krys::boo::xml
{
  /// @brief While alive, the loads libxml2 asks for (external entities, DTDs) are made for the given cached
  /// resource loader's document, under its rules about what may be loaded.
  class XMLDocumentParserScope
  {
    KRYS_NON_COPY_MOVABLE(XMLDocumentParserScope);
    using EntityLoaderFunction = _xmlParserInput *(*)(const char *, const char *, _xmlParserCtxt *);

  private:
    WeakPtr<CachedResourceLoader> _oldCachedResourceLoader;
    EntityLoaderFunction _oldEntityLoader {nullptr};

  public:
    XMLDocumentParserScope() = delete;
    explicit XMLDocumentParserScope(CachedResourceLoader *) noexcept;
    ~XMLDocumentParserScope() noexcept;

    KRYS_NODISCARD static WeakPtr<CachedResourceLoader> &CurrentCachedResourceLoader() noexcept;
  };
}
