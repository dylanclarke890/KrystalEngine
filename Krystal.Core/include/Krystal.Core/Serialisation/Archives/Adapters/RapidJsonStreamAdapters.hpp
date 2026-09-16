#pragma once

#include "Krystal.Core/IO/Streams/Stream.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include "Krystal.Core/Types/Span.hpp"
#include <cassert>

namespace krys::Serialisation
{
  class RapidJsonStreamWriterAdapter
  {
    KRYS_NON_COPY_MOVABLE(RapidJsonStreamWriterAdapter);

  private:
    io::IStreamWriter &_stream;

  public:
    typedef char Ch;

    RapidJsonStreamWriterAdapter(io::IStreamWriter &stream) : _stream(stream)
    {
    }

    void Put(Ch c)
    {
      Array<byte, 1> data = {static_cast<byte>(c)};
      _stream.Write(data);
    }

    Ch *PutBegin()
    {
      assert(false && "Not implemented!");
      return 0;
    }

    size_t PutEnd(Ch *)
    {
      assert(false && "Not implemented!");
      return 0;
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
    typedef char Ch;

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
      return (size_t)_stream.Position();
    }

    void Put(Ch)
    {
      assert(false && "Not implemented!");
    }

    Ch *PutBegin()
    {
      assert(false && "Not implemented!");
      return 0;
    }

    size_t PutEnd(Ch *)
    {
      assert(false && "Not implemented!");
      return 0;
    }
  };
}