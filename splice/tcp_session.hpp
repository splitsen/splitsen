
//          Copyright Jean Davy 2014-2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TCP_SESSION_HPP
#define TCP_SESSION_HPP

#ifndef BOOST_CONFIG_HPP
# include <boost/config.hpp>
#endif
#ifdef BOOST_HAS_PRAGMA_ONCE
# pragma once
#endif
#include "detail/config.hpp"

#include "common_types.hpp"
#include "logger/log_interface.hpp"

#include <boost/thread/mutex.hpp>
#include <boost/noncopyable.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio/strand.hpp>

namespace splice
{

  /// Represents a single connection from a client.
  template <typename up_t,typename log_t=no_log>
  class tcp_session
    :public boost::enable_shared_from_this<up_t>
    ,private boost::noncopyable
    ,protected log_t
  {
  public:
    using my_t=tcp_session<up_t,log_t>;
    using sp_up_t=boost::shared_ptr<up_t>;

    typedef boost::system::error_code   error_code;

    // Construct a session with the given io_service.
    tcp_session(boost::asio::io_service& io_service) BOOST_NOEXCEPT;

    // Construct a session with the given socket
    // in the context of Splice, socket is connected and this constructor 
    // is called when the first stream of data sent by connected client is 
    // not recognized.
    // So Splice will move this connected socket to a newest class in order
    // to try to handle that stream of data.
    // Socket move means std::move(socket), source socket will no more be connected
    // in favor of this new tcp_session::socket_ class member
    tcp_session(socket_t& socket) BOOST_NOEXCEPT;

    ~tcp_session() BOOST_NOEXCEPT;

    boost::asio::io_service& get_io_service() BOOST_NOEXCEPT;

  protected:

    // Start the first asynchronous operation for a mono protocol server
    void on_socket_connected()BOOST_NOEXCEPT;

    // Start the first asynchronous operation for a multi protocol server
    template< typename _t>
    void on_socket_connected(_t handshake_fail)BOOST_NOEXCEPT;

    // Handles completion of the first read operation after connection
    void on_first_read(
      incoming_data_ptr incoming_data
      ,std::size_t bytes_transferred
      ,const error_code& error)BOOST_NOEXCEPT;

    // Handles completion of the first read operation after connection
    // That is the first handshake stage
    template< typename _t>
    void on_first_read(
      _t handshake_fail,
      incoming_data_ptr incoming_data,
      std::size_t bytes_transferred,
      const error_code& error)BOOST_NOEXCEPT;

    template<typename _t>
    void handshake(_t handshake_fail,hand_shake_data_t& incoming)BOOST_NOEXCEPT;

    bool try_handshake(const hand_shake_data_t& incoming);

    void on_handshake_success()BOOST_NOEXCEPT;

    // Get the socket associated with the session.
    socket_t& get_socket()BOOST_NOEXCEPT;

    boost::asio::io_service::strand& get_strand()BOOST_NOEXCEPT;

    // Graceful closure of a connected socket
    void shutdown()BOOST_NOEXCEPT;

    // CRTP virtual functions
    void on_error(const char* file,unsigned line,const char* func
      ,std::string msg="")BOOST_NOEXCEPT;

    void on_error_code(const char* file,unsigned line,const char* func
      ,const boost::system::error_code& ec)BOOST_NOEXCEPT;

    void on_shutdown(
      const boost::system::error_code& shut_down_ec,
      const boost::system::error_code& close_ec)BOOST_NOEXCEPT;

    void on_wait_connect()BOOST_NOEXCEPT;

    // Launch an inquiry for an incoming string,
    // the 'on_read_string' function will be called
    // when the client wil write to socket.
    // If you remember 'incoming_data_ptr' data from the last previous
    // 'on_read_string', you should pass it as argument in order to avoid
    // a memory allocation, and so speed up the process.
    // Anyway this is optional, and so you may ignore that parameter.


    void async_read_string()BOOST_NOEXCEPT;

    void on_async_read_string(
      incoming_data_ptr incoming_data,
      std::size_t bytes_transferred,
      const error_code& error)BOOST_NOEXCEPT;

    void on_read_string(std::string& msg)BOOST_NOEXCEPT;

    void async_write(const std::string& msg)BOOST_NOEXCEPT;

    template<typename func_t>
    void async_write(const std::string& msg,func_t on_write_func)BOOST_NOEXCEPT;

    void on_write(boost::shared_ptr<std::string> msg,const error_code& error)BOOST_NOEXCEPT;

    void on_handshake_timeout(const error_code& error)BOOST_NOEXCEPT;

    void install_handshake_timeout(unsigned seconds_value=0)BOOST_NOEXCEPT;

    size_t handshake_timeout_cancel()BOOST_NOEXCEPT;

    up_t* cast_up()BOOST_NOEXCEPT;

    sp_up_t sp_cast_up()BOOST_NOEXCEPT;

    // Refer to tcp_session(socket_t&) contructor
    // CRTP overloadable
    socket_t& move_socket()BOOST_NOEXCEPT;

    template <typename up_t,typename below_t,unsigned protocol_count_>
    friend class multi_protocol;
    template <typename up_t,typename below_t>
    friend class mono_protocol;

  private:
    void log_constructor(const char* file,unsigned line,const char* func)BOOST_NOEXCEPT;

    boost::asio::deadline_timer handshake_timeout_;

    /// Strand to ensure the connection's handlers are not called concurrently.
    boost::asio::io_service::strand strand_;

    /// Socket for the connection.
    socket_t socket_;
    boost::mutex mutex_socket_;

  }; //class tcp_session

} // namespace splice {

#if defined(SPLICE_HEADER_ONLY)
# include "tcp_session.hxx"
#endif // defined(SPLICE_HEADER_ONLY)

#endif // #ifndef TCP_SESSION_HPP