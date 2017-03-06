#ifndef _PPC64_LE_SHA2_LL_ASM_H_
#define _PPC64_LE_SHA2_LL_ASM_H_

#include "base-types.h"

/*
 * As SHA2_ROUND is only defining the new elements and the rest is being
 * handled outside the macro, the new elements that should be 'a' and 'e' are
 * actually the previous ones.
 *
 * This approach avoids moving registers around. The drawback is that it'll
 * generate a big code due to different registers being used.
 */

#if SHA_BITS == 256

#define SHA2_ROUND(_a, _b, _c, _d, _e, _f, _g, _h, _kplusw) do {            \
    vector_base_type ch;                                                    \
    vector_base_type maj;                                                   \
    vector_base_type bsa;                                                   \
    vector_base_type bse;                                                   \
    vector_base_type tmp1;                                                  \
    vector_base_type tmp2;                                                  \
    vector_base_type vt1;                                                   \
    vector_base_type vt2;                                                   \
  __asm__ volatile (                                                        \
      "vsel %[ch],%[g],%[f],%[e]\n\t"       /* ch = Ch(e,f,g)            */ \
      "vxor %[maj],%[a],%[b]\n\t"           /* intermediate Maj          */ \
      "vsel %[maj],%[b],%[c],%[maj]\n\t"    /* maj = Maj(a,b,c)          */ \
      "vshasigmaw %[bsa],%[a],1,0\n\t"      /* bsa = BigSigma0(a)        */ \
      "vshasigmaw %[bse],%[e],1,0xf\n\t"    /* bse = BigSigma1(e)        */ \
      "vadduwm %[vt1],%[h],%[bse]\n\t"      /* vt1 = h + bse             */ \
      "vadduwm %[vt2],%[ch],%[kpw]\n\t"     /* vt2 = ch + kpw            */ \
      "vadduwm %[tmp1],%[vt1],%[vt2]\n\t"   /* tmp1 = h + bse + ch + kpw */ \
      "vadduwm %[tmp2],%[bsa],%[maj]\n\t"   /* tmp2 = bsa + maj          */ \
      "vadduwm %[d],%[d],%[tmp1]\n\t"       /* d = d + tmp1              */ \
      "vadduwm %[h],%[tmp1],%[tmp2]\n\t"    /* h = tmp1 + tmp2           */ \
    : /* output list                                                     */ \
      /* temporaries                                                     */ \
      [ch] "=&v" (ch),                                                      \
      [maj] "=&v" (maj),                                                    \
      [bsa] "=&v" (bsa),                                                    \
      [bse] "=&v" (bse),                                                    \
      [tmp1] "=&v" (tmp1),                                                  \
      [tmp2] "=&v" (tmp2),                                                  \
      [vt1] "=&v" (vt1),                                                    \
      [vt2] "=&v" (vt2),                                                    \
      /* output/input                                                    */ \
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

#define LOAD_H_VEC(_a, _b, _c, _d, _e, _f, _g, _h, _hptr, _vrb) do {        \
  base_type index;                                                          \
  vector_base_type tmp1;                                                    \
  __asm__ volatile (                                                        \
     "lvx %[a],0,%[hptr]\n\t"           /* load unaligned                */ \
     "addi %[idx],%[hptr],16\n\t"                                           \
     "lvsr %[vrb],0,%[idx]\n\t"                                             \
     "lvx %[e],0,%[idx]\n\t"            /* load unaligned                */ \
     "vperm %[a],%[e],%[a],%[vrb]\n\t"  /* a = {a,b,c,d}                 */ \
     "addi %[idx],%[hptr],16\n\t"                                           \
     "lvx %[tmp1],0,%[hptr]\n\t"        /* load unaligned                */ \
     /* e = {e,f,g,h}                                                    */ \
     "vperm %[e],%[tmp1],%[e],%[vrb]\n\t"                                   \
     "vsldoi %[b],%[a],%[a],12\n\t"     /* b = {b,c,d,a}                 */ \
     "vsldoi %[c],%[a],%[a],8\n\t"      /* c = {c,d,a,b}                 */ \
     "vsldoi %[d],%[a],%[a],4\n\t"      /* d = {d,a,b,c}                 */ \
     "vsldoi %[f],%[e],%[e],12\n\t"     /* f = {f,g,h,e}                 */ \
     "vsldoi %[g],%[e],%[e],8\n\t"      /* g = {g,h,e,f}                 */ \
     "vsldoi %[h],%[e],%[e],4\n\t"      /* h = {h,e,f,g}                 */ \
   : /* output list                                                      */ \
     /* temporaries                                                      */ \
     [idx] "=&r" (index),                                                   \
     /* actual outputs                                                   */ \
     /* a and e are read in the asm, hence should be reserved            */ \
     [a] "=&v" ((_a)),                                                      \
     [e] "=&v" ((_e)),                                                      \
     [tmp1] "=&v" ((tmp1)),                                                \
     [vrb] "=&v" ((_vrb)),                                                  \
     [b] "=v" ((_b)),                                                       \
     [c] "=v" ((_c)),                                                       \
     [d] "=v" ((_d)),                                                       \
     [f] "=v" ((_f)),                                                       \
     [g] "=v" ((_g)),                                                       \
     [h] "=v" ((_h))                                                        \
   : /* input list                                                       */ \
     [hptr] "r" ((_hptr))                                                   \
   : /* clobber list                                                     */ \
     "memory"                                                               \
  ); } while (0)                                                            \

#define DEQUE(_k, _kpw0, _kpw1, _kpw2, _kpw3) do {                          \
  __asm__ volatile (                                                        \
    "vmr %[kpw0],%[k]\n\t"                   /* kpw0 = {k[0],...}        */ \
    "vsldoi %[kpw1],%[k],%[k],12\n\t"        /* kpw1 = {k[1],...}        */ \
    "vsldoi %[kpw2],%[k],%[k],8\n\t"         /* kpw2 = {k[2],...}        */ \
    "vsldoi %[kpw3],%[k],%[k],4\n\t"         /* kpw3 = {k[3],...}        */ \
    : /* output list                                                     */ \
      /* k is read in the asm, hence should be reserved                  */ \
      [k] "+&v" ((_k)),                                                     \
      [kpw0] "=v" ((_kpw0)),                                                \
      [kpw1] "=v" ((_kpw1)),                                                \
      [kpw2] "=v" ((_kpw2)),                                                \
      [kpw3] "=v" ((_kpw3))                                                 \
    : /* input list                                                      */ \
    : /* clobber list                                                    */ \
  ); } while (0)

