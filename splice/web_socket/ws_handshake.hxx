
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

#include "ws_handshake.hpp"
#include <boost/asio/write.hpp>


namespace splice
{

    template <typename up_t,typename log_t>
    ws_handshake<up_t,log_t>::ws_handshake(boost::asio::io_service& io_service)
      :base_t(io_service)
    {
    }

    template <typename up_t,typename log_t>
    ws_handshake<up_t,log_t>::ws_handshake(socket_t& socket)
      :base_t(socket)
    {
    }

    template <typename up_t,typename log_t>
    void ws_handshake<up_t,log_t>::on_first_read( // called from on_connect
      incoming_data_ptr incoming_data
      ,std::size_t bytes_transferred
      ,const error_code& error)
    {
      log_trace(EZ_FLFT,"");

      if(error)
      {
        cast_up()->on_error_code(EZ_FLF,error);
        return;
      }

      first_write(hand_shake_data_t(incoming_data,bytes_transferred));
    }

    template <typename up_t,typename log_t>
    void ws_handshake<up_t,log_t>::first_write(hand_shake_data_t& incoming)
    {
      namespace ba=boost::asio;

      log_info_t(EZ_FLFT,
        "incoming=",incoming.data(),incoming.size());

      boost::logic::tribool result;
      http_request_t hs_req;
      boost::tie(result,boost::tuples::ignore)=request_parser().parse(
        hs_req,incoming.data(),incoming.data()+incoming.size());

      if(result)
      {
        http_reply_ptr reply(mk_http_reply(request_handler::handle_request(hs_req)));

        switch(reply->status_)
        {
        case http_reply_t::bad_request:
        default:
          cast_up()->on_error(EZ_FLF);
          return;
        case http_reply_t::switching_protocols:
          ba::async_write(get_socket(),reply->to_buffers(),
            get_strand().wrap(
            bind(&up_t::on_second_read,sp_cast_up(),
            reply,
            ba::placeholders::error)));
          break;
        }
      }
      else
      {
        cast_up()->on_error(EZ_FLF);
        return;
      }
    }

    template <typename up_t,typename log_t>
    void ws_handshake<up_t,log_t>::on_second_read(
      http_reply_ptr reply,
      const error_code& error)
    {
      using std::string;
      using boost::lexical_cast;

      log_trace(EZ_FLFT,"");

      if(error)
      {
        cast_up()->on_error_code(EZ_FLF,error);
        return;
      }
      log_trace(EZ_FLFT,
        string(" http_reply_t::")+lexical_cast<string>(reply->status_));

      switch(reply->status_)
      {
      case http_reply_t::switching_protocols:
        cast_up()->on_handshake_success();
        break;
      case http_reply_t::bad_request:
      default:
        cast_up()->on_bad_request(EZ_FLF);
        break;
      }
    }

    template <typename up_t,typename log_t>
    template< typename _t>
    void ws_handshake<up_t,log_t>::on_first_read( // called from on_connect
      _t handshake_fail,
      incoming_data_ptr incoming_data,
      std::size_t bytes_transferred,
      const error_code& error)
    {
      using string=std::string;

      if(error)
      {
        cast_up()->on_error_code(EZ_FLF,error);
        return;
      }

      const string request(incoming_data->c_array(),bytes_transferred);
      log_trace(EZ_FLFT,request);

      if(request.find("Sec-WebSocket-Key:")==string::npos)
      {
        log_info(EZ_FLFT,"can't be a web socket request");
        hand_shake_data_t incoming(incoming_data,bytes_transferred,hand_shake_data_t::type_t::http);
        handshake_fail(incoming,cast_up()->move_socket());
        return;
      }

      first_write(handshake_fail,hand_shake_data_t(incoming_data,bytes_transferred));
    }


