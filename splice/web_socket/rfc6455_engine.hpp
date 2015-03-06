
//          Copyright Jean Davy 2014-2015.
//          (jean dot davy dot 77 dot gmail dot com)
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// This code is strongly inspired by Alexander Iacobciuc,
// from his article "Building a basic HTML5 client/server application"
// http://www.codeproject.com/Articles/443660/Building-a-basic-HTML-client-server-application

#ifndef WEBSOCKET_ENGINE_HPP
#define WEBSOCKET_ENGINE_HPP

#ifndef BOOST_CONFIG_HPP
# include <boost/config.hpp>
#endif
#ifdef BOOST_HAS_PRAGMA_ONCE
# pragma once
#endif
#include "../detail/config.hpp"

#include <string>
#include <vector>

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4996 4503)
#endif
#include <boost/asio/buffer.hpp>
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#include <boost/noncopyable.hpp>
#include <boost/array.hpp>
#include <boost/cstdint.hpp>
#include <boost/logic/tribool.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/uuid/sha1.hpp>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/insert_linebreaks.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/archive/iterators/ostream_iterator.hpp>
#include <boost/lexical_cast.hpp>

namespace splice
{

  // For details on a base framing protocol check the following link
  // http://tools.ietf.org/html/rfc6455#section-5.2
  class rfc6455_engine
  {
  public:
    struct header
    {
      std::string name;
      std::string value;
    };

    /// A http reply to be sent to a client.
    class reply
    {
    public:
      /// The status of the reply.
      enum status_type : boost::uint16_t
      {
        switching_protocols = 101,
        bad_request = 400,
        internal_server_error = 500,
      } status_;

      static boost::asio::const_buffer to_buffer(status_type status)
      {
        switch (status)
        {
        case rfc6455_engine::reply::switching_protocols:
        return boost::asio::buffer(
          "HTTP/1.1 101 Switching Protocols\r\n");
        case rfc6455_engine::reply::bad_request:
        return boost::asio::buffer(
          "HTTP/1.1 400 Bad Request\r\n");
        default:
        return boost::asio::buffer(
          "HTTP/1.1 500 Internal Server Error\r\n");
        }
      }

      /// The headers to be included in the reply.
      typedef std::vector<header> headers_t;
      headers_t headers_;

      reply(status_type _status, size_t size = 0)
        : status_(_status)
        , headers_(size)
      {
      }

      /// Convert the reply into a vector of buffers. The buffers do not own the
      /// underlying memory blocks, therefore the reply object must remain valid and
      /// not be changed until the write operation has completed.
      typedef std::vector<boost::asio::const_buffer> buffers_t;
      buffers_t to_buffers() const
      {
        BOOST_ASSERT(!headers_.empty());

        buffers_t buffers(1 + 4 * headers_.size() + 1);

        size_t i = 0;
        buffers[i++] = to_buffer(status_);

        static const char crlf[] = { '\r', '\n' };
        static const char name_value_separator[] = { ':', ' ' };

        for (auto it = headers_.cbegin(); it != headers_.cend(); it++)
        {
          const headers_t::value_type& h = *it;
          buffers[i++] = boost::asio::buffer(h.name);
          buffers[i++] = boost::asio::buffer(name_value_separator);
          buffers[i++] = boost::asio::buffer(h.value);
          buffers[i++] = boost::asio::buffer(crlf);
        };

        buffers[i++] = boost::asio::buffer(crlf);

        return buffers;
      }

      /// Get a stock reply.
      static reply stock_reply(status_type status)
      {
        return reply(status);
      }

    };

    /// A http request received from a client.
    struct request
    {
      std::string method;
      std::string uri;
      int http_version_major;
      int http_version_minor;
      std::vector<header> headers;
    };