#define LOAD_W_PLUS_K(_k0, _k1, _k2, _k3, _w0, _w1, _w2, _w3, _vRb, _vRc,   \
    _j, _Rb, _Rc, _w, _k) do {                                              \
  base_type t0;                                                             \
  base_type t1;                                                             \
  vector_base_type vt0;                                                     \
  vector_base_type vt1;                                                     \
  vector_base_type vt2;                                                     \
  vector_base_type vt3;                                                     \
  vector_base_type vt4;                                                     \
  __asm__ volatile (                                                        \
    /* t1 = j * 4 (word size)                                            */ \
    "sldi    %[t1],%[index],%[c1]\n\t"                                      \
    /* alias to W[j] location                                            */ \
    "add     %[t1],%[t1],%[wptr]\n\t"                                       \
    "addi    %[t0],%[t1],%[c2]\n\t"                                         \
    /* unaligned load                                                    */ \
    "lvx     %[w0],0,%[t0]\n\t"                                             \
    /* set vrb according to alignment                                    */ \
    "lvsr    %[vrb],0,%[t1]\n\t"                                            \
    "addi    %[t0],%[t1],%[c3]\n\t"                                         \
    "lvx     %[w1],0,%[t0]\n\t"                                             \
    /* w0 = w[j-16] to w[j-13]                                           */ \
    "vperm   %[w0],%[w1],%[w0],%[vrb]\n\t"                                  \
    "addi    %[t0],%[t1],%[c4]\n\t"                                         \
    "lvx     %[w2],0,%[t0]\n\t"                                             \
    /* w1 = w[j-12] to w[j-9]                                            */ \
    "vperm   %[w1],%[w2],%[w1],%[vrb]\n\t"                                  \
    "addi    %[t0],%[t1],%[c5]\n\t"                                         \
    "lvx     %[w3],0,%[t0]\n\t"                                             \
    /* w2 = w[j-8] to w[j-5]                                             */ \
    "vperm   %[w2],%[w3],%[w2],%[vrb]\n\t"                                  \
    "addi    %[t0],%[t1],%[c0]\n\t"                                         \
    "lvx     %[vt0],0,%[t0]\n\t"                                            \
    /* w3 = w[j-4] to w[j-1]                                             */ \
    "vperm   %[w3],%[vt0],%[w3],%[vrb]\n\t"                                 \
    /* Load 4*4 k values                                                 */ \
    "sldi    %[t1],%[index],%[c1]\n\t"                                      \
    /* alias to k[j] location                                            */ \
    "add     %[t1],%[t1],%[kptr]\n\t"                                       \
    "addi    %[t0],%[t1],%[c2]\n\t"                                         \
    /* unaligned load                                                    */ \
    "lvx     %[vt0],0,%[t0]\n\t"                                            \
    /* set vrb according to alignment                                    */ \
    "lvsr    %[vrb],0,%[t1]\n\t"                                            \
    "addi    %[t0],%[t1],%[c3]\n\t"                                         \
    "lvx     %[vt1],0,%[t0]\n\t"                                            \
    /* vt0 = k[j-16] to k[j-13]                                          */ \
    "vperm   %[vt0],%[vt1],%[vt0],%[vrb]\n\t"                               \
    "addi    %[t0],%[t1],%[c4]\n\t"                                         \
    "lvx     %[vt2],0,%[t0]\n\t"                                            \
    /* vt1 = k[j-12] to k[j-9]                                           */ \
    "vperm   %[vt1],%[vt2],%[vt1],%[vrb]\n\t"                               \
    "addi    %[t0],%[t1],%[c5]\n\t"                                         \
    "lvx     %[vt3],0,%[t0]\n\t"                                            \
    /* vt2 = k[j-8] to k[j-5]                                            */ \
    "vperm   %[vt2],%[vt3],%[vt2],%[vrb]\n\t"                               \
    "addi    %[t0],%[t1],%[c0]\n\t"                                         \
    "lvx     %[vt4],0,%[t0]\n\t"                                            \
    /* vt3 = k[j-4] to k[j-1]                                            */ \
    "vperm   %[vt3],%[vt4],%[vt3],%[vrb]\n\t"                               \
                                                                            \
    /* parameter for vperm                                               */ \
    "lvsl    %[vrb],0,%[rb]\n\t"                                            \
    "lvsr    %[vrc],0,%[rc]\n\t"                                            \
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
      [vrc] "=v" ((_vRc)),                                                  \
      /* temporaries                                                     */ \
      [t0] "=&r" (t0),                                                      \
      [t1] "=&r" (t1),                                                      \
      [vt0] "=&v" (vt0),                                                    \
      [vt1] "=&v" (vt1),                                                    \
      [vt2] "=&v" (vt2),                                                    \
      [vt3] "=&v" (vt3),                                                    \
      [vt4] "=&v" (vt4)                                                     \
    : /* input list                                                      */ \
      [index] "r" ((_j)),                                                   \
      [rb] "r" ((_Rb)),                                                     \
      [rc] "r" ((_Rc)),                                                     \
      [wptr] "r" ((_w)),                                                    \
      [kptr] "r" ((_k)),                                                    \
      [c1] "i" (2),                                                         \
      [c0] "i" (0),                                                         \
      [c2] "i" (-64),                                                       \
      [c3] "i" (-48),                                                       \
      [c4] "i" (-32),                                                       \
      [c5] "i" (-16)                                                        \
    : /* clobber list                                                    */ \
      "memory"                                                              \
  ); } while (0)

