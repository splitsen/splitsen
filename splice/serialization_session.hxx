
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

#include "serialization_session.hpp"
#include <boost/asio/write.hpp>
#include <boost/asio/placeholders.hpp>


namespace splice
{

    template <typename up_t,typename msg_t,typename log_t>
    serialization_session<up_t,msg_t,log_t>::serialization_session(boost::asio::io_service& io_service)
      :base_t(io_service)
    {
    }

    template <typename up_t,typename msg_t,typename log_t>
    serialization_session<up_t,msg_t,log_t>::serialization_session(socket_t& socket)
      :base_t(socket)
    {
    }

    template <typename up_t,typename msg_t,typename log_t>
    void serialization_session<up_t,msg_t,log_t>::on_read(msg_ptr msg)
    {
      BOOST_STATIC_ASSERT_MSG(false,
        "on_read function must be defined in a server derived class");
    }

    template <typename up_t,typename msg_t,typename log_t>
    void serialization_session<up_t,msg_t,log_t>::on_handshake_success()
    {
      log_trace(EZ_FLFT,"");
      async_read();
    }

    template <typename up_t,typename msg_t,typename log_t>
    void serialization_session<up_t,msg_t,log_t>::handle_read(msg_ptr, const boost::system::error_code& error)
    {
      if (error)
      {
        cast_up()->on_error_code(EZ_FLF,error);
        return;
      }
    }

    template <typename up_t,typename msg_t,typename log_t>
    void serialization_session<up_t,msg_t,log_t>::handle_read_dataframe(
      incoming_data_ptr incoming_data,
      const boost::system::error_code& error,
      std::size_t bytes_transferred)
    {
      // async called by native_session::on_handshake_success
      // Asynchronously read a data structure from the socket.
      cast_up()->async_read();
    }

    template <typename up_t,typename msg_t,typename log_t>
    void serialization_session<up_t,msg_t,log_t>::async_write(msg_ptr msg)
    {
      cast_up()->async_write(msg,&up_t::on_write);
    }
    
    template <typename up_t,typename msg_t,typename log_t>
    template<typename func_t>
    void serialization_session<up_t,msg_t,log_t>::async_write(msg_ptr msg,func_t on_write_func)
    {
      namespace ba = boost::asio;
      namespace ph = ba::placeholders;

      write_tuple_ptr wt(boost::make_shared<write_tuple_t>(msg));

      // Serialize the data first so we know how large it is.
      std::ostringstream archive_stream;
      boost::archive::text_oarchive archive(archive_stream);
      archive << msg;
      std::string& outbound_data = boost::get<1>(*wt) = archive_stream.str();

      // Format the header.
      std::ostringstream header_stream;
      header_stream << std::setw(header_length_)
        << std::hex << outbound_data.size();
      if (!header_stream || header_stream.str().size() != header_length_)
      {
        // Something went wrong, inform the caller.
        cast_up()->on_error_code(EZ_FLF,ba::error::invalid_argument);
        return;
      }
      std::string& outbound_header = boost::get<2>(*wt) = header_stream.str();

      // Write the serialized data to the socket. We use "gather-write" to send
      // both the header and the data in a single write operation.
      std::vector<ba::const_buffer> buffers;
      buffers.push_back(ba::buffer(outbound_header));
      buffers.push_back(ba::buffer(outbound_data));

      ba::async_write(get_socket(), buffers,
        get_strand().wrap(bind(on_write_func, sp_cast_up(),
        ph::error, wt)));
    }

    template <typename up_t,typename msg_t,typename log_t>
    void serialization_session<up_t,msg_t,log_t>::on_write(const boost::system::error_code& ec, write_tuple_ptr wt)
    {
      msg_ptr msg=boost::get<0>(*wt);
      cast_up()->on_write_msg(ec,msg);
    }

    template <typename up_t,typename msg_t,typename log_t>
    void serialization_session<up_t,msg_t,log_t>::on_write_msg(const boost::system::error_code& ec,msg_ptr msg)
    {
      if (ec)
        cast_up()->on_error_code(EZ_FLF,ec);
    }

