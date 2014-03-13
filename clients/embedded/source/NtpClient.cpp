/*
 * Copyright (c) 2014, Jens Nielsen
 * All rights reserved.

 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of the <organization> nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL JENS NIELSEN BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "NtpClient.h"
#include "Ntp.h"
#include "Platform/Socket/Socket.h"
#include "Platform/Utils/Utils.h"
#include "applog.h"
#include "clock.h"

#include "FreeRTOS.h"
#include "task.h"

#include <string.h>
#include <tgmath.h>

static void
clock_filter(
    struct peer *peer,      /* peer structure pointer */
    double  sample_offset,      /* clock offset */
    double  sample_delay,       /* roundtrip delay */
    double  sample_disp     /* dispersion */
    );

NtpClient::NtpClient() : Platform::Runnable(false, SIZE_MEDIUM, PRIO_HIGH)
{
    startThread();
}

NtpClient::~NtpClient()
{
}



#define MAXDISPERSE 16. /* max dispersion */
#define NTP_SHIFT   8   /* clock filter stages */
#define NTP_FWEIGHT .5  /* clock filter weight */



struct peer {
    u_char  version;    /* version number */
    u_char  hmode;      /* local association mode */
    u_char  hpoll;      /* local poll interval */
    u_char  minpoll;    /* min poll interval */
    u_char  maxpoll;    /* max poll interval */
    u_int   flags;      /* association flags */
    u_char  cast_flags; /* additional flags */
    u_char  last_event; /* last peer error code */
    u_char  num_events; /* number of error events */
    u_char  ttl;        /* ttl/refclock mode */

    /*
     * Variables used by reference clock support
     */
#ifdef REFCLOCK
    struct refclockproc *procptr; /* refclock structure pointer */
    u_char  refclktype; /* reference clock type */
    u_char  refclkunit; /* reference clock unit number */
    u_char  sstclktype; /* clock type for system status word */
#endif /* REFCLOCK */

    /*
     * Variables set by received packet
     */
    u_char  leap;       /* local leap indicator */
    u_char  pmode;      /* remote association mode */
    u_char  stratum;    /* remote stratum */
    u_char  ppoll;      /* remote poll interval */
    s_char  precision;  /* remote clock precision */
    double  rootdelay;  /* roundtrip delay to primary source */
    double  rootdisp;   /* dispersion to primary source */
    uint32_t    refid;      /* remote reference ID */
    l_fp    reftime;    /* update epoch */

    /*
     * Variables used by authenticated client
     */
    //keyid_t keyid;        /* current key ID */
#ifdef OPENSSL
#define clear_to_zero opcode
    u_int32 opcode;     /* last request opcode */
    associd_t assoc;    /* peer association ID */
    u_int32 crypto;     /* peer status word */
    EVP_PKEY *pkey;     /* public key */
    const EVP_MD *digest;   /* message digest algorithm */
    char    *subject;   /* certificate subject name */
    char    *issuer;    /* certificate issuer name */
    struct cert_info *xinfo; /* issuer certificate */
    keyid_t pkeyid;     /* previous key ID */
    keyid_t hcookie;    /* host cookie */
    keyid_t pcookie;    /* peer cookie */
    const struct pkey_info *ident_pkey; /* identity key */
    BIGNUM  *iffval;    /* identity challenge (IFF, GQ, MV) */
    const BIGNUM *grpkey;   /* identity challenge key (GQ) */
    struct value cookval;   /* receive cookie values */
    struct value recval;    /* receive autokey values */
    struct exten *cmmd; /* extension pointer */
    u_long  refresh;    /* next refresh epoch */

    /*
     * Variables used by authenticated server
     */
    keyid_t *keylist;   /* session key ID list */
    int keynumber;  /* current key number */
    struct value encrypt;   /* send encrypt values */
    struct value sndval;    /* send autokey values */
#else /* OPENSSL */
#define clear_to_zero status
#endif /* OPENSSL */

