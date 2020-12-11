/****************************************************************************
*    Copyright © 2014-2020 Xorg
*
*    This program is free software: you can redistribute it and/or modify
*    it under the terms of the GNU General Public License as published by
*    the Free Software Foundation, either version 3 of the License, or
*    (at your option) any later version.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
****************************************************************************/

/*
* PROJECT CPU-X
* FILE core.h
*/

#ifndef _CPUX_H_
#define _CPUX_H_

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include <errno.h>
#include <libintl.h>

/* Software definition */
#define PRGNAME               "CPU-X"
#define PRGNAME_LOW           "cpu-x"
#define PRGAUTH               "X0rg"
#define PRGURL                "https://"PRGAUTH".github.io/"PRGNAME
#define PRGCPRGHT             "Copyright © 2014-2020 Xorg"
#define PRGLCNS               "GNU GPL v3"
#define LCNSURL               "https://www.gnu.org/licenses/gpl.txt"

/* Colors definition */
#define DEFAULT               "\x1b[0m"
#define BOLD_RED              "\x1b[1;31m"
#define BOLD_GREEN            "\x1b[1;32m"
#define BOLD_YELLOW           "\x1b[1;33m"
#define BOLD_BLUE             "\x1b[1;34m"
#define BOLD_MAGENTA          "\x1b[1;35m"

/* Utilities macro */
#define LOCATION              PRGNAME, (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__), __LINE__
#define GOTO_ERROR(str)       { snprintf(error_str, MAXSTR, str); goto error; }
#define ALLOC_CHECK(ptr)      if(ptr == NULL) { MSG_ERRNO("%s", _("FATAL ERROR: could not allocate memory")); exit(255); }
#define FREE(ptr)             { free(ptr); ptr = NULL; }
#define IS_ROOT               (getuid() == 0)
#define PRGINFO(out)          fprintf(out, "%s %s%s (%s %s, %s %s, %s %s)\n", PRGNAME, PRGVER, GITREV, __DATE__, __TIME__, SYSTEM_NAME, SYSTEM_PROCESSOR, C_COMPILER_ID, __VERSION__)
#define UNUSED(x)             (void)(x)

/* Formatted messages definition */
#define MSG_BUFF_LEN          128
#define MSG_STDOUT(fmt, ...)  fprintf(stdout, colorized_msg(DEFAULT, "%s", fmt), ##__VA_ARGS__)
#define MSG_STDERR(fmt, ...)  fprintf(stderr, colorized_msg(DEFAULT, "%s", fmt), ##__VA_ARGS__)
#define MSG_VERBOSE(fmt, ...) opts->verbose ? fprintf(stdout, colorized_msg(BOLD_GREEN,   "%s", fmt), ##__VA_ARGS__) : 0
#define MSG_DEBUG(fmt, ...)   opts->debug   ? fprintf(stdout, colorized_msg(BOLD_MAGENTA, "%s", fmt), ##__VA_ARGS__) : 0
#define MSG_WARNING(fmt, ...) fprintf(stdout, colorized_msg(BOLD_YELLOW, "%s", fmt), ##__VA_ARGS__)
#define MSG_ERROR(fmt, ...)   fprintf(stderr, colorized_msg(BOLD_RED, "%s:%s:%i: %s",      LOCATION, fmt), ##__VA_ARGS__)
#define MSG_ERRNO(fmt, ...)   fprintf(stderr, colorized_msg(BOLD_RED, "%s:%s:%i: %s (%s)", LOCATION, fmt, strerror(errno)), ##__VA_ARGS__)
#define _(msg)                gettext(msg)
#define N_(msg)               msg

/* Options definition */
#define OUT_GTK               (1 << 0)
#define OUT_NCURSES           (1 << 1)
#define OUT_DUMP              (1 << 2)
#define OUT_NO_CPUX           (1 << 10)
#define OUT_DMIDECODE         (1 << 11)
#define OUT_BANDWIDTH         (1 << 12)

/* Arrays definition */
#define NAME                  0
#define VALUE                 1
#define MAXSTR                80       /* Max string */
#define CACHEFIELDS           2        /* Nb of fields by cache frame */
#define RAMFIELDS             2        /* Nb of fields by bank */
#define GPUFIELDS             9        /* Nb of fields by GPU frame */
#define BENCHFIELDS           2        /* Nb of fields by bench frame */

/* SI unit prefixes */
#define UNIT_B                _("bytes")
// TRANSLATORS: kilo-Byte
#define UNIT_KB               _("kB")
// TRANSLATORS: Mega-Byte
#define UNIT_MB               _("MB")
// TRANSLATORS: Giga-Byte
#define UNIT_GB               _("GB")
// TRANSLATORS: Tera-Byte
#define UNIT_TB               _("TB")

/* Linux-specific paths definition */
#define SYS_DMI               "/sys/devices/virtual/dmi/id"
#define SYS_CPU               "/sys/devices/system/cpu/cpu"
#define SYS_DRM               "/sys/class/drm/card"
#define SYS_HWMON             "/sys/class/hwmon"
#define SYS_DEBUG             "/sys/kernel/debug"
#define SYS_DEBUG_DRI         SYS_DEBUG"/dri"

