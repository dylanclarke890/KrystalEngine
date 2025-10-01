#pragma once

#include "Krystal.IO/IStream.hpp"
#include "Krystal.Lib/Macros.hpp"
#include <cassert>

namespace Krys::Serialisation
{
  class RapidJsonStreamWriterAdapter
  {
  private:
    NO_COPY_MOVE(RapidJsonStreamWriterAdapter)

    IO::IStreamWriter &_stream;

  public:
    typedef char Ch;

    RapidJsonStreamWriterAdapter(IO::IStreamWriter &stream) : _stream(stream)
    {
    }

    void Put(Ch c)
    {
      byte b = static_cast<byte>(c);
      _stream.Write(&b, 1);
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
  private:
    NO_COPY_MOVE(RapidJsonStreamReaderAdapter)

    IO::IStreamReader &_stream;

  public:
    typedef char Ch;

    RapidJsonStreamReaderAdapter(IO::IStreamReader &stream) : _stream(stream)
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
      byte next {};
      if (_stream.Read(&next, 1) == 1)
      {
        return (Ch)next;
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