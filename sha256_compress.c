#include "sha2_compress.h"

/*
 * As SHA2_ROUND is only defining the new elements and the rest is being
 * handled outside the macro, the new elements that should be 'a' and 'e' are
 * actually the previous ones.
 *
 * This approach avoids moving registers around. The drawback is that it'll
 * generate a big code due to different registers being used.
 */

#define SHA2_ROUND(_a, _b, _c, _d, _e, _f, _g, _h, _kplusw) do {            \
    vector_base_type ch;                                                    \
    vector_base_type maj;                                                   \
    vector_base_type bsa;                                                   \
    vector_base_type bse;                                                   \
    vector_base_type vt1;                                                   \
    vector_base_type vt2;                                                   \
    vector_base_type vt3;                                                   \
    vector_base_type vt4;                                                   \
  __asm__ volatile (                                                        \
      "vsel %[ch],%[g],%[f],%[e]\n\t"       /* ch  = Ch(e,f,g)           */ \
      "vxor %[maj],%[a],%[b]\n\t"           /* intermediate Maj          */ \
      "vshasigmaw %[bse],%[e],1,0xf\n\t"    /* bse = BigSigma1(e)        */ \
      "vadduwm %[vt2],%[ch],%[kpw]\n\t"     /* vt2 = ch + kpw            */ \
      "vadduwm %[vt1],%[h],%[bse]\n\t"      /* vt1 = h + bse             */ \
      "vsel %[maj],%[b],%[c],%[maj]\n\t"    /* maj = Maj(a,b,c)          */ \
      "vadduwm %[vt3],%[vt1],%[vt2]\n\t"    /* vt3 = h + bse + ch + kpw  */ \
      "vshasigmaw %[bsa],%[a],1,0\n\t"      /* bsa = BigSigma0(a)        */ \
      "vadduwm %[vt4],%[bsa],%[maj]\n\t"    /* vt4 = bsa + maj           */ \
      "vadduwm %[d],%[d],%[vt3]\n\t"        /* d   = d + vt3             */ \
      "vadduwm %[h],%[vt3],%[vt4]\n\t"      /* h   = vt3 + vt4           */ \
    : /* output list                                                     */ \
      /* temporaries                                                     */ \
      [ch] "=&v" (ch),                                                      \
      [maj] "=&v" (maj),                                                    \
      [bsa] "=&v" (bsa),                                                    \
      [bse] "=&v" (bse),                                                    \
      [vt3] "=&v" (vt3),                                                    \
      [vt4] "=&v" (vt4),                                                    \
      [vt1] "=&v" (vt1),                                                    \
      [vt2] "=&v" (vt2),                                                    \
      /* outputs/inputs                                                  */ \
      [d] "+v" ((_d)),                                                      \
      [h] "+v" ((_h))                                                       \
    : /* input list                                                      */ \
      [a] "v" ((_a)),                                                       \
      [b] "v" ((_b)),                                                       \
      [c] "v" ((_c)),                                                       \
      [e] "v" ((_e)),                                                       \
      [f] "v" ((_f)),                                                       \
      [g] "v" ((_g)),                                                       \
      [kpw] "v" ((_kplusw))                                                 \
    : /* clobber list                                                    */ \
                                                                            \
  ); } while (0)

#define LOAD_H_VEC(_a, _e, _hptr, _vrb) do {                                \
  base_type index;                                                          \
  vector_base_type tmp1;                                                    \
  __asm__ volatile (                                                        \
     "lvx %[a],0,%[hptr]\n\t"             /* load unaligned              */ \
     "addi %[idx],%[hptr],16\n\t"                                           \
     "lvsr %[vrb],0,%[idx]\n\t"                                             \
     "lvx %[e],0,%[idx]\n\t"              /* load unaligned              */ \
     "vperm %[a],%[e],%[a],%[vrb]\n\t"    /* a = {a,b,c,d}               */ \
     "addi %[idx],%[idx],16\n\t"                                            \
     "lvx %[tmp1],0,%[idx]\n\t"           /* load unaligned              */ \
     "vperm %[e],%[tmp1],%[e],%[vrb]\n\t" /* e = {e,f,g,h}               */ \
   : /* output list                                                      */ \
     /* temporaries                                                      */ \
     [idx] "=&b" (index),                                                   \
     /* actual outputs                                                   */ \
     /* a and e are read in the asm, hence should be reserved            */ \
     [a] "=&v" ((_a)),                                                      \
     [e] "=&v" ((_e)),                                                      \
     [tmp1] "=&v" ((tmp1)),                                                 \
     [vrb] "=&v" ((_vrb))                                                   \
   : /* input list                                                       */ \
     [hptr] "b" ((_hptr))                                                   \
   : /* clobber list                                                     */ \
     "memory"                                                               \
  ); } while (0)                                                            \