    /// The handler for incoming http requests.
    class request_handler
      : private boost::noncopyable
    {
    public:
      /// Handle a request and produce a reply.
      static reply handle_request(const request& req)
      {
        auto it = find_if(req.headers.cbegin(), req.headers.cend(), [](const header& i) {return i.name == "Sec-WebSocket-Key"; });

        if (it == req.headers.cend())
          return reply::stock_reply(reply::bad_request);

        std::string key(it->value);

        const std::string magic_guid("258EAFA5-E914-47DA-95CA-C5AB0DC85B11");

        reply rep(reply::switching_protocols, 3);
        rep.headers_[0].name = "Upgrade";
        rep.headers_[0].value = "websocket";
        rep.headers_[1].name = "Connection";
        rep.headers_[1].value = "Upgrade";
        rep.headers_[2].name = "Sec-WebSocket-Accept";
        rep.headers_[2].value = to_base64(to_sha1(key + magic_guid));

        return rep;
      }

    private:
      /// Encode data using the SHA1 algorithm.
      static std::vector<unsigned char> to_sha1(const std::string& s)
      {
        boost::uuids::detail::sha1 cipher;
        cipher.process_bytes(s.c_str(), s.length());
        boost::uint32_t digest[5];
        cipher.get_digest(digest);

        std::vector<unsigned char> hash(20, 0);
        for (std::size_t i = 0; i < hash.size(); ++i)
        {
          hash[i] = (unsigned char)(digest[i >> 2] >> 8 * (3 - (i & 0x03)));
        }

        return hash;
      }

      /// Encode data using the Base64 algorithm.
      static std::string to_base64(const std::vector<unsigned char>& data)
      {
        BOOST_ASSERT(!data.empty());

        using namespace boost::archive::iterators;

        typedef
          insert_linebreaks <         // insert line breaks every 72 characters
          base64_from_binary<    // convert binary values ot base64 characters
          transform_width<   // retrieve 6 bit integers from a sequence of 8 bit bytes
          const char *,
          6,
          8
          >
          >
          , 72
          >
          base64_iterator;

        std::vector<unsigned char> buff(data);
        size_t number_of_padd_chars = (buff.size() % 3 != 0) ? 3 - (buff.size() % 3) : 0;
        buff.insert(buff.end(), number_of_padd_chars, '\0');

        base64_iterator begin(&buff[0]), end(&buff[0] + data.size());
        std::string result(begin, end);
        result.insert(result.end(), number_of_padd_chars, '=');

        return result;
      }

    };

    /// Parser for incoming requests.
    class request_parser
    {
    public:
      /// Construct ready to parse the request method.
      request_parser()
        : state_(method_start)
      {
      }


      /// Reset to initial parser state.
      void reset()
      {
        state_ = method_start;
      }

      /// Parse some data. The tribool return value is true when a complete request
      /// has been parsed, false if the data is invalid, indeterminate when more
      /// data is required. The InputIterator return value indicates how much of the
      /// input has been consumed.
      template <typename InputIterator>
      boost::tuple<boost::tribool, InputIterator> parse(request& req,
        InputIterator begin, InputIterator end)
      {
        while (begin != end)
        {
          boost::tribool result = consume(req, *begin++);
          if (result || !result)
            return boost::make_tuple(result, begin);
        }
        boost::tribool result = boost::indeterminate;
        return boost::make_tuple(result, begin);
      }