#define CALC_4W(_w0, _w1, _w2, _w3, _kpw0, _kpw1, _kpw2, _kpw3,             \
                _j, _vRb, _vRc, _k) do {                                    \
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
    /* vt1 = w[j-15], w[j-14], w[j-13], w[j-12]                          */ \
    "vperm      %[vt1],%[w1],%[w0],%[vrc]\n\t"                              \
    /* vt7 = w[j-7], w[j-6], w[j-5], w[j-4]                              */ \
    "vperm      %[vt7],%[w3],%[w2],%[vrc]\n\t"                              \
    /* vt8 = w[j-2], w[j-1], w[j-4], w[j-3]                              */ \
    "vperm      %[vt8],%[w3],%[w3],%[vrb]\n\t"                              \
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
      [t0] "=&r" (t0),                                                      \
      [t1] "=&r" (t1),                                                      \
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
      [index] "r" ((_j)),                                                   \
      [vrb] "v" ((_vRb)),                                                   \
      [vrc] "v" ((_vRc)),                                                   \
      [kptr] "r" ((_k)),                                                    \
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
  vector_base_type vrb;                                                     \
  vector_base_type vtmp0;                                                   \
  base_type rtmp;                                                           \
  __asm__ volatile(                                                         \
      "lvsr    %[vrb],0,%[hptr]\n\t"                                        \
      "lvx     %[vt0],0,%[hptr]\n\t"        /* vt0 = _h[0].._h[3]        */ \
      "lvx     %[vt5],%[offs],%[hptr]\n\t"  /* vt5 = _h[4].._h[8]        */ \
      "vperm   %[vt0],%[vt5],%[vt0],%[vrb]\n\t"                             \
      "lvx     %[vtmp0],%[offs2],%[hptr]\n\t"/* vt5 = _h[4].._h[8]       */ \
      "vperm   %[vt5],%[vtmp0],%[vt5],%[vrb]\n\t"                           \
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
      /* vtmp0 = {b+hptr[1], c+hptr[2], d+hptr[3], a+hptr[0]}            */ \
      "vsldoi %[vtmp0],%[vt0],%[vt0],12\n\t"                                \
      "mfvrwz %[rtmp], %[vtmp0]\n\t"        /* tmp = b+hptr[1]           */ \
      "stw    %[rtmp], 12(%[hptr])\n\t"     /* update h[2]               */ \
      /* vtmp0 = {c+hptr[2], d+hptr[3], a+hptr[0], b+hptr[1]}            */ \
      "vsldoi %[vtmp0],%[vtmp0],%[vtmp0],12\n\t"                            \
      "mfvrwz %[rtmp],%[vtmp0]\n\t"         /* rtmp = c+hptr[2]          */ \
      "stw    %[rtmp],0(%[hptr])\n\t"       /* update h[1]               */ \
      /* vtmp0 = {d+hptr[3], a+hptr[0], b+hptr[1], c+hptr[2]}            */ \
      "vsldoi %[vtmp0],%[vtmp0],%[vtmp0],12\n\t"                            \
      "mfvrwz %[rtmp],%[vtmp0]\n\t"         /* rtmp = d+hptr[3]          */ \
      "stw    %[rtmp],4(%[hptr])\n\t"       /* update h[0]               */ \
      "mfvrwz %[rtmp],%[vt5]\n\t"           /* tmp = e+hptr[4]           */ \
      "stw    %[rtmp],24(%[hptr])\n\t"      /* update h[7]               */ \
      /* vtmp0 = {f+hptr[5], g+hptr[6], d+hptr[3], h+hptr[7]}            */ \
      "vsldoi %[vtmp0],%[vt5],%[vt5],12\n\t"                                \
      "mfvrwz %[rtmp],%[vtmp0]\n\t"        /* rtmp = f+hptr[5]           */ \
      "stw    %[rtmp],28(%[hptr])\n\t"     /* update h[6]                */ \
      /* vtmp0 = {g+hptr[6], h+hptr[7], e+hptr[4], f+hptr[5]}            */ \
      "vsldoi %[vtmp0],%[vtmp0],%[vtmp0],12\n\t"                            \
      "mfvrwz %[rtmp],%[vtmp0]\n\t"        /* rtmp = g+hptr[6]           */ \
      "stw    %[rtmp],16(%[hptr])\n\t"     /* update h[5]                */ \
      /* vtmp0 = {h+hptr[7], e+hptr[4], f+hptr[5], g+hptr[6]}            */ \
      "vsldoi %[vtmp0],%[vtmp0],%[vtmp0],12\n\t"                            \
      "mfvrwz %[rtmp],%[vtmp0]\n\t"        /* rtmp = h+hptr[7]           */ \
      "stw    %[rtmp],20(%[hptr])\n\t"     /*                            */ \
    : /* output list                                                     */ \
      [vt0] "=&v" (vt0),                                                    \
      [vt1] "=&v" (vt1),                                                    \
      [vt2] "=&v" (vt2),                                                    \
      [vt3] "=&v" (vt3),                                                    \
      [vt4] "=&v" (vt4),                                                    \
      [vt5] "=&v" (vt5),                                                    \
      [vtmp0] "=&v" (vtmp0),                                                \
      [vrb] "=&v" (vrb),                                                    \
      [rtmp] "=&r" (rtmp)                                                   \
    : /* input list                                                      */ \
      [hptr] "r" ((_hptr)),                                                 \
      [a] "v" ((_a)),                                                       \
      [b] "v" ((_b)),                                                       \
      [c] "v" ((_c)),                                                       \
      [d] "v" ((_d)),                                                       \
      [e] "v" ((_e)),                                                       \
      [f] "v" ((_f)),                                                       \
      [g] "v" ((_g)),                                                       \
      [h] "v" ((_h)),                                                       \
      [offs] "r" (16),                                                      \
      [offs2] "r" (32)                                                      \
    : /* clobber list                                                    */ \
      "memory"                                                              \
  ); } while (0)

#else // SHA_BITS == 512

