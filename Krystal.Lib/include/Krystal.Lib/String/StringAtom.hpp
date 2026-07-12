#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/Hash.hpp"
#include "Krystal.Lib/String/String.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Lib/Types/Set.hpp"
#include <xutility>

namespace Krys
{
  class StringAtomPool
  {
  public:
    using InternedString = const utf8_string *;

    struct UTF8Hash
    {
      using is_transparent = void;

      size_t operator()(utf8_stringview v) const noexcept
      {
        return std::hash<utf8_stringview> {}(v);
      }

      size_t operator()(const utf8_string &s) const noexcept
      {
        return (*this)(utf8_stringview {s});
      }
    };

    struct UTF8KeyEqual
    {
      using is_transparent = void;

      bool operator()(utf8_stringview a, utf8_stringview b) const noexcept
      {
        return a == b;
      }

      bool operator()(const utf8_string &a, utf8_stringview b) const noexcept
      {
        return utf8_stringview {a} == b;
      }

      bool operator()(utf8_stringview a, const utf8_string &b) const noexcept
      {
        return a == utf8_stringview {b};
      }

      bool operator()(const utf8_string &a, const utf8_string &b) const noexcept
      {
        return a == b;
      }
    };

  private:
    Set<utf8_string, UTF8Hash, UTF8KeyEqual> _strings;

  public:
    StringAtomPool() noexcept = default;

    KRYS_NODISCARD InternedString GetOrAdd(utf8_stringview v) noexcept
    {
      if (auto it = _strings.find(v); it != _strings.end())
      {
        return std::addressof(*it);
      }

      // Only allocate when inserting a new unique string.
      auto [insertedIt, wasInserted] = _strings.insert(utf8_string(v));
      return std::addressof(*insertedIt);
    }

    KRYS_NODISCARD InternedString GetOrAdd(const utf8_string &s) noexcept
    {
      return GetOrAdd(utf8_stringview {s});
    }

    KRYS_NODISCARD InternedString GetOrAdd(utf8_string &&s) noexcept
    {
      if (auto it = _strings.find(utf8_stringview {s}); it != _strings.end())
      {
        return std::addressof(*it);
      }

      auto [insertedIt, wasInserted] = _strings.insert(std::move(s));
      return std::addressof(*insertedIt);
    }
  };

  class StringAtom
  {
    using InternedString = StringAtomPool::InternedString;

  private:
    InternedString _ptr {nullptr};

    static StringAtomPool &Pool()
    {
      static StringAtomPool globalPool {};
      return globalPool;
    }

    struct NullTag
    {
    };

    struct EmptyTag
    {
    };

    StringAtom(NullTag) noexcept : _ptr(nullptr)
    {
    }

    StringAtom(EmptyTag) noexcept : _ptr(Pool().GetOrAdd(utf8_stringview {u8""}))
    {
    }

  public:
    KRYS_NODISCARD static StringAtom Null() noexcept
    {
      static StringAtom nullAtom = StringAtom(NullTag {});
      return nullAtom;
    }

    KRYS_NODISCARD static StringAtom Empty() noexcept
    {
      static StringAtom emptyAtom = StringAtom(EmptyTag {});
      return emptyAtom;
    }

    StringAtom(const char8 *str) noexcept : _ptr(Pool().GetOrAdd(utf8_stringview {str}))
    {
    }

    StringAtom(utf8_stringview str) noexcept : _ptr(Pool().GetOrAdd(str))
    {
    }

    StringAtom(const utf8_string &str) noexcept : _ptr(Pool().GetOrAdd(str))
    {
    }

    StringAtom(utf8_string &&str) noexcept : _ptr(Pool().GetOrAdd(std::move(str)))
    {
    }

    KRYS_NODISCARD friend bool operator==(StringAtom a, StringAtom b) noexcept
    {
      return a._ptr == b._ptr;
    }

    KRYS_NODISCARD utf8_stringview View() const noexcept
    {
      return *_ptr;
    }
  };
}

namespace std
{
  template <>
  struct hash<Krys::StringAtom>
  {
    constexpr size_t operator()(const Krys::StringAtom &qName) const noexcept
    {
      if (qName == Krys::StringAtom::Null())
      {
        return 0;
      }

      return Krys::Hash::Combine(qName.View().data());
    }
  };
}