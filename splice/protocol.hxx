
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

#include "protocol.hpp"

#include <boost/bind.hpp>
#include <boost/static_assert.hpp>

namespace splice
{

  template <typename up_t,typename below_t>
    void protocol<up_t,below_t>::on_error(const boost::system::error_code& ec)
    {
      log_error(EZ_FLFT,ec.message());
    }

  template <typename up_t,typename below_t>
    protocol<up_t,below_t>::protocol()
      :base_t()
      ,acceptor_(io_service_)
    {
    }

  template <typename up_t,typename below_t>
    up_t* protocol<up_t,below_t>::cast_up()
    {
      return static_cast<up_t*>(this);
    }

  template <typename up_t,typename below_t>
    boost::asio::io_service& protocol<up_t,below_t>::get_io_service()
    {
      return io_service_;
    }

    // CRTP pure virtual function
  template <typename up_t,typename below_t>
    template <typename session_t>
    boost::shared_ptr<session_t> protocol<up_t,below_t>::construct_session()
    {
      BOOST_STATIC_ASSERT_MSG(false,
        "construct_session func must be defined in a server derived class");
    }

} // namespace splice

