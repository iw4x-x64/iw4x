#include <thread>
#include <string>
#include <chrono>
#include <iostream>

#include <libiw4x/utility/inter-process.hxx>

#undef NDEBUG
#include <cassert>

using namespace std;
using namespace iw4x::utility;

// Test basic create/connect/send/receive cycle.
//
static void
test_basic ()
{
  inter_process server;
  server.create ("test-pipe-basic");

  thread client ([&] ()
  {
    this_thread::sleep_for (chrono::milliseconds (100));

    inter_process c;
    c.open ("test-pipe-basic");

    string msg ("hello from client");
    c.send (msg);

    string response (c.receive ());
    assert (response == "hello from server");

    c.disconnect ();
  });

  server.connect ();

  string msg (server.receive ());
  assert (msg == "hello from client");

  server.send ("hello from server");

  client.join ();
  server.disconnect ();
}

// Test reconnection after disconnect.
//
static void
test_reconnect ()
{
  inter_process server;
  server.create ("test-pipe-reconnect");

  // First connection.
  //
  thread client1 ([&] ()
  {
    this_thread::sleep_for (chrono::milliseconds (100));

    inter_process c;
    c.open ("test-pipe-reconnect");
    c.send ("message 1");
    c.disconnect ();
  });

  server.connect ();
  string msg1 (server.receive ());
  assert (msg1 == "message 1");
  server.disconnect ();

  client1.join ();

  // Second connection.
  //
  thread client2 ([&] ()
  {
    this_thread::sleep_for (chrono::milliseconds (100));

    inter_process c;
    c.open ("test-pipe-reconnect");
    c.send ("message 2");
    c.disconnect ();
  });

  server.connect ();
  string msg2 (server.receive ());
  assert (msg2 == "message 2");
  server.disconnect ();

  client2.join ();
}

// Test empty message.
//
static void
test_empty ()
{
  inter_process server;
  server.create ("test-pipe-empty");

  thread client ([&] ()
  {
    this_thread::sleep_for (chrono::milliseconds (100));

    inter_process c;
    c.open ("test-pipe-empty");
    c.send ("");

    string response (c.receive ());
    assert (response.empty ());

    c.disconnect ();
  });

  server.connect ();

  string msg (server.receive ());
  assert (msg.empty ());

  server.send ("");

  client.join ();
  server.disconnect ();
}

// Test large message.
//
static void
test_large ()
{
  inter_process server;
  server.create ("test-pipe-large");

  string large_msg (2048, 'x');

  thread client ([&] ()
  {
    this_thread::sleep_for (chrono::milliseconds (100));

    inter_process c;
    c.open ("test-pipe-large");
    c.send (large_msg);

    string response (c.receive ());
    assert (response.size () == 2048);
    assert (response == string (2048, 'y'));

    c.disconnect ();
  });

  server.connect ();

  string msg (server.receive ());
  assert (msg.size () == 2048);
  assert (msg == large_msg);

  server.send (string (2048, 'y'));

  client.join ();
  server.disconnect ();
}

// Test connected() state tracking.
//
static void
test_state ()
{
  inter_process server;
  assert (!server.is_connected ());

  server.create ("test-pipe-state");
  assert (!server.is_connected ());

  thread client ([&] ()
  {
    this_thread::sleep_for (chrono::milliseconds (100));

    inter_process c;
    assert (!c.is_connected ());

    c.open ("test-pipe-state");
    assert (c.is_connected ());

    c.send ("test");
    c.disconnect ();
    assert (!c.is_connected ());
  });

  server.connect ();
  assert (server.is_connected ());

  server.receive ();
  server.disconnect ();
  assert (!server.is_connected ());

  client.join ();
}

// Test move semantics (implicit via close/reopen).
//
static void
test_reuse ()
{
  inter_process server;
  server.create ("test-pipe-move-1");

  thread client1 ([&] ()
  {
    this_thread::sleep_for (chrono::milliseconds (100));

    inter_process c;
    c.open ("test-pipe-move-1");
    c.send ("first");
    c.disconnect ();
  });

  server.connect ();
  server.receive ();
  server.close ();

  client1.join ();

  // Reuse same object for different pipe.
  //
  server.create ("test-pipe-move-2");

  thread client2 ([&] ()
  {
    this_thread::sleep_for (chrono::milliseconds (100));

    inter_process c;
    c.open ("test-pipe-move-2");
    c.send ("second");
    c.disconnect ();
  });

  server.connect ();
  string msg (server.receive ());
  assert (msg == "second");
  server.disconnect ();

  client2.join ();
}

int
main ()
{
  test_basic ();
  test_reconnect ();
  test_empty ();
  test_large ();
  test_state ();
  test_reuse ();
}
