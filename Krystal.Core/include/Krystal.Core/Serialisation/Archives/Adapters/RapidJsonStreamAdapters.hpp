#pragma once

#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/IO/Streams/Stream.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/Array.hpp"

namespace krys::serialisation
{
  class RapidJsonStreamWriterAdapter
  {
    KRYS_NON_COPY_MOVABLE(RapidJsonStreamWriterAdapter);

  private:
    io::IStreamWriter &_stream;

  public:
    using Ch = char;

    RapidJsonStreamWriterAdapter(io::IStreamWriter &stream) : _stream(stream)
    {
    }

    void Put(Ch c)
    {
      Array<byte, 1> data = {static_cast<byte>(c)};
      _stream.Write(data);
    }

    static Ch *PutBegin()
    {
      krys_not_implemented();
    }

    static size_t PutEnd(Ch *)
    {
      krys_not_implemented();
    }

    void Flush()
    {
      _stream.Flush();
    }
  };

  class RapidJsonStreamReaderAdapter
  {
    KRYS_NON_COPY_MOVABLE(RapidJsonStreamReaderAdapter);

  private:
    io::IStreamReader &_stream;

  public:
    using Ch = char;

    RapidJsonStreamReaderAdapter(io::IStreamReader &stream) : _stream(stream)
    {
    }

    Ch Peek() const
    {
      if (byte next; _stream.Peek(next))
      {
        return (Ch)next;
      }
      return '\0';
    }

    Ch Take()
    {
      Array<byte, 1> next {};
      if (_stream.Read(next) == 1)
      {
        return static_cast<Ch>(next[0]);
      }
      return '\0';
    }

    size_t Tell() const
    {
      return _stream.Position();
    }

    static void Put(Ch)
    {
      krys_not_implemented();
    }

    static Ch *PutBegin()
    {
      krys_not_implemented();
    }

    static size_t PutEnd(Ch *)
    {
      krys_not_implemented();
    }
  };
}
