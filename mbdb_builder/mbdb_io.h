#ifndef MBDB_READ_H_
# define MBDB_READ_H_

# include <arpa/inet.h>

# include <cstdint>
# include <string>
# include <vector>

template<typename T>
static void read(const char*& addr, T& output)
{
  output = *((const T*) addr);
  addr += sizeof (T);
}

template<>
void read<uint16_t>(const char*& addr, uint16_t& output)
{
  output = ntohs(*((uint16_t*) addr));
  addr += sizeof (uint16_t);
}

template<>
void read<uint32_t>(const char*& addr, uint32_t& output)
{
  output = ntohl(*((uint32_t*) addr));
  addr += sizeof (uint32_t);
}

template<>
void read<uint64_t>(const char*& addr, uint64_t& output)
{
  uint32_t tmp[2];

  read<uint32_t>(addr, tmp[0]);
  read<uint32_t>(addr, tmp[1]);
  output = ((uint64_t) tmp[0]) << 32 | tmp[1];
}

template<>
void read<std::string>(const char*& addr, std::string& output)
{
  uint16_t len;

  read<uint16_t>(addr, len);

  if (len == 0xffff)
    len = 0;

  output.assign(addr, len);

  addr += len;
}

template<>
void read<std::vector<uint8_t>>(const char*& addr, std::vector<uint8_t>& output)
{
  uint16_t len;

  read<uint16_t>(addr, len);

  if (len == 0xffff)
    len = 0;

  output.assign(addr, addr + len);

  addr += len;
}

// --------------------------------------------------------------------------------
template<typename T>
static void write(char*& addr, const T input)
{
    *((T*) addr) = input;
    addr += sizeof (T);
}

template<>
void write<uint16_t>(char*& addr, const uint16_t input)
{
    *((uint16_t*) addr) = htons(input);
    addr += sizeof (uint16_t);
}

template<>
void write<uint32_t>(char*& addr, const uint32_t input)
{
    *((uint32_t*) addr) = htonl(input);
    addr += sizeof (uint32_t);
}

template<>
void write<uint64_t>(char*& addr, const uint64_t input)
{
    uint32_t tmp[2];
    tmp[0] = (uint32_t)(input >> 32);
    tmp[1] = (uint32_t)input;
    write<uint32_t>(addr, tmp[0]);
    write<uint32_t>(addr, tmp[1]);
}

template<>
void write<std::string>(char*& addr, const std::string input)
{
    uint16_t len = input.length();
    write<uint16_t>(addr, len);
    memcpy(addr, input.data(), len);
    addr += len;
}

template<>
void write<std::vector<uint8_t>>(char*& addr, const std::vector<uint8_t> input)
{
    uint16_t len = input.size();
    write<uint16_t>(addr, len);
    memcpy(addr, input.data(), len);
    addr += len;
}

// --------------------------------------------------------------------------------
template<typename T>
static void skip(char*& addr)
{
    T tmp;
    const char* raddr = addr;
    read<T>(raddr, tmp);
    addr = (char *)raddr;
}
#endif /* !MBDB_READ_H_ */