    /*
     * Ephemeral state variables
     */
    u_char  status;     /* peer status */
    u_char  new_status; /* under-construction status */
    u_char  reach;      /* reachability register */
    int flash;      /* protocol error test tally bits */
    u_long  epoch;      /* reference epoch */
    int burst;      /* packets remaining in burst */
    int retry;      /* retry counter */
    int flip;       /* interleave mode control */
    int filter_nextpt;  /* index into filter shift register */
    double  filter_delay[NTP_SHIFT]; /* delay shift register */
    double  filter_offset[NTP_SHIFT]; /* offset shift register */
    double  filter_disp[NTP_SHIFT]; /* dispersion shift register */
    u_long  filter_epoch[NTP_SHIFT]; /* epoch shift register */
    u_char  filter_order[NTP_SHIFT]; /* filter sort index */
    l_fp    rec;        /* receive time stamp */
    l_fp    xmt;        /* transmit time stamp */
    l_fp    dst;        /* destination timestamp */
    l_fp    aorg;       /* origin timestamp */
    l_fp    borg;       /* alternate origin timestamp */
    double  offset;     /* peer clock offset */
    double  delay;      /* peer roundtrip delay */
    double  jitter;     /* peer jitter (squares) */
    double  disp;       /* peer dispersion */
    double  xleave;     /* interleave delay */
    double  bias;       /* bias for NIC asymmetry */

    /*
     * Variables used to correct for packet length and asymmetry.
     */
    double  t21;        /* outbound packet delay */
    int t21_bytes;  /* outbound packet length */
    int t21_last;   /* last outbound packet length */
    double  r21;        /* outbound data rate */
    double  t34;        /* inbound packet delay */
    int t34_bytes;  /* inbound packet length */
    double  r34;        /* inbound data rate */

    /*
     * End of clear-to-zero area
     */
    u_long  update;     /* receive epoch */
#define end_clear_to_zero update
    int unreach;    /* watchdog counter */
    int throttle;   /* rate control */
    u_long  outdate;    /* send time last packet */
    u_long  nextdate;   /* send time next packet */
    u_long  nextaction; /* peer local activity timeout (refclocks) */
    void (*action) (struct peer *); /* action timeout function */

    /*
     * Statistic counters
     */
    u_long  timereset;  /* time stat counters were reset */
    u_long  timereceived;   /* last packet received time */
    u_long  timereachable;  /* last reachable/unreachable time */

    u_long  sent;       /* packets sent */
    u_long  received;   /* packets received */
    u_long  processed;  /* packets processed */
    u_long  badauth;    /* bad authentication (TEST5) */
    u_long  bogusorg;   /* bogus origin (TEST2, TEST3) */
    u_long  oldpkt;     /* old duplicate (TEST1) */
    u_long  seldisptoolarge; /* bad header (TEST6, TEST7) */
    u_long  selbroken;  /* KoD received */
};



#define M_SUB(r_i, r_f, a_i, a_f)   /* r -= a */ \
    do { \
        register uint32_t lo_tmp; \
        register uint32_t hi_tmp; \
        \
        if ((a_f) == 0) { \
            (r_i) -= (a_i); \
        } else { \
            lo_tmp = ((r_f) & 0xffff) + ((-((s_fp)(a_f))) & 0xffff); \
            hi_tmp = (((r_f) >> 16) & 0xffff) \
                + (((-((s_fp)(a_f))) >> 16) & 0xffff); \
            if (lo_tmp & 0x10000) \
                hi_tmp++; \
            (r_f) = ((hi_tmp & 0xffff) << 16) | (lo_tmp & 0xffff); \
            \
            (r_i) += ~(a_i); \
            if (hi_tmp & 0x10000) \
                (r_i)++; \
        } \
    } while (0)
#define L_SUB(r, a) M_SUB((r)->l_ui, (r)->l_uf, (a)->l_ui, (a)->l_uf)


#define M_NEG(v_i, v_f)     /* v = -v */ \
    do { \
        if ((v_f) == 0) \
            (v_i) = -((s_fp)(v_i)); \
        else { \
            (v_f) = -((s_fp)(v_f)); \
            (v_i) = ~(v_i); \
        } \
    } while(0)