    private:
      /// Handle the next character of input.
      boost::tribool consume(request& req, char input)
      {
        switch (state_)
        {
        case method_start:
        if (!is_char(input) || is_ctl(input) || is_tspecial(input))
        {
          return false;
        }
        else
        {
          state_ = method;
          req.method.push_back(input);
          return boost::indeterminate;
        }
        case method:
        if (input == ' ')
        {
          state_ = uri;
          return boost::indeterminate;
        }
        else if (!is_char(input) || is_ctl(input) || is_tspecial(input))
        {
          return false;
        }
        else
        {
          req.method.push_back(input);
          return boost::indeterminate;
        }
        case uri:
        if (input == ' ')
        {
          state_ = http_version_h;
          return boost::indeterminate;
        }
        else if (is_ctl(input))
        {
          return false;
        }
        else
        {
          req.uri.push_back(input);
          return boost::indeterminate;
        }
        case http_version_h:
        if (input == 'H')
        {
          state_ = http_version_t_1;
          return boost::indeterminate;
        }
        else
        {
          return false;
        }
        case http_version_t_1:
        if (input == 'T')
        {
          state_ = http_version_t_2;
          return boost::indeterminate;
        }
        else
        {
          return false;
        }
        case http_version_t_2:
        if (input == 'T')
        {
          state_ = http_version_p;
          return boost::indeterminate;
        }
        else
        {
          return false;
        }
        case http_version_p:
        if (input == 'P')
        {
          state_ = http_version_slash;
          return boost::indeterminate;
        }
        else
        {
          return false;
        }
        case http_version_slash:
        if (input == '/')
        {
          req.http_version_major = 0;
          req.http_version_minor = 0;
          state_ = http_version_major_start;
          return boost::indeterminate;
        }
        else
        {
          return false;
        }
        case http_version_major_start:
        if (is_digit(input))
        {
          req.http_version_major = req.http_version_major * 10 + input - '0';
          state_ = http_version_major;
          return boost::indeterminate;
        }
        else
        {
          return false;
        }
        case http_version_major:
        if (input == '.')
        {
          state_ = http_version_minor_start;
          return boost::indeterminate;
        }
        else if (is_digit(input))
        {
          req.http_version_major = req.http_version_major * 10 + input - '0';
          return boost::indeterminate;
        }
        else
        {
          return false;
        }
        case http_version_minor_start:
        if (is_digit(input))
        {
          req.http_version_minor = req.http_version_minor * 10 + input - '0';
          state_ = http_version_minor;
          return boost::indeterminate;
        }
        else
        {
          return false;
        }
        case http_version_minor:
        if (input == '\r')
        {
          state_ = expecting_newline_1;
          return boost::indeterminate;
        }
        else if (is_digit(input))
        {
          req.http_version_minor = req.http_version_minor * 10 + input - '0';
          return boost::indeterminate;
        }
        else
        {
          return false;
        }
        case expecting_newline_1:
        if (input == '\n')
        {
          state_ = header_line_start;
          return boost::indeterminate;
        }
        else
        {
          return false;
        }
        case header_line_start:
        if (input == '\r')
        {
          state_ = expecting_newline_3;
          return boost::indeterminate;
        }
        else if (!req.headers.empty() && (input == ' ' || input == '\t'))
        {
          state_ = header_lws;
          return boost::indeterminate;
        }
        else if (!is_char(input) || is_ctl(input) || is_tspecial(input))
        {
          return false;
        }
        else
        {
          req.headers.push_back(header());
          req.headers.back().name.push_back(input);
          state_ = header_name;
          return boost::indeterminate;
        }
        case header_lws:
        if (input == '\r')
        {
          state_ = expecting_newline_2;
          return boost::indeterminate;
        }
        else if (input == ' ' || input == '\t')
        {
          return boost::indeterminate;
        }
        else if (is_ctl(input))
        {
          return false;
        }
        else
        {
          state_ = header_value;
          req.headers.back().value.push_back(input);
          return boost::indeterminate;
        }
        case header_name:
        if (input == ':')
        {
          state_ = space_before_header_value;
          return boost::indeterminate;
        }
        else if (!is_char(input) || is_ctl(input) || is_tspecial(input))
        {
          return false;
        }
        else
        {
          req.headers.back().name.push_back(input);
          return boost::indeterminate;
        }
        case space_before_header_value:
        if (input == ' ')
        {
          state_ = header_value;
          return boost::indeterminate;
        }
        else
        {
          return false;
        }
        case header_value:
        if (input == '\r')
        {
          state_ = expecting_newline_2;
          return boost::indeterminate;
        }
        else if (is_ctl(input))
        {
          return false;
        }
        else
        {
          req.headers.back().value.push_back(input);
          return boost::indeterminate;
        }
        case expecting_newline_2:
        if (input == '\n')
        {
          state_ = header_line_start;
          return boost::indeterminate;
        }
        else
        {
          return false;
        }
        case expecting_newline_3:
        return (input == '\n');
        default:
        return false;
        }
      }

      /// Check if a byte is an HTTP character.
      static bool is_char(int c)
      {
        return c >= 0 && c <= 127;
      }

      /// Check if a byte is an HTTP control character.
      static bool is_ctl(int c)
      {
        return (c >= 0 && c <= 31) || (c == 127);
      }

      /// Check if a byte is defined as an HTTP tspecial character.
      static bool is_tspecial(int c)
      {
        switch (c)
        {
        case '(': case ')': case '<': case '>': case '@':
        case ',': case ';': case ':': case '\\': case '"':
        case '/': case '[': case ']': case '?': case '=':
        case '{': case '}': case ' ': case '\t':
        return true;
        default:
        return false;
        }
      }

