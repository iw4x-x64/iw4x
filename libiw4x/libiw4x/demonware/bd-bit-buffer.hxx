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
    // Wire-format type tags for the bit-level serialization protocol.
    //
    // Each typed field is prefixed by a 5-bit tag that identifies the type
    // of the data that follows. The tag values are fixed by the wire format.
    //
    enum class bit_type_tag : std::uint8_t
    {
      boolean  = 1,
      byte     = 3,
      int16    = 5,
      uint16   = 6,
      int32    = 7,
      uint32   = 8,
      int64    = 9,
      uint64   = 10,
      floating = 13,
      string   = 16,
      blob     = 19
    };

    // Raw bit-level writer.
    //
    // Writes individual bits or groups of bits into a growable byte buffer.
    // Bits are packed LSB-first within each byte.
    //
    class LIBIW4X_SYMEXPORT bit_writer
    {
    public:
      bit_writer ();

      // Write a single bit.
      //
      void
      write_bit (bool);

      // Write the n least-significant bits of value (1 <= n <= 64).
      //
      void
      write_bits (std::uint64_t value, unsigned int n);

      // Write a sequence of complete bytes as bits.
      //
      void
      write_bytes (const std::uint8_t*, std::size_t);

      // Raw data access.
      //
      const std::uint8_t*
      data () const {return buffer_.data ();}

      // Size in bytes (rounded up from the current bit position).
      //
      std::size_t
      size () const;

      // Exact size in bits.
      //
      std::size_t
      bit_size () const {return bit_pos_;}

    private:
      void
      ensure_capacity (std::size_t additional_bits);

    private:
      std::vector<std::uint8_t> buffer_;
      std::size_t bit_pos_;
    };

    // Raw bit-level reader.
    //
    // Reads individual bits or groups of bits from a byte buffer. Note that the
    // reader does not own the data.
    //
    class LIBIW4X_SYMEXPORT bit_reader
    {
    public:
      bit_reader (const std::uint8_t* data, std::size_t size_bytes)
        : data_ (data), size_ (size_bytes), bit_pos_ (0) {}

      // Read a single bit. Returns false if past the end of the buffer.
      //
      bool
      read_bit (bool&);

      // Read n bits into the least-significant bits of value (1 <= n <= 64).
      // Returns false if there are not enough bits remaining.
      //
      bool
      read_bits (std::uint64_t& value, unsigned int n);

      // Read a sequence of complete bytes. Returns false if there are not
      // enough bits remaining.
      //
      bool
      read_bytes (std::uint8_t*, std::size_t);

      // Current bit position.
      //
      std::size_t
      position () const {return bit_pos_;}

      // Set the current bit position (e.g., to rewind before reading).
      //
      void
      set_position (std::size_t bit) {bit_pos_ = bit;}

    private:
      const std::uint8_t* data_;
      std::size_t size_;  // In bytes.
      std::size_t bit_pos_;
    };

    // Typed bit-buffer writer.
    //
    // Serializes typed fields using the 5-bit type tag protocol. Each field
    // is prefixed with a 5-bit tag written via the underlying bit_writer,
    // followed by the value bits.
    //
    class LIBIW4X_SYMEXPORT bit_buffer_writer
    {
    public:
      bit_buffer_writer () = default;

      // Typed field writers.
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
      write_float (float);

      void
      write_string (const char*);

      void
      write_blob (const std::uint8_t*, std::size_t);

      // Write raw bytes without a type tag prefix.
      //
      void
      write_bytes (const std::uint8_t* data, std::size_t size)
      {
        writer_.write_bytes (data, size);
      }

      // Access the underlying bit writer (e.g., for raw bit operations).
      //
      bit_writer&
      writer () {return writer_;}

      const bit_writer&
      writer () const {return writer_;}

      // Forwarded accessors.
      //
      const std::uint8_t*
      data () const {return writer_.data ();}

      std::size_t
      size () const {return writer_.size ();}

      std::size_t
      bit_size () const {return writer_.bit_size ();}

    private:
      void
      write_tag (bit_type_tag);

    private:
      bit_writer writer_;
    };

    // Typed bit-buffer reader.
    //
    // Deserializes typed fields from a bit buffer using the 5-bit type tag
    // protocol. Returns false from each read method if the data is truncated
    // or the type tag does not match the expected value.
    //
    class LIBIW4X_SYMEXPORT bit_buffer_reader
    {
    public:
      bit_buffer_reader (const std::uint8_t* data, std::size_t size_bytes)
        : reader_ (data, size_bytes) {}

      // Typed field readers.
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

      // Read a null-terminated string up to max_len characters (not counting
      // the null terminator). Returns false if the data is truncated, the
      // type tag does not match, or no null terminator is found within
      // max_len characters.
      //
      bool
      read_string (std::string&, std::size_t max_len);

      bool
      read_blob (std::vector<std::uint8_t>&);

      // Access the underlying bit reader.
      //
      bit_reader&
      reader () {return reader_;}

      const bit_reader&
      reader () const {return reader_;}

      // Forwarded state.
      //
      std::size_t
      position () const {return reader_.position ();}

      void
      set_position (std::size_t bit) {reader_.set_position (bit);}

    private:
      bool
      verify_tag (bit_type_tag);

    private:
      bit_reader reader_;
    };
  }
}
