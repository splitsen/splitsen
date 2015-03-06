
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

#include "mono_protocol.hpp"

namespace splice
{
  template <typename up_t,typename below_t>
  template <typename session_t>
  void mono_protocol<up_t,below_t>::start_accept()
  {
    if(!get_acceptor().is_open())
      return;

    boost::shared_ptr<session_t> new_session=
      cast_up()->construct_session();
    new_session->cast_up()->on_wait_connect();

    get_acceptor().async_accept(new_session->get_socket(),
      boost::bind(&up_t::on_accept<session_t>,cast_up(),
      new_session,
      boost::asio::placeholders::error));
  }

  template <typename up_t,typename below_t>
  template <typename session_t>
  void mono_protocol<up_t,below_t>::on_accept(
    boost::shared_ptr<session_t> session,
    const boost::system::error_code& error)
  {
    if(!get_acceptor().is_open())
      return;

    if(!error)
      session->cast_up()->on_socket_connected();
    else
      cast_up()->on_error(error);

    start_accept<session_t>(); // loop for next connection
  }

} // namespace splice