      /// Check if a byte is a digit.
      static bool is_digit(int c)
      {
        return c >= '0' && c <= '9';
      }

      /// The current state of the parser.
      enum state : boost::uint8_t
      {
        method_start,
        method,
        uri,
        http_version_h,
        http_version_t_1,
        http_version_t_2,
        http_version_p,
        http_version_slash,
        http_version_major_start,
        http_version_major,
        http_version_minor_start,
        http_version_minor,
        expecting_newline_1,
        header_line_start,
        header_lws,
        header_name,
        space_before_header_value,
        header_value,
        expecting_newline_2,
        expecting_newline_3
      } state_;
    };

    // A structure to hold websocket frame data. 
    class data_frame
    {
    public:
      typedef std::vector<boost::asio::const_buffer> buffers_t;

      bool fin_;
      enum operation_code : boost::uint8_t
      {
        continuation_frame, // denotes a continuation frame
        text_frame, // denotes a text frame
        binary_frame,   // denotes a binary frame
        connection_close, // denotes a connection close
        ping, //denotes a ping
        pong,   // denotes a pong
        reserved  // %x3 - 7 are reserved for further non - control frames
      } opcode_;

      bool mask_;
      boost::int8_t fin_opcode_;
      boost::int8_t mask_payload_len_;
      boost::int8_t payload_len_;
      boost::uint16_t extended_payload_len16_;
      boost::uint64_t extended_payload_len64_;
      boost::array<boost::uint8_t, 4> masking_key_;
      std::vector<boost::uint8_t> payload_;


      data_frame()
        : fin_(true)
        , opcode_(text_frame)
        , mask_(false)
        , fin_opcode_(0)
        , mask_payload_len_(0)
        , payload_len_(0)
        , extended_payload_len16_(0)
        , extended_payload_len64_(0)
      {
      }

      data_frame(const std::string& str)
        : fin_(true)
        , opcode_(text_frame)
        , mask_(false)
        , fin_opcode_(0)
        , mask_payload_len_(0)
        , payload_len_(0)
        , extended_payload_len16_(0)
        , extended_payload_len64_(0)
        , payload_(str.begin(), str.end())
      {
      }

      /// Convert the data_frame into a vector of buffers. 
      std::vector<boost::asio::const_buffer> to_buffers()
      {
        using namespace boost::asio;

        std::vector<const_buffer> buffers;

        if (fin_)
          fin_opcode_ |= 0x80;

        switch (opcode_)
        {
        case continuation_frame:    fin_opcode_ |= 0x0; break;
        case text_frame:            fin_opcode_ |= 0x1; break;
        case binary_frame:          fin_opcode_ |= 0x2; break;
        case connection_close:      fin_opcode_ |= 0x8; break;
        case ping:                  fin_opcode_ |= 0x9; break;
        case pong:                  fin_opcode_ |= 0xA; break;
        default:                    fin_opcode_ |= 0xF; break;
        }

        buffers.push_back(buffer(static_cast<const void*>(&fin_opcode_), sizeof(fin_opcode_)));

        if (payload_.size() < 126)
        {
          mask_payload_len_ = static_cast<boost::uint8_t>(payload_.size());
          buffers.push_back(buffer(static_cast<const void*>
            (&mask_payload_len_), sizeof(mask_payload_len_)));
          buffers.push_back(buffer(payload_));
        }
        else if (payload_.size() < 65537)
        {
          mask_payload_len_ = 126;
          extended_payload_len16_ = static_cast<boost::uint16_t>(payload_.size());
          extended_payload_len16_ = data_frame_parser::hton16(extended_payload_len16_);

          buffers.push_back(buffer(static_cast<const void*>
            (&mask_payload_len_), sizeof(mask_payload_len_)));
          buffers.push_back(buffer(static_cast<const void*>
            (&extended_payload_len16_), sizeof(extended_payload_len16_)));
          buffers.push_back(buffer(payload_));
        }
        else
        {
          mask_payload_len_ = 127;
          extended_payload_len64_ = payload_.size();
          extended_payload_len64_ = data_frame_parser::hton64(extended_payload_len64_);

          buffers.push_back(buffer(static_cast<const void*>
            (&mask_payload_len_), sizeof(mask_payload_len_)));
          buffers.push_back(buffer(static_cast<const void*>
            (&extended_payload_len64_), sizeof(extended_payload_len64_)));
          buffers.push_back(buffer(payload_));
        }

        return buffers;
      }
    };