#define M_LFPTOD(r_i, r_uf, d)          /* l_fp to double */ \
    do { \
        register l_fp l_tmp; \
        \
        l_tmp.l_i = (r_i); \
        l_tmp.l_f = (r_uf); \
        if (l_tmp.l_i < 0) { \
            M_NEG(l_tmp.l_i, l_tmp.l_uf); \
            (d) = -((double)l_tmp.l_i + ((double)l_tmp.l_uf) / FRAC); \
        } else { \
            (d) = (double)l_tmp.l_i + ((double)l_tmp.l_uf) / FRAC; \
        } \
    } while (0)
#define LFPTOD(v, d)    M_LFPTOD((v)->l_ui, (v)->l_uf, (d))

#define LOGTOD(a)   ((a) < 0 ? 1. / (1L << -(a)) : \
                1L << (int)(a)) /* log2 to double */

#define ULOGTOD(a)  (1L << (int)(a)) /* ulog2 to double */


static s_char   sys_precision = -10;        /* local clock precision (log2 s) << set to ms precision */
static double   clock_phi = 15e-6;  /* dispersion rate (s/s) */

void NtpClient::run()
{
    uint8_t buf[48];
    struct peer peer;
    double  t34, t21;
    double  p_offset, p_del, p_disp;
    l_fp    p_rec, p_xmt, p_org, ci;
    bool timeSet = false;

    memset( &peer, 0, sizeof(struct peer) );

    while( !isCancellationPending() )
    {
        Socket s( SOCKTYPE_DATAGRAM );
        std::set<Socket*> readset;
        bool success = false;

        //log( LOG_NOTICE ) << "NTP: Updating time";

        if ( s.Connect( /*"pool.ntp.org"*/"time.windows.com", "123" ) == 0 )
        {
            memset( buf, 0, sizeof(buf) );
//            buf[0] = 0x23;
            struct pkt* pkt = (struct pkt*)buf;
            pkt->li_vn_mode = 0x23; // no warning, version 4, I'm a client

            if ( timeSet )
            {
                uint32_t frac = xTaskGetTickCount() % 1000;
                frac = (FRAC / 1000) * frac;
                pkt->xmt.l_ui = Htonl(getTime() + 2208988800UL);
                pkt->xmt.l_uf = Htonl(frac);
            }
            if ( s.Send( buf, sizeof(buf) ) == sizeof( buf ) )
            {
                readset.insert( &s );
                if ( select( &readset, NULL, NULL, 5000 ) == 1 )
                {
                    if ( s.Receive( buf, sizeof(buf) ) == sizeof(buf) )
                    {
                        //uint32_t ntptime;
                        pkt = (struct pkt*)buf;

                        peer.processed++;
                        NTOHL_FP(&pkt->org, &p_org);
                        NTOHL_FP(&pkt->rec, &p_rec);
                        NTOHL_FP(&pkt->xmt, &p_xmt);

                        if ( timeSet == false )
                        {
                            /* convert to 1970 epoch and set */
                            /* todo some day maybe:
                             * this is directly towards the embedded client wall clock implementation,
                             * since there is no platform agnostic way to set time and it's a PITA to
                             * handle all the different types used to define time..
                             */
                            setTime( p_xmt.Ul_i.Xl_ui - 2208988800UL );
                            timeSet = true;
                            /* I guess would be something like this for linux
                            tv.tv_sec = ntptime - 2208988800UL;
                            tv.tv_usec = the next 4 bytes;
                            settimeofday( &tv, NULL );*/
                        }
                        else
                        {
                            l_fp t4;
                            t4.Ul_i.Xl_ui = getTime() + 2208988800UL;
                            t4.Ul_f.Xl_uf = xTaskGetTickCount() % 1000;
                            t4.Ul_f.Xl_uf = (FRAC / 1000) * t4.Ul_f.Xl_uf;
                            ci = p_xmt;             /* t3 - t4 */
                            L_SUB(&ci, &t4); //L_SUB(&ci, &peer.dst);
                            LFPTOD(&ci, t34);
                            ci = p_rec;             /* t2 - t1 */
                            L_SUB(&ci, &p_org);
                            LFPTOD(&ci, t21);
                            p_del = fabs(t21 - t34);
                            p_offset = (t21 + t34) / 2.;

                            p_offset += peer.bias;
                            p_disp = LOGTOD(sys_precision) + LOGTOD(pkt->precision) +
                            clock_phi * p_del;

                            clock_filter(&peer, p_offset, p_del, p_disp);
                        }
                        log( LOG_NOTICE ) << "NTP: Success";

                        success = true;
                    }
                    else
                    {
                        log( LOG_NOTICE ) << "NTP: Failed to receive";
                    }
                }
                else
                {
                    //log( LOG_NOTICE ) << "NTP: Nothing on select";
                }
            }
            else
            {
                log( LOG_NOTICE ) << "NTP: Failed to send";
            }
        }
        else
        {
            log( LOG_NOTICE ) << "Failed to connect";
        }

        sleep_ms(4700);//success ? 39600000 : 30*60*1000); //get time every 11 hours, or try again in 30 minutes if failed
    }
}

