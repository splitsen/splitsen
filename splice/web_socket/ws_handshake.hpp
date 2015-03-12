
//          Copyright Jean Davy 2014-2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef WEBSOCKET_HANDSHAKE_HPP
#define WEBSOCKET_HANDSHAKE_HPP

#ifndef BOOST_CONFIG_HPP
# include <boost/config.hpp>
#endif
#ifdef BOOST_HAS_PRAGMA_ONCE
# pragma once
#endif
#include "../detail/config.hpp"

#include "../tcp_session.hpp"
#include "rfc6455_engine.hpp"

namespace splice
{

  /// Represents a single connection from a client.
  template < typename up_t,typename log_t=no_log>
  class ws_handshake
    : public tcp_session<up_t,log_t>
  {
  public:
    using base_t=tcp_session<up_t,log_t>;
    using my_t=ws_handshake<up_t,log_t>;

    // mono protocol server ----------------------------------------------------
    /// Construct a connection with the given io_service.
    ws_handshake(boost::asio::io_service& io_service);

    ws_handshake(socket_t& socket);

  protected:
    // mono protocol server
    // First read operation
    void on_first_read( // called from on_connect
      incoming_data_ptr incoming_data
      ,std::size_t bytes_transferred
      ,const error_code& error);

    // mono protocol server
    // Handles completion of the first read operation
    void first_write(hand_shake_data_t& incoming);

    // mono protocol server
    // Second read operation will call on_handshake_success()
    void on_second_read(
      http_reply_ptr reply,
      const error_code& error);

    // multi protocol server ---------------------------------------------------
    // First read operation
    template< typename _t>
    void on_first_read( // called from on_connect
      _t handshake_fail,
      incoming_data_ptr incoming_data,
      std::size_t bytes_transferred,
      const error_code& error);


    // multi protocol server <=> mono
    // First read operation
    template< typename _t>
    void first_write(
      _t handshake_fail,
      hand_shake_data_t& incoming);

    // multi protocol server
    // Second read operation, handles completion of first_write
    // Instead of calling on_handshake_success(),
    // read the html client GUID that must be sent by js
    // immediately after socket connected as first write.
    template< typename _t>
    void on_second_read(
      _t handshake_fail,
      http_reply_ptr reply,
      const error_code& error);

    template< typename _t>
    void guid_read(
      _t handshake_fail);

    template< typename _t>
    void do_custom_handshake(
      _t handshake_fail,
      hand_shake_data_t& in);

    template< typename _t>
    void on_guid_read(
      _t handshake_fail,
      incoming_data_ptr incoming_data,
      frame_parser_ptr frame_parser,
      const error_code& error,
      std::size_t bytes_transferred);

    template<typename _t>
    void handshake(
      _t handshake_fail,
      hand_shake_data_t& incoming);

    // CRTP virtual function
    void on_bad_request(const char* file,unsigned line,const char* func);

    template<typename _t>
    http_reply_ptr mk_http_reply(_t p);

    frame_parser_ptr mk_frame_parser();

    friend class base_t;

  }; // class ws_handshake

} // namespace splice {

#if defined(SPLICE_HEADER_ONLY)
# include "ws_handshake.hxx"
#endif // defined(SPLICE_HEADER_ONLY)

#endif // WEBSOCKET_HANDSHAKE_HPP