#define SHA2_ROUND(_a, _b, _c, _d, _e, _f, _g, _h, _kplusw) do {            \
    vector_base_type ch;                                                    \
    vector_base_type maj;                                                   \
    vector_base_type bsa;                                                   \
    vector_base_type bse;                                                   \
    vector_base_type tmp1;                                                  \
    vector_base_type tmp2;                                                  \
    vector_base_type vt1;                                                   \
    vector_base_type vt2;                                                   \
  __asm__ volatile (                                                        \
      "vsel %[ch],%[g],%[f],%[e]\n\t"       /* ch   = Ch(e,f,g)          */ \
      "vxor %[maj],%[a],%[b]\n\t"           /* intermediate Maj          */ \
      "vsel %[maj],%[b],%[c],%[maj]\n\t"    /* maj  = Maj(a,b,c)         */ \
      "vshasigmad %[bsa],%[a],1,0\n\t"      /* bsa  = BigSigma0(a)       */ \
      "vshasigmad %[bse],%[e],1,0xf\n\t"    /* bse  = BigSigma1(e)       */ \
      "vaddudm %[vt1],%[h],%[bse]\n\t"      /* vt1  = h + bse            */ \
      "vaddudm %[vt2],%[ch],%[kpw]\n\t"     /* vt2  = ch + kpw           */ \
      "vaddudm %[tmp1],%[vt1],%[vt2]\n\t"   /* tmp1 = h + bse + ch + kpw */ \
      "vaddudm %[tmp2],%[bsa],%[maj]\n\t"   /* tmp2 = bsa + maj          */ \
      "vaddudm %[d],%[d],%[tmp1]\n\t"       /* d    = d + tmp1           */ \
      "vaddudm %[h],%[tmp1],%[tmp2]\n\t"    /* h    = tmp1 + tmp2        */ \
    : /* output list                                                     */ \
      /* temporaries                                                     */ \
      [ch] "=&v" (ch),                                                      \
      [maj] "=&v" (maj),                                                    \
      [bsa] "=&v" (bsa),                                                    \
      [bse] "=&v" (bse),                                                    \
      [tmp1] "=&v" (tmp1),                                                  \
      [tmp2] "=&v" (tmp2),                                                  \
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

#define LOAD_H_VEC(_a, _b, _c, _d, _e, _f, _g, _h, _hptr, _vrb) do {        \
  vector_base_type tmp1;                                                    \
  base_type index;                                                          \
  __asm__ volatile (                                                        \
     "lvx %[a],0,%[hptr]\n\t"          /* load unaligned                 */ \
     "addi %[idx],%[hptr],16\n\t"                                           \
     "lvsr %[vrb],0,%[idx]\n\t"        /* set vrb according to alignment */ \
     "lvx %[c],0,%[idx]\n\t"           /* load unaligned                 */ \
     "vperm %[a],%[c],%[a],%[vrb]\n\t" /* a = {a,b}                      */ \
     "addi %[idx],%[hptr],32\n\t"                                           \
     "lvx %[e],0,%[idx]\n\t"           /* load unaligned                 */ \
     "vperm %[c],%[e],%[c],%[vrb]\n\t" /* c = {c,d}                      */ \
     "addi %[idx],%[hptr],48\n\t"                                           \
     "lvx %[g],0,%[idx]\n\t"           /* load unaligned                 */ \
     "vperm %[e],%[g],%[e],%[vrb]\n\t" /* e = {e,f}                      */ \
     "addi %[idx],%[hptr],64\n\t"                                           \
     "lvx %[tmp1],0,%[idx]\n\t"        /* load unaligned                 */ \
     /* g = {g,h}                                                        */ \
     "vperm %[g],%[tmp1],%[g],%[vrb]\n\t"                                   \
     "vsldoi %[b],%[a],%[a],8 \n\t"    /* b = {b,a}                      */ \
     "vsldoi %[d],%[c],%[c],8 \n\t"    /* d = {d,c}                      */ \
     "vsldoi %[f],%[e],%[e],8 \n\t"    /* f = {f,e}                      */ \
     "vsldoi %[h],%[g],%[g],8 \n\t"    /* h = {h,g}                      */ \
   : /* output list                                                      */ \
     /* temporaries                                                      */ \
     [idx] "=&r" (index),                                                   \
     [tmp1] "=&v" ((tmp1)),                                                 \
     /* actual outputs                                                   */ \
     /* a,c,e,g are read in the asm, hence should be reserved            */ \
     [a] "=&v" ((_a)),                                                      \
     [c] "=&v" ((_c)),                                                      \
     [e] "=&v" ((_e)),                                                      \
     [g] "=&v" ((_g)),                                                      \
     [vrb] "=&v" ((_vrb)),                                                  \
     [b] "=v" ((_b)),                                                       \
     [d] "=v" ((_d)),                                                       \
     [f] "=v" ((_f)),                                                       \
     [h] "=v" ((_h))                                                        \
   : /* input list                                                       */ \
     [hptr] "r" ((_hptr))                                                   \
   : /* clobber list                                                     */ \
     "memory"                                                               \
  ); } while (0)                                                            \

#define DEQUE(_k, _kpw0, _kpw1) do {                                        \
  __asm__ volatile (                                                        \
    "vmr         %[kpw0], %[k]\n\t"          /* kpw0 = {k[0],k[1]}       */ \
    "vsldoi      %[kpw1], %[k], %[k], 8\n\t" /* kpw1 = {k[1],k[0]}       */ \
    : /* output list                                                     */ \
      [kpw0] "=v" ((_kpw0)),                                                \
      [kpw1] "=v" ((_kpw1)),                                                \
      /* k is read in the asm, hence should be reserved                  */ \
      [k] "+&v" ((_k))                                                      \
    : /* input list                                                      */ \
    : /* clobber list                                                    */ \
  ); } while (0)

