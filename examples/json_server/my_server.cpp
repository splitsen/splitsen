#include "my_server.hpp"

using namespace std;
using namespace splice;

my_server::my_server(const string& address, const string& port)
    :base_t(address, port)
{
    start_accept<my_session>();
}

my_server::session_ptr my_server::construct_session()
{
    return boost::make_shared<my_session>(get_io_service());
}

void my_server::on_run(size_t thread_count)
{
    cout << "Server working with " << thread_count << " thread(s)" << endl;
    cout << "Press Ctrl+C (Ctrl+Break) to exit." << endl;
}

void my_server::on_error(const boost::system::error_code& ec)
{
    cerr << "Error:" << ec.message();
}
