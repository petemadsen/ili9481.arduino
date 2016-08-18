#ifndef BITSTREAM_H
#define BITSTREAM_H


class BitStream
{
public:
  void set_addr(const uint8_t* addr)
  {
    p = (uint8_t*)addr;
    mask = 0;
  }

  bool next()
  {
    if(mask == 0) {
      data = pgm_read_byte(p++);
      mask = 0x80;
    }
    bool b = data & mask;
    mask >>= 1;
    
    return b;
  }

private:
  uint8_t* p; // pointer to current byte in PROGMEM
  uint8_t mask;
  uint8_t data;
};


#endif