#define LOAD_W_PLUS_K(_k0, _k1, _k2, _k3, _k4, _k5, _k6, _k7,               \
                      _w0, _w1, _w2, _w3, _w4, _w5, _w6, _w7,               \
                      _vRb, _j, _Rb, _k, _w) do {                           \
  base_type t0;                                                             \
  base_type t1;                                                             \
  vector_base_type vt0;                                                     \
  __asm__ volatile (                                                        \
    /* j * 8 (double word size)                                          */ \
    "sldi    %[t1],%[index],3\n\t"                                          \
    /* alias to W[j] location                                            */ \
    "add     %[t1],%[t1],%[wptr]\n\t"                                       \
    "addi    %[t0],%[t1],-128\n\t"                                          \
    /* unaligned load                                                    */ \
    "lvx     %[w0],0,%[t0]\n\t"                                             \
    /* set vrb according to alignment                                    */ \
    "lvsr    %[vrb],0,%[t1]\n\t"                                            \
    "addi    %[t0],%[t1],-112\n\t"                                          \
    /* unaligned load                                                    */ \
    "lvx     %[w1],0,%[t0]        \n\t"                                     \
    /* w0 = w[j-16] to w[j-15]                                           */ \
    "vperm   %[w0],%[w1],%[w0],%[vrb]        \n\t"                          \
    "addi    %[t0],%[t1],-96        \n\t"                                   \
    /* unaligned load                                                    */ \
    "lvx     %[w2],0,%[t0]        \n\t"                                     \
    /* w1 = w[j-14] to w[j-13]                                           */ \
    "vperm   %[w1],%[w2],%[w1],%[vrb]        \n\t"                          \
    "addi    %[t0],%[t1],-80        \n\t"                                   \
    /* unaligned load                                                    */ \
    "lvx     %[w3],0,%[t0]        \n\t"                                     \
    /* w2 = w[j-12] to w[j-11]                                           */ \
    "vperm   %[w2],%[w3],%[w2],%[vrb]        \n\t"                          \
    "addi    %[t0],%[t1],-64        \n\t"                                   \
    /* unaligned load                                                    */ \
    "lvx     %[w4],0,%[t0]        \n\t"                                     \
    /* w3 = w[j-10] to w[j-9]                                            */ \
    "vperm   %[w3],%[w4],%[w3],%[vrb]        \n\t"                          \
    "addi    %[t0],%[t1],-48        \n\t"                                   \
    /* unaligned load                                                    */ \
    "lvx     %[w5],0,%[t0]        \n\t"                                     \
    /* w4 = w[j-8] to w[j-7]                                             */ \
    "vperm   %[w4],%[w5],%[w4],%[vrb]        \n\t"                          \
    "addi    %[t0],%[t1],-32        \n\t"                                   \
    /* unaligned load                                                    */ \
    "lvx     %[w6],0,%[t0]        \n\t"                                     \
    /* w5 = w[j-6] to w[j-5]                                             */ \
    "vperm   %[w5],%[w6],%[w5],%[vrb]        \n\t"                          \
    "addi    %[t0],%[t1],-16        \n\t"                                   \
    /* unaligned load                                                    */ \
    "lvx     %[w7],0,%[t0]        \n\t"                                     \
    /* w6 = w[j-4] to w[j-3]                                             */ \
    "vperm   %[w6],%[w7],%[w6],%[vrb]        \n\t"                          \
    "addi    %[t0],%[t1],0        \n\t"                                     \
    /* unaligned load                                                    */ \
    "lvx     %[vt0],0,%[t0]        \n\t"                                    \
    /* w7 = w[j-2] to w[j-1]                                             */ \
    "vperm   %[w7],%[vt0],%[w7],%[vrb]        \n\t"                         \
    /* Load k values                                                     */ \
    /* j * 8 (word size)                                                 */ \
    "sldi    %[t1],%[index],3        \n\t"                                  \
    /* alias to k[j] location                                            */ \
    "add     %[t1],%[t1],%[kptr]        \n\t"                               \
    "addi    %[t0],%[t1],-128        \n\t"                                  \
    /* load k[j-16] to k[j-15] to vector                                 */ \
    "lvx     %[k0],0,%[t0]        \n\t"                                     \
    /* set vrb according to alignment                                    */ \
    "lvsr    %[vrb],0,%[t1]        \n\t"                                    \
    "addi    %[t0],%[t1],-112        \n\t"                                  \
    /* load k[j-14] to k[j-13] to vector                                 */ \
    "lvx     %[k1],0,%[t0]        \n\t"                                     \
    /* k0 = w[j-16] to w[j-15]                                           */ \
    "vperm   %[k0],%[k1],%[k0],%[vrb]        \n\t"                          \
    "addi    %[t0],%[t1],-96        \n\t"                                   \
    /* load k[j-12] to k[j-11] to vector                                 */ \
    "lvx     %[k2],0,%[t0]        \n\t"                                     \
    /* k1 = w[j-16] to w[j-15]                                           */ \
    "vperm   %[k1],%[k2],%[k1],%[vrb]        \n\t"                          \
    "addi    %[t0],%[t1],-80        \n\t"                                   \
    /* load k[j-10] to k[j-9] to vector                                  */ \
    "lvx     %[k3],0,%[t0]        \n\t"                                     \
    /* k1 = w[j-16] to w[j-15]                                           */ \
    "vperm   %[k2],%[k3],%[k2],%[vrb]        \n\t"                          \
    "addi    %[t0],%[t1],-64        \n\t"                                   \
    /* load k[j-8] to k[j-7] to vector                                   */ \
    "lvx     %[k4],0,%[t0]        \n\t"                                     \
    /* k1 = w[j-16] to w[j-15]                                           */ \
    "vperm   %[k3],%[k4],%[k3],%[vrb]        \n\t"                          \
    "addi    %[t0],%[t1],-48        \n\t"                                   \
    /* load k[j-6] to k[j-5] to vector                                   */ \
    "lvx     %[k5],0,%[t0]        \n\t"                                     \
    /* k1 = w[j-16] to w[j-15]                                           */ \
    "vperm   %[k4],%[k5],%[k4],%[vrb]        \n\t"                          \
    "addi    %[t0],%[t1],-32        \n\t"                                   \
    /* load k[j-4] to k[j-3] to vector                                   */ \
    "lvx     %[k6],0,%[t0]        \n\t"                                     \
    /* k1 = w[j-16] to w[j-15]                                           */ \
    "vperm   %[k5],%[k6],%[k5],%[vrb]        \n\t"                          \
    "addi    %[t0],%[t1],-16        \n\t"                                   \
    /* load k[j-2] to k[j-1] to vector                                   */ \
    "lvx     %[k7],0,%[t0]        \n\t"                                     \
    /* k1 = w[j-16] to w[j-15]                                           */ \
    "vperm   %[k6],%[k7],%[k6],%[vrb]        \n\t"                          \
    "addi    %[t0],%[t1],0        \n\t"                                     \
    /* load k[j-2] to k[j-1] to vector                                   */ \
    "lvx     %[vt0],0,%[t0]        \n\t"                                    \
    /* k1 = w[j-16] to w[j-15]                                           */ \
    "vperm   %[k7],%[vt0],%[k7],%[vrb]        \n\t"                         \
    /* calculate k+w                                                     */ \
    "vaddudm %[k0],%[k0],%[w0]\n\t"                                         \
    "vaddudm %[k1],%[k1],%[w1]\n\t"                                         \
    "vaddudm %[k2],%[k2],%[w2]\n\t"                                         \
    "vaddudm %[k3],%[k3],%[w3]\n\t"                                         \
    "vaddudm %[k4],%[k4],%[w4]\n\t"                                         \
    "vaddudm %[k5],%[k5],%[w5]\n\t"                                         \
    "vaddudm %[k6],%[k6],%[w6]\n\t"                                         \
    "vaddudm %[k7],%[k7],%[w7]\n\t"                                         \
    "lvsl    %[vrb],0,%[rb]\n\t"         /* parameter for vperm          */ \
    : /* output list                                                     */ \
      /* temporaries                                                     */ \
      [t0] "=&r" (t0),                                                      \
      [t1] "=&r" (t1),                                                      \
      /* actual outputs                                                  */ \
      [vrb] "=v" ((_vRb)),                                                  \
      [vt0] "=v" ((vt0)),                                                   \
      [w0] "=v" ((_w0)),                                                    \
      [w1] "=v" ((_w1)),                                                    \
      [w2] "=v" ((_w2)),                                                    \
      [w3] "=v" ((_w3)),                                                    \
      [w4] "=v" ((_w4)),                                                    \
      [w5] "=v" ((_w5)),                                                    \
      [w6] "=v" ((_w6)),                                                    \
      [w7] "=v" ((_w7)),                                                    \
      [k0] "=v" ((_k0)),                                                    \
      [k1] "=v" ((_k1)),                                                    \
      [k2] "=v" ((_k2)),                                                    \
      [k3] "=v" ((_k3)),                                                    \
      [k4] "=v" ((_k4)),                                                    \
      [k5] "=v" ((_k5)),                                                    \
      [k6] "=v" ((_k6)),                                                    \
      [k7] "=v" ((_k7))                                                     \
    : /* input list                                                      */ \
      [index] "r" ((_j)),                                                   \
      [rb] "r" ((_Rb)),                                                     \
      [wptr] "r" ((_w)),                                                    \
      [kptr] "r" ((_k))                                                     \
    : /* clobber list                                                    */ \
      "memory"                                                              \
  ); } while (0)

