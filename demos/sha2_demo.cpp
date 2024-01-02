#include "../mitm_sequential.hpp"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <array>

/* We would like to call C function defined in `sha256.c` */
extern "C"{
 void sha256_process(uint32_t state[8], const uint8_t data[], uint32_t length);
 }




/* More aesthitically pleasing than uintXY_t */
using u8  = uint8_t ;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using i8  = int8_t ;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

#define NWORDS_DIGEST 8
#define WORD_SIZE 4 // bytes
#define NBYTES_DIGEST (NWORDS_DIGEST * WORD_SIZE)
/*
 * repr is an encapsulation of whatever data used.
 * It should have
 * I : Constructor
 * II: Destructor
 */
struct SHA2_out_repr {
  u32* state; /* output */

  /* Constructor */
  SHA2_out_repr() : state{new u32[NWORDS_DIGEST]} { }
  /* It could've been written as:
   * SHA2_out_repr(){ state = new u32[NBYTEST_DIGEST]; }
   */
  ~SHA2_out_repr()
  {
    delete[] state; 
  }
};

struct SHA2_inp_repr {
  u8* data; /* output */

  /* Constructor */
  SHA2_inp_repr() : data{new u8[64]} {}
  /* It could've been written as:
   * SHA2_inp_repr(){ state = new u8[64]; }
   */
  ~SHA2_inp_repr()
  {
    delete[] data;
  }
};



/*
 * Implement functions related to inp/out as specified in AbstractDomain
 */
class SHA2_OUT_DOMAIN : mitm::AbstractDomain<SHA2_out_repr>
{
public:
  // Not only a shortcut, it's necessry to avoid error when specializing a class
  // In template: 't' is a private member of 'mitm::AbstractDomain<SHA2_out_repr>'
  using t = SHA2_out_repr;
  
  const static int length = NWORDS_DIGEST;
  const static size_t n_elements = (1LL<<32)<<length;
  /* todo: randomize */
  void randomize(t& x, mitm::PRNG& prng) const
  {
    u32 data[8]; /* store 256 bits of random data  */
    data[0] = prng.rand();
    data[1] = prng.rand();
    data[2] = prng.rand();
    data[3] = prng.rand();
    data[4] = prng.rand();
    data[5] = prng.rand();
    data[6] = prng.rand();
    data[7] = prng.rand();

    std::memcpy(x.state, data, 32);
  }

  
  inline
  bool is_equal(t& x, t& y) const
  {
    return (0 == std::memcmp(x.state, y.state, NBYTES_DIGEST));
  }

  inline
  void serialize(const t& in, u8* out) const
  {
    std::memcpy(out, in.state, NBYTES_DIGEST);
  }

  inline
  void unserialize(const u8* in, t& out) const
  {
    std::memcpy(out.state, in, NBYTES_DIGEST);
  }


  inline
  void copy(const t& in, t& out)
  {
    std::memcpy(out.state, in.state, NBYTES_DIGEST);
  }

  inline
  int extract_1_bit(const t& inp) const
  {
    return (1&inp.state[0]);
  }

  inline
  u64 hash(const t& x) const
  {
    /* in case we are only extracting one word digest */
    constexpr size_t _2nd_idx = std::min(NWORDS_DIGEST, 1);

    return (static_cast<u64>(x.state[_2nd_idx]) << WORD_SIZE) | x.state[0];
  }

  
};


/*
 *
 */

class SHA2_INP_DOMAIN : mitm::AbstractDomain<SHA2_inp_repr>
{
public:
  const static int length = 64;
  const static size_t n_elements = -1;
  using t = SHA2_inp_repr; /* */

  void randomize(t& x, mitm::PRNG& prng) const
  {
    u64 data[8]; /* store 512 bits of random data  */
    data[0] = prng.rand();
    data[1] = prng.rand();
    data[2] = prng.rand();
    data[3] = prng.rand();
    data[4] = prng.rand();
    data[5] = prng.rand();
    data[6] = prng.rand();
    data[7] = prng.rand();

    std::memcpy(x.data, data, 64);
  }

  
  inline
  bool is_equal(t& x, t& y) const
  {
    return (0 == std::memcmp(x.data, y.data, 64));
  }

  inline
  void serialize(const t& in, u8* out) const
  {
    std::memcpy(out, in.data, 64);
  }

  inline
  void unserialize(const u8* in, t& out) const
  {
    std::memcpy(out.data, in, 64);
  }


  inline
  void copy(const t& in, t& out)
  {
    std::memcpy(out.data, in.data, 64);
  }

  inline
  int extract_1_bit(const t& inp) const
  {
    return (1&inp.data[0]);
  }
  
  inline
  u64 hash(const t& x) const
  {
    /* in case we are only extracting one word digest */
    return (static_cast<u64>(x.data[0]) << 0 * 8) |
           (static_cast<u64>(x.data[1]) << 1 * 8) |    
           (static_cast<u64>(x.data[2]) << 2 * 8) |    
           (static_cast<u64>(x.data[3]) << 3 * 8) |
           (static_cast<u64>(x.data[4]) << 4 * 8) |
           (static_cast<u64>(x.data[5]) << 5 * 8) |
           (static_cast<u64>(x.data[6]) << 6 * 8) |
           (static_cast<u64>(x.data[7]) << 7 * 8) ;
  }
};

// sha256_process(uint32_t state[8], const uint8_t data[], uint32_t length);

class SHA2_Problem
    : mitm::AbstractProblem<SHA2_INP_DOMAIN, SHA2_INP_DOMAIN, SHA2_OUT_DOMAIN>
{
public:
  using C_t = SHA2_out_repr;
  /* It's a special case that B_t = A_t */
  using B_t = SHA2_inp_repr;
  using A_t = SHA2_inp_repr;

  inline
  void f(const A_t& x, C_t& y) const
  {
    u32 state[8] = { 0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
		     0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};
    sha256_process(state, x.data, 64);
    std::memcpy(y.state, state, NBYTES_DIGEST);
  }

  inline /* In this case it's same as f  */
  void g(const B_t& x, C_t& y) const  { f(x, y); }

  inline
  void send_C_to_A(C_t& inp_C, A_t& out_A) const
  {
    /* remove any junk in A */
    std::memset(out_A.data, embedding_n, 64);
    std::memcpy(out_A.data, inp_C.state, NBYTES_DIGEST);
  }

  inline void send_C_to_B(C_t& inp_C, B_t& out_B) const
  { send_C_to_A(inp_C, out_B); }

  /* change */
  void update_embedding(mitm::PRNG& rng) { ++embedding_n; }

private:
  /* Changes the extra bits in the input to embedding_n */
  int embedding_n = 0;
  
  
};


int main(int argc, char* argv[])
{
  SHA2_Problem Pb;
  mitm::collision<SHA2_Problem>(Pb);
}