#define DEQUE(_k, _kpw1, _kpw2, _kpw3) do {                          \
  __asm__ volatile (                                                        \
    "vsldoi %[kpw1],%[k],%[k],12\n\t"        /* kpw1 = {k[1],...}        */ \
    "vsldoi %[kpw2],%[k],%[k],8\n\t"         /* kpw2 = {k[2],...}        */ \
    "vsldoi %[kpw3],%[k],%[k],4\n\t"         /* kpw3 = {k[3],...}        */ \
    : /* output list                                                     */ \
      /* k is read in the asm, hence should be reserved                  */ \
      [k] "+&v" ((_k)),                                                     \
      [kpw1] "=v" ((_kpw1)),                                                \
      [kpw2] "=v" ((_kpw2)),                                                \
      [kpw3] "=v" ((_kpw3))                                                 \
    : /* input list                                                      */ \
    : /* clobber list                                                    */ \
  ); } while (0)

#define LOAD_W_PLUS_K(_k0, _k1, _k2, _k3, _w0, _w1, _w2, _w3, _vRb,         \
    _w, _k) do {                                                            \
  base_type t0;                                                             \
  vector_base_type vsp8;                                                    \
  vector_base_type vsp16;                                                   \
  vector_base_type shiftarg;                                                \
  vector_base_type vt0;                                                     \
  vector_base_type vt1;                                                     \
  vector_base_type vt2;                                                     \
  vector_base_type vt3;                                                     \
  vector_base_type vt4;                                                     \
  __asm__ volatile (                                                        \
    /* set vrb according to alignment                                    */ \
    "lvsr    %[vrb],0,%[wptr]\n\t"                                          \
    /* unaligned load                                                    */ \
    "lvx     %[w0],0,%[wptr]\n\t"                                           \
    "addi    %[t0],%[wptr],16\n\t"                                          \
    "lvx     %[w1],0,%[t0]\n\t"                                             \
    /* w0 = w[j-16] to w[j-13]                                           */ \
    "vperm   %[w0],%[w1],%[w0],%[vrb]\n\t"                                  \
    "addi    %[t0],%[wptr],32\n\t"                                          \
    "lvx     %[w2],0,%[t0]\n\t"                                             \
    /* w1 = w[j-12] to w[j-9]                                            */ \
    "vperm   %[w1],%[w2],%[w1],%[vrb]\n\t"                                  \
    "addi    %[t0],%[wptr],48\n\t"                                          \
    "lvx     %[w3],0,%[t0]\n\t"                                             \
    /* w2 = w[j-8] to w[j-5]                                             */ \
    "vperm   %[w2],%[w3],%[w2],%[vrb]\n\t"                                  \
    "addi    %[t0],%[wptr],64\n\t"                                          \
    "lvx     %[vt0],0,%[t0]\n\t"                                            \
    /* w3 = w[j-4] to w[j-1]                                             */ \
    "vperm   %[w3],%[vt0],%[w3],%[vrb]\n\t"                                 \
    /* set vrb according to alignment                                    */ \
    "lvsr    %[vrb],0,%[kptr]\n\t"                                          \
    /* unaligned load                                                    */ \
    "lvx     %[vt0],0,%[kptr]\n\t"                                          \
    "addi    %[t0],%[kptr],16\n\t"                                          \
    "lvx     %[vt1],0,%[t0]\n\t"                                            \
    /* vt0 = k[j-16] to k[j-13]                                          */ \
    "vperm   %[vt0],%[vt1],%[vt0],%[vrb]\n\t"                               \
    "addi    %[t0],%[kptr],32\n\t"                                          \
    "lvx     %[vt2],0,%[t0]\n\t"                                            \
    /* vt1 = k[j-12] to k[j-9]                                           */ \
    "vperm   %[vt1],%[vt2],%[vt1],%[vrb]\n\t"                               \
    "addi    %[t0],%[kptr],48\n\t"                                          \
    "lvx     %[vt3],0,%[t0]\n\t"                                            \
    /* vt2 = k[j-8] to k[j-5]                                            */ \
    "vperm   %[vt2],%[vt3],%[vt2],%[vrb]\n\t"                               \
    "addi    %[t0],%[kptr],64\n\t"                                          \
    "lvx     %[vt4],0,%[t0]\n\t"                                            \
    /* vt3 = k[j-4] to k[j-1]                                            */ \
    "vperm   %[vt3],%[vt4],%[vt3],%[vrb]\n\t"                               \
                                                                            \
    /* Swap bytes */                                                        \
    "vspltisw %[vsp16],8\n\t"                                               \
    "vspltish %[vsp8],8\n\t"                                                \
    "vspltisw %[shiftarg],1\n\t"                                            \
    "vsl      %[vsp16],%[vsp16],%[shiftarg]\n\t"                            \
    "vrlh     %[w0],%[w0],%[vsp8]\n\t"                                      \
    "vrlh     %[w1],%[w1],%[vsp8]\n\t"                                      \
    "vrlh     %[w2],%[w2],%[vsp8]\n\t"                                      \
    "vrlh     %[w3],%[w3],%[vsp8]\n\t"                                      \
    "vrlw     %[w0],%[w0],%[vsp16]\n\t"                                     \
    "vrlw     %[w1],%[w1],%[vsp16]\n\t"                                     \
    "vrlw     %[w2],%[w2],%[vsp16]\n\t"                                     \
    "vrlw     %[w3],%[w3],%[vsp16]\n\t"                                     \
    /* Add _w to k                                                       */ \
    "vadduwm %[k0],%[vt0],%[w0]\n\t"                                        \
    "vadduwm %[k1],%[vt1],%[w1]\n\t"                                        \
    "vadduwm %[k2],%[vt2],%[w2]\n\t"                                        \
    "vadduwm %[k3],%[vt3],%[w3]\n\t"                                        \
    : /* output list                                                     */ \
      /* actual outputs                                                  */ \
      [k0] "=v" ((_k0)),                                                    \
      [k1] "=v" ((_k1)),                                                    \
      [k2] "=v" ((_k2)),                                                    \
      [k3] "=v" ((_k3)),                                                    \
      [w0] "=v" ((_w0)),                                                    \
      [w1] "=v" ((_w1)),                                                    \
      [w2] "=v" ((_w2)),                                                    \
      [w3] "=v" ((_w3)),                                                    \
      [vrb] "=v" ((_vRb)),                                                  \
      /* temporaries                                                     */ \
      [t0] "=&b" (t0),                                                      \
      [vt0] "=&v" (vt0),                                                    \
      [vt1] "=&v" (vt1),                                                    \
      [vt2] "=&v" (vt2),                                                    \
      [vt3] "=&v" (vt3),                                                    \
      [vt4] "=&v" (vt4),                                                    \
      [shiftarg] "=&v" (shiftarg),                                          \
      [vsp8] "=&v" (vsp8),                                                  \
      [vsp16] "=&v" (vsp16)                                                 \
    : /* input list                                                      */ \
      [wptr] "b" ((_w)),                                                    \
      [kptr] "b" ((_k))                                                     \
    : /* clobber list                                                    */ \
      "memory"                                                              \
  ); } while (0)