    template <typename up_t,typename log_t>
    template< typename _t>
    void ws_handshake<up_t,log_t>::first_write(
      _t handshake_fail,
      hand_shake_data_t& incoming)
    {
      namespace ba=boost::asio;

      log_info_t(EZ_FLFT,"",incoming.data(),incoming.size());

      boost::logic::tribool result;
      http_request_t hs_req;
      boost::tie(result,boost::tuples::ignore)=request_parser().parse(
        hs_req,incoming.data(),incoming.data()+incoming.size());

      if(result)
      {
        http_reply_ptr reply(mk_http_reply(request_handler::handle_request(hs_req)));

        switch(reply->status_)
        {
        case http_reply_t::bad_request:
        default:
          log_warning(EZ_FLFT,std::string("[handshake_fail]"));
          handshake_fail(incoming,move_socket());
          return;
        case http_reply_t::switching_protocols:
        {// ok read the GUID
          auto tt=boost::protect(handshake_fail);
          ba::async_write(get_socket(),reply->to_buffers(),
            get_strand().wrap(
            bind(&up_t::on_second_read<decltype(tt)>,sp_cast_up(),
            tt,
            reply,
            ba::placeholders::error)));
        }
        break;
        }
      }
      else
      {
        log_warning(EZ_FLFT,std::string("[handshake_fail]"));
        handshake_fail(incoming,move_socket());
      }
    }

    template <typename up_t,typename log_t>
    template< typename _t>
    void ws_handshake<up_t,log_t>::on_second_read(
      _t handshake_fail,
      http_reply_ptr reply,
      const error_code& error)
    {
      namespace ba=boost::asio;

      log_trace(EZ_FLFT,"");

      if(error)
      {
        cast_up()->on_error_code(EZ_FLF,error);
        return;
      }

      switch(reply->status_)
      {
      case http_reply_t::switching_protocols:
        cast_up()->guid_read(handshake_fail);
        break;
      case http_reply_t::bad_request:
      default:
        cast_up()->on_bad_request(EZ_FLF);
        break;
      }
    }

    template <typename up_t,typename log_t>
    template< typename _t>
    void ws_handshake<up_t,log_t>::guid_read(
      _t handshake_fail)
    {
      namespace ba=boost::asio;

      log_trace(EZ_FLFT,"");

      // Set a deadline for the GUID handshake operation.
      //install_handshake_timeout();

      incoming_data_ptr incoming_data(mk_incoming_data());
      auto tt=boost::protect(handshake_fail);
      get_socket().async_read_some(ba::buffer(*incoming_data),
        get_strand().wrap(
        boost::bind(&up_t::on_guid_read<decltype(tt)>,sp_cast_up(),
        tt, // handshake_fail
        incoming_data,
        mk_frame_parser(),
        ba::placeholders::error,
        ba::placeholders::bytes_transferred)));
    }

    template <typename up_t,typename log_t>
    template< typename _t>
    void ws_handshake<up_t,log_t>::do_custom_handshake(
      _t handshake_fail,
      hand_shake_data_t& in)
    {
      BOOST_ASSERT(in.tag_==hand_shake_data_t::type_t::ws_guid);

      if(!cast_up()->try_handshake(in))
      {
        log_warning(EZ_FLFT,std::string("[handshake_fail] tag=")
          +boost::lexical_cast<std::string>(in.tag_)
          +" incoming="+in.string());
        handshake_fail(in,move_socket());
      }
      else
        cast_up()->on_handshake_success();
    }

