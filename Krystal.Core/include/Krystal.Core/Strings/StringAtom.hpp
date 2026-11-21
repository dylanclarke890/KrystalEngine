#pragma once

#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Containers/HashMap.hpp"
#include "Krystal.Core/Hashing/FastHash.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Move.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Strings/ASCII.hpp"
#include "Krystal.Core/Strings/String.hpp"
#include "Krystal.Core/Types/Markable.hpp"
#include "Krystal.Core/Types/Null.hpp"
#include <utility>

namespace krys
{
  class StringAtomPool;

  /// @brief An interned string and the number of atoms that refer to it.
  class StringAtomImpl
  {
    friend class StringAtom;
    friend class StringAtomPool;
    KRYS_NON_COPY_MOVABLE(StringAtomImpl);

  private:
    utf8_string _string;
    // Of the string's contents, so an atom made again after its string left the pool hashes the same.
    size_t _hash;
    // Not atomic: an atom is only used on the thread that made it.
    mutable uint32 _refCount {0};
    // The pool the string is interned in; null once the pool is gone (its thread has ended).
    StringAtomPool *_pool {nullptr};

  public:
    StringAtomImpl(utf8_string string, StringAtomPool &pool) noexcept
        : _string(krys::move(string)), _hash(FastHash<utf8_stringview> {}(_string)), _pool(&pool)
    {
    }

    KRYS_NODISCARD utf8_stringview View() const noexcept
    {
      return _string;
    }

    /// @brief A hash of the string's contents.
    KRYS_NODISCARD size_t Hash() const noexcept
    {
      return _hash;
    }
  };

  /// @brief An atom's string, for keeping an atom in raw memory such as a union (see
  /// `StringAtom::LeakStorage`).
  using StringAtomStorage = const StringAtomImpl *;

  /// @brief The strings interned on one thread. An atom's string leaves its pool when the last atom that
  /// refers to it goes, as WebKit's `AtomStringImpl`s leave their `AtomStringTable`.
  class StringAtomPool
  {
    KRYS_NON_COPY_MOVABLE(StringAtomPool);

  private:
    // Keyed by a view of each atom's string, which stays put as the atoms are on the heap.
    HashMap<utf8_stringview, StringAtomImpl *> _atoms;

  public:
    StringAtomPool() noexcept = default;

    ~StringAtomPool() noexcept
    {
      // Atoms can outlive their thread's pool (a static atom is destroyed after the main thread's
      // thread-local storage), so they free their string themselves from now on.
      for (auto &[view, atom] : _atoms)
      {
        atom->_pool = nullptr;
      }
    }

    /// @brief The calling thread's pool. Atoms belong to the thread that made them, as WebKit's do: two
    /// atoms for the same string made on different threads aren't equal, and an atom mustn't be copied or
    /// destroyed on another thread.
    KRYS_NODISCARD static StringAtomPool &Current() noexcept
    {
      thread_local StringAtomPool pool {};
      return pool;
    }

    KRYS_NODISCARD StringAtomImpl &GetOrAdd(utf8_stringview string) noexcept
    {
      if (auto it = _atoms.find(string); it != _atoms.end())
      {
        return *it->second;
      }

      return Add(utf8_string(string));
    }

    KRYS_NODISCARD StringAtomImpl &GetOrAdd(utf8_string &&string) noexcept
    {
      if (auto it = _atoms.find(utf8_stringview {string}); it != _atoms.end())
      {
        return *it->second;
      }

      return Add(krys::move(string));
    }

    /// @brief Forgets @p atom's string, as the last atom that referred to it has gone.
    void Remove(const StringAtomImpl &atom) noexcept
    {
      _atoms.erase(atom.View());
    }

    /// @brief The number of strings interned on this thread.
    KRYS_NODISCARD size_t Size() const noexcept
    {
      return _atoms.size();
    }

  private:
    KRYS_NODISCARD StringAtomImpl &Add(utf8_string string) noexcept
    {
      auto *atom = new StringAtomImpl(krys::move(string), *this);
      _atoms.emplace(atom->View(), atom);
      return *atom;
    }
  };

  /// @brief An interned string: atoms for the same string (made on the same thread) share it, so they compare
  /// and hash as pointers. The null atom has no string.
  class StringAtom
  {
  private:
    const StringAtomImpl *_impl {nullptr};

