
//          Copyright Jean Davy 2014-2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef SERIALIZATION_SESSION_HPP
#define SERIALIZATION_SESSION_HPP

#ifndef BOOST_CONFIG_HPP
# include <boost/config.hpp>
#endif
#ifdef BOOST_HAS_PRAGMA_ONCE
# pragma once
#endif
#include "detail/config.hpp"

#include "serialization_engine.hpp"

namespace splice
{

  // TODO must be able to serialize msg_t, not only msg_ptr

  /// Represents a single connection from a client.
  template <typename up_t,typename msg_t,typename log_t=no_log>
  class serialization_session
    : public ser_eng_base<up_t,msg_t,log_t>
  {
  public:
    typedef ser_eng_base<up_t,msg_t,log_t>            base_t;
    typedef serialization_session<up_t,msg_t,log_t>   my_t;

    serialization_session(boost::asio::io_service& io_service);

    serialization_session(socket_t& socket);

    void on_read(msg_ptr msg);

  protected:

    void on_handshake_success();

    /// Handle completion of a read operation.
    void handle_read(msg_ptr,const boost::system::error_code& error);

    void handle_read_dataframe(
      incoming_data_ptr incoming_data,
      const boost::system::error_code& error,
      std::size_t bytes_transferred);

    //
    // The code below is greatly inspired of asio\example\cpp03\serialization\connection.hpp
    // with following copyright:
    // Copyright (c) 2003-2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
    //
    // Distributed under the Boost Software License, Version 1.0. (See accompanying
    // file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
    //
    /// This class provides serialization primitives on top of a socket.
    /**
    * Each message sent using this class consists of:
    * @li An 8-byte header containing the length of the serialized data in
    * hexadecimal.
    * @li The serialized data.
    */

    /// Asynchronously write a data structure to the socket.
    void async_write(msg_ptr msg);

    template <typename func_t>
    void async_write(msg_ptr msg,func_t on_write_func);

    void on_write(const boost::system::error_code& ec,write_tuple_ptr wt);

    void on_write_msg(const boost::system::error_code& ec,msg_ptr msg);


    /// Asynchronously read a data structure from the socket.
    void async_read();

    /// Handle a completed read of a message header. The handler is passed using
    /// a tuple since boost::bind seems to have trouble binding a function object
    /// created using boost::bind as a parameter.
    void on_read_header(
      boost::shared_ptr<std::vector<char>> inbound_header,
      const boost::system::error_code& ec, // Result of operation.
      std::size_t bytes_transferred);       // Number of bytes read.

    /// Handle a completed read of message data.
    void on_read_data(
      boost::shared_ptr<std::vector<char>> inbound_data,
      const boost::system::error_code& ec, // Result of operation.
      std::size_t bytes_transferred);  // Number of bytes read.

    void on_error_ex_ec(std::exception& ex,const boost::system::error_code& ec);

    friend class base_t;
    friend class base_t::base_t;

  private:

    /// The size of a fixed length header.
    enum { header_length_=8 };

  }; // class serialization_session

} // namespace splice {

#if defined(SPLICE_HEADER_ONLY)
# include "serialization_session.hxx"
#endif // defined(SPLICE_HEADER_ONLY)

#endif // SERIALIZATION_SESSION_HPP