    /*
    class data_frame
    {
    public:
    typedef std::vector<boost::asio::const_buffer> buffers_t;

    bool fin_;
    enum operation_code : boost::uint8_t
    {
    continuation_frame, // denotes a continuation frame
    text_frame, // denotes a text frame
    binary_frame,   // denotes a binary frame
    connection_close, // denotes a connection close
    ping, //denotes a ping
    pong,   // denotes a pong
    reserved  // %x3 - 7 are reserved for further non - control frames
    } opcode_;
    */

    /// Parser for incoming dataframes.
    class data_frame_parser
    {
    public:
      /// Construct ready to parse the data_frame.
      data_frame_parser()
        : state_(fin_opcode)
      {
      }

      /// Parse some data. The tribool return value is true when a complete data_frame
      /// has been parsed, false if the data is invalid, indeterminate when more
      /// data is required. The InputIterator return value indicates how much of the
      /// input has been consumed.
      template <typename InputIterator>
      boost::tuple<boost::tribool, InputIterator> parse(data_frame& frame,
        InputIterator begin, InputIterator end)
      {
        while (begin != end)
        {
          boost::tribool result = consume(frame, *begin++);
          if (result || !result)
            return boost::make_tuple(result, begin);
        }
        boost::tribool result = boost::indeterminate;
        return boost::make_tuple(result, begin);
      }

      // TODO use Boost.Endian
      // http://www.boost.org/doc/libs/develop/libs/endian/doc/conversion.html

      /// Convert a uint16_t from the network byte order to the host byte order.
      static boost::uint16_t ntoh16(boost::uint16_t net16)
      {
        using namespace boost;
        static const int32_t num = 42;

        // Check the endianness.
        if (*reinterpret_cast<const char*>(&num) == num)
        {
          // Convert to the little-endian.
          uint16_t host16 = ((net16 & 0x00FFULL) << 8) | ((net16 & 0xFF00ULL) >> 8);
          return host16;
        }

        return net16;
      }

      /// Convert a uint16_t from the host byte order to the network byte order.
      static boost::uint16_t hton16(boost::uint16_t host16)
      {
        using namespace boost;
        static const int32_t num = 42;

        // Check the endianness.
        if (*reinterpret_cast<const char*>(&num) == num)
        {
          // Convert to the big-endian.
          uint16_t net16 = ((host16 & 0x00FFULL) << 8) | ((host16 & 0xFF00ULL) >> 8);
          return net16;
        }

        return host16;
      }


      /// Convert a uint64_t from the network byte order to the host byte order.
      static boost::uint64_t ntoh64(boost::uint64_t net64)
      {
        using namespace boost;
        static const int32_t num = 42;

        // Check the endianness.
        if (*reinterpret_cast<const char*>(&num) == num)
        {
          // Convert to the little-endian.
          uint64_t host64 =
            ((net64 & 0x00000000000000FFULL) << 56) |
            ((net64 & 0x000000000000FF00ULL) << 40) |
            ((net64 & 0x0000000000FF0000ULL) << 24) |
            ((net64 & 0x00000000FF000000ULL) << 8) |
            ((net64 & 0x000000FF00000000ULL) >> 8) |
            ((net64 & 0x0000FF0000000000ULL) >> 24) |
            ((net64 & 0x00FF000000000000ULL) >> 40) |
            ((net64 & 0xFF00000000000000ULL) >> 56);

          return host64;
        }

        return net64;
      }

      /// Convert a uint64_t from the host byte order to the network byte order.
      static boost::uint64_t hton64(boost::uint64_t host64)
      {
        using namespace boost;
        static const int32_t num = 42;

        // Check the endianness.
        if (*reinterpret_cast<const char*>(&num) == num)
        {
          // Convert to the big-endian.
          uint64_t net64 =
            ((host64 & 0x00000000000000FFULL) << 56) |
            ((host64 & 0x000000000000FF00ULL) << 40) |
            ((host64 & 0x0000000000FF0000ULL) << 24) |
            ((host64 & 0x00000000FF000000ULL) << 8) |
            ((host64 & 0x000000FF00000000ULL) >> 8) |
            ((host64 & 0x0000FF0000000000ULL) >> 24) |
            ((host64 & 0x00FF000000000000ULL) >> 40) |
            ((host64 & 0xFF00000000000000ULL) >> 56);

          return net64;
        }

        return host64;
      }

