
//          Copyright Jean Davy 2014-2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_CONFIG_HPP
# include <boost/config.hpp>
#endif
#ifdef BOOST_HAS_PRAGMA_ONCE
# pragma once
#endif
#include "detail/config.hpp"

#include "tcp_session.hpp"

#include <boost/bind.hpp>
#include <boost/bind/protect.hpp>
#include <boost/thread/lock_guard.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/placeholders.hpp>

namespace splice
{

  template <typename up_t,typename log_t>
  tcp_session<up_t,log_t>::tcp_session(boost::asio::io_service& io_service)
    :strand_(io_service)
    ,socket_(io_service)
    ,handshake_timeout_(io_service)
  {
    log_constructor(EZ_FLF);
  }

  template <typename up_t,typename log_t>
  tcp_session<up_t,log_t>::tcp_session(socket_t& socket)
    :socket_(std::move(socket))
    ,strand_(socket.get_io_service())
    ,handshake_timeout_(socket.get_io_service())
  {
    log_constructor(EZ_FLF);
  }

  template <typename up_t,typename log_t>
  void tcp_session<up_t,log_t>::log_constructor(const char* file,unsigned line,
    const char* func)
  {
    std::stringstream ss;
    ss<<"(0x"<<std::hex<<cast_up()<<") "<<typeid(up_t).name()<<" constructor";
    log_info(file,line,func,cast_up(),ss.str());
  }

  template <typename up_t,typename log_t>
  tcp_session<up_t,log_t>::~tcp_session()
  {
    std::stringstream ss;
    ss<<"(0x"<<std::hex<<cast_up()<<") "<<typeid(up_t).name()<<" destructor";
    log_info(EZ_FLFT,ss.str());

    shutdown();
  }

  template <typename up_t,typename log_t>
  void tcp_session<up_t,log_t>::on_socket_connected()
  {
    namespace ba=boost::asio;

    log_trace(EZ_FLFT,"");

    incoming_data_ptr incoming_data(mk_incoming_data());
    get_socket().async_read_some(ba::buffer(*incoming_data),
      get_strand().wrap(boost::bind(&up_t::on_first_read,sp_cast_up(),
      incoming_data,
      ba::placeholders::bytes_transferred,
      ba::placeholders::error)));
  }

  template <typename up_t,typename log_t>
  void tcp_session<up_t,log_t>::on_first_read(
    incoming_data_ptr incoming_data
    ,std::size_t bytes_transferred
    ,const error_code& error)
  {
    BOOST_STATIC_ASSERT_MSG(false,
      "void on_first_read func must be defined in a derived class");
  }

  template <typename up_t,typename log_t>
  template< typename _t>
  void tcp_session<up_t,log_t>::on_socket_connected(_t handshake_fail)
  {
    namespace ba=boost::asio;

    log_trace(EZ_FLFT,"");

    //install_handshake_timeout();

    incoming_data_ptr incoming_data(mk_incoming_data());
    auto tt=boost::protect(handshake_fail);
    get_socket().async_read_some(ba::buffer(*incoming_data),
      get_strand().wrap(boost::bind( // TODO Item 34: Prefer lambdas to std::bind
      &up_t::on_first_read<decltype(tt)>,sp_cast_up(),
      tt,// handshake_fail
      incoming_data,
      ba::placeholders::bytes_transferred,
      ba::placeholders::error)));
  }

  template <typename up_t,typename log_t>
  template< typename _t>
  void tcp_session<up_t,log_t>::on_first_read(
    _t handshake_fail,
    incoming_data_ptr incoming_data,
    std::size_t bytes_transferred,
    const error_code& error)
  {
    log_trace(EZ_FLFT,"");

    cast_up()->handshake_timeout_cancel();

    if(error)
    {
      // this is not a protocol error but a system one
      // so don't call handshake_fail
      cast_up()->on_error_code(EZ_FLF,error);
      return;
    }
  }

  template <typename up_t,typename log_t>
  template<typename _t>
  void tcp_session<up_t,log_t>::handshake(
    _t handshake_fail,
    hand_shake_data_t& incoming)
  {
    if(cast_up()->try_handshake(incoming))
      cast_up()->on_handshake_success();
    else
      handshake_fail(incoming,cast_up()->move_socket());
  }

  template <typename up_t,typename log_t>
  bool tcp_session<up_t,log_t>::try_handshake(const hand_shake_data_t& incoming)
  {
    BOOST_STATIC_ASSERT_MSG(false,
      "void do_handshake func must be defined in a derived class");
  }

  template <typename up_t,typename log_t>
  void tcp_session<up_t,log_t>::on_handshake_success()
  {
    BOOST_STATIC_ASSERT_MSG(false,
      "void on_handshake_success func must be defined in a derived class");
  }

  template <typename up_t,typename log_t>
  socket_t& tcp_session<up_t,log_t>::get_socket()
  {
    return socket_;
  }