#define SQUARE(x) ((x) * (x))
#define SQRT(x) (sqrt(x))
#define DIFF(x, y) (SQUARE((x) - (y)))
#define min(a,b)    (((a) < (b)) ? (a) : (b))
#define max(a,b)    (((a) > (b)) ? (a) : (b))
#define min3(a,b,c) min(min((a),(b)), (c))


/*
 * clock_filter - add incoming clock sample to filter register and run
 *        the filter procedure to find the best sample.
 */
static u_char   allan_xpt = 11; /* Allan intercept (log2 s) */
static double   sys_maxdist = 1.5; /* selection threshold */
#define CLOCK_SGATE 3.  /* popcorn spike gate */


void
clock_filter(
    struct peer *peer,      /* peer structure pointer */
    double  sample_offset,      /* clock offset */
    double  sample_delay,       /* roundtrip delay */
    double  sample_disp     /* dispersion */
    )
{
    double  dst[NTP_SHIFT];     /* distance vector */
    int ord[NTP_SHIFT];     /* index vector */
    int i, j, k, m;
    double  dtemp, etemp;

    u_long current_time = getTime() + 2208988800UL;

    /*
     * A sample consists of the offset, delay, dispersion and epoch
     * of arrival. The offset and delay are determined by the on-
     * wire protocol. The dispersion grows from the last outbound
     * packet to the arrival of this one increased by the sum of the
     * peer precision and the system precision as required by the
     * error budget. First, shift the new arrival into the shift
     * register discarding the oldest one.
     */
    j = peer->filter_nextpt;
    peer->filter_offset[j] = sample_offset;
    peer->filter_delay[j] = sample_delay;
    peer->filter_disp[j] = sample_disp;
    peer->filter_epoch[j] = current_time;
    j = (j + 1) % NTP_SHIFT;
    peer->filter_nextpt = j;

    /*
     * Update dispersions since the last update and at the same
     * time initialize the distance and index lists. Since samples
     * become increasingly uncorrelated beyond the Allan intercept,
     * only under exceptional cases will an older sample be used.
     * Therefore, the distance list uses a compound metric. If the
     * dispersion is greater than the maximum dispersion, clamp the
     * distance at that value. If the time since the last update is
     * less than the Allan intercept use the delay; otherwise, use
     * the sum of the delay and dispersion.
     */
    dtemp = clock_phi * (current_time - peer->update);
    peer->update = current_time;
    for (i = NTP_SHIFT - 1; i >= 0; i--) {
        if (i != 0)
            peer->filter_disp[j] += dtemp;
        if (peer->filter_disp[j] >= MAXDISPERSE) {
            peer->filter_disp[j] = MAXDISPERSE;
            dst[i] = MAXDISPERSE;
        } else if (peer->update - peer->filter_epoch[j] >
            ULOGTOD(allan_xpt)) {
            dst[i] = peer->filter_delay[j] +
                peer->filter_disp[j];
        } else {
            dst[i] = peer->filter_delay[j];
        }
        ord[i] = j;
        j = (j + 1) % NTP_SHIFT;
    }

    /*
     * If the clock discipline has stabilized, sort the samples by
     * distance.
     */
    /*if ( sys_leap != LEAP_NOTINSYNC)*/ {
        for (i = 1; i < NTP_SHIFT; i++) {
            for (j = 0; j < i; j++) {
                if (dst[j] > dst[i]) {
                    k = ord[j];
                    ord[j] = ord[i];
                    ord[i] = k;
                    etemp = dst[j];
                    dst[j] = dst[i];
                    dst[i] = etemp;
                }
            }
        }
    }

    /*
     * Copy the index list to the association structure so ntpq
     * can see it later. Prune the distance list to leave only
     * samples less than the maximum dispersion, which disfavors
     * uncorrelated samples older than the Allan intercept. To
     * further improve the jitter estimate, of the remainder leave
     * only samples less than the maximum distance, but keep at
     * least two samples for jitter calculation.
     */
    m = 0;
    for (i = 0; i < NTP_SHIFT; i++) {
        peer->filter_order[i] = (u_char) ord[i];
        if (dst[i] >= MAXDISPERSE || (m >= 2 && dst[i] >=
            sys_maxdist))
            continue;
        m++;
    }

    /*
     * Compute the dispersion and jitter. The dispersion is weighted
     * exponentially by NTP_FWEIGHT (0.5) so it is normalized close
     * to 1.0. The jitter is the RMS differences relative to the
     * lowest delay sample.
     */
    peer->disp = peer->jitter = 0;
    k = ord[0];
    for (i = NTP_SHIFT - 1; i >= 0; i--) {
        j = ord[i];
        peer->disp = NTP_FWEIGHT * (peer->disp +
            peer->filter_disp[j]);
        if (i < m)
            peer->jitter += DIFF(peer->filter_offset[j],
                peer->filter_offset[k]);
    }

    /*
     * If no acceptable samples remain in the shift register,
     * quietly tiptoe home leaving only the dispersion. Otherwise,
     * save the offset, delay and jitter. Note the jitter must not
     * be less than the precision.
     */
    if (m == 0) {
        //clock_select();
        log(LOG_NOTICE) << "no acceptable samples";
        return;
    }

    etemp = fabs(peer->offset - peer->filter_offset[k]);
    peer->offset = peer->filter_offset[k];
    peer->delay = peer->filter_delay[k];
    if (m > 1)
        peer->jitter /= m - 1;
    peer->jitter = max(SQRT(peer->jitter), LOGTOD(sys_precision));

    /*
     * If the the new sample and the current sample are both valid
     * and the difference between their offsets exceeds CLOCK_SGATE
     * (3) times the jitter and the interval between them is less
     * than twice the host poll interval, consider the new sample
     * a popcorn spike and ignore it.
     */
    if (peer->disp < sys_maxdist && peer->filter_disp[k] <
        sys_maxdist && etemp > CLOCK_SGATE * peer->jitter &&
        peer->filter_epoch[k] - peer->epoch < 2. *
        ULOGTOD(peer->hpoll)) {
        /*snprintf(tbuf, sizeof(tbuf), "%.6f s", etemp);
        report_event(PEVNT_POPCORN, peer, tbuf);*/
        log(LOG_NOTICE) << "something about popcorn";
        return;
    }

    /*
     * A new minimum sample is useful only if it is later than the
     * last one used. In this design the maximum lifetime of any
     * sample is not greater than eight times the poll interval, so
     * the maximum interval between minimum samples is eight
     * packets.
     */
    if (peer->filter_epoch[k] <= peer->epoch) {
#if DEBUG
    if (debug)
        printf("clock_filter: old sample %lu\n", current_time -
            peer->filter_epoch[k]);
#endif
        return;
    }
    peer->epoch = peer->filter_epoch[k];

    log(LOG_NOTICE) << "adjust clock " << peer->offset;

    /*
     * The mitigated sample statistics are saved for later
     * processing. If not synchronized or not in a burst, tickle the
     * clock select algorithm.
     */
    /*record_peer_stats(&peer->srcadr, ctlpeerstatus(peer),
        peer->offset, peer->delay, peer->disp, peer->jitter);*/
#ifdef DEBUG
    if (debug)
        printf(
            "clock_filter: n %d off %.6f del %.6f dsp %.6f jit %.6f\n",
            m, peer->offset, peer->delay, peer->disp,
            peer->jitter);
#endif
    //if (peer->burst == 0 || sys_leap == LEAP_NOTINSYNC)
    //  clock_select();
}


void NtpClient::destroy()
{
    cancelThread();
}
