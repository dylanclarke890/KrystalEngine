/*
 * Copyright (C) 2008-2019 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/FileAPI/File.hpp"

namespace krys::boo
{
  namespace html
  {
    class FileInputType;
  }

  namespace dom
  {
    class DataTransfer;
  }

  /// @brief A list of files: a file input's, or a drop's.
  /// @see https://w3c.github.io/FileAPI/#filelist-section
  class FileList final : public RefCounted<FileList>
  {
    // FileLists can only be changed by their owners.
    friend class dom::DataTransfer;
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
