
//          Copyright Jean Davy 2014-2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef PROTOCOL_HPP
#define PROTOCOL_HPP

#ifndef BOOST_CONFIG_HPP
# include <boost/config.hpp>
#endif
#ifdef BOOST_HAS_PRAGMA_ONCE
# pragma once
#endif
#include "detail/config.hpp"

#include "logger/log_interface.hpp"
#include "common_types.hpp"

namespace splice
{

  template <typename up_t,typename below_t>
  class protocol: public below_t
  {
  public:
    using my_t=protocol<up_t,below_t>;
    using base_t=below_t;
    using error_code=boost::system::error_code;

  protected:
    protocol();

    up_t* cast_up();

    boost::asio::io_service& get_io_service();
    boost::asio::ip::tcp::acceptor& get_acceptor()
    { return acceptor_; };

    // CRTP pure virtual function
    template <typename session_t>
    boost::shared_ptr<session_t> construct_session()BOOST_NOEXCEPT;

    void on_error(const boost::system::error_code& ec)BOOST_NOEXCEPT;
  private:
    /// The io_service used to perform asynchronous operations.
    boost::asio::io_service io_service_;
    /// Acceptor used to listen for incoming connections.
    boost::asio::ip::tcp::acceptor acceptor_;
  };

} // namespace splice

#if defined(SPLICE_HEADER_ONLY)
# include "protocol.hxx"
#endif // defined(SPLICE_HEADER_ONLY)

#endif // #ifndef PROTOCOL_HPP
