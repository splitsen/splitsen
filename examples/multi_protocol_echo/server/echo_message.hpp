
//          Copyright Jean Davy 2014-2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/property_tree/json_parser.hpp>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/local_time/local_time.hpp>

#include <boost/serialization/string.hpp>
#include <boost/serialization/shared_ptr.hpp>


// http://www.boost.org/doc/libs/1_57_0/doc/html/date_time/serialization.html
#include <boost/date_time/posix_time/time_serialize.hpp>

#include <boost/serialization/base_object.hpp>
#include <boost/serialization/assume_abstract.hpp>


// used by my_serialization_session and my_ws_session
enum class message_identification:int8_t
{
  // Each message id starts with client or server,
  // this starting word means the emitter.
  first, // do not use, insert new values after this one

  client_echo_timed,
  server_echo_timed,

  last // do not use, insert new values before this one
};


template<> inline
std::string boost::lexical_cast<std::string>(
  const message_identification& id)
{
  switch(id)
  {
  case message_identification::first:
    return std::string("first");
  case message_identification::client_echo_timed:
    return std::string("client_echo_timed");
  case message_identification::server_echo_timed:
    return std::string("server_echo_timed");
  case message_identification::last:
    return std::string("last");
  }

  std::stringstream ss;
  ss<<"unknown:"<<(int)id;
  return ss.str();
}

struct client_server_protocol
{
  static message_identification get_id(boost::property_tree::ptree& pt)
  {
    BOOST_STATIC_ASSERT(sizeof(message_identification)==sizeof(int8_t));
    auto tmp=pt.get<int8_t>("id");
    BOOST_ASSERT(static_cast<int8_t>(message_identification::first)<tmp
      && tmp<static_cast<int8_t>(message_identification::last));

    return static_cast<message_identification>(tmp);
  }

  virtual message_identification get_id() const=0;

  // number of milliseconds since midnight January 1, 1970
  // is similar to jscript Date.UTC(    
  // http://www.w3schools.com/jsref/jsref_utc.asp
  int64_t UTC() const //Coordinated Universal Time 
  {
    using namespace boost::posix_time;
    using namespace boost::gregorian;
    ptime now=microsec_clock::universal_time();
    time_duration remaining=now-ptime(date(1970,Jan,1));
    return remaining.total_milliseconds();
  }

  template<class Archive>
  void serialize(Archive& ar,const unsigned int /*version*/)
  {
    // id_ is not serialized, it is setted by derived class on constructor
  }
};
BOOST_SERIALIZATION_ASSUME_ABSTRACT(client_server_protocol)

struct client_echo_timed_t : client_server_protocol
{
  std::string msg_;
  int64_t sent_;

  virtual message_identification get_id() const
  {
    return message_identification::client_echo_timed;
  }

  client_echo_timed_t(boost::property_tree::ptree& pt)
    :msg_(pt.get<std::string>("msg_"))
    ,sent_(pt.get<int64_t>("sent_"))
  {
  }

  client_echo_timed_t()
    :sent_(UTC())
  {
  }

  template<class Archive>
  void serialize(Archive& ar,const unsigned int /*version*/)
  {
    ar & boost::serialization::base_object<client_server_protocol>(*this);
    ar & msg_ & sent_;
  }
};

struct server_echo_timed_t : client_server_protocol
{
  std::string msg_;
  int64_t client_sent_;
  int64_t server_sent_;

  virtual message_identification get_id() const
  {
    return message_identification::server_echo_timed;
  }

  server_echo_timed_t()
  {}

  server_echo_timed_t(const client_echo_timed_t& cli)
    :msg_(cli.msg_)
    ,client_sent_(cli.sent_)
    ,server_sent_(UTC())
  {
  }

  std::string json() const
  {
    boost::property_tree::ptree pt;

    pt.add("id",static_cast<int8_t>(get_id()));
    pt.add("client_sent_",client_sent_);
    pt.add("msg_",msg_);
    // conversion to be unserstandable to jscript 
    pt.add("server_sent_",server_sent_);

    std::stringstream ss;
    write_json(ss,pt);
    return ss.str();
  }

  template<class Archive>
  void serialize(Archive& ar,const unsigned int /*version*/)
  {
    ar & boost::serialization::base_object<client_server_protocol>(*this);
    ar & msg_ & client_sent_ & server_sent_;
  }
};

