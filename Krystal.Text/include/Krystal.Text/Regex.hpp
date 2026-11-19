#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Strings/String.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

// ICU's compiled regular expression (`unicode/uregex.h`); only the sources include ICU.
struct URegularExpression;

namespace krys::text
{
  /// @brief A compiled regular expression, in ICU's syntax (close to ECMAScript's), over UTF-8 text.
  class Regex
  {
    KRYS_NON_COPYABLE(Regex);

  private:
    URegularExpression *_regex {nullptr};

    explicit Regex(URegularExpression *regex) noexcept : _regex(regex)
    {
    }

  public:
    /// @brief Compiles @p pattern; unknown escapes are errors, as with ECMAScript's `u` and `v` flags.
    /// @return The expression, or null if @p pattern isn't valid.
    KRYS_NODISCARD static Maybe<Regex> Compile(utf8_stringview pattern) noexcept;

    Regex(Regex &&other) noexcept;

    Regex &operator=(Regex &&other) noexcept;

    ~Regex() noexcept;

    /// @brief Whether the expression matches the whole of @p text (as if anchored with `^(?:` and `)$`).
    KRYS_NODISCARD bool MatchesWhole(utf8_stringview text) const noexcept;
  };
}
