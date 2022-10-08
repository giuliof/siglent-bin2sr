#ifndef STREAM_HPP_
#define STREAM_HPP_

template<typename T, class S>
T deserialize(S& stream)
{
  T ret;
  stream.read((char*)&ret, sizeof(T));
  return ret;
}

#endif // STREAM_HPP_