#define CALC_2W(_w0, _w1, _w2, _w3, _w4, _w5, _w6, _w7, _kpw0, _kpw1,       \
                _j, _vRb, _k) do {                                          \
  base_type t0;                                                             \
  base_type t1;                                                             \
  vector_base_type vt0;                                                     \
  vector_base_type vt1;                                                     \
  vector_base_type vt2;                                                     \
  vector_base_type vt3;                                                     \
  vector_base_type vt4;                                                     \
  __asm__ volatile (                                                        \
    "sldi       %[t1],%[index],%[c1]\n\t"      /* j * 8 (doubleword size)*/ \
    "add        %[t0],%[t1],%[kptr]\n\t"       /* alias to k[j] location */ \
    "lvx        %[vt3],0,%[t0]\n\t"                                         \
    "vperm      %[vt1],%[w1],%[w0],%[vrb]\n\t" /* vt1 = w[j-15] , w[j-14]*/ \
    "vperm      %[vt2],%[w5],%[w4],%[vrb]\n\t" /* vt2 = w[j-7] , w[j-6]  */ \
    /* vt1 = s0(w[j-15]) , s0(w[j-14])                                   */ \
    "vshasigmad %[vt1],%[vt1],0,0\n\t"                                      \
    /* vt4 = s1(w[j-2]) , s1(w[j-1])                                     */ \
    "vshasigmad %[vt4],%[w7],0,%[six1]\n\t"                                 \
    /* vt1 = s0(w[j-15]) + w[j-7] , s0(w[j-14]) + w[j-6]                 */ \
    "vaddudm    %[vt1],%[vt1],%[vt2]\n\t"                                   \
    /* vt0 = s1(w[j-2]) + w[j-16] , s1(w[j-1]) + w[j-15]                 */ \
    "vaddudm    %[vt0],%[vt4],%[w0]\n\t"                                    \
    /* vt1 = s0(w[j-15]) + w[j-7] + s1(w[j-2]) + w[j-16],  // w[j]       */ \
    /*       s0(w[j-14]) + w[j-6] + s1(w[j-1]) + w[j-15]   // w[j+1]     */ \
    "vaddudm    %[vt1],%[vt1],%[vt0]\n\t"                                   \
    /* Updating w0 to w7 to hold the "new previous" 16 values from w.    */ \
    "vor        %[w0],%[w1],%[w1]\n\t"                                      \
    "vor        %[w1],%[w2],%[w2]\n\t"                                      \
    "vor        %[w2],%[w3],%[w3]\n\t"                                      \
    "vor        %[w3],%[w4],%[w4]\n\t"                                      \
    "vor        %[w4],%[w5],%[w5]\n\t"                                      \
    "vor        %[w5],%[w6],%[w6]\n\t"                                      \
    "vor        %[w6],%[w7],%[w7]\n\t"                                      \
    "vor        %[w7],%[vt1],%[vt1]\n\t"                                    \
    /* store k + w to vt1 (2 values at once)                             */ \
    "vaddudm    %[vt1],%[vt1],%[vt3]\n\t"                                   \
    /* Deque values in vt1 to kpw0 and kpw1                              */ \
    "vmr        %[kpw0], %[vt1]\n\t"                                        \
    "vsldoi     %[kpw1], %[vt1],%[vt1], 8\n\t"                              \
  : /* output list                                                       */ \
    /* temporaries                                                       */ \
    [t0] "=&r" (t0),                                                        \
    [t1] "=&r" (t1),                                                        \
    [vt0] "=&v" (vt0),                                                      \
    [vt1] "=&v" (vt1),                                                      \
    [vt2] "=&v" (vt2),                                                      \
    [vt3] "=&v" (vt3),                                                      \
    [vt4] "=&v" (vt4),                                                      \
    /* outputs/inputs                                                    */ \
    [w0] "+v" ((_w0)),                                                      \
    [w1] "+v" ((_w1)),                                                      \
    [w2] "+v" ((_w2)),                                                      \
    [w3] "+v" ((_w3)),                                                      \
    [w4] "+v" ((_w4)),                                                      \
    [w5] "+v" ((_w5)),                                                      \
    [w6] "+v" ((_w6)),                                                      \
    [w7] "+v" ((_w7)),                                                      \
    /* actual outputs                                                    */ \
    [kpw0] "=v" ((_kpw0)),                                                  \
    [kpw1] "=v" ((_kpw1))                                                   \
  : /* input list                                                        */ \
    [index] "r" ((_j)),                                                     \
    [kptr] "r" ((_k)),                                                      \
    [vrb] "v" ((_vRb)),                                                     \
    [c1] "i" (3),                                                           \
    [six1] "i" (0xf)                                                        \
  : /* clobber list                                                      */ \
    "memory"                                                                \
  ); } while (0)

