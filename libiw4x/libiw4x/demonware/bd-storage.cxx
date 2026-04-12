#include <libiw4x/demonware/bd-storage.hxx>

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include <libiw4x/demonware/bd-bit-buffer.hxx>
#include <libiw4x/demonware/bd-remote-task-manager.hxx>
#include <libiw4x/logger.hxx>

using namespace std;
using namespace std::filesystem;

namespace iw4x
{
  namespace demonware
  {
    namespace
    {
      // The bdStorage service ID.
      //
      constexpr uint8_t storage_service_id (10);

      // Known bdStorage sub-function IDs.
      //
      constexpr uint8_t sub_set_file (1);
      constexpr uint8_t sub_set_user_file (2);
      constexpr uint8_t sub_get_file (5);
      constexpr uint8_t sub_get_user_file (7);
      constexpr uint8_t sub_get_publisher_file_info (8);

      // Synthetic file IDs.
      //
      // Since we are not a real Demonware backend, we don't have proper
      // database file IDs. Instead, we use synthetic IDs to distinguish between
      // different types of storage files during download requests.
      //
      constexpr uint64_t publisher_file_id (1);
      constexpr uint64_t user_file_id (2);

      // Publisher file context.
      //
      constexpr const char* publisher_filename ("playlists.patch2");

      vector<uint8_t> publisher_data;
      bool publisher_loaded (false);

      // User file context.
      //
      constexpr const char* user_file_dir ("players");
      constexpr const char* user_filename ("mpdata");

      vector<uint8_t> user_data;
      bool user_loaded (false);
      bool user_exists (false);

      // Read a file into a byte vector.
      //
      // Notice that we return an empty vector on failure. This is deliberate
      // because a missing file is often a legitimate state (for example, a
      // brand new player without an mpdata file yet).
      //
      vector<uint8_t>
      read_file_data (const path& p, const char* label)
      {
        namespace fs = filesystem;

        if (!fs::exists (p) || !fs::is_regular_file (p))
        {
          log::trace_l1 << "dw: storage: " << label
                        << " not found: " << p.string ();
          return {};
        }

        auto size (file_size (p));

        if (size == 0)
        {
          log::trace_l1 << "dw: storage: " << label
                        << " is empty: " << p.string ();
          return {};
        }

        vector<uint8_t> data (static_cast<size_t> (size));
        ifstream f (p, ios::binary);

        if (!f.read (reinterpret_cast<char*> (data.data ()),
                     static_cast<streamsize> (size)))
        {
          log::warning << "dw: storage: failed to read " << label << ": "
                       << p.string ();
          return {};
        }

        log::info << "dw: storage: loaded " << label << ": " << p.string ()
                  << " (" << size << "B)";

        return data;
      }

      // Write a byte vector to a file.
      //
      // We ensure the parent directories exist before attempting to write.
      // Return true on success so the caller knows if the cache should be
      // updated safely.
      //
      bool
      write_file_data (const path& p,
                       const uint8_t* data,
                       size_t size,
                       const char* label)
      {
        create_directories (p.parent_path ());

        ofstream f (p, ios::binary | ios::trunc);

        if (!f.write (reinterpret_cast<const char*> (data),
                      static_cast<streamsize> (size)))
        {
          log::warning << "dw: storage: failed to write " << label << ": "
                       << p.string ();
          return false;
        }

        log::info << "dw: storage: saved " << label << ": " << p.string ()
                  << " (" << size << "B)";

        return true;
      }

      // The publisher file (playlists) is relatively static, so we only need
      // to load it once from disk and then serve it from memory.
      //
      const vector<uint8_t>&
      ensure_publisher_data ()
      {
        if (!publisher_loaded)
        {
          publisher_data = read_file_data (publisher_filename, "publisher file");
          publisher_loaded = true;
        }

        return publisher_data;
      }

      // Ensure the user data is loaded into memory.
      //
      // Similar to publisher data, but tracks explicit existence state so we
      // can gracefully handle new players who don't have files yet.
      //
      void
      ensure_user_data ()
      {
        if (!user_loaded)
        {
          path p (path (user_file_dir) / user_filename);
          user_data = read_file_data (p, "user file");
          user_exists = !user_data.empty ();
          user_loaded = true;
        }
      }

