#include <libiw4x/demonware/bd-byte-buffer.hxx>

#include <cstring>

using namespace std;

namespace iw4x
{
  namespace demonware
  {
    // byte_buffer_writer
    //

    void byte_buffer_writer::
    append (const void* data, size_t size)
    {
      auto p (static_cast<const uint8_t*> (data));
      buffer_.insert (buffer_.end (), p, p + size);
    }

    void byte_buffer_writer::
    append_tag (byte_type_tag tag)
    {
      buffer_.push_back (static_cast<uint8_t> (tag));
    }

    void byte_buffer_writer::
    write_bool (bool v)
    {
      append_tag (byte_type_tag::boolean);

      uint8_t b (v ? 1 : 0);
      append (&b, 1);
    }

    void byte_buffer_writer::
    write_uint8 (uint8_t v)
    {
      append_tag (byte_type_tag::boolean);
      append (&v, 1);
    }

    void byte_buffer_writer::
    write_uint32 (uint32_t v)
    {
      append_tag (byte_type_tag::integer32);
      append (&v, sizeof (v));
    }

    void byte_buffer_writer::
    write_int32 (int32_t v)
    {
      append_tag (byte_type_tag::integer32);
      append (&v, sizeof (v));
    }

    void byte_buffer_writer::
    write_uint64 (uint64_t v)
    {
      append_tag (byte_type_tag::integer64);
      append (&v, sizeof (v));
    }

    void byte_buffer_writer::
    write_int64 (int64_t v)
    {
      append_tag (byte_type_tag::integer64);
      append (&v, sizeof (v));
    }

    void byte_buffer_writer::
    write_float (float v)
    {
      append_tag (byte_type_tag::floating);

      uint32_t len (sizeof (float));
      append (&len, sizeof (len));
      append (&v, sizeof (v));
    }

    void byte_buffer_writer::
    write_string (const string& s)
    {
      append_tag (byte_type_tag::string);

      // Length includes the null terminator.
      //
      uint32_t len (static_cast<uint32_t> (s.size () + 1));
      append (&len, sizeof (len));
      append (s.data (), s.size ());

      uint8_t null (0);
      append (&null, 1);
    }

    void byte_buffer_writer::
    write_blob (const uint8_t* data, size_t size)
    {
      append_tag (byte_type_tag::blob);

      uint32_t len (static_cast<uint32_t> (size));
      append (&len, sizeof (len));
      append (data, size);
    }

    void byte_buffer_writer::
    write_struct_header (uint32_t error_code)
    {
      append_tag (byte_type_tag::struct_header);
      append (&error_code, sizeof (error_code));
    }

    void byte_buffer_writer::
    write_array_count (uint32_t count)
    {
      append_tag (byte_type_tag::array_count);
      append (&count, sizeof (count));
    }

    // byte_buffer_reader
    //

    bool byte_buffer_reader::
    consume (void* out, size_t size)
    {
      if (pos_ + size > size_)
        return false;

      memcpy (out, data_ + pos_, size);
      pos_ += size;
      return true;
    }

    bool byte_buffer_reader::
    verify_tag (byte_type_tag expected)
    {
      if (pos_ >= size_)
        return false;

      if (data_[pos_] != static_cast<uint8_t> (expected))
        return false;

      ++pos_;
      return true;
    }

    bool byte_buffer_reader::
    read_bool (bool& v)
    {
      if (!verify_tag (byte_type_tag::boolean))
        return false;

      uint8_t b;
      if (!consume (&b, 1))
        return false;

      v = (b != 0);
      return true;
    }

    bool byte_buffer_reader::
    read_uint8 (uint8_t& v)
    {
      if (!verify_tag (byte_type_tag::boolean))
        return false;

      return consume (&v, 1);
    }

    bool byte_buffer_reader::
    read_uint32 (uint32_t& v)
    {
      if (!verify_tag (byte_type_tag::integer32))
        return false;

      return consume (&v, sizeof (v));
    }

    bool byte_buffer_reader::
    read_int32 (int32_t& v)
    {
      if (!verify_tag (byte_type_tag::integer32))
        return false;

      return consume (&v, sizeof (v));
    }

    bool byte_buffer_reader::
    read_uint64 (uint64_t& v)
    {
      if (!verify_tag (byte_type_tag::integer64))
        return false;

      return consume (&v, sizeof (v));
    }

    bool byte_buffer_reader::
    read_int64 (int64_t& v)
    {
      if (!verify_tag (byte_type_tag::integer64))
        return false;

      return consume (&v, sizeof (v));
    }

    bool byte_buffer_reader::
    read_float (float& v)
    {
      if (!verify_tag (byte_type_tag::floating))
        return false;

      uint32_t len;
      if (!consume (&len, sizeof (len)))
        return false;

      if (len != sizeof (float))
        return false;

      return consume (&v, sizeof (v));
    }

    bool byte_buffer_reader::
    read_string (string& s)
    {
      if (!verify_tag (byte_type_tag::string))
        return false;

      uint32_t len;
      if (!consume (&len, sizeof (len)))
        return false;

      if (len == 0 || pos_ + len > size_)
        return false;

      // Length includes the null terminator.
      //
      s.assign (reinterpret_cast<const char*> (data_ + pos_), len - 1);
      pos_ += len;
      return true;
    }

    bool byte_buffer_reader::
    read_blob (vector<uint8_t>& out)
    {
      if (!verify_tag (byte_type_tag::blob))
        return false;

      uint32_t len;
      if (!consume (&len, sizeof (len)))
        return false;

      if (pos_ + len > size_)
        return false;

      out.assign (data_ + pos_, data_ + pos_ + len);
      pos_ += len;
      return true;
    }

    bool byte_buffer_reader::
    read_struct_header (uint32_t& error_code)
    {
      if (!verify_tag (byte_type_tag::struct_header))
        return false;

      return consume (&error_code, sizeof (error_code));
    }
  }
}
