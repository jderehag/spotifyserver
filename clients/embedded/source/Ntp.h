/*
 * Ntp.h
 *
 *  Created on: 9 Mar 2014
 *      Author: Jesse
 */

#ifndef NTP_H_
#define NTP_H_

#include <stdint.h>

typedef uint32_t u_fp;
typedef int32_t s_fp;
typedef unsigned char u_char;
typedef signed char s_char;
typedef unsigned long u_long;

typedef struct {
    union {
        uint32_t Xl_ui;
        int32_t Xl_i;
    } Ul_i;
    union {
        uint32_t Xl_uf;
        int32_t Xl_f;
    } Ul_f;
} l_fp;

#define l_ui    Ul_i.Xl_ui      /* unsigned integral part */
#define l_i Ul_i.Xl_i       /* signed integral part */
#define l_uf    Ul_f.Xl_uf      /* unsigned fractional part */
#define l_f Ul_f.Xl_f       /* signed fractional part */

struct pkt {
    u_char  li_vn_mode; /* peer leap indicator */
    u_char  stratum;    /* peer stratum */
    u_char  ppoll;      /* peer poll interval */
    s_char  precision;  /* peer clock precision */
    u_fp    rootdelay;  /* roundtrip delay to primary source */
    u_fp    rootdisp;   /* dispersion to primary source*/
    uint32_t    refid;      /* reference id */
    l_fp    reftime;    /* last update time */
    l_fp    org;        /* originate time stamp */
    l_fp    rec;        /* receive time stamp */
    l_fp    xmt;        /* transmit time stamp */
};

#define NTOHL_FP(n, h)  do { (h)->l_ui = Ntohl((n)->l_ui); \
                 (h)->l_uf = Ntohl((n)->l_uf); } while (0)


/*
 * Stuff for extracting things from li_vn_mode
 */
#define PKT_MODE(li_vn_mode)    ((u_char)((li_vn_mode) & 0x7))
#define PKT_VERSION(li_vn_mode) ((u_char)(((li_vn_mode) >> 3) & 0x7))
#define PKT_LEAP(li_vn_mode)    ((u_char)(((li_vn_mode) >> 6) & 0x3))

/*
 * Stuff for putting things back into li_vn_mode
 */
#define PKT_LI_VN_MODE(li, vn, md) \
    ((u_char)((((li) << 6) & 0xc0) | (((vn) << 3) & 0x38) | ((md) & 0x7)))

#define FRAC        4294967296.     /* 2^32 as a double */

#endif /* NTP_H_ */
