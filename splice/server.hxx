
//          Copyright Jean Davy 2014-2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_CONFIG_HPP
# include <boost/config.hpp>
#endif
#ifdef BOOST_HAS_PRAGMA_ONCE
# pragma once
#endif
#include "detail/config.hpp"

#include "server.hpp"

#include <boost/thread/thread.hpp>

namespace splice
{

  template <typename up_t,typename protocol_t>
  ez_server<up_t,protocol_t>::ez_server(const std::string& address
    ,const std::string& port
    ,size_t max_connections=boost::asio::socket_base::max_connections)
    :base_t()
    ,signals_(get_io_service())
  {
    BOOST_ASSERT(max_connections>=1);

    using namespace std;
    using namespace boost;
    using namespace boost::asio;

    // Register to handle the signals that indicate when the server should exit.
    // It is safe to register for the same signal multiple times in a program,
    // provided all registration for the specified signal is made through Asio.
    signals_.add(SIGINT);
    signals_.add(SIGTERM);
#if defined(SIGQUIT)
    signals_.add(SIGQUIT);
#endif // defined(SIGQUIT)
    signals_.async_wait(bind(&up_t::on_stop,this));

    // Open the acceptor with the option to reuse the address (i.e. SO_REUSEADDR).
    ip::tcp::resolver resolver(get_io_service());
    ip::tcp::resolver::query query(address,port);
    boost::system::error_code ec;
    ip::tcp::endpoint endpoint=*resolver.resolve(query,ec);
    if(ec||
      get_acceptor().open(endpoint.protocol(),ec)||
      get_acceptor().set_option(ip::tcp::acceptor::reuse_address(true),ec)||
      get_acceptor().bind(endpoint,ec)||
      get_acceptor().listen(max_connections,ec)
      )
      cast_up()->on_error(ec);
  }

  template <typename up_t,typename protocol_t>
  void ez_server<up_t,protocol_t>::run(unsigned thread_count=0)
  {
    // The io_service::run() call will block until all asynchronous operations
    // have finished. While the server is running, there is always at least one
    // asynchronous operation outstanding: the asynchronous accept call waiting
    // for new incoming connections.

    if(!thread_count)
    {
      // Number of hardware threads available on the current 
      // system(e.g.number of CPUs or cores or hyperthreading units)
      // or 0 if this information is not available.
      const unsigned count=boost::thread::hardware_concurrency();
      thread_count=count?count:1;
    }

    if(thread_count=1)
    {
      cast_up()->on_run(1);
      get_io_service().run();
      return;
    }

    boost::thread_group g;
    for(unsigned i=0; i<thread_count; i++)
      g.create_thread(boost::bind(&boost::asio::io_service::run,
      &get_io_service()));
    cast_up()->on_run(thread_count);
    g.join_all();
  }

  template <typename up_t,typename protocol_t>
  void ez_server<up_t,protocol_t>::on_stop()
  {
    get_io_service().stop();
  }

} // namespace splice
