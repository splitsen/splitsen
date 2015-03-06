
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

#include "multi_protocol.hpp"

namespace splice
{

  template <typename up_t,typename below_t,unsigned protocol_count_>
  unsigned multi_protocol<up_t,below_t,protocol_count_>::get_protocol_count() const
  { return protocol_count_; }

  template <typename up_t,typename below_t,unsigned protocol_count_>
  template <typename session_t>
  void multi_protocol<up_t,below_t,protocol_count_>::start_accept()
  {
    BOOST_STATIC_ASSERT(protocol_count_>=1);

    if(!get_acceptor().is_open())
      return;

    boost::shared_ptr<session_t> new_session=cast_up()->construct_session();
    new_session->on_wait_connect();

    get_acceptor().async_accept(new_session->get_socket(),
      boost::bind(&up_t::on_accept<session_t>,cast_up(),
      new_session,
      boost::asio::placeholders::error));
  }

  template <typename up_t,typename below_t,unsigned protocol_count_>
  template <typename session_t>
  void multi_protocol<up_t,below_t,protocol_count_>::on_accept(
    boost::shared_ptr<session_t> session,
    const boost::system::error_code& error)
  {
    if(!get_acceptor().is_open())
      return;

    BOOST_ASSERT(session->get_socket().is_open());
    if(!error)
      session->cast_up()->on_socket_connected(
      bind(&up_t::on_handshake_fail,this,
      protocol_count_-1,
      _2,// socket
      _1)); //pair incoming_data, boost::asio::placeholders::bytes_transferred
    else
      cast_up()->on_error(error);

    start_accept<session_t>(); // loop for next connection
  }

  template <typename up_t,typename below_t,unsigned protocol_count_>
  template< typename _t>
  void multi_protocol<up_t,below_t,protocol_count_>::do_next_handshake(
    _t handshake_fail_handler,
    socket_t* socket,
    unsigned protocol_index,
    hand_shake_data_t& incoming_data)
  {
    BOOST_STATIC_ASSERT_MSG(false,
      "do_next_handshake func must be defined in a server derived class");
  }

  template <typename up_t,typename below_t,unsigned protocol_count_>
  void multi_protocol<up_t,below_t,protocol_count_>::on_handshake_fail(
    unsigned protocol_index,
    socket_t& socket, // _2
    hand_shake_data_t& incoming)// _1
  {
    log_info_t(EZ_FLFT,
      "incoming= ",incoming.data(),incoming.size());

    if(!protocol_index)
    {
      log_warning_t(EZ_FLFT,std::string("Unable to connect:")
        ,incoming.data(),incoming.size());
      return;
    }

    --protocol_index;
    cast_up()->do_next_handshake(
      bind(&up_t::on_handshake_fail,this,
      protocol_index,
      _2, // socket
      _1) // incoming
      ,socket,protocol_index,incoming);
  }

} // namespace splice