#define CALC_4W(_w0, _w1, _w2, _w3, _kpw0, _kpw1, _kpw2, _kpw3,             \
                _j, _k) do {                                                \
  base_type t0;                                                             \
  base_type t1;                                                             \
  vector_base_type vt0;                                                     \
  vector_base_type vt1;                                                     \
  vector_base_type vt2;                                                     \
  vector_base_type vt3;                                                     \
  vector_base_type vt4;                                                     \
  vector_base_type vt5;                                                     \
  vector_base_type vt6;                                                     \
  vector_base_type vt7;                                                     \
  vector_base_type vt8;                                                     \
  __asm__ volatile (                                                        \
    "sldi       %[t1],%[index],2\n\t"         /* j * 4 (word size)       */ \
    "add        %[t0],%[t1],%[kptr]\n\t"      /* alias to k[j] location  */ \
    "lvx        %[vt6],0,%[t0]\n\t"                                         \
    "addi       %[t0],%[t0],16\n\t"           /* alias to k[j+4] location*/ \
    "lvsr       %[vt2],0,%[kptr]\n\t"                                       \
    "lvx        %[vt1],0,%[t0]\n\t"           /* treat unaligned case    */ \
    "vperm      %[vt6],%[vt1],%[vt6],%[vt2]\n\t"                            \
    /* vt1 = w[j-15], w[j-14], w[j-13], w[j-12]                          */ \
    "vsldoi     %[vt1],%[w1],%[w0],12\n\t"                                  \
    /* vt7 = w[j-7], w[j-6], w[j-5], w[j-4]                              */ \
    "vsldoi     %[vt7],%[w3],%[w2],12\n\t"                                  \
    /* vt8 = w[j-2], w[j-1], w[j-4], w[j-3]                              */ \
    "vsldoi     %[vt8],%[w3],%[w3],8\n\t"                                   \
    /* vt1 = s0(w[j-15]) , s0(w[j-14]) , s0(w[j-13]) , s0(w[j-12])       */ \
    "vshasigmaw %[vt1],%[vt1],0,0\n\t"                                      \
    /* vt2 = s1(w[j-2]) , s1(w[j-1]) , s1(w[j-4]) , s1(w[j-3])           */ \
    "vshasigmaw %[vt2],%[vt8],0,%[six1]\n\t"                                \
    /* vt3 = s0(w[j-15]) + w[j-7],                                       */ \
    /*       s0(w[j-14]) + w[j-6],                                       */ \
    /*       s0(w[j-13]) + w[j-5],                                       */ \
    /*       s0(w[j-12]) + w[j-4]                                        */ \
    "vadduwm    %[vt3],%[vt1],%[vt7]\n\t"                                   \
    /* vt4 = s0(w[j-15]) + w[j-7] + w[j-16],                             */ \
    /*       s0(w[j-14]) + w[j-6] + w[j-15],                             */ \
    /*       s0(w[j-13]) + w[j-5] + w[j-14],                             */ \
    /*       s0(w[j-12]) + w[j-4] + w[j-13]                              */ \
    "vadduwm    %[vt4],%[vt3],%[w0]\n\t"                                    \
    /* vt5 = s0(w[j-15]) + w[j-7] + w[j-16] + s1(w[j-2]), // w[j]        */ \
    /*       s0(w[j-14]) + w[j-6] + w[j-15] + s1(w[j-1]), // w[j+1]      */ \
    /*       s0(w[j-13]) + w[j-5] + w[j-14] + s1(w[j-4]), // UNDEFINED   */ \
    /*       s0(w[j-12]) + w[j-4] + w[j-13] + s1(w[j-3])  // UNDEFINED   */ \
    "vadduwm    %[vt5],%[vt4],%[vt2]\n\t"                                   \
    /* At this point, vt5[0] and vt5[1] are the correct values to be     */ \
    /* stored at w[j] and w[j+1].                                        */ \
    /* vt5[2] and vt5[3] are not considered                              */ \
    /* vt0 = s1(w[j]) , s1(s(w[j+1]) , UNDEFINED , UNDEFINED             */ \
    "vshasigmaw %[vt0],%[vt5],0,%[six1]\n\t"                                \
    /* vt2 = s1(w[j-2]) , s1(w[j-1]) , s1(w[j]) , s1(w[j+1])             */ \
    /*  NOTE: %x[vtN] corresponds to the equivalent VSX register         */ \
    "xxmrgld    %x[vt2],%x[vt0],%x[vt2]\n\t"                                \
    /* vt5 = s0(w[j-15]) + w[j-7] + w[j-16] + s1(w[j-2]), // w[j]        */ \
    /*       s0(w[j-14]) + w[j-6] + w[j-15] + s1(w[j-1]), // w[j+1]      */ \
    /*       s0(w[j-13]) + w[j-5] + w[j-14] + s1(w[j]),   // w[j+2]      */ \
    /*       s0(w[j-12]) + w[j-4] + w[j-13] + s1(w[j+1])  // w[j+4]      */ \
    "vadduwm    %[vt5],%[vt4],%[vt2]\n\t"                                   \
    /* Updating w0 to w3 to hold the "new previous" 16 values from w.    */ \
    "vor        %[w0],%[w1],%[w1]\n\t"                                      \
    "vor        %[w1],%[w2],%[w2]\n\t"                                      \
    "vor        %[w2],%[w3],%[w3]\n\t"                                      \
    "vor        %[w3],%[vt5],%[vt5]\n\t"                                    \
    /* store k + w to vt5 (4 values at once)                             */ \
    "vadduwm    %[kpw0],%[vt5],%[vt6]\n\t"                                  \
    /* Deque results in kpw0 to kpw1, kpw2 and kpw3                      */ \
    "vsldoi     %[kpw1],%[kpw0],%[kpw0],12\n\t"                             \
    "vsldoi     %[kpw2],%[kpw0],%[kpw0],8\n\t"                              \
    "vsldoi     %[kpw3],%[kpw0],%[kpw0],4\n\t"                              \
    : /* output list                                                     */ \
      /* temporaries                                                     */ \
      [t0] "=&b" (t0),                                                      \
      [t1] "=&b" (t1),                                                      \
      [vt0] "=&v" (vt0),                                                    \
      [vt1] "=&v" (vt1),                                                    \
      [vt2] "=&v" (vt2),                                                    \
      [vt3] "=&v" (vt3),                                                    \
      [vt4] "=&v" (vt4),                                                    \
      [vt5] "=&v" (vt5),                                                    \
      [vt6] "=&v" (vt6),                                                    \
      [vt7] "=&v" (vt7),                                                    \
      [vt8] "=&v" (vt8),                                                    \
      /* output/input                                                    */ \
      [w0] "+v" ((_w0)),                                                    \
      [w1] "+v" ((_w1)),                                                    \
      [w2] "+v" ((_w2)),                                                    \
      [w3] "+v" ((_w3)),                                                    \
      /* actual outputs                                                  */ \
      [kpw0] "=v" ((_kpw0)),                                                \
      [kpw1] "=v" ((_kpw1)),                                                \
      [kpw2] "=v" ((_kpw2)),                                                \
      [kpw3] "=v" ((_kpw3))                                                 \
    : /* input list                                                      */ \
      [index] "b" ((_j)),                                                   \
      [kptr] "b" ((_k)),                                                    \
      [six1] "i" (0xf)                                                      \
    : /* clobber list                                                    */ \
      "memory"                                                              \
  ); } while (0)