  template <typename up_t,typename log_t>
  boost::asio::io_service::strand& tcp_session<up_t,log_t>::get_strand()
  {
    return strand_;
  }

  template <typename up_t,typename log_t>
  boost::asio::io_service& tcp_session<up_t,log_t>::get_io_service()
  {
    return get_socket().get_io_service();
  }

  template <typename up_t,typename log_t>
  void tcp_session<up_t,log_t>::shutdown()
  {
    {
      std::stringstream ss;
      if(socket_.is_open())
        ss<<" is open";
      else
        ss<<" is close";
      std::string msg(ss.str());
      log_trace(EZ_FLFT,msg);
    }

    boost::system::error_code ec_shutdown,ec_close;
    {
      boost::lock_guard<boost::mutex> lock(mutex_socket_);
      if(!socket_.is_open())
        return;

      // Different ways a socket may be shutdown,
      // http://www.boost.org/doc/libs/1_57_0/doc/html/boost_asio/reference/basic_stream_socket/shutdown_type.html
      // http://www.boost.org/doc/libs/1_57_0/doc/html/boost_asio/reference/basic_stream_socket/shutdown.html
      socket_.shutdown(socket_t::shutdown_both,ec_shutdown);
      // For portable behaviour with respect to graceful closure of a connected socket, call shutdown() before closing the socket,
      // http://www.boost.org/doc/libs/1_57_0/doc/html/boost_asio/reference/basic_stream_socket/close/overload2.html
      socket_.close(ec_close);
    }

    cast_up()->on_shutdown(ec_shutdown,ec_close);
  }

  template <typename up_t,typename log_t>
  void tcp_session<up_t,log_t>::on_error(const char* file,unsigned line
    ,const char* func
    ,std::string msg="")
  {
    log_error(file,line,func,cast_up()
      ,msg.empty()?std::string("[on_error]"):msg);
    shutdown();
  }

  template <typename up_t,typename log_t>
  void tcp_session<up_t,log_t>::on_error_code(const char* file,unsigned line,const char* func
    ,const boost::system::error_code& ec)
  {
    std::stringstream ss;
    ss<<"[on_error]"<<ec<<"="<<ec.message();
    log_error(file,line,func,cast_up(),ss.str());
    shutdown();
  }

  template <typename up_t,typename log_t>
  void tcp_session<up_t,log_t>::on_shutdown(
    const boost::system::error_code& shut_down_ec,
    const boost::system::error_code& close_ec)
  {
    if(shut_down_ec)
    {
      std::stringstream ss;
      ss<<"[on_error]"<<shut_down_ec<<"="<<shut_down_ec.message();
      cast_up()->log_error(EZ_FLFT,ss.str());
    }
    if(close_ec)
    {
      std::stringstream ss;
      ss<<"[on_error]"<<close_ec<<"="<<close_ec.message();
      cast_up()->log_error(EZ_FLFT,ss.str());
    }
    if(!shut_down_ec&&!close_ec)
      cast_up()->log_info(EZ_FLFT,"");
  }

  template <typename up_t,typename log_t>
  void tcp_session<up_t,log_t>::on_wait_connect()
  {
    log_trace(EZ_FLFT,"");
  }

  template <typename up_t,typename log_t>
  void tcp_session<up_t,log_t>::async_read_string()
  {
    namespace ba=boost::asio;

    log_trace(EZ_FLFT,"");

    incoming_data_ptr incoming_data(mk_incoming_data());
    get_socket().async_read_some(ba::buffer(*incoming_data),
      get_strand().wrap(
      boost::bind(&up_t::on_async_read_string,sp_cast_up(),
      incoming_data,
      ba::placeholders::bytes_transferred,
      ba::placeholders::error)));
  }

  template <typename up_t,typename log_t>
  void tcp_session<up_t,log_t>::on_async_read_string(
    incoming_data_ptr incoming_data,
    std::size_t bytes_transferred,
    const error_code& error)
  {
    log_trace(EZ_FLFT,"");

    if(error)
    {
      // this is not a protocol error but a system one
      // so don't call handshake_fail
      cast_up()->on_error_code(EZ_FLF,error);
      return;
    }

    cast_up()->on_read_string(
      std::string(incoming_data->data(),bytes_transferred));
  }

  template <typename up_t,typename log_t>
  void tcp_session<up_t,log_t>::on_read_string(std::string& msg)
  {
    BOOST_STATIC_ASSERT_MSG(false,
      "void \'on_read_string\' function must be defined in a derived class");
  }

  template <typename up_t,typename log_t>
  void tcp_session<up_t,log_t>::async_write(const std::string& msg)
  {
    async_write(msg,&up_t::on_write);
  }

