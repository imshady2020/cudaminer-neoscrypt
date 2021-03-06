#ifndef __MINER_H__
#define __MINER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "neoscrypt.h"

#ifndef int8_t
typedef signed char int8_t;
#endif

#ifndef int16_t
typedef signed short int16_t;
#endif

#ifndef int32_t
typedef signed int int32_t;
#endif

#ifndef int64_t
#ifndef WIN32
typedef signed long int64_t;
#else
typedef signed long long int64_t;
#endif
#endif

#ifndef uint8_t
typedef unsigned char uint8_t;
#endif

#ifndef uint16_t
typedef unsigned short uint16_t;
#endif

#ifndef uint32_t
typedef unsigned int uint32_t;
#endif

#ifndef uint64_t
#ifndef WIN32
typedef unsigned long uint64_t;
#else
typedef unsigned long long uint64_t;
#endif
#endif

#include "cudaminer-config.h"

#include <sys/time.h>
#include <pthread.h>
#include <jansson.h>
#include <curl/curl.h>

#define EXIT_CODE_OK            0
#define EXIT_CODE_USAGE         1
#define EXIT_CODE_POOL_TIMEOUT  2
#define EXIT_CODE_SW_INIT_ERROR 3
#define EXIT_CODE_CUDA_NODEVICE 4
#define EXIT_CODE_CUDA_ERROR    5
#define EXIT_CODE_TIME_LIMIT    0
#define EXIT_CODE_KILLED        7

#ifdef WIN32
#define snprintf(...) _snprintf(__VA_ARGS__)
#define strdup(x) _strdup(x)
#define strncasecmp(x,y,z) _strnicmp(x,y,z)
#define strcasecmp(x,y) _stricmp(x,y)
#include <BaseTsd.h>
#undef HAVE_ALLOCA_H
#undef HAVE_SYSLOG_H
#ifdef _MSC_VER
typedef SSIZE_T ssize_t;
#endif
#endif

#ifdef STDC_HEADERS
# include <stdlib.h>
# include <stddef.h>
#else
# ifdef HAVE_STDLIB_H
#  include <stdlib.h>
# endif
#endif

#ifdef HAVE_ALLOCA_H
# include <alloca.h>
#elif !defined alloca
# ifdef __GNUC__
#  define alloca __builtin_alloca
# elif defined _AIX
#  define alloca __alloca
# elif defined _MSC_VER
#  include <malloc.h>
#  define alloca _alloca
# elif !defined HAVE_ALLOCA
#  ifdef  __cplusplus
extern "C"
#  endif
void *alloca (size_t);
# endif
#endif

#include "compat.h"

#ifdef __INTELLISENSE__
/* should be in stdint.h but... */
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
typedef __int32 int32_t;
typedef unsigned __int32 uint32_t;
typedef __int16 int16_t;
typedef unsigned __int16 uint16_t;
typedef __int16 int8_t;
typedef unsigned __int16 uint8_t;

typedef unsigned __int32 time_t;
typedef char *  va_list;
#endif

#if defined(__CUDA_ARCH__) && __CUDA_ARCH__ > 0
# define _ALIGN(x) __align__(x)
#elif _MSC_VER
# define _ALIGN(x) __declspec(align(x))
#else
# define _ALIGN(x) __attribute__ ((aligned(x)))
#endif

#undef unlikely
#undef likely
#if defined(__GNUC__) && (__GNUC__ > 2) && defined(__OPTIMIZE__)
#define unlikely(expr) (__builtin_expect(!!(expr), 0))
#define likely(expr) (__builtin_expect(!!(expr), 1))
#else
#define unlikely(expr) (expr)
#define likely(expr) (expr)
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#endif

#ifndef max
# define max(a, b)  ((a) > (b) ? (a) : (b))
#endif
#ifndef min
# define min(a, b)  ((a) < (b) ? (a) : (b))
#endif

#ifndef UINT32_MAX
/* for gcc 4.4 */
#define UINT32_MAX UINT_MAX
#endif

static inline bool is_windows(void) {
#ifdef WIN32
        return true;
#else
        return false;
#endif
}