    private:
      /// Handle the next character of input.
      boost::tribool consume(data_frame& frame, boost::uint8_t input)
      {
        switch (state_)
        {
        case fin_opcode:
        {
          frame.fin_ = get_bits(input, 7, 1) == 1;

          switch (get_bits(input, 0, 4))
          {
          case 0:
          frame.opcode_ = data_frame::continuation_frame;
          break;
          case 0x1:
          frame.opcode_ = data_frame::text_frame;
          break;
          case 0x2:
          frame.opcode_ = data_frame::binary_frame;
          break;
          case 0x8:
          frame.opcode_ = data_frame::connection_close;
          break;
          case 0x9:
          frame.opcode_ = data_frame::ping;
          break;
          case 0xA:
          frame.opcode_ = data_frame::pong;
          break;
          default:
          frame.opcode_ = data_frame::reserved;
          }

          state_ = mask_payload_len;

          return boost::indeterminate;
        }
        case mask_payload_len:
        {
          frame.mask_ = get_bits(input, 7, 1) == 1;
          frame.payload_len_ = get_bits(input, 0, 7);

          if (frame.payload_len_ == 0)
            return true;
          else if (frame.payload_len_ == 126 || frame.payload_len_ == 127)
            state_ = extended_payload_len1;
          else
          {
            if (frame.mask_)
              state_ = masking_key1;
            else
              state_ = payload;
          }

          return boost::indeterminate;
        }
        case extended_payload_len1:
        {
          if (frame.payload_len_ == 126)
            frame.extended_payload_len16_ = input;
          else if (frame.payload_len_ == 127)
            frame.extended_payload_len64_ = input;

          state_ = extended_payload_len2;

          return boost::indeterminate;
        }
        case extended_payload_len2:
        {
          if (frame.payload_len_ == 126)
          {
            boost::uint16_t temp = input;
            temp = temp << 8;
            frame.extended_payload_len16_ |= temp;
            frame.extended_payload_len16_ = ntoh16(frame.extended_payload_len16_);

            if (frame.mask_)
              state_ = masking_key1;
            else
              state_ = payload;
          }
          else if (frame.payload_len_ == 127)
          {
            boost::uint64_t temp = input;
            temp = temp << 8;
            frame.extended_payload_len64_ |= temp;

            state_ = extended_payload_len3;
          }

          return boost::indeterminate;
        }

        case extended_payload_len3:
        {
          boost::uint64_t temp = input;
          temp = temp << 16;
          frame.extended_payload_len64_ |= temp;

          state_ = extended_payload_len4;

          return boost::indeterminate;
        }

        case extended_payload_len4:
        {
          boost::uint64_t temp = input;
          temp = temp << 24;
          frame.extended_payload_len64_ |= temp;

          state_ = extended_payload_len5;

          return boost::indeterminate;
        }
        case extended_payload_len5:
        {
          boost::uint64_t temp = input;
          temp = temp << 32;
          frame.extended_payload_len64_ |= temp;

          state_ = extended_payload_len6;

          return boost::indeterminate;
        }
        case extended_payload_len6:
        {
          boost::uint64_t temp = input;
          temp = temp << 40;
          frame.extended_payload_len64_ |= temp;

          state_ = extended_payload_len7;

          return boost::indeterminate;
        }
        case extended_payload_len7:
        {
          boost::uint64_t temp = input;
          temp = temp << 48;
          frame.extended_payload_len64_ |= temp;

          state_ = extended_payload_len8;

          return boost::indeterminate;
        }
        case extended_payload_len8:
        {
          boost::uint64_t temp = input;
          temp = temp << 56;
          frame.extended_payload_len64_ |= temp;
          frame.extended_payload_len64_ = ntoh64(frame.extended_payload_len64_);

          if (frame.mask_)
            state_ = masking_key1;
          else
            state_ = payload;

          return boost::indeterminate;
        }
        case masking_key1:
        {
          frame.masking_key_[0] = input;
          state_ = masking_key2;

          return boost::indeterminate;
        }
        case masking_key2:
        {
          frame.masking_key_[1] = input;
          state_ = masking_key3;

          return boost::indeterminate;
        }
        case masking_key3:
        {
          frame.masking_key_[2] = input;
          state_ = masking_key4;

          return boost::indeterminate;
        }
        case masking_key4:
        {
          frame.masking_key_[3] = input;
          state_ = payload;

          return boost::indeterminate;
        }
        case payload:
        {
          boost::uint8_t mask = frame.masking_key_[frame.payload_.size() % 4];
          frame.payload_.push_back(input ^ mask);

          if (frame.payload_len_ == 127)
          {
            if (frame.payload_.size() == frame.extended_payload_len64_)
              return true;
            else
              return boost::indeterminate;
          }
          else if (frame.payload_len_ == 126)
          {
            if (frame.payload_.size() == frame.extended_payload_len16_)
              return true;
            else
              return boost::indeterminate;
          }
          else
          {
            if (frame.payload_.size() == static_cast<std::vector<char>::size_type>(frame.payload_len_))
              return true;
            else
              return boost::indeterminate;
          }
        }

        default:
        return false;
        }
      }

