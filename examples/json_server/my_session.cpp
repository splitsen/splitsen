#include "my_session.hpp"

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/exceptions.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/local_time/local_time.hpp>
#include <boost/filesystem.hpp>

#include "protocol_messages.hpp"

using namespace std;
using namespace splice;

my_session::my_session(boost::asio::io_service& io_service)
  : base_t(io_service)
{
}

string my_session::on_message(const msg_question_echo_timed& msg)
{
  msg_response_echo_timed send(msg);

  using namespace boost::posix_time;
  using namespace boost::gregorian;
  ptime now=microsec_clock::universal_time();
  time_duration remaining=now-ptime(date(1970,Jan,1));
  send.server_received=remaining.total_milliseconds();

  return send.json();
}

string my_session::on_message(const msg_question_files_current_directory& json_msg)
{
  msg_response_files_current_directory response(json_msg);

  using namespace boost::filesystem;
  path cp=current_path();

  directory_iterator end;
  for(directory_iterator it(cp); it!=end; ++it)
  {
    if(!is_regular_file(it->status()))
      continue;
    string file=path(it->path()).make_preferred().string();
    if(file.length()>json_msg.max_length)
    {
      // shorten path to max_length inserting "..." in middle
      auto midLeft=file.begin();
      advance(midLeft,file.length()/2);
      auto midRight=midLeft;
      advance(midLeft,json_msg.max_length/2-distance(file.begin(),midLeft)-2);
      string left(file.begin(),midLeft);

      advance(midRight,distance(midRight,file.end())-json_msg.max_length/2+1);
      string right(midRight,file.end());

      file=left+"..."+right;
    }
    response.files.push_back(file);
  }

  return response.json();
}

void my_session::on_read(const string& json_msg)
{
  std::cout<<"Handle incoming message:"<<json_msg<<std::endl<<std::endl;

  using namespace boost::property_tree;

  try
  {
    ptree pt;
    {
      stringstream ss(json_msg);
      read_json(ss,pt);
    }
    string answer;

    const unsigned char msgId=pt.get<unsigned char>("id");
    switch(msgId)
    {
    case message_identification::question_echo_timed:
      answer=on_message(msg_question_echo_timed(pt));
      break;
    case message_identification::question_files_current_directory:
      answer=on_message(msg_question_files_current_directory(pt));
      break;
    default:
      cerr<<"unknown protocol message id:"<<msgId<<endl;
      // Here is a choice, if you want or not to continue speaking
      // with a client sending wrong message(s).
      // get_io_service().stop(); // kindly kill server
      // shut_down(); // close the socket
      return; // ignore that wrong message
    }

    async_write(answer);
  }
  catch(ptree_error& err)
  {
    // same issue handling that in default case above
    cerr<<__FILE__<<"@"<<__LINE__<<":"<<
      "property tree exception:"<<err.what()<<endl;
  }
}

void my_session::on_connect()
{
  cout<<"new client session "<<hex<<this<<endl;
}

void my_session::on_shutdown(
  const boost::system::error_code& shut_down_ec,
  const boost::system::error_code& close_ec)
{
  base_t::on_shutdown(shut_down_ec,close_ec);
  cout<<"shutdown session "<<hex<<this<<endl;
}
