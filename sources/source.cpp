// Copyright 2020 by CrestoniX

#include <functional>
#include <random>
#include <chrono>
#include "header.hpp"
#include <boost/bind/bind.hpp>
#include <ctime>
boost::asio::io_context IoContext;


class talk_to_svr {
 public:
  explicit talk_to_svr(const std::string& username)
      : sock_(IoContext), started_(true), username_(username) {}
  void connect(boost::asio::ip::tcp::endpoint ep) { sock_.connect(ep); }

  void write(const std::string& msg)
  { sock_.write_some(boost::asio::buffer(msg)); }

  void do_ask_clients()
  {
    write("ask_clients\n");
    read_answer();
  }


  void on_login() {
    do_ask_clients();
  }
  void on_ping(const std::string & msg)
  {
    std::istringstream in(msg);
    std::string answer;
    in >> answer >> answer;
    if ( answer == "client_list_changed")
      do_ask_clients();
  }
  void on_clients(const std::string & msg)
  {
    std::string clients = msg.substr(8);
    std::cout << username_ << ", new client list:" << clients;
  }

  void write_request()
  {
      write("ping\n");
  }
  void process_msg()
  {
    std::cout << buff_ << std::endl;
    std::string msg = buff_;
    if ( msg.find("login ") == 0) on_login();
    else if ( msg.find("ping") == 0) on_ping(msg);
    else if ( msg.find("clients ") == 0) on_clients(msg);
    else
      std::cout << "invalid msg " << msg << std::endl;
  }
  size_t read_complete(const boost::system::error_code & err, size_t bytes) {
    if (err) return 0;
    bool found = std::find(buff_, buff_ + bytes, '\n') <
                 buff_ + bytes;
    return found ? 0 : 1;
  }

  void read_answer()
  {
    already_read_ = 0;
    read(sock_, boost::asio::buffer(buff_),
         boost::bind(&talk_to_svr::read_complete,
                     this, boost::placeholders::_1, boost::placeholders::_2));
    process_msg();
  }

  void loop()
  {
    srand(time(NULL));
    write("login " + username_ + "\n");
    read_answer();
    while ( started_)
    {
      write_request();
      read_answer();
      sleep(rand_r(NULL)%7+1);
    }
  }
  std::string username() const { return username_; }

 private:
  boost::asio::ip::tcp::socket sock_;
  enum { max_msg = 1024 };
  int already_read_;
  char buff_[1024];
  bool started_;
  std::string username_;
};



int main() {
  boost::asio::ip::tcp::endpoint ep(
      boost::asio::ip::address::from_string("127.0.0.1"), 60013);
  std::string client_name;
  std::cin >> client_name;
    talk_to_svr client(client_name);
    try
    {
      client.connect(ep);
      client.loop();
    }
    catch(boost::system::system_error & err)
    {
      std::cout << "client terminated " << std::endl;
    }
  }