  template <typename up_t,typename log_t>
  template<typename func_t>
  void tcp_session<up_t,log_t>::async_write(const std::string& msg,func_t on_write_func)
  {
    namespace ba=boost::asio;

    log_trace(EZ_FLFT,"");

    boost::shared_ptr<std::string> str(boost::make_shared<std::string>(msg));
    ba::async_write(get_socket(),ba::buffer(str->c_str(),str->length()),
      get_strand().wrap(bind(on_write_func,sp_cast_up(),
      str,ba::placeholders::error)));
  }

  template <typename up_t,typename log_t>
  void tcp_session<up_t,log_t>::on_write(boost::shared_ptr<std::string> msg,const error_code& error)
  {
    log_trace(EZ_FLFT,"");

    if(error)
      cast_up()->on_error_code(EZ_FLF,error);
  }

  template <typename up_t,typename log_t>
  void tcp_session<up_t,log_t>::on_handshake_timeout(
    const error_code& error)
  {
    //log_trace(EZ_FLFT,"");

    //if(error)
    //{
    //  if(error==boost::asio::error::operation_aborted)
    //    // launched by successfull handshake
    //    log_info(EZ_FLFT,"canceled by successfull handshake");
    //  else
    //    cast_up()->on_error_code(EZ_FLF,error);
    //  return;
    //}

    //const bool KO=handshake_timeout_.expires_at()<=
    //  boost::asio::deadline_timer::traits_type::now();

    //log_info(EZ_FLFT,KO?"over deadline":"on-going deadline");

    //if(KO)
    //{ // The deadline has passed, 
    //  // cancel any outstanding asynchronous operations.
    //  if(socket_==nullptr)
    //  {
    //    log_error(EZ_FLFT,"socket_==nullptr but timer still alive");
    //    return;
    //  }
    //  error_code ec;
    //  get_socket().cancel(ec);
    //  if(ec)
    //    cast_up()->on_error_code(EZ_FLF,ec);
    //}
  }

  template <typename up_t,typename log_t>
  void tcp_session<up_t,log_t>::install_handshake_timeout(unsigned seconds_value=0)
  {
    //using boost::bind;
    //namespace ba=boost::asio;
    //namespace placeholders=ba::placeholders;
    //namespace posix_time=boost::posix_time;

    //if(seconds_value==0)
    //  return;

    //error_code ec;
    //std::size_t count = handshake_timeout_.cancel(ec);
    //std::stringstream ss;
    //ss<<"count="<<count<<ec;
    //log_trace(EZ_FLFT,ss.str());
    //
    //if(count>1)
    //{
    //  // no more that one handshake timer running,
    //  // if on-going (cf. multiple handshake), cancel to launch again
    //  cast_up()->on_error_code(EZ_FLF,ec);
    //  return;
    //}

    //// define the callback
    //handshake_timeout_.async_wait(
    //  bind(&up_t::on_handshake_timeout,sp_cast_up()
    //  ,placeholders::error));

    //// Set a deadline for the handshake operation,
    //// Cancels any pending asynchronous waits, and
    //// returns number of asynchronous operations that were cancelled
    //count = handshake_timeout_.expires_from_now(posix_time::seconds(seconds_value),ec);
    //ss=std::stringstream(); // TODO another way to reset ?
    //ss<<"handshake_timeout_.expires_from_now="<<count
    //  <<" ec="<<ec;
    //log_info(EZ_FLFT,ss.str());

    ////if(count)
    ////  //Start new asynchronous wait.
    ////  handshake_timeout_.async_wait(
    ////    bind(&up_t::on_handshake_timeout,sp_cast_up()
    ////    ,placeholders::error));

    ////if(ec)
    ////  cast_up()->on_error_code(EZ_FLF,ec);
  }

  template <typename up_t,typename log_t>
  size_t tcp_session<up_t,log_t>::handshake_timeout_cancel()
  {
    return 0;
    //log_trace(EZ_FLFT,"");

    //error_code ec;
    ////return The number of asynchronous operations that were cancelled
    //std::size_t count=handshake_timeout_.cancel(ec);
    ////if(!(count<=1||ec))
    //{
    //  std::stringstream ss;
    //  ss<<"count="<<count<<" "<<ec<<"="<<ec.message();
    //  log_error(EZ_FLFT,ss.str());
    //}

    //return count;
  }

  template <typename up_t,typename log_t>
  up_t* tcp_session<up_t,log_t>::cast_up()
  {
    return static_cast<up_t*>(this);
  }

  template <typename up_t,typename log_t>
  boost::shared_ptr<up_t> tcp_session<up_t,log_t>::sp_cast_up()
  {
    return boost::shared_ptr<up_t>(shared_from_this());
  }

  template <typename up_t,typename log_t>
  socket_t& tcp_session<up_t,log_t>::move_socket()
  {
    log_trace(EZ_FLFT,"");
    handshake_timeout_cancel();
    return socket_;
  }

}//  namespace splice