    template <typename up_t,typename msg_t,typename log_t>
    void serialization_session<up_t,msg_t,log_t>::async_read()
    {
      log_trace(EZ_FLFT,"");

      namespace ph = boost::asio::placeholders;
      namespace ba = boost::asio;

      // Issue a read operation to read exactly the number of bytes in a header.
      // TODO two vector buffer are created, only one should be OK
      boost::shared_ptr<std::vector<char>> inbound_header(
        boost::make_shared<std::vector<char>>(header_length_));
      get_socket().async_read_some(ba::buffer(*inbound_header, header_length_),
        get_strand().wrap(boost::bind(&up_t::on_read_header, sp_cast_up(),
        inbound_header,
        ph::error,
        ph::bytes_transferred)));
    }

    template <typename up_t,typename msg_t,typename log_t>
    void serialization_session<up_t,msg_t,log_t>::on_read_header(
      boost::shared_ptr<std::vector<char>> inbound_header,
      const boost::system::error_code& ec, // Result of operation.
      std::size_t bytes_transferred)       // Number of bytes read.
    {
      namespace ph = boost::asio::placeholders;
      namespace ba = boost::asio;

      log_trace(EZ_FLFT,"");

      if (ec)
      {
        cast_up()->on_error_code(EZ_FLF,ec);
        return;
      }
      BOOST_ASSERT(bytes_transferred == header_length_);

      // Determine the length of the serialized data.
      std::istringstream is(std::string(inbound_header->begin(), inbound_header->end()));
      std::size_t inbound_data_size = 0;
      if (!(is >> std::hex >> inbound_data_size))
      {
        // Header doesn't seem to be valid. Inform the caller.
        // TODO create splice specific error
        cast_up()->on_error_code(EZ_FLF,boost::system::error_code(ba::error::invalid_argument));
        return;
      }

      // Start an asynchronous call to receive the data.
      /// Holds the inbound data.
      boost::shared_ptr<std::vector<char>> 
        inbound_data(boost::make_shared<std::vector<char>>(inbound_data_size));
      BOOST_ASSERT(inbound_data->size() == inbound_data_size);
      get_socket().async_read_some(ba::buffer(*inbound_data),
        get_strand().wrap(boost::bind(&up_t::on_read_data, sp_cast_up(),
        inbound_data,
        ph::error,
        ph::bytes_transferred)));
    }

    template <typename up_t,typename msg_t,typename log_t>
    void serialization_session<up_t,msg_t,log_t>::on_read_data(
      boost::shared_ptr<std::vector<char>> inbound_data,
      const boost::system::error_code& ec, // Result of operation.
      std::size_t bytes_transferred)  // Number of bytes read.
    {
      log_trace(EZ_FLFT,"");

      if (ec)
      {
        cast_up()->on_error_code(EZ_FLF,ec);
        return;
      }
      BOOST_ASSERT(inbound_data->size()==bytes_transferred);

      // Extract the data structure from the data just received.
      msg_ptr msg;
      try
      {
        std::string archive_data(inbound_data->begin(), inbound_data->end());
        std::istringstream archive_stream(archive_data);
        boost::archive::text_iarchive archive(archive_stream);
        archive >> msg;
      }
      catch (std::exception& ex)
      {
        // Unable to decode data.
        on_error_ex_ec(ex, boost::system::error_code(boost::asio::error::invalid_argument));
        return;
      }

      //// Inform caller that data has been received ok.
      // buffer vector should be passed as parameter if on_read send_back
      // in order to avoid a new buffer allocation
      cast_up()->on_read(msg);
    }

    template <typename up_t,typename msg_t,typename log_t>
    void serialization_session<up_t,msg_t,log_t>::on_error_ex_ec(std::exception& ex, const boost::system::error_code& ec)
    {
    }

} // namespace splice {