#define UPDATE_SHA_STATE(_a, _b, _c, _d, _e, _f, _g, _h, _hptr) do {        \
  vector_base_type vt0;                                                     \
  vector_base_type vt1;                                                     \
  vector_base_type vt2;                                                     \
  vector_base_type vt3;                                                     \
  vector_base_type vt4;                                                     \
  vector_base_type vt5;                                                     \
  vector_base_type vt6;                                                     \
  vector_base_type vrb;                                                     \
  base_type rtmp;                                                           \
  __asm__ volatile(                                                         \
      "lvsr    %[vrb],0,%[hptr]\n\t"                                        \
      "lvx     %[vt0],0,%[hptr]\n\t"        /* vt0 = _h[0].._h[3]        */ \
      "lvx     %[vt5],%[offs],%[hptr]\n\t"  /* vt5 = _h[4].._h[8]        */ \
      "vperm   %[vt0],%[vt5],%[vt0],%[vrb]\n\t"                             \
      "lvx     %[vt6],%[offs2],%[hptr]\n\t" /* vt5 = _h[4].._h[8]        */ \
      "vperm   %[vt5],%[vt6],%[vt5],%[vrb]\n\t"                             \
      "vmrglw  %[vt1],%[b],%[a]\n\t"        /* vt1 = {a, b, ?, ?}        */ \
      "vmrglw  %[vt2],%[d],%[c]\n\t"        /* vt2 = {c, d, ?, ?}        */ \
      "vmrglw  %[vt3],%[f],%[e]\n\t"        /* vt3 = {e, f, ?, ?}        */ \
      "vmrglw  %[vt4],%[h],%[g]\n\t"        /* vt4 = {g, h, ?, ?}        */ \
      "xxmrgld %x[vt1],%x[vt2],%x[vt1]\n\t" /* vt1 = {a, b, c, d}        */ \
      "xxmrgld %x[vt3],%x[vt4],%x[vt3]\n\t" /* vt3 = {e, f, g, h}        */ \
      /* vt0 = {a+_h[0], b+_h[1], c+_h[2], d+_h[3]}                      */ \
      "vadduwm %[vt0],%[vt0],%[vt1]\n\t"                                    \
      /* vt5 = {e+_h[4], f+_h[5], g+_h[6], h+_h[7]                       */ \
      "vadduwm %[vt5],%[vt5],%[vt3]\n\t"                                    \
      "mfvrwz %[rtmp], %[vt0]\n\t"          /* rtmp = a+hptr[0]          */ \
      "stw    %[rtmp], 8(%[hptr])\n\t"      /* update h[3]               */ \
      /* vt6 = {b+hptr[1], c+hptr[2], d+hptr[3], a+hptr[0]}              */ \
      "vsldoi %[vt6],%[vt0],%[vt0],12\n\t"                                  \
      "mfvrwz %[rtmp], %[vt6]\n\t"          /* tmp = b+hptr[1]           */ \
      "stw    %[rtmp], 12(%[hptr])\n\t"     /* update h[2]               */ \
      /* vt6 = {c+hptr[2], d+hptr[3], a+hptr[0], b+hptr[1]}              */ \
      "vsldoi %[vt6],%[vt6],%[vt6],12\n\t"                                  \
      "mfvrwz %[rtmp],%[vt6]\n\t"           /* rtmp = c+hptr[2]          */ \
      "stw    %[rtmp],0(%[hptr])\n\t"       /* update h[1]               */ \
      /* vt6 = {d+hptr[3], a+hptr[0], b+hptr[1], c+hptr[2]}              */ \
      "vsldoi %[vt6],%[vt6],%[vt6],12\n\t"                                  \
      "mfvrwz %[rtmp],%[vt6]\n\t"           /* rtmp = d+hptr[3]          */ \
      "stw    %[rtmp],4(%[hptr])\n\t"       /* update h[0]               */ \
      "mfvrwz %[rtmp],%[vt5]\n\t"           /* tmp = e+hptr[4]           */ \
      "stw    %[rtmp],24(%[hptr])\n\t"      /* update h[7]               */ \
      /* vt6 = {f+hptr[5], g+hptr[6], d+hptr[3], h+hptr[7]}              */ \
      "vsldoi %[vt6],%[vt5],%[vt5],12\n\t"                                  \
      "mfvrwz %[rtmp],%[vt6]\n\t"           /* rtmp = f+hptr[5]          */ \
      "stw    %[rtmp],28(%[hptr])\n\t"      /* update h[6]               */ \
      /* vt6 = {g+hptr[6], h+hptr[7], e+hptr[4], f+hptr[5]}              */ \
      "vsldoi %[vt6],%[vt6],%[vt6],12\n\t"                                  \
      "mfvrwz %[rtmp],%[vt6]\n\t"           /* rtmp = g+hptr[6]          */ \
      "stw    %[rtmp],16(%[hptr])\n\t"      /* update h[5]               */ \
      /* vt6 = {h+hptr[7], e+hptr[4], f+hptr[5], g+hptr[6]}              */ \
      "vsldoi %[vt6],%[vt6],%[vt6],12\n\t"                                  \
      "mfvrwz %[rtmp],%[vt6]\n\t"           /* rtmp = h+hptr[7]          */ \
      "stw    %[rtmp],20(%[hptr])\n\t"                                      \
    : /* output list                                                     */ \
      /* temporaries                                                     */ \
      [vt0] "=&v" (vt0),                                                    \
      [vt1] "=&v" (vt1),                                                    \
      [vt2] "=&v" (vt2),                                                    \
      [vt3] "=&v" (vt3),                                                    \
      [vt4] "=&v" (vt4),                                                    \
      [vt5] "=&v" (vt5),                                                    \
      [vt6] "=&v" (vt6),                                                    \
      [vrb] "=&v" (vrb),                                                    \
      [rtmp] "=&b" (rtmp)                                                   \
    : /* input list                                                      */ \
      [hptr] "b" ((_hptr)),                                                 \
      [a] "v" ((_a)),                                                       \
      [b] "v" ((_b)),                                                       \
      [c] "v" ((_c)),                                                       \
      [d] "v" ((_d)),                                                       \
      [e] "v" ((_e)),                                                       \
      [f] "v" ((_f)),                                                       \
      [g] "v" ((_g)),                                                       \
      [h] "v" ((_h)),                                                       \
      [offs] "b" (16),                                                      \
      [offs2] "b" (32)                                                      \
    : /* clobber list                                                    */ \
      "memory"                                                              \
  ); } while (0)

