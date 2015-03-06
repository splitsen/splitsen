
//          Copyright Jean Davy 2014-2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef MULTI_PROTOCOL_HPP
#define MULTI_PROTOCOL_HPP

#ifndef BOOST_CONFIG_HPP
# include <boost/config.hpp>
#endif
#ifdef BOOST_HAS_PRAGMA_ONCE
# pragma once
#endif
#include "detail/config.hpp"

#include <string>
#include <iostream>

#include "protocol.hpp"

namespace splice
{

  template <typename up_t,typename below_t,unsigned protocol_count_>
  class multi_protocol: public protocol<up_t,below_t>
  {
  protected:
    unsigned get_protocol_count() const;

    // Initiate an asynchronous accept operation.
    template <typename session_t>
    void start_accept()BOOST_NOEXCEPT;

    // Handle completion of an asynchronous accept operation.
    template <typename session_t>
    void on_accept(
      boost::shared_ptr<session_t> session,
      const boost::system::error_code& error)BOOST_NOEXCEPT;

    template< typename _t>
    void do_next_handshake(
      _t handshake_fail_handler,
      socket_t* socket,
      unsigned protocol_index,
      hand_shake_data_t& incoming_data)BOOST_NOEXCEPT;

    // only called when server is multiple protocol
    void on_handshake_fail(
      unsigned protocol_index,
      socket_t& socket, // _2
      hand_shake_data_t& incoming)BOOST_NOEXCEPT;// _1
  };

} // namespace splice

#if defined(SPLICE_HEADER_ONLY)
# include "multi_protocol.hxx"
#endif // defined(SPLICE_HEADER_ONLY)

#endif // #ifndef MULTI_PROTOCOL_HPP
