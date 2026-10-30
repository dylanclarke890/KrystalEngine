#pragma once

#include "Krystal.Booey/Platform/Network/FormData.hpp"

namespace krys::boo::FormDataBuilder
{
  /// @brief Encodes @p string as `application/x-www-form-urlencoded` data.
  void EncodeStringAsFormData(List<uint8> &buffer, Span<const uint8> string) noexcept;

  /// @brief Appends a name and value pair, encoded as @p encodingType (URL-encoded or plain text).
  void AddKeyValuePairAsFormData(List<uint8> &buffer, Span<const uint8> key, Span<const uint8> value,
                                 FormData::EncodingType encodingType) noexcept;

  // Helper functions used by HTMLFormElement for multi-part form data.
  KRYS_NODISCARD List<uint8> GenerateUniqueBoundaryString() noexcept;
  void BeginMultiPartHeader(List<uint8> &buffer, Span<const uint8> boundary, Span<const uint8> name) noexcept;
  void AddBoundaryToMultiPartHeader(List<uint8> &buffer, Span<const uint8> boundary,
                                    bool isLastBoundary = false) noexcept;
  void AddFilenameToMultiPartHeader(List<uint8> &buffer, utf8_stringview filename) noexcept;
  void AddContentTypeToMultiPartHeader(List<uint8> &buffer, utf8_stringview mimeType) noexcept;
  void FinishMultiPartHeader(List<uint8> &buffer) noexcept;
}
