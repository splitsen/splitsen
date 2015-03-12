
//          Copyright Jean Davy 2014-2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef WEBSOCKET_SESSION_HPP
#define WEBSOCKET_SESSION_HPP

#ifndef BOOST_CONFIG_HPP
# include <boost/config.hpp>
#endif
#ifdef BOOST_HAS_PRAGMA_ONCE
# pragma once
#endif
#include "../detail/config.hpp"

#include "ws_handshake.hpp"

namespace splice
{

  /// Represents a single connection from a client.
  template < typename up_t,typename log_t=no_log >
  class ws_session
    : public  ws_handshake<up_t,log_t>
  {
  public:
    using base_t=ws_handshake<up_t,log_t>;
    using my_t=ws_session<up_t,log_t>;

    /// Construct a connection with the given io_service.
    ws_session(boost::asio::io_service& io_service);

    ws_session(socket_t& socket);

  protected:
    void on_handshake_success();

    template<typename _t>
    data_frame_ptr mk_data_frame(_t p);

    /// Deliver a message to the client.
    void async_write(const std::string& msg);

    template<typename func_t>
    void async_write(const std::string& msg,func_t on_write_func);

    // All these functions below are equivalent of pure virtual functions,
    // so must be defined in a derived class.
    void on_read(std::string&  df);

    void on_read_data_frame(
      incoming_data_ptr& incoming_data,
      data_frame& read_frame);

    void on_read_dataframe(
      incoming_data_ptr incoming_data,
      frame_parser_ptr frame_parser,
      const error_code& error,
      std::size_t bytes_transferred);

    void on_write_dataframe(
      data_frame_ptr df,
      const error_code& error);

    friend class base_t;
    friend class base_t::base_t;
  };

} // namespace splice {

#if defined(SPLICE_HEADER_ONLY)
# include "ws_session.hxx"
#endif // defined(SPLICE_HEADER_ONLY)

#endif // WEBSOCKET_SESSION_HPP