#if ((__GNUC__ > 4) || (__GNUC__ == 4 && __GNUC_MINOR__ >= 3))
#define WANT_BUILTIN_BSWAP
#else
#define bswap_32(x) ((((x) << 24) & 0xff000000u) | (((x) << 8) & 0x00ff0000u) \
                   | (((x) >> 8) & 0x0000ff00u) | (((x) >> 24) & 0x000000ffu))
#define bswap_64(x) (((uint64_t) bswap_32((uint32_t)((x) & 0xffffffffu)) << 32) \
                   | (uint64_t) bswap_32((uint32_t)((x) >> 32)))
#endif

static inline uint32_t swab32(uint32_t v)
{
#ifdef WANT_BUILTIN_BSWAP
	return __builtin_bswap32(v);
#else
	return bswap_32(v);
#endif
}

static inline uint64_t swab64(uint64_t v)
{
#ifdef WANT_BUILTIN_BSWAP
	return __builtin_bswap64(v);
#else
	return bswap_64(v);
#endif
}

static inline void swab256(void *dest_p, const void *src_p)
{
	uint32_t *dest = (uint32_t *) dest_p;
	const uint32_t *src = (const uint32_t *) src_p;

	dest[0] = swab32(src[7]);
	dest[1] = swab32(src[6]);
	dest[2] = swab32(src[5]);
	dest[3] = swab32(src[4]);
	dest[4] = swab32(src[3]);
	dest[5] = swab32(src[2]);
	dest[6] = swab32(src[1]);
	dest[7] = swab32(src[0]);
}

#ifdef HAVE_SYS_ENDIAN_H
#include <sys/endian.h>
#endif

#if !HAVE_DECL_BE32DEC
static inline uint32_t be32dec(const void *pp)
{
	const uint8_t *p = (uint8_t const *)pp;
	return ((uint32_t)(p[3]) + ((uint32_t)(p[2]) << 8) +
	    ((uint32_t)(p[1]) << 16) + ((uint32_t)(p[0]) << 24));
}
#endif

#if !HAVE_DECL_LE32DEC
static inline uint32_t le32dec(const void *pp)
{
	const uint8_t *p = (uint8_t const *)pp;
	return ((uint32_t)(p[0]) + ((uint32_t)(p[1]) << 8) +
	    ((uint32_t)(p[2]) << 16) + ((uint32_t)(p[3]) << 24));
}
#endif

#if !HAVE_DECL_BE32ENC
static inline void be32enc(void *pp, uint32_t x)
{
	uint8_t *p = (uint8_t *)pp;
	p[3] = x & 0xff;
	p[2] = (x >> 8) & 0xff;
	p[1] = (x >> 16) & 0xff;
	p[0] = (x >> 24) & 0xff;
}
#endif

#if !HAVE_DECL_LE32ENC
static inline void le32enc(void *pp, uint32_t x)
{
	uint8_t *p = (uint8_t *)pp;
	p[0] = x & 0xff;
	p[1] = (x >> 8) & 0xff;
	p[2] = (x >> 16) & 0xff;
	p[3] = (x >> 24) & 0xff;
}
#endif

#if !HAVE_DECL_BE16DEC
static inline uint16_t be16dec(const void *pp)
{
	const uint8_t *p = (uint8_t const *)pp;
	return ((uint16_t)(p[1]) + ((uint16_t)(p[0]) << 8));
}
#endif

#if !HAVE_DECL_BE16ENC
static inline void be16enc(void *pp, uint16_t x)
{
	uint8_t *p = (uint8_t *)pp;
	p[1] = x & 0xff;
	p[0] = (x >> 8) & 0xff;
}
#endif

#if !HAVE_DECL_LE16DEC
static inline uint16_t le16dec(const void *pp)
{
	const uint8_t *p = (uint8_t const *)pp;
	return ((uint16_t)(p[0]) + ((uint16_t)(p[1]) << 8));
}
#endif

#if !HAVE_DECL_LE16ENC
static inline void le16enc(void *pp, uint16_t x)
{
	uint8_t *p = (uint8_t *)pp;
	p[0] = x & 0xff;
	p[1] = (x >> 8) & 0xff;
}
#endif