/* FreeBSD-specific paths definition */
#define DEV_PCI               "/dev/pci"


enum EnTabNumber
{
	NO_CPU, NO_CACHES, NO_MOTHERBOARD, NO_MEMORY, NO_SYSTEM, NO_GRAPHICS, NO_BENCH, NO_ABOUT
};

enum EnObjects
{
	TABCPU, TABCACHES, TABMOTHERBOARD, TABMEMORY, TABSYSTEM, TABGRAPHICS, TABBENCH, TABABOUT,
	FRAMPROCESSOR, FRAMCLOCKS, FRAMCACHE,
	FRAML1CACHE, FRAML2CACHE, FRAML3CACHE, FRAML4CACHE, FRAMTEST,
	FRAMMOTHERBOARD, FRAMBIOS, FRAMCHIPSET,
	FRAMBANK0, FRAMBANK1, FRAMBANK2, FRAMBANK3, FRAMBANK4, FRAMBANK5, FRAMBANK6, FRAMBANK7,
	FRAMOPERATINGSYSTEM, FRAMMEMORY,
	FRAMGPU1, FRAMGPU2, FRAMGPU3, FRAMGPU4,
	FRAMPRIMESLOW, FRAMPRIMEFAST, FRAMPARAM,
	FRAMABOUT, FRAMLICENSE,
	LASTOBJ
};

enum EnTabCpu
{
	VENDOR, CODENAME, PACKAGE, TECHNOLOGY, VOLTAGE, SPECIFICATION, FAMILY, EXTFAMILY, MODEL, EXTMODEL, TEMPERATURE, STEPPING, INSTRUCTIONS,
	CORESPEED, MULTIPLIER, BUSSPEED, USAGE,
	LEVEL1D, LEVEL1I, LEVEL2, LEVEL3,
	SOCKETS, CORES, THREADS,
	LASTCPU
};

enum EnTabCaches
{
	L1SIZE, L1SPEED,
	L2SIZE, L2SPEED,
	L3SIZE, L3SPEED,
	L4SIZE, L4SPEED,
	LASTCACHES
};

enum EnTabMotherboard
{
	MANUFACTURER, MBMODEL, REVISION,
	BRAND, BIOSVERSION, DATE, ROMSIZE,
	CHIPVENDOR, CHIPMODEL,
	LASTMOTHERBOARD
};

enum EnTabMemory
{
	BANK0, BANK1, BANK2, BANK3,
	BANK4, BANK5, BANK6, BANK7,
	LASTMEMORY
};

enum EnTabSystem
{
	KERNEL, DISTRIBUTION, HOSTNAME, UPTIME, COMPILER,
	USED, BUFFERS, CACHED, FREE, SWAP,
	LASTSYSTEM
};

enum EnTabSystemExtra
{
	BARUSED, BARBUFFERS, BARCACHED, BARFREE, BARSWAP,
	LASTBAR
};

enum EnTabGraphics
{
	GPU1VENDOR, GPU1DRIVER, GPU1MODEL, GPU1TEMPERATURE, GPU1USAGE, GPU1CORECLOCK, GPU1MEMCLOCK, GPU1VOLTAGE, GPU1POWERAVG,
	GPU2VENDOR, GPU2DRIVER, GPU2MODEL, GPU2TEMPERATURE, GPU2USAGE, GPU2CORECLOCK, GPU2MEMCLOCK, GPU2VOLTAGE, GPU2POWERAVG,
	GPU3VENDOR, GPU3DRIVER, GPU3MODEL, GPU3TEMPERATURE, GPU3USAGE, GPU3CORECLOCK, GPU3MEMCLOCK, GPU3VOLTAGE, GPU3POWERAVG,
	GPU4VENDOR, GPU4DRIVER, GPU4MODEL, GPU4TEMPERATURE, GPU4USAGE, GPU4CORECLOCK, GPU4MEMCLOCK, GPU4VOLTAGE, GPU4POWERAVG,
	LASTGRAPHICS
};

enum EnGpuDrv
{
	GPUDRV_FGLRX, GPUDRV_AMDGPU, GPUDRV_RADEON,                                       // AMD
	GPUDRV_INTEL,                                                                     // Intel
	GPUDRV_NVIDIA, GPUDRV_NVIDIA_BUMBLEBEE, GPUDRV_NOUVEAU, GPUDRV_NOUVEAU_BUMBLEBEE, // NVIDIA
	GPUDRV_VFIO,
	GPUDRV_UNKNOWN
};

enum EnTabBench
{
	PRIMESLOWSCORE, PRIMESLOWRUN,
	PRIMEFASTSCORE, PRIMEFASTRUN,
	PARAMDURATION,  PARAMTHREADS,
	LASTBENCH
};

enum EnTabAbout
{
	DESCRIPTION,
	VERSIONSTR, AUTHOR, SITE,
	COPYRIGHT, LICENSE, NOWARRANTY,
	LASTABOUT
};

enum EnOptKeymap
{
	ARROWS,
	EMACS,
	INVERTED_T,
	VIM,
	LASTKEYMAP
};

