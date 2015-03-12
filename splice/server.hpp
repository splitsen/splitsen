
//          Copyright Jean Davy 2014-2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef EZ_SERVER_HPP
#define EZ_SERVER_HPP

#ifndef BOOST_CONFIG_HPP
# include <boost/config.hpp>
#endif
#ifdef BOOST_HAS_PRAGMA_ONCE
# pragma once
#endif
#include "detail/config.hpp"

#include <string>
#include <iostream>

#include <boost/asio.hpp>

#include "mono_protocol.hpp"
#include "multi_protocol.hpp"

namespace splice
{

  template <typename up_t,typename protocol_t>
  class ez_server: public protocol_t
    ,private boost::noncopyable
  {
  public:
    using base_t=protocol_t;
    using my_t=ez_server<up_t,protocol_t>;
    using error_code=boost::system::error_code;

    // Construct the server to listen on the specified TCP address and port
    ez_server(const std::string& address
      ,const std::string& port
      ,size_t max_connections=boost::asio::socket_base::max_connections);

    /// Run the server's io_service loop.
    void run(unsigned thread_count=0)BOOST_NOEXCEPT;

    // CRTP virtual functions
    void on_run(size_t thread_count) {}

    // CRTP virtual functions
    void on_accepted(const boost::system::error_code& error) {}

  private:
    /// Handle a request to stop the server.
    void on_stop();

    /// The signal_set is used to register for process termination notifications.
    boost::asio::signal_set signals_;
  };

} // namespace splice

// mono protocol
template <typename up_t,typename log_t=splice::no_log>
using ez_server_mono=splice::ez_server<up_t
  ,splice::mono_protocol<up_t,log_t>>;

// multi protocol
template <typename up_t
  ,unsigned protocol_count_,typename log_t=splice::no_log>
using ez_server_multi_prot=splice::ez_server<up_t
    ,splice::multi_protocol<up_t,log_t,protocol_count_>>;

#if defined(SPLICE_HEADER_ONLY)
# include "server.hxx"
#endif // defined(SPLICE_HEADER_ONLY)

#endif // WEBSOCKET_SERVER_HPP