    struct EmptyTag
    {
    };

    struct AdoptTag
    {
    };

    StringAtom(EmptyTag) noexcept : StringAtom(StringAtomPool::Current().GetOrAdd(utf8_stringview {u8""}))
    {
    }

    StringAtom(const StringAtomImpl *impl, AdoptTag) noexcept : _impl(impl)
    {
    }

    explicit StringAtom(const StringAtomImpl &impl) noexcept : _impl(&impl)
    {
      Ref(_impl);
    }

    template <typename TNeedsConverting, typename TConvert>
    KRYS_NODISCARD StringAtom ConvertASCIICase(TNeedsConverting needsConverting,
                                               TConvert convert) const noexcept
    {
      auto view = View();
      auto first = view.begin();
      while (first != view.end() && !needsConverting(*first))
      {
        ++first;
      }
      if (first == view.end())
      {
        return *this;
      }

      utf8_string converted(view);
      for (auto it = converted.begin() + (first - view.begin()); it != converted.end(); ++it)
      {
        *it = convert(*it);
      }
      return StringAtom(krys::move(converted));
    }

    static void Ref(const StringAtomImpl *impl) noexcept
    {
      if (impl)
      {
        krys_debug_assert(!impl->_pool || impl->_pool == &StringAtomPool::Current());
        ++impl->_refCount;
      }
    }

    static void Deref(const StringAtomImpl *impl) noexcept
    {
      if (!impl)
      {
        return;
      }

      krys_debug_assert(!impl->_pool || impl->_pool == &StringAtomPool::Current());
      krys_debug_assert(impl->_refCount);
      if (--impl->_refCount == 0)
      {
        if (impl->_pool)
        {
          impl->_pool->Remove(*impl);
        }
        delete impl;
      }
    }

  public:
    /// @brief The null atom, which has no string.
    constexpr StringAtom() noexcept = default;

    /// @brief The null atom, which has no string.
    constexpr StringAtom(null_t) noexcept
    {
    }

    /// @brief The null atom, for functions that return a reference to an atom or the null one.
    KRYS_NODISCARD static const StringAtom &Null() noexcept
    {
      // Shared by every thread: the null atom has no string, so it never touches a pool.
      static const StringAtom nullAtom;
      return nullAtom;
    }

    /// @brief The calling thread's empty atom.
    KRYS_NODISCARD static const StringAtom &Empty() noexcept
    {
      thread_local const StringAtom emptyAtom = StringAtom(EmptyTag {});
      return emptyAtom;
    }

    /// @brief The atom for a string literal (implicit, as literals are names the code already knows).
    StringAtom(const char8 *str) noexcept
        : StringAtom(StringAtomPool::Current().GetOrAdd(utf8_stringview {str}))
    {
    }

    /// @brief The atom for @p str. Explicit, as interning a string that came from elsewhere costs a lookup
    /// and maybe an allocation, which call sites should show.
    explicit StringAtom(utf8_stringview str) noexcept : StringAtom(StringAtomPool::Current().GetOrAdd(str))
    {
    }

    explicit StringAtom(const utf8_string &str) noexcept
        : StringAtom(StringAtomPool::Current().GetOrAdd(utf8_stringview {str}))
    {
    }

    explicit StringAtom(utf8_string &&str) noexcept
        : StringAtom(StringAtomPool::Current().GetOrAdd(krys::move(str)))
    {
    }

    StringAtom(const StringAtom &other) noexcept : _impl(other._impl)
    {
      Ref(_impl);
    }

    StringAtom(StringAtom &&other) noexcept : _impl(std::exchange(other._impl, nullptr))
    {
    }

    // NOLINTNEXTLINE(bugprone-unhandled-self-assignment): Ref before Deref.
    StringAtom &operator=(const StringAtom &other) noexcept
    {
      Ref(other._impl);
      Deref(_impl);
      _impl = other._impl;
      return *this;
    }

    StringAtom &operator=(StringAtom &&other) noexcept
    {
      if (this != &other)
      {
        Deref(_impl);
        _impl = std::exchange(other._impl, nullptr);
      }
      return *this;
    }

    ~StringAtom() noexcept
    {
      Deref(_impl);
    }