typedef struct
{
	int8_t  cpu_vendor_id;
	int32_t cpu_model, cpu_ext_model, cpu_ext_family;
	char *cpuid_raw_file;
} LibcpuidData;

typedef struct
{
	uint8_t  test_count;
	uint32_t size[LASTCACHES / CACHEFIELDS];
	uint32_t speed[LASTCACHES / CACHEFIELDS];
	char     **test_name;
} BandwidthData;

typedef struct
{
	uint32_t mem_usage[SWAP - USED + 1];
	uint32_t mem_total;
	uint32_t swap_total;
} MemoryData;

typedef struct
{
	enum EnGpuDrv gpu_driver[LASTGRAPHICS / GPUFIELDS];
	char *device_path[LASTGRAPHICS / GPUFIELDS];
} GraphicsData;

typedef struct
{
	bool     run, fast_mode;
	unsigned duration, threads;
	uint32_t primes, start, elapsed;
	uint64_t num;
	pthread_t first_thread;
	pthread_mutex_t mutex_num, mutex_primes;
} BenchData;

typedef struct
{
	char *objects[LASTOBJ];
	char *tab_cpu[2][LASTCPU];
	char *tab_caches[2][LASTCACHES];
	char *tab_motherboard[2][LASTMOTHERBOARD];
	char *tab_memory[2][LASTMEMORY];
	char *tab_system[2][LASTSYSTEM];
	char *tab_graphics[2][LASTGRAPHICS];
	char *tab_bench[2][LASTBENCH];
	char *tab_about[LASTABOUT];

	int     cpu_freq, socket_fd;
	uint8_t cpu_count, cache_count, dimm_count, gpu_count;
	double  bus_freq, cpu_min_mult, cpu_max_mult;
	bool    reload;

	LibcpuidData  *l_data;
	BandwidthData *w_data;
	MemoryData    *m_data;
	GraphicsData  *g_data;
	BenchData     *b_data;
} Labels;

typedef struct
{
	bool     color, verbose, debug, issue, use_network, with_daemon, debug_database, freq_fallback;
	uint8_t  selected_page, selected_core, bw_test;
	uint16_t output_type, refr_time;
	enum EnOptKeymap keymap;
} Options;

typedef struct
{
	char **dim_names;
	char **dim_values;
	int  last;
} Arrays;

extern Options *opts;


/***************************** Defined in main.c *****************************/

/* Free memory after display labels */
void labels_free(Labels *data);


/***************************** Defined in util.c *****************************/

/* An asprintf-like function, but which can clean some parts of 'str' if 'clean_str' is true
 * - It calls vasprintf if 'fmt' is a valid string
 * - If 'clean_str' is true, it removes "unvalid args" from 'str' until next "valid arg"
     E.g.: casprintf(&str, false, "%i nm", 0): str = "0 nm"
           casprintf(&str, true,  "%i nm", 0): str = ""
	   casprintf(&str, true,  "%i nm", 32): str = "32 nm"
	   casprintf(&str, true,  "%i KB %i-way", -1, 12): str = "12-way" */
int casprintf(char **str, bool clean_str, const char *fmt, ...);

/* Return a formatted string */
char *format(char *str, ...);

/* Similar to format(), but string can be colorized */
char *colorized_msg(const char *color, const char *str, ...);

/* Open a file and put its content in a variable ('str' accept printf-like format) */
int fopen_to_str(char **buffer, char *str, ...);

/* Run a command and put output in a variable ('str' accept printf-like format) */
int popen_to_str(char **buffer, char *str, ...);

/* Check if a command exists */
bool command_exists(char *cmd);

/* Load a kernel module (return 0 on success) */
int load_module(char *module, int *fd);

/* Get a filename located in a directory corresponding to given request */
enum RequestSensor { RQT_CPU_TEMPERATURE, RQT_CPU_TEMPERATURE_OTHERS, RQT_CPU_VOLTAGE, RQT_GPU_TEMPERATURE, RQT_GPU_DRM, RQT_GPU_HWMON };
int request_sensor_path(char *base_dir, char **cached_path, enum RequestSensor which);

/* Start daemon in background */
const char *start_daemon(bool graphical);

/* Check if daemon is running */
bool daemon_is_alive(void);


/***************************** External headers *****************************/

/* Fill labels by calling core functions */
int fill_labels(Labels *data);

/* Refresh some labels */
int do_refresh(Labels *data);

/* Establish connection to daemon */
int connect_to_daemon(Labels *data);

/* Dmidecode main function */
int dmidecode(int quiet, void *cpux_pdata);

/* Call Dmidecode through CPU-X but do nothing else */
int run_dmidecode(void);

/* Call Bandwidth through CPU-X but do nothing else */
int run_bandwidth(void);

/* Perform a multithreaded benchmark (compute prime numbers) */
void start_benchmarks(Labels *data);

/* Load and apply settings from GSettings */
void load_settings(void);

/* Start CPU-X in GTK mode */
void start_gui_gtk(int *argc, char **argv[], Labels *data);

/* Start CPU-X in NCurses mode */
void start_tui_ncurses(Labels *data);


#endif /* _CPUX_H_ */