      // Write a bdLobbyFileHeader into a reply bit_buffer_writer.
      //
      // The layout of this header is dictated by the bdBitBuffer wire format.
      // We have to write these exact fields in this exact order.
      //
      //  uint64(10) : file_id
      //  uint32(8)  : file_size
      //  uint32(8)  : create_time
      //  bool(1)    : has_data
      //  bool(1)    : visibility
      //  uint64(10) : owner_id
      //  string(16) : filename
      //
      void
      write_file_header (bit_buffer_writer& reply,
                         uint64_t file_id,
                         uint32_t file_size,
                         const char* filename)
      {
        reply.write_uint64 (file_id);
        reply.write_uint32 (file_size);
        reply.write_uint32 (0);          // create_time
        reply.write_bool (true);         // has_data
        reply.write_bool (false);        // visibility
        reply.write_uint64 (0);          // owner_id
        reply.write_string (filename);
      }

      // Handle a bdStorage request.
      //
      // The basic idea is to dispatch based on the sub-function ID. Notice that
      // we return true as long as we successfully formulated a reply buffer,
      // even if the requested file itself was not found. Returning false would
      // drop the connection.
      //
      bool
      storage_handler (uint8_t service_id,
                       uint8_t sub_function_id,
                       bit_buffer_reader& request,
                       bit_buffer_writer& reply)
      {
        log::trace_l1 << "dw: storage: handler service="
                      << static_cast<int> (service_id)
                      << " sub=" << static_cast<int> (sub_function_id);

        switch (sub_function_id)
        {
          // Get publisher file info.
          //
          // The game wants to know if the publisher file exists and how big it
          // is. If we don't have it, we just return a 0 size and 0 file count.
          //
          case sub_get_publisher_file_info:
          {
            auto& data (ensure_publisher_data ());

            if (data.empty ())
            {
              log::warning << "dw: storage: publisher file not available";
              reply.write_uint32 (0);
              reply.write_uint8 (0);
              return true;
            }

            auto file_size (static_cast<uint32_t> (data.size ()));

            log::info << "dw: storage: getPublisherFileInfo -> "
                      << publisher_filename << " (" << file_size << "B)";

            reply.write_uint32 (0);
            reply.write_uint8 (1);
            reply.write_uint32 (file_size);
            write_file_header (reply,
                               publisher_file_id,
                               file_size,
                               publisher_filename);

            return true;
          }

          // Get user file info (existence check).
          //
          // If the file exists on disk, return its header so the game issues a
          // follow-up sub=5 to download the actual data. If it doesn't exist,
          // return no results. The game will handle this gracefully by treating
          // the user as a new player and initializing default stats.
          //
          case sub_get_user_file:
          {
            ensure_user_data ();

            if (!user_exists)
            {
              log::info << "dw: storage: getUserFile -> " << user_filename
                        << " (not found, using defaults)";

              reply.write_uint32 (0);
              reply.write_uint8 (0);
              return true;
            }

            auto file_size (static_cast<uint32_t> (user_data.size ()));

            log::info << "dw: storage: getUserFile -> " << user_filename << " ("
                      << file_size << "B)";

            reply.write_uint32 (0);
            reply.write_uint8 (1);
            reply.write_uint32 (file_size);
            write_file_header (reply, user_file_id, file_size, user_filename);

            return true;
          }

          // Download a file by file_id.
          //
          // The request format here is a byte padding followed by the uint64
          // file_id (based on reversing 0x14031F300). This file_id was assigned
          // by our sub=8 or sub=7 response headers earlier and is now simply
          // echoed back to us.
          //
          case sub_get_file:
          {
            uint8_t pad;
            uint64_t file_id;

            if (!request.read_uint8 (pad) || !request.read_uint64 (file_id))
            {
              log::warning << "dw: storage: getFile parse error";
              reply.write_uint32 (0);
              reply.write_uint8 (0);
              return true;
            }

            log::trace_l1 << "dw: storage: getFile file_id=" << file_id;

            if (file_id == user_file_id)
            {
              ensure_user_data ();

              if (!user_exists || user_data.empty ())
              {
                log::warning << "dw: storage: user file not available for download";
                reply.write_uint32 (0);
                reply.write_uint8 (0);
                return true;
              }

              auto file_size (static_cast<uint32_t> (user_data.size ()));

              log::info << "dw: storage: getFile -> " << user_filename << " ("
                        << file_size << "B)";

              reply.write_uint32 (0);
              reply.write_uint8 (1);
              reply.write_uint32 (file_size);
              write_file_header (reply, user_file_id, file_size, user_filename);
              reply.write_blob (user_data.data (), user_data.size ());

              return true;
            }

            // Default route. We assume the game wants the publisher playlists.
            //
            auto& data (ensure_publisher_data ());

            if (data.empty ())
            {
              log::warning << "dw: storage: publisher file not available for download";
              reply.write_uint32 (0);
              reply.write_uint8 (0);
              return true;
            }

            auto file_size (static_cast<uint32_t> (data.size ()));

            log::info << "dw: storage: getFile -> " << publisher_filename << " ("
                      << file_size << "B)";

            reply.write_uint32 (0);
            reply.write_uint8 (1);
            reply.write_uint32 (file_size);
            write_file_header (reply,
                               publisher_file_id,
                               file_size,
                               publisher_filename);
            reply.write_blob (data.data (), data.size ());

            return true;
          }

          // Save a file by filename.
          //
          // The wire format is somewhat involved (from reversing 0x14031F970):
          // byte(0) + bool(vis) + string(filename, no null) +
          // raw_byte(0x00, no type tag) + bool(vis2) + blob(file_data).
          //
          // Notice that our read_string() implementation automatically consumes
          // the raw 0x00 as the null terminator, so no manual skipping is
          // needed here.
          //
          case sub_set_file:
          {
            uint8_t pad;
            bool vis;
            string filename;

            if (!request.read_uint8 (pad) || !request.read_bool (vis) ||
                !request.read_string (filename, 127))
            {
              log::warning << "dw: storage: setFile parse error (header)";
              reply.write_uint32 (0);
              reply.write_uint8 (0);
              return true;
            }

            bool vis2;
            vector<uint8_t> blob;

            if (!request.read_bool (vis2) || !request.read_blob (blob))
            {
              log::warning << "dw: storage: setFile parse error (blob)";
              reply.write_uint32 (0);
              reply.write_uint8 (0);
              return true;
            }

            log::info << "dw: storage: setFile \"" << filename << "\" ("
                      << blob.size () << "B)";

            path p (path (user_file_dir) / filename);

            write_file_data (p, blob.data (), blob.size (), filename.c_str ());

            reply.write_uint32 (0);
            reply.write_uint8 (0);
            return true;
          }

          // Save player stats (mpdata).
          //
          // The wire format (from reversing 0x14031F820) is relatively simple:
          // byte(0) + uint64(owner_id) + blob(file_data).
          //
          // We write the file to disk and, importantly, update our in-memory
          // cache so that any subsequent reads in this session reflect the
          // newly saved data.
          //
          case sub_set_user_file:
          {
            uint8_t pad;
            uint64_t owner_id;
            vector<uint8_t> blob;

            if (!request.read_uint8 (pad) || !request.read_uint64 (owner_id) ||
                !request.read_blob (blob))
            {
              log::warning << "dw: storage: setUserFile parse error";
              reply.write_uint32 (0);
              reply.write_uint8 (0);
              return true;
            }

            log::info << "dw: storage: setUserFile owner=" << owner_id
                      << " size=" << blob.size ();

            path p (path (user_file_dir) / user_filename);

            if (write_file_data (p, blob.data (), blob.size (), "user file"))
            {
              user_data = move (blob);
              user_exists = true;
              user_loaded = true;
            }

            reply.write_uint32 (0);
            reply.write_uint8 (0);
            return true;
          }

          default:
          {
            log::warning << "dw: storage: unhandled sub="
                         << static_cast<int> (sub_function_id);
            return false;
          }
        }
      }
    }

    storage::storage ()
    {
      // Register our request handler with the remote task manager.
      //
      remote_task_manager::register_handler (storage_service_id,
                                             &storage_handler);

      log::info << "dw: storage service registered (id="
                << static_cast<int> (storage_service_id) << ")";
    }
  }
}
