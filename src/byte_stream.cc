#include "byte_stream.hh"
#include <string>

using namespace std;

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ) {}

bool Writer::is_closed() const
{
  // Your code here.
  return closed_;
}

void Writer::push( string data )
{
  // Your code here.
  if ( Writer::is_closed() or Writer::available_capacity() == 0 or data.empty() ) {
    return;
  }

  if ( data.size() > Writer::available_capacity() ) {
    data.resize( Writer::available_capacity() );
  }

  total_pushed_ += data.size();
  total_buffered_ += data.size();
  Writer::stream_.emplace( std::move(data) );

  return;
}

void Writer::close()
{
  // Your code here.
//   while(!stream_.empty()) stream_.pop();
//   total_buffered_ = 0;
  Writer::closed_ = true;
}

uint64_t Writer::available_capacity() const
{
  // Your code here.
  return Writer::capacity_ - total_buffered_;
}

uint64_t Writer::bytes_pushed() const
{
  // Your code here.
  return Writer::total_pushed_;
}

bool Reader::is_finished() const
{
  // Your code here.
  return closed_ and total_buffered_ == 0;
}

uint64_t Reader::bytes_popped() const
{
  // Your code here.
  return total_poped_;
}

string_view Reader::peek() const
{
  // Your code here.
  return Reader::stream_.empty() ? std::string_view {}
                                 : std::string_view { Reader::stream_.front() }.substr( remove_size_ );
}

void Reader::pop( uint64_t len )
{
  // Your code here.
  //   (void)len;
  while ( !Reader::stream_.empty() and len ) {
    const uint64_t size { stream_.front().size() - remove_size_ };
    if ( len < size ) {
      remove_size_ += len;
	  total_poped_ += len;
	  total_buffered_ -= len;
      break;
    }

    remove_size_ = 0;
    len -= size;
	total_poped_ += size;
	total_buffered_ -= size;

    stream_.pop();
  }
}

uint64_t Reader::bytes_buffered() const
{
  // Your code here.
  return total_buffered_;
}