/* used for struct work */
void *aligned_calloc(int size);
void aligned_free(void *ptr);

#if JANSSON_MAJOR_VERSION >= 2
#define JSON_LOADS(str, err_ptr) json_loads((str), 0, (err_ptr))
#else
#define JSON_LOADS(str, err_ptr) json_loads((str), (err_ptr))
#endif

#define USER_AGENT PACKAGE_NAME "/" PACKAGE_VERSION

extern int scanhash_neoscrypt(int thr_id, uint32_t *pdata,
  const uint32_t *ptarget, uint32_t max_nonce, uint64_t *hashes_done, uint hash_mode);

/* api related */
void *api_thread(void *userdata);
void api_set_throughput(int thr_id, uint32_t throughput);

struct cgpu_info {
	uint8_t gpu_id;
	uint8_t thr_id;
	int accepted;
	int rejected;
	int hw_errors;
	double khashes;
	uint8_t intensity_int;
	uint8_t has_monitoring;
	float gpu_temp;
	uint16_t gpu_fan;
	uint16_t gpu_fan_rpm;
	uint16_t gpu_arch;
	int gpu_clock;
	int gpu_memclock;
	size_t gpu_mem;
	uint32_t gpu_usage;
	double gpu_vddc;
	int16_t gpu_pstate;
	int16_t gpu_bus;
	uint16_t gpu_vid;
	uint16_t gpu_pid;

	int8_t nvml_id;
	int8_t nvapi_id;

	char gpu_sn[64];
	char gpu_desc[64];
	float intensity;
	uint32_t throughput;
};

struct thr_api {
	int id;
	pthread_t pth;
	struct thread_q	*q;
};

struct stats_data {
	uint32_t uid;
	uint32_t tm_stat;
	uint32_t hashcount;
	uint32_t height;
	double difficulty;
	double hashrate;
	uint8_t thr_id;
	uint8_t gpu_id;
	uint8_t hashfound;
	uint8_t ignored;
};

struct hashlog_data {
	uint32_t tm_sent;
	uint32_t height;
	uint32_t njobid;
	uint32_t nonce;
	uint32_t scanned_from;
	uint32_t scanned_to;
	uint32_t last_from;
	uint32_t tm_add;
	uint32_t tm_upd;
};

/* end of api */

struct thr_info {
	int		id;
	pthread_t	pth;
	struct thread_q	*q;
	struct cgpu_info gpu;
};

struct work_restart {
	volatile unsigned long	restart;
	char			padding[128 - sizeof(unsigned long)];
};

extern bool opt_benchmark;
extern bool opt_debug;
extern bool opt_quiet;
extern bool opt_protocol;
extern bool opt_tracegpu;
extern int opt_n_threads;
extern int opt_n_gputhreads;
extern bool opt_cpumining;
extern int num_cpus;
extern int active_gpus;
extern int opt_timeout;
extern bool want_longpoll;
extern bool have_longpoll;
extern bool want_stratum;
extern bool have_stratum;
extern char *opt_cert;
extern char *opt_proxy;
extern long opt_proxy_type;
extern bool use_syslog;
extern bool use_colors;
extern struct thr_info *thr_info;
extern int longpoll_thr_id;
extern int stratum_thr_id;
extern int api_thr_id;
extern struct work_restart *work_restart;
extern bool opt_trust_pool;
extern uint16_t opt_vote;

extern uint64_t global_hashrate;
extern double   global_diff;

extern bool scan_abort_flag;

#define MAX_GPUS 32
extern char* device_name[MAX_GPUS];
extern int device_map[MAX_GPUS];
extern long  device_sm[MAX_GPUS];
extern uint32_t gpus_intensity[MAX_GPUS];

extern void format_hashrate(double hashrate, char *output);
extern void applog(int prio, const char *fmt, ...);
extern void gpulog(int prio, int thr_id, const char *fmt, ...);
extern json_t *json_rpc_call(CURL *curl, const char *url, const char *userpass,
	const char *rpc_req, bool, bool, int *);
extern double throughput2intensity(uint32_t throughput);
extern void cbin2hex(char *out, const char *in, size_t len);
extern char *bin2hex(const unsigned char *in, size_t len);
extern bool hex2bin(unsigned char *p, const char *hexstr, size_t len);
extern int timeval_subtract(struct timeval *result, struct timeval *x,
	struct timeval *y);
