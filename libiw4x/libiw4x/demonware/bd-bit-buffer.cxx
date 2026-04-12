#include <libiw4x/demonware/bd-bit-buffer.hxx>

#include <cassert>
#include <cstring>

using namespace std;

namespace iw4x
{
  namespace demonware
  {
    // bit_writer
    //

    bit_writer::
    bit_writer ()
      : bit_pos_ (0)
    {
      buffer_.resize (64, 0);
    }

    size_t bit_writer::
    size () const
    {
      return (bit_pos_ + 7) / 8;
    }

    void bit_writer::
    ensure_capacity (size_t additional_bits)
    {
      size_t required_bytes ((bit_pos_ + additional_bits + 7) / 8);

      if (required_bytes > buffer_.size ())
        buffer_.resize (required_bytes + 64, 0);
    }

    void bit_writer::
    write_bit (bool b)
    {
      ensure_capacity (1);

      if (b)
      {
        size_t byte_index (bit_pos_ / 8);
        unsigned int bit_offset (bit_pos_ % 8);

        buffer_[byte_index] |= static_cast<uint8_t> (1u << bit_offset);
      }

      ++bit_pos_;
    }

    void bit_writer::
    write_bits (uint64_t value, unsigned int n)
    {
      assert (n >= 1 && n <= 64);

      ensure_capacity (n);

      for (unsigned int i (0); i < n; ++i)
        write_bit ((value >> i) & 1);
    }

    void bit_writer::
    write_bytes (const uint8_t* data, size_t len)
    {
      for (size_t i (0); i < len; ++i)
        write_bits (data[i], 8);
    }

    // bit_reader
    //

    bool bit_reader::
    read_bit (bool& b)
    {
      if (bit_pos_ >= size_ * 8)
        return false;

      size_t byte_index (bit_pos_ / 8);
      unsigned int bit_offset (bit_pos_ % 8);

      b = (data_[byte_index] >> bit_offset) & 1;

      ++bit_pos_;
      return true;
    }

    bool bit_reader::
    read_bits (uint64_t& value, unsigned int n)
    {
      assert (n >= 1 && n <= 64);

      if (bit_pos_ + n > size_ * 8)
        return false;

      value = 0;

      for (unsigned int i (0); i < n; ++i)
      {
        bool b;
        if (!read_bit (b))
          return false;

        if (b)
          value |= (static_cast<uint64_t> (1) << i);
      }

      return true;
    }

    bool bit_reader::
    read_bytes (uint8_t* out, size_t len)
    {
      for (size_t i (0); i < len; ++i)
      {
        uint64_t v;
        if (!read_bits (v, 8))
          return false;

        out[i] = static_cast<uint8_t> (v);
      }

      return true;
    }

    // bit_buffer_writer
    //

    void bit_buffer_writer::
    write_tag (bit_type_tag tag)
    {
      writer_.write_bits (static_cast<uint8_t> (tag), 5);
    }

    void bit_buffer_writer::
    write_bool (bool v)
    {
      write_tag (bit_type_tag::boolean);
      writer_.write_bit (v);
    }

    void bit_buffer_writer::
    write_uint8 (uint8_t v)
    {
      write_tag (bit_type_tag::byte);
      writer_.write_bits (v, 8);
    }

    void bit_buffer_writer::
    write_uint32 (uint32_t v)
    {
      write_tag (bit_type_tag::uint32);
      writer_.write_bits (v, 32);
    }

    void bit_buffer_writer::
    write_int32 (int32_t v)
    {
      write_tag (bit_type_tag::int32);

      // Reinterpret the signed value as unsigned for bit-level serialization.
      //
      uint32_t u;
      memcpy (&u, &v, sizeof (u));
      writer_.write_bits (u, 32);
    }

    void bit_buffer_writer::
    write_uint64 (uint64_t v)
    {
      write_tag (bit_type_tag::uint64);
      writer_.write_bits (v, 64);
    }

    void bit_buffer_writer::
    write_float (float v)
    {
      write_tag (bit_type_tag::floating);

      uint32_t u;
      memcpy (&u, &v, sizeof (u));
      writer_.write_bits (u, 32);
    }

    void bit_buffer_writer::
    write_string (const char* s)
    {
      write_tag (bit_type_tag::string);

      // Write each character including the null terminator.
      //
      size_t len (strlen (s) + 1);
      writer_.write_bytes (reinterpret_cast<const uint8_t*> (s), len);
    }

    void bit_buffer_writer::
    write_blob (const uint8_t* data, size_t len)
    {
      write_tag (bit_type_tag::blob);

      // Write the length as a typed uint32, then the raw bytes.
      //
      write_uint32 (static_cast<uint32_t> (len));
      writer_.write_bytes (data, len);
    }

    // bit_buffer_reader
    //

    bool bit_buffer_reader::
    verify_tag (bit_type_tag expected)
    {
      uint64_t tag;
      if (!reader_.read_bits (tag, 5))
        return false;

      return static_cast<uint8_t> (tag) == static_cast<uint8_t> (expected);
    }

    bool bit_buffer_reader::
    read_bool (bool& v)
    {
      if (!verify_tag (bit_type_tag::boolean))
        return false;

      return reader_.read_bit (v);
    }

    bool bit_buffer_reader::
    read_uint8 (uint8_t& v)
    {
      if (!verify_tag (bit_type_tag::byte))
        return false;

      uint64_t u;
      if (!reader_.read_bits (u, 8))
        return false;

      v = static_cast<uint8_t> (u);
      return true;
    }

    bool bit_buffer_reader::
    read_uint32 (uint32_t& v)
    {
      if (!verify_tag (bit_type_tag::uint32))
        return false;

      uint64_t u;
      if (!reader_.read_bits (u, 32))
        return false;

      v = static_cast<uint32_t> (u);
      return true;
    }

    bool bit_buffer_reader::
    read_int32 (int32_t& v)
    {
      if (!verify_tag (bit_type_tag::int32))
        return false;

      uint64_t u;
      if (!reader_.read_bits (u, 32))
        return false;

      uint32_t u32 (static_cast<uint32_t> (u));
      memcpy (&v, &u32, sizeof (v));
      return true;
    }

    bool bit_buffer_reader::
    read_uint64 (uint64_t& v)
    {
      if (!verify_tag (bit_type_tag::uint64))
        return false;

      return reader_.read_bits (v, 64);
    }

    bool bit_buffer_reader::
    read_string (string& s, size_t max_len)
    {
      if (!verify_tag (bit_type_tag::string))
        return false;

      s.clear ();

      for (size_t i (0); i <= max_len; ++i)
      {
        uint64_t c;
        if (!reader_.read_bits (c, 8))
          return false;

        if (c == 0)
          return true;

        s += static_cast<char> (c);
      }

      return false; // no null terminator within max_len charactersb
    }

    bool bit_buffer_reader::
    read_blob (vector<uint8_t>& out)
    {
      if (!verify_tag (bit_type_tag::blob))
        return false;

      uint32_t length;
      if (!read_uint32 (length))
        return false;

      out.resize (static_cast<size_t> (length));
      return reader_.read_bytes (out.data (), length);
    }
  }
}
