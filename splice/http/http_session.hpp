
//          Copyright Jean Davy 2014-2015.
//          (jean dot davy dot 77 dot gmail dot com)
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// This code is strongly inspired by boost-asio hhtp example:
// www.boost.org/libs/asio
// www.boost.org/doc/libs/1_57_0/doc/html/boost_asio/example/cpp03/http/server/
// http_session.hpp
//
// Copyright (c) 2003-2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HTTP_SESSION_HPP
#define HTTP_SESSION_HPP

#ifndef BOOST_CONFIG_HPP
# include <boost/config.hpp>
#endif
#ifdef BOOST_HAS_PRAGMA_ONCE
# pragma once
#endif
#include "../detail/config.hpp"

#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include "reply.hpp"
#include "request.hpp"
#include "request_handler.hpp"
#include "request_parser.hpp"

#include "../tcp_session.hpp"

namespace splice
{

  /// Represents a single http_session from a client.
  template <typename up_t,typename log_t=no_log>
  class http_session
    :public tcp_session<up_t,log_t>
  {
  public:
    using base_t=tcp_session<up_t,log_t>;

    /// Construct a http_session with the given io_service.
    explicit http_session(socket_t& socket
      ,http::server::request_handler& handler);

  protected:

    template<typename _t>
    void handshake(
      _t handshake_fail,
      hand_shake_data_t& incoming);

    // Start the first asynchronous operation for a multi protocol server
    template< typename _t>
    void on_first_read(
      _t handshake_fail,
      incoming_data_ptr incoming_data,
      std::size_t bytes_transferred,
      const boost::system::error_code& ec);

    /// Handle completion of a read operation.
    void handle_read(incoming_data_ptr incoming_data
      ,const boost::system::error_code& e
      ,std::size_t bytes_transferred);

    /// Handle completion of a write operation.
    void handle_write(const boost::system::error_code& e);

    friend class base_t;

  private:
    /// The handler used to process the incoming request.
    http::server::request_handler& request_handler_;

    /// The incoming request.
    http::server::request request_;

    /// The parser for the incoming request.
    http::server::request_parser request_parser_;

    /// The reply to be sent back to the client.
    http::server::reply reply_;
  };

} // namespace splice

#if defined(SPLICE_HEADER_ONLY)
# include "http_session.hxx"
#endif // defined(SPLICE_HEADER_ONLY)

#endif // HTTP_SESSION_HPP