extern bool fulltest(const uint32_t *hash, const uint32_t *target);
extern void diff_to_target(uint32_t *target, double diff);
extern void get_currentalgo(char* buf, int sz);
extern uint32_t device_intensity(int thr_id, const char *func, uint32_t defcount);

struct stratum_job {
	char *job_id;
	unsigned char prevhash[32];
	size_t coinbase_size;
	unsigned char *coinbase;
	unsigned char *xnonce2;
	int merkle_count;
	unsigned char **merkle;
	unsigned char version[4];
	unsigned char nbits[4];
	unsigned char ntime[4];
	bool clean;
	unsigned char nreward[2];
	uint32_t height;
	double diff;
};

struct stratum_ctx {
	char *url;

	CURL *curl;
	char *curl_url;
	char curl_err_str[CURL_ERROR_SIZE];
	curl_socket_t sock;
	size_t sockbuf_size;
	char *sockbuf;
	pthread_mutex_t sock_lock;

	double next_diff;

	char *session_id;
	size_t xnonce1_size;
	unsigned char *xnonce1;
	size_t xnonce2_size;
	struct stratum_job job;
	pthread_mutex_t work_lock;

	struct timeval tv_submit;
	uint32_t answer_msec;
	uint32_t disconnects;
	time_t tm_connected;

	int srvtime_diff;
};

struct work {
	uint32_t data[64]; //42 needed for bitcredit
	uint32_t target[8];
	uint32_t maxvote;
	uint32_t midstate[8];
	char job_id[128];
	size_t xnonce2_len;
	uchar xnonce2[32];

	union {
		uint32_t u32[2];
		uint64_t u64[1];
	} noncerange;

	double difficulty;
	uint32_t height;

	uint32_t scanned_from;
	uint32_t scanned_to;
};

bool stratum_socket_full(struct stratum_ctx *sctx, int timeout);
bool stratum_send_line(struct stratum_ctx *sctx, char *s);
char *stratum_recv_line(struct stratum_ctx *sctx);
bool stratum_connect(struct stratum_ctx *sctx, const char *url);
void stratum_disconnect(struct stratum_ctx *sctx);
bool stratum_subscribe(struct stratum_ctx *sctx);
bool stratum_authorize(struct stratum_ctx *sctx, const char *user, const char *pass,bool extranonce);
bool stratum_handle_method(struct stratum_ctx *sctx, const char *s);

void hashlog_remember_submit(struct work* work, uint32_t nonce);
void hashlog_remember_scan_range(struct work* work);
uint32_t hashlog_already_submittted(char* jobid, uint32_t nounce);
uint32_t hashlog_get_last_sent(char* jobid);
uint64_t hashlog_get_scan_range(char* jobid);
int  hashlog_get_history(struct hashlog_data *data, int max_records);
void hashlog_purge_old(void);
void hashlog_purge_job(char* jobid);
void hashlog_purge_all(void);
void hashlog_dump_job(char* jobid);
void hashlog_getmeminfo(uint64_t *mem, uint32_t *records);

void stats_remember_speed(int thr_id, uint32_t hashcount, double hashrate, uint8_t found, uint32_t height);
double stats_get_speed(int thr_id, double def_speed);
int  stats_get_history(int thr_id, struct stats_data *data, int max_records);
void stats_purge_old(void);
void stats_purge_all(void);
void stats_getmeminfo(uint64_t *mem, uint32_t *records);

struct thread_q;

extern struct thread_q *tq_new(void);
extern void tq_free(struct thread_q *tq);
extern bool tq_push(struct thread_q *tq, void *data);
extern void *tq_pop(struct thread_q *tq, const struct timespec *abstime);
extern void tq_freeze(struct thread_q *tq);
extern void tq_thaw(struct thread_q *tq);

void proper_exit(int reason);

size_t time2str(char* buf, time_t timer);
char* atime2str(time_t timer);

void print_hash_tests(void);

#ifdef __cplusplus
}
#endif

#endif /* __MINER_H__ */
