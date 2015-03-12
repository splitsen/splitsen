//
// http_session.cpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_CONFIG_HPP
# include <boost/config.hpp>
#endif
#ifdef BOOST_HAS_PRAGMA_ONCE
# pragma once
#endif
#include "../detail/config.hpp"

#include "http_session.hpp"
#include <vector>
#include <boost/bind.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/placeholders.hpp>

namespace splice
{

  template <typename up_t,typename log_t>
  http_session<up_t,log_t>::http_session
    (socket_t& socket
    ,http::server::request_handler& handler)
    :base_t(socket)
    ,request_handler_(handler)
  {
  }

  template <typename up_t,typename log_t>
  template<typename _t>
  void http_session<up_t,log_t>::handshake(
    _t handshake_fail,
    hand_shake_data_t& incoming)
  {
    log_info_t(EZ_FLFT,
      "incoming= ",incoming.data(),incoming.size());

    if(incoming.tag_!=hand_shake_t::http)
    {
      handshake_fail(incoming,move_socket());
      return;
    }

    const boost::system::error_code ec;
    on_first_read(
      handshake_fail,
      incoming.data_,
      incoming.size(),
      ec);
  }

  template <typename up_t,typename log_t>
  template< typename _t>
  void http_session<up_t,log_t>::on_first_read(
    _t handshake_fail,
    incoming_data_ptr incoming_data,
    std::size_t bytes_transferred,
    const boost::system::error_code& ec)
  {
    namespace ba=boost::asio;
    using namespace http;
    using namespace http::server;
    using string=std::string;

    log_trace(EZ_FLFT,"");
    if(ec)
    {
      if(ec!=boost::asio::error::operation_aborted)
        cast_up()->on_error_code(EZ_FLF,ec);
      return;
    }

    //install_handshake_timeout();

    const string request(incoming_data->c_array(),bytes_transferred);
    log_trace(EZ_FLFT,request);

    if(request.find("Sec-WebSocket-Key:")!=string::npos)
    {
      log_info(EZ_FLFT,"could be a web socket request");
      hand_shake_data_t incoming(incoming_data,bytes_transferred,hand_shake_t::http);
      handshake_fail(incoming,cast_up()->move_socket());
      return;
    }

    boost::tribool result;
    boost::tie(result,boost::tuples::ignore)=request_parser_.parse(
      request_,incoming_data->data(),incoming_data->data()+bytes_transferred);

    if(result)
    {
      log_trace(EZ_FLFT,"if(result)");
      request_handler_.handle_request(request_,reply_);
      boost::asio::async_write(get_socket(),reply_.to_buffers(),
        get_strand().wrap(
        boost::bind(&http_session::handle_write,shared_from_this(),
        boost::asio::placeholders::error)));
    }
    else if(!result)
    {
      log_trace(EZ_FLFT,"else if(!result)");
      hand_shake_data_t incoming(incoming_data,bytes_transferred,hand_shake_t::http);
      handshake_fail(incoming,cast_up()->move_socket());
      return;

      //reply_=reply::stock_reply(reply::bad_request);
      //boost::asio::async_write(get_socket(),reply_.to_buffers(),
      //  get_strand().wrap(
      //  boost::bind(&http_session::handle_write,shared_from_this(),
      //  boost::asio::placeholders::error)));
    }
    else
    {
      log_trace(EZ_FLFT,"else");
      get_socket().async_read_some(boost::asio::buffer(*incoming_data),
        get_strand().wrap(
        boost::bind(&http_session::handle_read,shared_from_this(),
        incoming_data,
        boost::asio::placeholders::error,
        boost::asio::placeholders::bytes_transferred)));
    }
  }

  template <typename up_t,typename log_t>
  void http_session<up_t,log_t>::handle_read(
    incoming_data_ptr buffer,
    const boost::system::error_code& e,
    std::size_t bytes_transferred)
  {
    namespace ba=boost::asio;
    using namespace http;
    using namespace http::server;
    using string=std::string;

    log_trace(EZ_FLFT,"");

    if(!e)
    {
      boost::tribool result;
      boost::tie(result,boost::tuples::ignore)=request_parser_.parse(
        request_,buffer->data(),buffer->data()+bytes_transferred);

      if(result)
      {
        request_handler_.handle_request(request_,reply_);
        boost::asio::async_write(get_socket(),reply_.to_buffers(),
          get_strand().wrap(
          boost::bind(&http_session::handle_write,shared_from_this(),
          boost::asio::placeholders::error)));
      }
      else if(!result)
      {
        reply_=reply::stock_reply(reply::bad_request);
        boost::asio::async_write(get_socket(),reply_.to_buffers(),
          get_strand().wrap(
          boost::bind(&http_session::handle_write,shared_from_this(),
          boost::asio::placeholders::error)));
      }
      else
      {
        get_socket().async_read_some(boost::asio::buffer(*buffer),
          get_strand().wrap(
          boost::bind(&http_session::handle_read,shared_from_this(),
          buffer,
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred)));
      }
    }
    else if(e!=boost::asio::error::operation_aborted)
    {
      log_trace(EZ_FLFT,e.message());
      get_socket().close();
    }
  }

  template <typename up_t,typename log_t>
  void http_session<up_t,log_t>::handle_write(const boost::system::error_code& e)
  {
    if(!e)
    {
      // Initiate graceful http_session closure.
      boost::system::error_code ignored_ec;
      get_socket().shutdown(boost::asio::ip::tcp::socket::shutdown_both,ignored_ec);
    }

    if(e!=boost::asio::error::operation_aborted)
    {
      log_trace(EZ_FLFT,e.message());
      get_socket().close();
    }
  }

} // namespace splice
