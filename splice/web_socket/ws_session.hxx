
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
#include "../detail/config.hpp"

#include "ws_session.hpp"
#include <boost/asio/write.hpp>
#include <boost/asio/placeholders.hpp>

namespace splice
{

  template <typename up_t,typename log_t>
  ws_session<typename up_t,typename log_t>::ws_session(boost::asio::io_service& io_service)
    : base_t(io_service)
  {
  }

  template <typename up_t,typename log_t>
  ws_session<typename up_t,typename log_t>::ws_session(socket_t& socket)
    :base_t(socket)
  {
  }

  template <typename up_t,typename log_t>
  void ws_session<typename up_t,typename log_t>::on_handshake_success()
  {
    namespace ba=boost::asio;

    log_trace(EZ_FLFT,"");

    incoming_data_ptr incoming_data(mk_incoming_data());
    get_socket().async_read_some(ba::buffer(*incoming_data),
      get_strand().wrap(
      bind(&up_t::on_read_dataframe,sp_cast_up(),
      incoming_data,
      mk_frame_parser(),
      ba::placeholders::error,
      ba::placeholders::bytes_transferred)));
  }

  template <typename up_t,typename log_t>
  template<typename _t>
  data_frame_ptr ws_session<typename up_t,typename log_t>::mk_data_frame(_t p)
  {
    return boost::make_shared<data_frame>(p);
  }

  template <typename up_t,typename log_t>
  void ws_session<typename up_t,typename log_t>::async_write(const std::string& msg)
  {
    async_write(msg,&up_t::on_write_dataframe);
  }

  template <typename up_t,typename log_t>
  template<typename func_t>
  void ws_session<typename up_t,typename log_t>::async_write(const std::string& msg,func_t on_write_func)
  {
    namespace ba=boost::asio;

    log_trace(EZ_FLFT,msg);

    data_frame_ptr df(mk_data_frame(msg));
    ba::async_write(get_socket(),df->to_buffers(),
      get_strand().wrap(bind(on_write_func,sp_cast_up(),
      df,
      ba::placeholders::error)));
  }

  template <typename up_t,typename log_t>
  void ws_session<typename up_t,typename log_t>::on_read(std::string&  df)
  {
    BOOST_STATIC_ASSERT_MSG(false,
      "void on_read func must be defined in a ws_session derived class");
  }

  template <typename up_t,typename log_t>
  void ws_session<typename up_t,typename log_t>::on_read_data_frame(
    incoming_data_ptr& incoming_data,
    data_frame& read_frame)
  {
    namespace ba=boost::asio;
    using std::string;
    using boost::lexical_cast;

    log_trace(EZ_FLFT,
      string("operation code=")+lexical_cast<string>(read_frame.opcode_));

    switch(read_frame.opcode_)
    {
    case data_frame::text_frame:
      cast_up()->on_read(
        std::string(read_frame.payload_.begin(),read_frame.payload_.end()));

      get_socket().async_read_some(ba::buffer(*incoming_data),
        get_strand().wrap(
        bind(&up_t::on_read_dataframe,sp_cast_up(),
        incoming_data,
        mk_frame_parser(),
        ba::placeholders::error,
        ba::placeholders::bytes_transferred)));
      break;
    case data_frame::pong:
    case data_frame::ping:
      get_socket().async_read_some(ba::buffer(*incoming_data),
        get_strand().wrap(
        bind(&up_t::on_read_dataframe,sp_cast_up(),
        incoming_data,
        mk_frame_parser(),
        ba::placeholders::error,
        ba::placeholders::bytes_transferred)));
      break;
    case data_frame::continuation_frame:
    case data_frame::binary_frame:
    case data_frame::connection_close:
    case data_frame::reserved:
    default:
      cast_up()->on_error(EZ_FLF
        ,"invalid opcode:"+lexical_cast<string>(read_frame.opcode_));
    }
  }

  template <typename up_t,typename log_t>
  void ws_session<typename up_t,typename log_t>::on_read_dataframe(
    incoming_data_ptr incoming_data,
    frame_parser_ptr frame_parser,
    const error_code& error,
    std::size_t bytes_transferred)
  {
    namespace ba=boost::asio;
    using std::string;
    using boost::lexical_cast;

    log_trace(EZ_FLFT,"");

    if(error)
    {
      cast_up()->on_error_code(EZ_FLF,error);
      return;
    }

    boost::tribool result;
    data_frame read_frame;
    boost::tie(result,boost::tuples::ignore)=frame_parser->parse(
      read_frame,incoming_data->data(),incoming_data->data()+bytes_transferred);

    log_trace(EZ_FLFT,
      string("result=")+lexical_cast<string>(result.value)
      +string(" ")+lexical_cast<string>(bytes_transferred)
      +string(" bytes transferred"));

    switch(result.value)
    {
    case false: // data is invalid
      cast_up()->on_error(EZ_FLF);
      break;
    case boost::tribool::indeterminate_value: // is 2
      // more data is required
      get_socket().async_read_some(ba::buffer(*incoming_data),
        get_strand().wrap(
        bind(&up_t::on_read_dataframe,sp_cast_up(),
        incoming_data,
        frame_parser,
        ba::placeholders::error,
        ba::placeholders::bytes_transferred)));
      break;
    case true: // a complete data_frame has been parsed
      cast_up()->on_read_data_frame(incoming_data,read_frame);
    }
  }

  template <typename up_t,typename log_t>
  void ws_session<typename up_t,typename log_t>::on_write_dataframe(
    data_frame_ptr df,
    const error_code& error)
  {
    log_trace(EZ_FLFT,"");

    if(error)
      cast_up()->on_error(EZ_FLF);
  }

} // namespace splice {
