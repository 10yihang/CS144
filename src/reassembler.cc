#include "reassembler.hh"
#include "byte_stream.hh"
#include <algorithm>

using namespace std;

auto Reassembler::find( uint64_t pos ) noexcept
{
  auto it { buf_.lower_bound( pos ) };
  if ( it != buf_.end() and it->first == pos ) {
    return it;
  }
  if ( it == buf_.begin() ) {
    return it;
  }
  if ( const auto pit { prev( it ) }; pit->first + pit->second.length() > pos ) {
    // const auto res = buf_.emplace(pos, pit->second.substr(pos - pit->first));
    const auto res = buf_.emplace_hint( it, pos, pit->second.substr( pos - pit->first ) );
    pit->second.resize( pos - pit->first );
    return res;
    // return res.first;
  }
  return it;
}

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring )
{
  // Your code here.
  auto try_close = [&]() noexcept -> void {
    if ( is_end and end_index == writer().bytes_pushed() and buf_.empty() ) {
      output_.writer().close();
    }
  };

  if ( data.empty() ) {
    if ( !is_end and is_last_substring ) {
      is_end = true;
      end_index = first_index;
    }
    return try_close();
  }

  if ( writer().is_closed() or writer().available_capacity() == 0 )
    return;

  const uint64_t unassemble_index { writer().bytes_pushed() };
  const uint64_t unacceptable_index { unassemble_index + writer().available_capacity() };
  if ( first_index >= unacceptable_index or first_index + data.length() <= unassemble_index )
    return;

  if ( first_index + data.length() > unacceptable_index ) {
    data.resize( unacceptable_index - first_index );
    is_last_substring = false;
  }

  if ( first_index < unassemble_index ) {
    const uint64_t offset { unassemble_index - first_index };
    data = data.substr( offset );
    first_index = unassemble_index;
  }

  if ( !is_end and is_last_substring ) {
    is_end = true;
    end_index = first_index + data.length();
  }

  const auto upper { find( first_index + data.length() ) };
  const auto lower { find( first_index ) };
  std::ranges::for_each( lower, upper, [&]( auto& p ) noexcept { total_pending_ -= p.second.length(); } );
  total_pending_ += data.length();
  buf_.erase( lower, upper );
  buf_.emplace( first_index, move( data ) );

  while ( !buf_.empty() ) {
    auto &&[index, str] { *buf_.begin() };
    if ( index != writer().bytes_pushed() )
      break;

    total_pending_ -= str.length();
    output_.writer().push( { std::move( str ) } );
    buf_.erase( buf_.begin() );
  }
  return try_close();
}

uint64_t Reassembler::bytes_pending() const
{
  // Your code here.
  return Reassembler::total_pending_;
}
