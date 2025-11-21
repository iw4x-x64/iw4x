#include <libiw4x/network/init.hxx>

#include <iostream>

using namespace std;

namespace iw4x
{
  namespace network
  {
    namespace
    {
      int
      net_send_packet (size_t length, const char* buffer, netadr_t* address)
      {
        try
        {
          SOCKET s (*ip_socket);

          if (s == INVALID_SOCKET || s == 0)
          {
            int error_code (WSAGetLastError ());
            throw system_error (error_code, system_category (), "sendto");
          }

          // IW4 always stores four octets in ip[] and keeps port in network
          // byte order.
          //
          sockaddr_in sa;
          sa.sin_family = AF_INET;
          memcpy (&sa.sin_addr, address->ip, sizeof (sa.sin_addr));
          sa.sin_port = address->port;

          int r (sendto (s,
                         buffer,
                         static_cast<int> (length),
                         0,
                         reinterpret_cast<sockaddr*> (&sa),
                         sizeof (sockaddr_in)));

          if (r == SOCKET_ERROR)
          {
            int error_code (WSAGetLastError ());
            throw system_error (error_code, system_category (), "sendto");
          }

#ifdef LIBIW4X_DEVELOP
          cout << "sent " << r << " bytes" << endl;
#endif

          return true;
        }
        catch (const exception& e)
        {
          cerr << "error:" << e.what ();
          exit (1);
        }
      }
    }

    void
    init (scheduler& s)
    {
      s.post ("com_frame",
              []
      {
        try
        {
          SOCKET s (socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP));

          if (s == INVALID_SOCKET)
          {
            int error_code (WSAGetLastError ());
            throw system_error (error_code, system_category (), "socket");
          }

          // Configure non-blocking mode.
          //
          u_long mode (1);
          if (ioctlsocket (s, FIONBIO, &mode) == SOCKET_ERROR)
          {
            int error_code (WSAGetLastError ());
            closesocket (s);
            throw system_error (error_code, system_category (), "ioctlsocket");
          }

          sockaddr_in bind_address {};
          bind_address.sin_family = AF_INET;
          bind_address.sin_addr.s_addr = INADDR_ANY;
          bind_address.sin_port = htons (0);

          if (::bind (s,
                      reinterpret_cast<const sockaddr*> (&bind_address),
                      sizeof (bind_address)) == SOCKET_ERROR)
          {
            int error_code (WSAGetLastError ());
            throw system_error (error_code, system_category (), "bind");
          }

          // Query assigned port since it's (currently) ephemeral.
          //
          sockaddr_in bound_address {};
          int address_length (sizeof (bound_address));

          if (getsockname (s,
                           reinterpret_cast<sockaddr*> (&bound_address),
                           &address_length) == 0)
          {
            cout << "bound to: " << ntohs (bound_address.sin_port) << endl;
          }

          // Store socket in game's internal ip_socket.
          //
          *ip_socket = s;

          // While the scheduled task would normally install the hook before
          // NET_SendPacket and thus guarantee that the socket is initialized
          // in time, relying on such ordering here would be fragile: this code
          // runs inside the engine and we have no firm contract on when these
          // pieces are called.
          //
          minhook::create (NET_SendPacket, &net_send_packet);
        }
        catch (const exception& e)
        {
          cerr << "error:" << e.what () << endl;
          exit (1);
        }
      });
    }
  }
}