      /// Get a number of bits at the specified offset.
      boost::uint8_t get_bits(char b, boost::uint8_t offset, boost::uint8_t count)
      {
        return (b >> offset) & ((1 << count) - 1);
      }

      /// The current state of the parser.
      enum state : boost::uint8_t
      {
        fin_opcode,
        mask_payload_len,
        extended_payload_len1,
        extended_payload_len2,
        extended_payload_len3,
        extended_payload_len4,
        extended_payload_len5,
        extended_payload_len6,
        extended_payload_len7,
        extended_payload_len8,
        masking_key1,
        masking_key2,
        masking_key3,
        masking_key4,
        payload
      } state_;
    };

  };

  /// The http reply to be sent back to the client.
  typedef rfc6455_engine::reply                       http_reply_t;
  typedef boost::shared_ptr<http_reply_t>             http_reply_ptr;
  typedef boost::shared_ptr<const http_reply_t>       c_http_reply_ptr;

  /// The parser for the incoming messages.
  typedef rfc6455_engine::data_frame_parser           frame_parser_t;
  typedef boost::shared_ptr<frame_parser_t>           frame_parser_ptr;
  typedef boost::shared_ptr<const frame_parser_t>     c_frame_parser_ptr;

  /// The incoming http request.
  typedef rfc6455_engine::request                     http_request_t;
  typedef rfc6455_engine::request                     request;
  typedef rfc6455_engine::request_handler             request_handler;
  typedef rfc6455_engine::request_parser              request_parser;

  typedef rfc6455_engine::data_frame                  data_frame;
  typedef boost::shared_ptr<data_frame>               data_frame_ptr;
  typedef boost::shared_ptr<const data_frame>         c_data_frame_ptr;

} // namespace splice {

template<> inline
std::string boost::lexical_cast<std::string>(const splice::rfc6455_engine::reply::status_type& arg)
{
  using namespace splice;

  switch (arg)
  {
  case rfc6455_engine::reply::switching_protocols:
  return "switching_protocols";
  case rfc6455_engine::reply::bad_request:
  return "bad_request";
  case rfc6455_engine::reply::internal_server_error:
  return "internal_server_error";
  default:
  return "? " + lexical_cast<std::string>((int)arg);
  };
}

template<> inline
std::string boost::lexical_cast<std::string>
  (const splice::rfc6455_engine::data_frame::operation_code& arg)
{
  using namespace splice;

  switch (arg)
  {
  case rfc6455_engine::data_frame::continuation_frame:
  return "continuation_frame";
  case rfc6455_engine::data_frame::text_frame:
  return "text_frame";
  case rfc6455_engine::data_frame::binary_frame:
  return "binary_frame";
  case rfc6455_engine::data_frame::connection_close:
  return "connection_close";
  case rfc6455_engine::data_frame::ping:
  return "ping";
  case rfc6455_engine::data_frame::pong:
  return "pong";
  default:
  return "? " + lexical_cast<std::string>((int)arg);
  }

} //namespace splice

#endif // #ifndef WEBSOCKET_ENGINE_HPP
