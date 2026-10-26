#pragma once

#include "Krystal.Booey/FileAPI/File.hpp"

namespace krys::boo
{
  namespace html
  {
    class FileInputType;
  }

  class DataTransfer;

  /// @brief A list of files: a file input's, or a drop's.
  /// @see https://w3c.github.io/FileAPI/#filelist-section
  class FileList final : public RefCounted<FileList>
  {
    // FileLists can only be changed by their owners.
    friend class DataTransfer;
    friend class html::FileInputType;
    KRYS_NON_COPY_MOVABLE(FileList);

  private:
    List<Ref<File>> _files;

  public:
    FileList() noexcept = default;
    explicit FileList(List<Ref<File>> &&files) noexcept : _files(krys::move(files))
    {
    }

    KRYS_NODISCARD static Ref<FileList> Create() noexcept
    {
      return CreateRef<FileList>();
    }
    KRYS_NODISCARD static Ref<FileList> Create(List<Ref<File>> &&files) noexcept
    {
      return CreateRef<FileList>(krys::move(files));
    }

    /// @see https://w3c.github.io/FileAPI/#dfn-length
    KRYS_NODISCARD uint32 Length() const noexcept
    {
      return static_cast<uint32>(_files.size());
    }
    /// @see https://w3c.github.io/FileAPI/#dfn-item
    KRYS_NODISCARD File *Item(uint32 index) const noexcept;

    KRYS_NODISCARD bool IsEmpty() const noexcept
    {
      return _files.empty();
    }
    KRYS_NODISCARD List<utf8_string> Paths() const noexcept;

    KRYS_NODISCARD const List<Ref<File>> &Files() const noexcept
    {
      return _files;
    }
    KRYS_NODISCARD const File &GetFile(uint32 index) const noexcept
    {
      return *_files[index];
    }

  private:
    void Append(Ref<File> &&file) noexcept
    {
      _files.push_back(krys::move(file));
    }
    void Clear() noexcept
    {
      _files.clear();
    }
  };
}
