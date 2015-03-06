
//          Copyright Jean Davy 2014-2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef SPLICE_COMMON_TYPES_HPP
#define SPLICE_COMMON_TYPES_HPP

#ifndef BOOST_CONFIG_HPP
# include <boost/config.hpp>
#endif
#ifdef BOOST_HAS_PRAGMA_ONCE
# pragma once
#endif

#include "detail/config.hpp"

#include <utility>
#include <string>

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/array.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/asio/ip/tcp.hpp>

namespace splice
{
  // The buffer for incoming data. http://www.boost.org/doc/libs/1_57_0/doc/html/boost/array.html
  typedef boost::array<char,8192>                     incoming_data_t; //TODO: https://trello.com/c/lHa7pNiZ
  typedef boost::shared_ptr<incoming_data_t>          incoming_data_ptr;
  typedef boost::shared_ptr<const incoming_data_t>    c_incoming_data_ptr;


  struct hand_shake_data_t
  {
    enum class type_t:char
    {
      http, // first read by 'http protocol session'
      web_socket, // first read by 'web socket protocol session' 
      ws_guid, // web socket protocol is accepted, next step is to test GUID
      none //
    };

    const type_t tag_;

    incoming_data_ptr data_;

    std::size_t size_; // is bytes transferred

    hand_shake_data_t(incoming_data_ptr& data,std::size_t size
      ,type_t tag=type_t::none)
      :tag_(tag)
      ,data_(data)
      ,size_(size)
    {
    }

    const char* data() const 
    { 
      return data_->data(); 
    }

    size_t size() const { return size_; }

    const std::string string() const { return std::string(data(),size()); }
  };

  inline incoming_data_ptr mk_incoming_data()
  {
    return boost::make_shared < incoming_data_t >();
  }

  typedef boost::asio::ip::tcp::socket                socket_t;


} //namespace splice {


template<> inline
std::string boost::lexical_cast<std::string>(const splice::hand_shake_data_t::type_t& t)
{
  switch(t)
  {
  case splice::hand_shake_data_t::type_t::http:
    return std::string("http");
  case splice::hand_shake_data_t::type_t::web_socket:
    return std::string("web_socket");
  case splice::hand_shake_data_t::type_t::ws_guid:
    return std::string("ws_guid");
  case splice::hand_shake_data_t::type_t::none:
    return std::string("none");
  }

  std::stringstream ss;
  ss<<"unknown:"<<(int)t;
  return ss.str();
}

#endif // #ifndef SPLICE_COMMON_TYPES_HPP