#define UPDATE_SHA_STATE(_a, _b, _c, _d, _e, _f, _g, _h, _hptr) do {        \
  vector_base_type vt0;                                                     \
  vector_base_type vt1;                                                     \
  vector_base_type vt2;                                                     \
  vector_base_type vt3;                                                     \
  vector_base_type vt4;                                                     \
  vector_base_type vt5;                                                     \
  vector_base_type vt6;                                                     \
  vector_base_type vt7;                                                     \
  vector_base_type vrb;                                                     \
  vector_base_type vtmp0;                                                   \
  base_type rtmp0;                                                          \
  __asm__ volatile(                                                         \
      "lvsr    %[vrb],0,%[hptr]\n\t"                                        \
      "lvx     %[vt0],0,%[hptr]\n\t"        /* vt0 = {_h[0], _h[1]}      */ \
      "lvx     %[vt1],%[offs1],%[hptr]\n\t" /* vt1 = {_h[2], _h[3]}      */ \
      "vperm   %[vt0],%[vt1],%[vt0],%[vrb]\n\t"                             \
      "lvx     %[vt2],%[offs2],%[hptr]\n\t"/*                            */ \
      "vperm   %[vt1],%[vt2],%[vt1],%[vrb]\n\t"                             \
      "xxmrgld %x[vt4],%x[b],%x[a]\n\t"     /* vt4 = {a, b}              */ \
      "xxmrgld %x[vt5],%x[d],%x[c]\n\t"     /* vt5 = {c, d}              */ \
      "xxmrgld %x[vt6],%x[f],%x[e]\n\t"     /* vt6 = {e, f}              */ \
      "xxmrgld %x[vt7],%x[h],%x[g]\n\t"     /* vt7 = {g, h}              */ \
      "lvx     %[vt3],%[offs3],%[hptr]\n\t" /* vt3 = {_h[6], _h[7]}      */ \
      "vperm   %[vt2],%[vt3],%[vt2],%[vrb]\n\t"                             \
      "lvx     %[vtmp0],%[offs4],%[hptr]\n\t"/*                          */ \
      "vperm   %[vt3],%[vtmp0],%[vt3],%[vrb]\n\t"                           \
      "vaddudm %[vt0],%[vt0],%[vt4]\n\t"    /* vt0 = {_h[0]+a, _h[1]+b}  */ \
      "vaddudm %[vt1],%[vt1],%[vt5]\n\t"    /* vt1 = {_h[2]+c, _h[3]+d}  */ \
      "vaddudm %[vt2],%[vt2],%[vt6]\n\t"    /* vt2 = {_h[4]+e, _h[5]+f}  */ \
      "vaddudm %[vt3],%[vt3],%[vt7]\n\t"    /* vt3 = {_h[6]+g, _h[7]+h}  */ \
      "mfvrd   %[rtmp0],%[vt0]\n\t"                                         \
      "std     %[rtmp0],8(%[hptr])\n\t"                                     \
      "vsldoi  %[vtmp0],%[vt0],%[vt0],8\n\t"                                \
      "mfvrd   %[rtmp0],%[vtmp0]\n\t"                                       \
      "std     %[rtmp0],0(%[hptr])\n\t"                                     \
      "mfvrd   %[rtmp0],%[vt1]\n\t"                                         \
      "std     %[rtmp0],24(%[hptr])\n\t"                                    \
      "vsldoi  %[vtmp0],%[vt1],%[vt1],8\n\t"                                \
      "mfvrd   %[rtmp0],%[vtmp0]\n\t"                                       \
      "std     %[rtmp0],16(%[hptr])\n\t"                                    \
      "mfvrd   %[rtmp0],%[vt2]\n\t"                                         \
      "std     %[rtmp0],40(%[hptr])\n\t"                                    \
      "vsldoi  %[vtmp0],%[vt2],%[vt2],8\n\t"                                \
      "mfvrd   %[rtmp0],%[vtmp0]\n\t"                                       \
      "std     %[rtmp0],32(%[hptr])\n\t"                                    \
      "mfvrd   %[rtmp0],%[vt3]\n\t"                                         \
      "std     %[rtmp0],56(%[hptr])\n\t"                                    \
      "vsldoi  %[vtmp0],%[vt3],%[vt3],8\n\t"                                \
      "mfvrd   %[rtmp0],%[vtmp0]\n\t"                                       \
      "std     %[rtmp0],48(%[hptr])\n\t"                                    \
    : /* output list                                                     */ \
      /* temporaries                                                     */ \
      [vt0] "=&v" (vt0),                                                    \
      [vt1] "=&v" (vt1),                                                    \
      [vt2] "=&v" (vt2),                                                    \
      [vt3] "=&v" (vt3),                                                    \
      [vt4] "=&v" (vt4),                                                    \
      [vt5] "=&v" (vt5),                                                    \
      [vt6] "=&v" (vt6),                                                    \
      [vt7] "=&v" (vt7),                                                    \
      [vrb] "=&v" (vrb),                                                    \
      [vtmp0] "=&v" (vtmp0),                                                \
      [rtmp0] "=&r" ((rtmp0))                                               \
    : /* input list                                                      */ \
      [hptr] "r" ((_hptr)),                                                 \
      [a] "v" ((_a)),                                                       \
      [b] "v" ((_b)),                                                       \
      [c] "v" ((_c)),                                                       \
      [d] "v" ((_d)),                                                       \
      [e] "v" ((_e)),                                                       \
      [f] "v" ((_f)),                                                       \
      [g] "v" ((_g)),                                                       \
      [h] "v" ((_h)),                                                       \
      [offs1] "r" (16),                                                     \
      [offs2] "r" (32),                                                     \
      [offs3] "r" (48),                                                     \
      [offs4] "r" (64)                                                      \
    : /* clobber list                                                    */ \
      "memory"                                                              \
  ); } while (0)

#endif

