#include <splice/http/http_session.hpp>

#include <utility>
#include <set>

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

#include "my_logger.hpp"

// serves index.html
class my_http_session:public splice::http_session<my_http_session,my_logger>
{
public:
  using base_t=splice::http_session<my_http_session,my_logger>;

  // Copy required files from src folder to dest.
  // Setup host name and port number for html files
  static std::pair<boost::system::error_code,bool> html_preprocess
    (const std::string& src
    ,const std::string& dest
    ,const std::string& value_of_hostName
    ,const std::string& value_of_portNumber
    )
  {
    using namespace std;
    using namespace boost::filesystem;

    boost::system::error_code ec;

    if((!exists(src,ec)||!is_directory(src,ec))
      ||(!exists(dest,ec)||!is_directory(dest,ec)))
      return make_pair(ec,false);

    if(equivalent(path(src),path(dest),ec))
      return make_pair(ec,false);

    const std::set<path> required=
    {"index.html"
    ,"json_client_multiprotocol.html"
    ,"admin_dashboard.html"
    ,"splice_top_horz.png"
    };

    for(auto& it:required)
    {
      if(!is_regular_file(path(src)/it,ec)||ec)
        return make_pair(ec,false);
    }

    for(auto& it:required)
    {
      path from=path(src)/it;
      path to=path(dest)/it;

      copy_file(from,to,copy_option::overwrite_if_exists,ec);
      if(ec)
        return make_pair(ec,false);

      if(it.extension()!=".html")
        continue;

      auto size=file_size(to,ec);
      if(ec)
        return make_pair(ec,false);

      // setup host name and port number
      std::string buffer(boost::numeric_cast<size_t>(size),0);
      {
        boost::filesystem::ifstream(to).read(&buffer[0],size);
      }

      const pair<const string,const string> fr[]=
      {make_pair("value_of_hostName"
      ,string("\"")+value_of_hostName+string("\""))
      ,make_pair("value_of_portNumber",value_of_portNumber)
      };

      for(const auto &it:fr) // access by const reference
      {
        auto i=buffer.find(it.first);
        if(i==string::npos)
          return make_pair(ec,false);
        buffer.replace(i,it.first.length(),it.second.c_str());
      }
      boost::filesystem::ofstream(to).write
        (buffer.c_str(),buffer.length());
    }

    return make_pair(ec,true);
  }

  my_http_session(splice::socket_t& socket
    ,http::server::request_handler& request_handler)
    :base_t(socket,request_handler)
  {
  }

protected:
  friend class my_server;
  friend base_t;
};

