
//          Copyright Jean Davy 2014-2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include "echo_message.hpp"

#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT_GUID(client_echo_timed_t,"1")
BOOST_CLASS_EXPORT_GUID(server_echo_timed_t,"2")

void serialization_test()
{
  try
  {
    client_echo_timed_t msg;
    msg.msg_=std::string("coucou");

    boost::shared_ptr<client_server_protocol> msg_w=boost::make_shared<client_echo_timed_t>(msg);

    std::string persist;
    {
      std::ostringstream archive_stream;
      {
        boost::archive::text_oarchive archive(archive_stream);
        archive<<msg_w;
      }
      persist=archive_stream.str();
    }

    auto count_of_bytes=persist.length();

    boost::shared_ptr<client_server_protocol> msg_r;
    {
      std::istringstream archive_stream(persist);
      boost::archive::text_iarchive archive(archive_stream);
      archive>>msg_r;
    }

    bool rc=dynamic_cast<client_echo_timed_t&>(*msg_w).msg_==
      dynamic_cast<client_echo_timed_t&>(*msg_r).msg_;

    BOOST_ASSERT(rc);
  }
  catch(std::exception& e)
  {
    std::cerr<<"Exception: "<<e.what()<<"\n";
    BOOST_ASSERT(false);
  }
}


