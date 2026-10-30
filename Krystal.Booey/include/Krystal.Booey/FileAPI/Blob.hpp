#pragma once

#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Booey/FileAPI/BlobPropertyBag.hpp"
#include "Krystal.Booey/Platform/Network/BlobData.hpp"
#include "Krystal.Booey/Platform/SharedBuffer.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/String.hpp"
#include "Krystal.Core/Types/Variant.hpp"

namespace krys::boo
{
  class Blob;

  /// @brief A part a blob is made from: bytes, another blob, or a string (encoded as UTF-8).
  using BlobPartVariant = Variant<List<uint8>, RefPtr<Blob>, utf8_string>;

  /// @brief Immutable bytes with a type: made from parts, or a file's.
  /// @see https://w3c.github.io/FileAPI/#blob-section
  class Blob : public RefCounted<Blob>
  {
    KRYS_NON_COPY_MOVABLE(Blob);

  private:
    utf8_string _type;
    mutable Maybe<uint64> _size;
    size_t _memoryCost {0};
    Ref<BlobData> _data;

  public:
    /// @brief An empty blob.
    KRYS_NODISCARD static Ref<Blob> Create() noexcept;
    /// @see https://w3c.github.io/FileAPI/#constructorBlob
    KRYS_NODISCARD static Ref<Blob> Create(List<BlobPartVariant> &&blobPartVariants,
                                           const BlobPropertyBag &propertyBag) noexcept;
    KRYS_NODISCARD static Ref<Blob> Create(List<uint8> &&data, utf8_stringview contentType) noexcept;
    KRYS_NODISCARD static Ref<Blob> Create(Ref<SharedBuffer> &&buffer, utf8_stringview contentType) noexcept;

    virtual ~Blob() noexcept = default;

    /// @see https://w3c.github.io/FileAPI/#dfn-type
    KRYS_NODISCARD utf8_stringview Type() const noexcept
    {
      return _type;
    }

    /// @see https://w3c.github.io/FileAPI/#dfn-size
    KRYS_NODISCARD uint64 Size() const noexcept;
    KRYS_NODISCARD virtual bool IsFile() const noexcept
    {
      return false;
    }

    /// @brief The ranges of memory and files the blob's bytes are in.
    KRYS_NODISCARD const BlobData &Data() const noexcept
    {
      return *_data;
    }

    // The checks described in the File API spec.
    KRYS_NODISCARD static bool IsValidContentType(utf8_stringview contentType) noexcept;
    // The normalization procedure described in the File API spec.
    KRYS_NODISCARD static utf8_string NormalizedContentType(utf8_stringview contentType) noexcept;

    /// @see https://w3c.github.io/FileAPI/#dfn-slice
    KRYS_NODISCARD Ref<Blob> Slice(int64 start, int64 end, utf8_stringview contentType) const noexcept;

    /// @brief Reads the blob's bytes.
    /// @return The bytes, or `NotFoundError` if a file it's from has gone, or `NotReadableError` if one
    /// can't be read.
    /// @see https://w3c.github.io/FileAPI/#dom-blob-bytes
    KRYS_NODISCARD dom::ExceptionOr<List<uint8>> Bytes() const noexcept;
    /// @brief Reads the blob's bytes, decoded as UTF-8.
    /// @see https://w3c.github.io/FileAPI/#dom-blob-text
    KRYS_NODISCARD dom::ExceptionOr<utf8_string> Text() const noexcept;

    KRYS_NODISCARD size_t MemoryCost() const noexcept
    {
      return _memoryCost;
    }

  protected:
    enum ReferencingExistingBlobConstructor : bool
    {
      ReferencingExistingBlob
    };

    Blob(Ref<BlobData> &&data, utf8_stringview type, Maybe<uint64> size, size_t memoryCost) noexcept;
    Blob(List<BlobPartVariant> &&blobPartVariants, const BlobPropertyBag &propertyBag) noexcept;
    /// @brief A blob with @p blob's bytes and type.
    Blob(ReferencingExistingBlobConstructor, const Blob &blob) noexcept;
  };
}
