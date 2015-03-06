#include "my_logger.hpp"
#include <boost/thread.hpp>

boost::mutex my_logger::mtx_;
std::ofstream my_logger::ofs_("my_logger.log",std::ofstream::out);

void my_logger::track(char severity,const char* file,unsigned line,
  const char* func,void* obj,const std::string& msg)
{
  using namespace std;

  auto ffn=file_func_name(severity,file,line,func,obj);
  ffn+=" "+msg;

  stringstream ss;
  ss<<boost::this_thread::get_id()<<"|"<<ffn<<endl;

  BOOST_ASSERT(!signal_.empty());
  signal_(ss.str());

  boost::lock_guard<boost::mutex> lck(mtx_);
  ofs_<<ss.str();
}
