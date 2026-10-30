#include "Krystal.Booey/Core/BidiContext.hpp"

#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Utils/NeverDestroyed.hpp"

namespace krys::boo
{
  BidiContext::BidiContext(uint8 level, UCharDirection direction, bool override, BidiEmbeddingSource source,
                           BidiContext *parent) noexcept
      : _level(level), _direction(static_cast<uint32>(direction)), _override(override),
        _source(static_cast<uint32>(source)), _parent(ShareRefPtr(parent))
  {
  }

  Ref<BidiContext> BidiContext::CreateUncached(uint8 level, UCharDirection direction, bool override,
                                               BidiEmbeddingSource source, BidiContext *parent) noexcept
  {
    return AdoptRef(*new BidiContext(level, direction, override, source, parent));
  }

  Ref<BidiContext> BidiContext::Create(uint8 level, UCharDirection direction, bool override,
                                       BidiEmbeddingSource source, BidiContext *parent) noexcept
  {
    krys_debug_assert(direction == (level % 2 ? U_RIGHT_TO_LEFT : U_LEFT_TO_RIGHT));

    if (parent)
    {
      return CreateUncached(level, direction, override, source, parent);
    }

    krys_debug_assert(level <= 1);
    if (!level)
    {
      if (!override)
      {
        static NeverDestroyed<RefPtr<BidiContext>> ltrContext =
          CreateUncached(0, U_LEFT_TO_RIGHT, false, BidiEmbeddingSource::FromStyleOrDOM, nullptr);
        return ltrContext.get();
      }

      static NeverDestroyed<RefPtr<BidiContext>> ltrOverrideContext =
        CreateUncached(0, U_LEFT_TO_RIGHT, true, BidiEmbeddingSource::FromStyleOrDOM, nullptr);
      return ltrOverrideContext.get();
    }

    if (!override)
    {
      static NeverDestroyed<RefPtr<BidiContext>> rtlContext =
        CreateUncached(1, U_RIGHT_TO_LEFT, false, BidiEmbeddingSource::FromStyleOrDOM, nullptr);
      return rtlContext.get();
    }

    static NeverDestroyed<RefPtr<BidiContext>> rtlOverrideContext =
      CreateUncached(1, U_RIGHT_TO_LEFT, true, BidiEmbeddingSource::FromStyleOrDOM, nullptr);
    return rtlOverrideContext.get();
  }

  namespace
  {
    Ref<BidiContext> CopyContextAndRebaselineLevel(BidiContext &context, BidiContext *parent) noexcept
    {
      auto newLevel = parent ? parent->Level() : uint8 {0};
      if (context.Dir() == U_RIGHT_TO_LEFT)
      {
        newLevel = NextGreaterOddLevel(newLevel);
      }
      else if (parent)
      {
        newLevel = NextGreaterEvenLevel(newLevel);
      }
      return BidiContext::Create(newLevel, context.Dir(), context.Override(), context.Source(), parent);
    }
  }

  // The BidiContext stack must be immutable -- they're re-used for re-layout after
  // DOM modification/editing -- so we copy all the non-Unicode contexts, and
  // recalculate their levels.
  Ref<BidiContext> BidiContext::CopyStackRemovingUnicodeEmbeddingContexts() noexcept
  {
    List<BidiContext *> contexts;
    for (auto *ancestor = this; ancestor; ancestor = ancestor->Parent())
    {
      if (ancestor->Source() != BidiEmbeddingSource::FromUnicode)
      {
        contexts.push_back(ancestor);
      }
    }
    krys_debug_assert(!contexts.empty());
    auto topContext = CopyContextAndRebaselineLevel(*contexts.back(), nullptr);
    for (auto i = contexts.size() - 1; i; --i)
    {
      topContext = CopyContextAndRebaselineLevel(*contexts[i - 1], topContext.get());
    }
    return topContext;
  }

  bool operator==(const BidiContext &c1, const BidiContext &c2) noexcept
  {
    if (&c1 == &c2)
    {
      return true;
    }
    if (c1.Level() != c2.Level() || c1.Override() != c2.Override() || c1.Dir() != c2.Dir()
        || c1.Source() != c2.Source())
    {
      return false;
    }
    if (!c1.Parent())
    {
      return !c2.Parent();
    }
    return c2.Parent() && *c1.Parent() == *c2.Parent();
  }
}
