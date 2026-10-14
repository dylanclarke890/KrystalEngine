#pragma once

#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/Pair.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class StyleSheetContents;
}

namespace krys::boo::style
{
  /// @brief Shares the parsed contents of constructed style sheets that were given the same text and context.
  class StyleSheetContentsCache
  {
    KRYS_NON_COPY_MOVABLE(StyleSheetContentsCache);

  public:
    using Key = Pair<css::CSSOMString, css::ParserContext>;

  private:
    struct KeyHash
    {
      KRYS_NODISCARD size_t operator()(const Key &key) const noexcept;
    };

    HashMap<Key, Ref<css::StyleSheetContents>, KeyHash> _cache;

  public:
    StyleSheetContentsCache() noexcept;
    ~StyleSheetContentsCache() noexcept;

    KRYS_NODISCARD static StyleSheetContentsCache &Singleton() noexcept;

    KRYS_NODISCARD RefPtr<css::StyleSheetContents> Get(const Key &key) const noexcept;
    void Add(Key &&key, Ref<css::StyleSheetContents> contents) noexcept;
    void Clear() noexcept;
  };
}
