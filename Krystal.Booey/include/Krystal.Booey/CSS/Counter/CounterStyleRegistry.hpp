#pragma once

#include "Krystal.Booey/CSS/Counter/CounterStyle.hpp"
#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/HashSet.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  using CounterStyleMap = HashMap<CSSOMStringAtom, Ref<CounterStyle>>;

  /// @brief The counter styles of a document: its author `@counter-style` rules, over the predefined (user
  /// agent) counter styles shared by every document.
  class CounterStyleRegistry
  {
  private:
    CounterStyleMap _authorCounterStyles;
    bool _hasUnresolvedReferences {true};

  public:
    CounterStyleRegistry() noexcept = default;

    KRYS_NODISCARD static Ref<CounterStyle> DecimalCounter() noexcept;

    /// @brief The counter style named @p name, with its references resolved.
    // FIXME: Take a Style::CounterStyle once the computed list-style-type and counter() values are ported.
    KRYS_NODISCARD Ref<CounterStyle> ResolvedCounterStyle(const CSSOMStringAtom &name) noexcept;
    void ResolveReferencesIfNeeded() noexcept;

    void AddCounterStyle(const CounterStyleDescriptors &descriptors) noexcept;

    static void AddUserAgentCounterStyle(const CounterStyleDescriptors &descriptors) noexcept;
    static void ResolveUserAgentReferences() noexcept;

    KRYS_NODISCARD bool HasAuthorCounterStyles() const noexcept
    {
      return !_authorCounterStyles.empty();
    }

    void ClearAuthorCounterStyles() noexcept;

    KRYS_NODISCARD bool operator==(const CounterStyleRegistry &other) const noexcept;

  private:
    KRYS_NODISCARD static CounterStyleMap &UserAgentCounterStyles() noexcept;

    // If no map is passed on, user-agent counter styles map will be used
    static void ResolveFallbackReference(CounterStyle &counter, CounterStyleMap *map = nullptr) noexcept;
    static void ResolveExtendsReference(CounterStyle &counter, CounterStyleMap *map = nullptr) noexcept;
    static void ResolveExtendsReference(CounterStyle &counter, HashSet<CounterStyle *> &countersInChain,
                                        CounterStyleMap *map = nullptr) noexcept;

    KRYS_NODISCARD static Ref<CounterStyle> GetCounterStyle(const CSSOMStringAtom &name,
                                                            CounterStyleMap *map = nullptr) noexcept;

    void Invalidate() noexcept;
  };
}