    /// @brief Takes back an atom given up with `LeakStorage`.
    KRYS_NODISCARD static StringAtom AdoptStorage(StringAtomStorage storage) noexcept
    {
      return StringAtom(storage, AdoptTag {});
    }

    /// @brief Adds a reference to an atom kept as storage, for copying it.
    static void RefStorage(StringAtomStorage storage) noexcept
    {
      Ref(storage);
    }

    /// @brief Releases an atom kept as storage, as its destructor would.
    static void DerefStorage(StringAtomStorage storage) noexcept
    {
      Deref(storage);
    }

    KRYS_NODISCARD friend bool operator==(const StringAtom &a, const StringAtom &b) noexcept
    {
      return a._impl == b._impl;
    }

    /// @brief Whether the atom is the null atom.
    KRYS_NODISCARD friend constexpr bool operator==(const StringAtom &atom, null_t) noexcept
    {
      return !atom._impl;
    }

    /// @brief Whether the atom's string is @p string, without interning it. The null atom has no string, so
    /// it equals none (not even an empty one).
    KRYS_NODISCARD friend bool operator==(const StringAtom &atom, utf8_stringview string) noexcept
    {
      return atom._impl && atom._impl->View() == string;
    }

    KRYS_NODISCARD friend bool operator==(const StringAtom &atom, const char8 *string) noexcept
    {
      return atom == utf8_stringview {string};
    }

    KRYS_NODISCARD friend bool operator==(const StringAtom &atom, const utf8_string &string) noexcept
    {
      return atom == utf8_stringview {string};
    }

    /// @brief The interned string; empty for the null atom.
    KRYS_NODISCARD utf8_stringview View() const noexcept
    {
      return _impl ? _impl->View() : utf8_stringview {};
    }

    /// @brief The atom with ASCII upper-case letters lowered: this atom itself, without interning anything,
    /// if it has none (most names don't).
    KRYS_NODISCARD StringAtom ToASCIILower() const noexcept
    {
      return ConvertASCIICase([](char8 c) { return text::IsASCIIUpper(c); },
                              [](char8 c) { return text::ToASCIILower(c); });
    }

    /// @brief The atom with ASCII lower-case letters raised: this atom itself if it has none.
    KRYS_NODISCARD StringAtom ToASCIIUpper() const noexcept
    {
      return ConvertASCIICase([](char8 c) { return text::IsASCIILower(c); },
                              [](char8 c) { return text::ToASCIIUpper(c); });
    }

    /// @brief A copy of the interned string; empty for the null atom.
    KRYS_NODISCARD utf8_string ToString() const noexcept
    {
      return utf8_string(View());
    }

    /// @brief A hash of the string's contents (zero for the null atom). Unlike the atom's address, it's the
    /// same for every atom made from the string, even after the string has left the pool and come back.
    KRYS_NODISCARD size_t ExistingHash() const noexcept
    {
      return _impl ? _impl->Hash() : 0uz;
    }

    /// @brief The atom's string, which stays valid while an atom refers to it.
    KRYS_NODISCARD StringAtomStorage Storage() const noexcept
    {
      return _impl;
    }

    /// @brief Gives up the atom's reference to its string, for keeping it in raw memory: the storage must
    /// later be passed to `AdoptStorage` or `DerefStorage`.
    KRYS_NODISCARD StringAtomStorage LeakStorage() && noexcept
    {
      return std::exchange(_impl, nullptr);
    }
  };

  // Selectors and CSS values keep atoms in unions and cast them back.
  static_assert(sizeof(StringAtom) == sizeof(StringAtomStorage));

  /// @brief The null atom marks an empty `Markable<StringAtom>`.
  template <>
  struct MarkableTraits<StringAtom>
  {
    KRYS_NODISCARD static bool IsEmptyValue(const StringAtom &atom) noexcept
    {
      return atom == null;
    }

    KRYS_NODISCARD static StringAtom EmptyValue() noexcept
    {
      return null;
    }
  };

  /// @brief An atom's hash is the one it made of its string when it was created, so hashing one never reads
  /// the string.
  template <>
  struct FastHash<StringAtom>
  {
    using is_avalanching = std::true_type;

    KRYS_NODISCARD size_t operator()(const StringAtom &atom) const noexcept
    {
      return atom.ExistingHash();
    }
  };
}
