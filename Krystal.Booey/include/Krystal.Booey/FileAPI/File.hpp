#pragma once

#include "Krystal.Booey/FileAPI/Blob.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo
{
  /// @brief A blob with a name: a file on disk, or one made from parts.
  /// @see https://w3c.github.io/FileAPI/#file-section
  class File final : public Blob
  {
  private:
    utf8_string _path;
    utf8_string _name;
    Maybe<int64> _lastModifiedDateOverride;
    mutable Maybe<bool> _isDirectory;

  public:
    /// @see https://w3c.github.io/FileAPI/#dfn-FilePropertyBag
    struct PropertyBag : BlobPropertyBag
    {
      Maybe<int64> LastModified;
    };

    /// @brief The file at @p path, named @p nameOverride if it isn't empty (or else by the path's last
    /// part), typed by its extension.
    KRYS_NODISCARD static Ref<File> Create(utf8_stringview path, utf8_stringview nameOverride = {}) noexcept;
    /// @see https://w3c.github.io/FileAPI/#file-constructor
    KRYS_NODISCARD static Ref<File> Create(List<BlobPartVariant> &&blobPartVariants, utf8_stringview filename,
                                           const PropertyBag &propertyBag) noexcept;
    /// @brief A file with @p blob's bytes and type, named @p name.
    KRYS_NODISCARD static Ref<File> Create(const Blob &blob, utf8_stringview name) noexcept;
    /// @brief A copy of @p existingFile, named @p name if given.
    KRYS_NODISCARD static Ref<File> Create(const File &existingFile, Maybe<utf8_stringview> name) noexcept;

    KRYS_NODISCARD bool IsFile() const noexcept override
    {
      return true;
    }

    /// @brief The file's path, or empty for a file made from parts.
    KRYS_NODISCARD utf8_stringview Path() const noexcept
    {
      return _path;
    }

    /// @see https://w3c.github.io/FileAPI/#dfn-name
    KRYS_NODISCARD utf8_stringview Name() const noexcept
    {
      return _name;
    }

    /// @brief When the file was last modified, in milliseconds since the epoch: as given, or else the file's
    /// (or now, if it can't be read).
    /// @see https://w3c.github.io/FileAPI/#dfn-lastModified
    KRYS_NODISCARD int64 LastModified() const noexcept;
    KRYS_NODISCARD const Maybe<int64> &LastModifiedOverride() const noexcept
    {
      return _lastModifiedDateOverride;
    }

    /// @brief The MIME type of the file at @p path, from its extension (empty if unknown).
    KRYS_NODISCARD static utf8_string ContentTypeForFile(utf8_stringview path) noexcept;

    KRYS_NODISCARD bool IsDirectory() const noexcept;

  private:
    File(Ref<BlobData> &&data, utf8_stringview type, utf8_string &&path, utf8_string &&name) noexcept;
    File(List<BlobPartVariant> &&blobPartVariants, utf8_stringview filename,
         const PropertyBag &propertyBag) noexcept;
    File(const Blob &blob, utf8_stringview name) noexcept;
    File(const File &file, Maybe<utf8_stringview> name) noexcept;

    static void ComputeNameAndContentType(utf8_stringview path, utf8_stringview nameOverride,
                                          utf8_string &effectiveName,
                                          utf8_string &effectiveContentType) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::File)
  KRYS_NODISCARD static bool IsType(const krys::boo::Blob &blob) noexcept
  {
    return blob.IsFile();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
