#pragma once

#include <string>
#include <vector>
#include <cstddef>
#include <cstdint>

#include <libiw4x/export.hxx>

namespace iw4x
{
  namespace demonware
  {
    // Wire-format type tags for the byte-level serialization protocol.
    //
    // Each typed field in the buffer is prefixed by a one-byte tag that
    // identifies the type of the data that follows. The tag values are fixed
    // by the wire format.
    //
    enum class byte_type_tag : std::uint8_t
    {
      boolean        = 1,
      integer32      = 3,
      integer64      = 5,
      floating       = 6,
      string         = 7,
      blob           = 8,
      struct_header  = 10,
      array_count    = 16
    };

    // Byte-level typed buffer writer.
    //
    // Serializes typed fields into a contiguous byte buffer. Each field is
    // prefixed with a one-byte type tag followed by the data in native byte
    // order.
    //
    class LIBIW4X_SYMEXPORT byte_buffer_writer
    {
    public:
      byte_buffer_writer () = default;

      // Typed field writers. Each writes a type tag followed by the value.
      //
      void
      write_bool (bool);

      void
      write_uint8 (std::uint8_t);

      void
      write_uint32 (std::uint32_t);

      void
      write_int32 (std::int32_t);

      void
      write_uint64 (std::uint64_t);

      void
      write_int64 (std::int64_t);

      // Write a float value. The wire format is: type tag, uint32 length
      // (always sizeof (float)), then the raw float bytes.
      //
      void
      write_float (float);

      // Write a string value. The wire format is: type tag, uint32 length
      // (including null terminator), chars, null byte.
      //
      void
      write_string (const std::string&);

      // Write a binary blob. The wire format is: type tag, uint32 length,
      // raw bytes.
      //
      void
      write_blob (const std::uint8_t*, std::size_t);

      // Write a structure header (type tag 10 followed by a uint32 error
      // code).
      //
      void
      write_struct_header (std::uint32_t error_code);

      // Write an array count (type tag 16 followed by a uint32 count).
      //
      void
      write_array_count (std::uint32_t count);

      // Raw data access.
      //
      const std::uint8_t*
      data () const {return buffer_.data ();}

      std::size_t
      size () const {return buffer_.size ();}

      bool
      empty () const {return buffer_.empty ();}

      // Detach the internal buffer.
      //
      std::vector<std::uint8_t>
      release () {return std::move (buffer_);}

    private:
      void
      append (const void*, std::size_t);

      void
      append_tag (byte_type_tag);

    private:
      std::vector<std::uint8_t> buffer_;
    };

    // Byte-level typed buffer reader.
    //
    // Deserializes typed fields from a contiguous byte buffer. Returns false
    // from each read method if the data is truncated or the type tag does not
    // match the expected value.
    //
    // Note that the reader does not own the data.
    //
    class LIBIW4X_SYMEXPORT byte_buffer_reader
    {
    public:
      byte_buffer_reader (const std::uint8_t* data, std::size_t size)
        : data_ (data), size_ (size), pos_ (0) {}

      // Typed field readers. Each verifies the expected type tag, then reads
      // the value. Returns false if the buffer is truncated or the tag does
      // not match.
      //
      bool
      read_bool (bool&);

      bool
      read_uint8 (std::uint8_t&);

      bool
      read_uint32 (std::uint32_t&);

      bool
      read_int32 (std::int32_t&);

      bool
      read_uint64 (std::uint64_t&);

      bool
      read_int64 (std::int64_t&);

      bool
      read_float (float&);

      bool
      read_string (std::string&);

      bool
      read_blob (std::vector<std::uint8_t>&);

      bool
      read_struct_header (std::uint32_t& error_code);

      // Reader state.
      //
      std::size_t
      position () const {return pos_;}

      std::size_t
      remaining () const {return pos_ < size_ ? size_ - pos_ : 0;}

      bool
      empty () const {return pos_ >= size_;}

    private:
      // Consume size bytes from the buffer into out. Returns false if there
      // are not enough bytes remaining.
      //
      bool
      consume (void* out, std::size_t size);

      // Verify that the next byte matches the expected type tag and advance
      // past it. Returns false if the buffer is empty or the tag does not
      // match.
      //
      bool
      verify_tag (byte_type_tag);

    private:
      const std::uint8_t* data_;
      std::size_t size_;
      std::size_t pos_;
    };
  }
}