    template <typename up_t,typename log_t>
    template< typename _t>
    void ws_handshake<up_t,log_t>::on_guid_read(
      _t handshake_fail,
      incoming_data_ptr incoming_data,
      frame_parser_ptr frame_parser,
      const error_code& error,
      std::size_t bytes_transferred)
    {
      namespace ba=boost::asio;
      using std::string;
      using boost::lexical_cast;

      log_trace(EZ_FLFT,"");

      cast_up()->handshake_timeout_cancel();

      if(error)
      {
        cast_up()->on_error_code(EZ_FLF,error);
        return;
      }

      boost::tribool result;
      data_frame read_frame;
      boost::tie(result,boost::tuples::ignore)=frame_parser->parse(
        read_frame,incoming_data->data(),incoming_data->data()+bytes_transferred);

      log_info(EZ_FLFT,
        string("result=")+lexical_cast<string>(result.value)
        +string(" ")+lexical_cast<string>(bytes_transferred)
        +string(" bytes transferred"));

      switch(result.value)
      {
      case false: // data is invalid
        cast_up()->on_error(EZ_FLF);
        break;
      case boost::tribool::indeterminate_value:
        //// more data is required
        //get_socket().async_read_some(ba::buffer(*incoming_data),
        //  get_strand().wrap(
        //  bind(&up_t::handle_read_dataframe,sp_cast_up(),
        //  incoming_data,
        //  frame_parser,
        //  ba::placeholders::error,
        //  ba::placeholders::bytes_transferred)));
        break;
      case true: // a complete data_frame has been parsed
        frame_parser=mk_frame_parser(); // reset
        switch(read_frame.opcode_)
        {
        case data_frame::text_frame:
        {
          //std::string(read_frame.payload_.begin(),read_frame.payload_.end())));
          auto size=std::distance(read_frame.payload_.begin(),read_frame.payload_.end());
          incoming_data_ptr idp=boost::make_shared<incoming_data_t>();
          std::copy(read_frame.payload_.begin(),read_frame.payload_.end(),idp->begin());
          auto in=hand_shake_data_t(idp,size,hand_shake_data_t::type_t::ws_guid);
          cast_up()->do_custom_handshake(handshake_fail,in);
        }
          break;
          //case data_frame::pong:
          //case data_frame::ping:
          //  get_socket().async_read_some(ba::buffer(*incoming_data),
          //    get_strand().wrap(
          //    bind(&up_t::handle_read_dataframe,sp_cast_up(),
          //    incoming_data,
          //    frame_parser,
          //    ba::placeholders::error,
          //    ba::placeholders::bytes_transferred)));
          //  break;
          //case data_frame::continuation_frame:
          //case data_frame::binary_frame:
          //case data_frame::connection_close:
          //case data_frame::reserved:
        default:
          cast_up()->on_error(EZ_FLF);
        }
      }
    }

    template <typename up_t,typename log_t>
    template<typename _t>
    void ws_handshake<up_t,log_t>::handshake(
      _t handshake_fail,
      hand_shake_data_t& incoming)
    { // ws_handshake::handshake function can be successfull
      // 1- when coming from another ws_handshake session, in that case
      // a third read has been issued, that could be the gui,
      // 2- when coming from an http session, in that case it comes from the first read.
      // incoming.tag_ allows to identify these two cases.

      log_info_t(EZ_FLFT,
        "incoming= ",incoming.data(),incoming.size());

      switch(incoming.tag_)
      {
      case hand_shake_data_t::type_t::http: 
        // First read done by http, try to connect on
        // second stage of web_socket protocol
        cast_up()->first_write(handshake_fail,incoming);
        break;
      case hand_shake_data_t::type_t::ws_guid: 
        cast_up()->do_custom_handshake(handshake_fail,incoming);
        break;
      default:
        handshake_fail(incoming,move_socket());
      }
    }

    template <typename up_t,typename log_t>
    void ws_handshake<up_t,log_t>::on_bad_request(const char* file,unsigned line,const char* func)
    {
      log_error(file,line,func,cast_up(),"on_bad_request");
      shutdown();
    }

    template <typename up_t,typename log_t>
    template<typename _t>
    http_reply_ptr ws_handshake<up_t,log_t>::mk_http_reply(_t p)
    {
      return boost::make_shared<http_reply_t>(p);
    }

    template <typename up_t,typename log_t>
    frame_parser_ptr ws_handshake<up_t,log_t>::mk_frame_parser()
    {
      return boost::make_shared<frame_parser_t>();
    }

} // namespace splice {