void sha2_transform(base_type* _h, unsigned char* w) {
  vector_base_type a, b, c, d, e, f, g, h;
  vector int vRb;

  vector_base_type w0, w1, w2, w3;
  vector_base_type kplusw0, kplusw1, kplusw2, kplusw3;
  vector_base_type kpw0, kpw1, kpw2, kpw3;

  LOAD_H_VEC(a, e, _h, vRb);
  DEQUE  (a, b, c, d);
  DEQUE  (e, f, g, h);

  // Load 16 elements from w out of the loop
  LOAD_W_PLUS_K(kplusw0, kplusw1, kplusw2, kplusw3, w0, w1, w2, w3, vRb, w, k);

  // Loop unrolling, from 0 to 15
  DEQUE  (kplusw0, kpw1, kpw2, kpw3);
  SHA2_ROUND(a, b, c, d, e, f, g, h, kplusw0);
  SHA2_ROUND(h, a, b, c, d, e, f, g, kpw1);
  SHA2_ROUND(g, h, a, b, c, d, e, f, kpw2);
  SHA2_ROUND(f, g, h, a, b, c, d, e, kpw3);

  DEQUE  (kplusw1, kpw1, kpw2, kpw3);
  SHA2_ROUND(e, f, g, h, a, b, c, d, kplusw1);
  SHA2_ROUND(d, e, f, g, h, a, b, c, kpw1);
  SHA2_ROUND(c, d, e, f, g, h, a, b, kpw2);
  SHA2_ROUND(b, c, d, e, f, g, h, a, kpw3);

  DEQUE  (kplusw2, kpw1, kpw2, kpw3);
  SHA2_ROUND(a, b, c, d, e, f, g, h, kplusw2);
  SHA2_ROUND(h, a, b, c, d, e, f, g, kpw1);
  SHA2_ROUND(g, h, a, b, c, d, e, f, kpw2);
  SHA2_ROUND(f, g, h, a, b, c, d, e, kpw3);

  DEQUE  (kplusw3, kpw1, kpw2, kpw3);
  SHA2_ROUND(e, f, g, h, a, b, c, d, kplusw3);
  SHA2_ROUND(d, e, f, g, h, a, b, c, kpw1);
  SHA2_ROUND(c, d, e, f, g, h, a, b, kpw2);
  SHA2_ROUND(b, c, d, e, f, g, h, a, kpw3);

  // Prepare j to the next elements
  int j = 16;

  // From 16 to W_SIZE (64) in 8 steps
  while (j < W_SIZE) {
    CALC_4W(w0, w1, w2, w3, kpw0, kpw1, kpw2, kpw3, j, k);
    SHA2_ROUND(a, b, c, d, e, f, g, h, kpw0);
    SHA2_ROUND(h, a, b, c, d, e, f, g, kpw1);
    SHA2_ROUND(g, h, a, b, c, d, e, f, kpw2);
    SHA2_ROUND(f, g, h, a, b, c, d, e, kpw3);
    j += 4;

    CALC_4W(w0, w1, w2, w3, kpw0, kpw1, kpw2, kpw3, j, k);
    SHA2_ROUND(e, f, g, h, a, b, c, d, kpw0);
    SHA2_ROUND(d, e, f, g, h, a, b, c, kpw1);
    SHA2_ROUND(c, d, e, f, g, h, a, b, kpw2);
    SHA2_ROUND(b, c, d, e, f, g, h, a, kpw3);
    j += 4;
  }

  UPDATE_SHA_STATE(a, b, c, d, e, f, g, h, _h);
}