void sha2_transform(base_type* _h, base_type* w) {
  vector_base_type a, b, c, d, e, f, g, h;
  int Rb = 8; /* Parameter for lvsl */
  vector int vRb;

#if SHA_BITS == 256

  vector_base_type w0, w1, w2, w3;
  vector_base_type kplusw0, kplusw1, kplusw2, kplusw3;
  vector_base_type kpw0, kpw1, kpw2, kpw3;

  int Rc = 4; /* Parameter for lvsl */
  vector int vRc;
  int j = 16;

  LOAD_H_VEC(a, b, c, d, e, f, g, h, _h, vRb);

  // Load 16 elements from w out of the loop
  LOAD_W_PLUS_K(kplusw0, kplusw1, kplusw2, kplusw3, w0, w1, w2, w3, vRb, vRc,
                j, Rb, Rc, w, k);

  // Loop unrolling, from 0 to 15
  DEQUE  (kplusw0, kpw0, kpw1, kpw2, kpw3);
  SHA2_ROUND(a, b, c, d, e, f, g, h, kpw0);
  SHA2_ROUND(h, a, b, c, d, e, f, g, kpw1);
  SHA2_ROUND(g, h, a, b, c, d, e, f, kpw2);
  SHA2_ROUND(f, g, h, a, b, c, d, e, kpw3);

  DEQUE  (kplusw1, kpw0, kpw1, kpw2, kpw3);
  SHA2_ROUND(e, f, g, h, a, b, c, d, kpw0);
  SHA2_ROUND(d, e, f, g, h, a, b, c, kpw1);
  SHA2_ROUND(c, d, e, f, g, h, a, b, kpw2);
  SHA2_ROUND(b, c, d, e, f, g, h, a, kpw3);

  DEQUE  (kplusw2, kpw0, kpw1, kpw2, kpw3);
  SHA2_ROUND(a, b, c, d, e, f, g, h, kpw0);
  SHA2_ROUND(h, a, b, c, d, e, f, g, kpw1);
  SHA2_ROUND(g, h, a, b, c, d, e, f, kpw2);
  SHA2_ROUND(f, g, h, a, b, c, d, e, kpw3);

  DEQUE  (kplusw3, kpw0, kpw1, kpw2, kpw3);
  SHA2_ROUND(e, f, g, h, a, b, c, d, kpw0);
  SHA2_ROUND(d, e, f, g, h, a, b, c, kpw1);
  SHA2_ROUND(c, d, e, f, g, h, a, b, kpw2);
  SHA2_ROUND(b, c, d, e, f, g, h, a, kpw3);

  // From 16 to W_SIZE (64) in 8 steps
  while (j < W_SIZE) {
    CALC_4W(w0, w1, w2, w3, kpw0, kpw1, kpw2, kpw3, j, vRb, vRc, k);
    SHA2_ROUND(a, b, c, d, e, f, g, h, kpw0);
    SHA2_ROUND(h, a, b, c, d, e, f, g, kpw1);
    SHA2_ROUND(g, h, a, b, c, d, e, f, kpw2);
    SHA2_ROUND(f, g, h, a, b, c, d, e, kpw3);
    j += 4;

    CALC_4W(w0, w1, w2, w3, kpw0, kpw1, kpw2, kpw3, j, vRb, vRc, k);
    SHA2_ROUND(e, f, g, h, a, b, c, d, kpw0);
    SHA2_ROUND(d, e, f, g, h, a, b, c, kpw1);
    SHA2_ROUND(c, d, e, f, g, h, a, b, kpw2);
    SHA2_ROUND(b, c, d, e, f, g, h, a, kpw3);
    j += 4;
  }

#else // SHA_BITS == 512

  vector_base_type w0, w1, w2, w3, w4, w5, w6, w7;
  vector_base_type kplusw0, kplusw1, kplusw2, kplusw3, kplusw4, kplusw5,
                   kplusw6, kplusw7;
  vector_base_type kpw0, kpw1;

  int j = 16;

  LOAD_H_VEC(a, b, c, d, e, f, g, h, _h, vRb);

  // Load 16 elements from w out of the loop
  LOAD_W_PLUS_K(kplusw0, kplusw1, kplusw2, kplusw3, kplusw4, kplusw5, kplusw6,
                kplusw7, w0, w1, w2, w3, w4, w5, w6, w7, vRb, j, Rb, k, w);

  // Loop unrolling, from 0 to 15
  DEQUE(kplusw0, kpw0, kpw1);
  SHA2_ROUND(a, b, c, d, e, f, g, h, kpw0);
  SHA2_ROUND(h, a, b, c, d, e, f, g, kpw1);

  DEQUE(kplusw1, kpw0, kpw1);
  SHA2_ROUND(g, h, a, b, c, d, e, f, kpw0);
  SHA2_ROUND(f, g, h, a, b, c, d, e, kpw1);

  DEQUE(kplusw2, kpw0, kpw1);
  SHA2_ROUND(e, f, g, h, a, b, c, d, kpw0);
  SHA2_ROUND(d, e, f, g, h, a, b, c, kpw1);

  DEQUE(kplusw3, kpw0, kpw1);
  SHA2_ROUND(c, d, e, f, g, h, a, b, kpw0);
  SHA2_ROUND(b, c, d, e, f, g, h, a, kpw1);

  DEQUE(kplusw4, kpw0, kpw1);
  SHA2_ROUND(a, b, c, d, e, f, g, h, kpw0);
  SHA2_ROUND(h, a, b, c, d, e, f, g, kpw1);

  DEQUE(kplusw5, kpw0, kpw1);
  SHA2_ROUND(g, h, a, b, c, d, e, f, kpw0);
  SHA2_ROUND(f, g, h, a, b, c, d, e, kpw1);

  DEQUE(kplusw6, kpw0, kpw1);
  SHA2_ROUND(e, f, g, h, a, b, c, d, kpw0);
  SHA2_ROUND(d, e, f, g, h, a, b, c, kpw1);

  DEQUE(kplusw7, kpw0, kpw1);
  SHA2_ROUND(c, d, e, f, g, h, a, b, kpw0);
  SHA2_ROUND(b, c, d, e, f, g, h, a, kpw1);


  // From 16 to W_SIZE (80) in 8 steps
  while (j < W_SIZE) {
    CALC_2W(w0, w1, w2, w3, w4, w5, w6, w7, kpw0, kpw1, j, vRb, k);
    SHA2_ROUND(a, b, c, d, e, f, g, h, kpw0);
    SHA2_ROUND(h, a, b, c, d, e, f, g, kpw1);
    j += 2;

    CALC_2W(w0, w1, w2, w3, w4, w5, w6, w7, kpw0, kpw1, j, vRb, k);
    SHA2_ROUND(g, h, a, b, c, d, e, f, kpw0);
    SHA2_ROUND(f, g, h, a, b, c, d, e, kpw1);
    j += 2;

    CALC_2W(w0, w1, w2, w3, w4, w5, w6, w7, kpw0, kpw1, j, vRb, k);
    SHA2_ROUND(e, f, g, h, a, b, c, d, kpw0);
    SHA2_ROUND(d, e, f, g, h, a, b, c, kpw1);
    j += 2;

    CALC_2W(w0, w1, w2, w3, w4, w5, w6, w7, kpw0, kpw1, j, vRb, k);
    SHA2_ROUND(c, d, e, f, g, h, a, b, kpw0);
    SHA2_ROUND(b, c, d, e, f, g, h, a, kpw1);
    j += 2;
  }
#endif

  UPDATE_SHA_STATE(a, b, c, d, e, f, g, h, _h);
}

#endif // _PPC64_LE_SHA2_LL_ASM_H_
