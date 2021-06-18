/*!
 * @file src/utilities.c
 * @brief Defines severy utility functions around debug printing and an alternative memory management using a fixed size memory pool.
 *
 * fine-print: Copyright (c) 2020-2021, David Hamilton <david@davidohamilton.com>. This software may be freely copied and used under GPLv2 (see LICENSE.txt in root directory).
 */

#include "utilities.h"
#include "looper.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <assert.h>
#include "debug.h"
#include "bit_things.h"

#define LINE_WIDTH 80

// Note about bit order of things in the frame control field:
// field order: version, type, subtype followed by toDS, FromDS, morefrag, retry, PS, moredata, protected, order
// so MSB byte order is subtype, type, version, order, protected, moredata, PS, retry, morefrag, FromDS, toDS
// fc[7..0] == subtype [7..4] + type[3..2] + version[1..0]
// flags[7..0] == order[7] + protected [6] + more_data[5] + PS[4] + retry[3] + more_frag[2] + FromDS, toDS

void get_frame_control(frame_control_t *fc, uint8_t *wfpkt, uint32_t size) {
    unsigned int radiotap_len = wfpkt[2] + (wfpkt[3] << 8);
    uint8_t fc_field = wfpkt[radiotap_len];
    uint8_t flags = wfpkt[radiotap_len+1];
    fc->version = getasb(fc_field,1,1) + getasb(fc_field,0,0);
    fc->type_subtype = getasb(fc_field,3,5) + getasb(fc_field,2,4) +
                       getasb(fc_field,7,3) + getasb(fc_field,6,2) + getasb(fc_field,5,1) + getasb(fc_field,4,0);
    if (fc->type_subtype != 0x08) q2print("fc_field: %02x, flags: %02x, TS: %02x", fc_field, flags, fc->type_subtype);
    fc->toDS         = getb(flags,0);
    fc->FromDS       = getb(flags,1);
    fc->morefrag     = getb(flags,2);
    fc->retry        = getb(flags,3);
    fc->PS           = getb(flags,4);
    fc->moredata     = getb(flags,5);
    fc->protected_MF = getb(flags,6);
    fc->order        = getb(flags,7);
    for (int i=0; i<6; i++) fc->dest[i] = wfpkt[radiotap_len+4+i];
}


bool is_ack(uint8_t *data, int size) {
  frame_control_t fc;
  get_frame_control(&fc, data, size);
  return (fc.type_subtype == ACK);
}

bool is_beacon(uint8_t *data, int size) {
  frame_control_t fc;
  get_frame_control(&fc, data, size);
  return (fc.type_subtype == BEACON);
}



#ifndef DEBUG

void start_printing_worker() {}

void stop_printing_worker() {}

void q2print(const char *fmt, ...) {}

void q2print_hex(uint8_t *data, int data_len) {}

void q2log_hex(bool condtion, uint8_t *data, int data_len) {}

void q2print_wftype(uint8_t *wfpkt, uint32_t size, char * msg) {}

void q2log(bool condition, const char *fmt, ...) {}

void q2log_wftype(bool condition, uint8_t *wfpkt, uint32_t size, char * msg) {}


#else

#ifndef QUEUED_PRINT

pthread_mutex_t printlock;

void start_printing_worker() {
    pthread_mutex_init(&printlock, NULL);
}

void stop_printing_worker() {}

void q2log(bool condition, const char *fmt, ...) {
    va_list args;
    struct timeval now;
    if (condition) {
        pthread_mutex_lock(&(printlock));
        gettimeofday(&(now), NULL);
        printf("%010ld.%06ld  ", now.tv_sec, now.tv_usec);
        va_start(args, fmt);
        vprintf(fmt, args);
        printf("\n");
        pthread_mutex_unlock(&(printlock));
    }
}

void q2log_wftype(bool condition, uint8_t *wfpkt, uint32_t size, char * msg) {
    if (condition) q2print_wftype(wfpkt, size, msg);
}

void q2print(const char *fmt, ...) {
    va_list args;
    struct timeval now;
    pthread_mutex_lock(&(printlock));
    gettimeofday(&(now), NULL);
    printf("%010ld.%06ld  ", now.tv_sec, now.tv_usec);
    va_start(args, fmt);
    vprintf(fmt, args);
    printf("\n");
    pthread_mutex_unlock(&(printlock));
}

void q2log_hex(bool condition, uint8_t *data, int data_len) {
    if (condition) q2print_hex(data, data_len);
}

void q2print_hex(uint8_t *data, int data_len) {
    int data_i, display_i;
    struct timeval now;
    pthread_mutex_lock(&(printlock));
    gettimeofday(&now, NULL);
    printf("%010ld.%06ld  ", now.tv_sec, now.tv_usec);
    display_i = 0;
    for (data_i=0; data_i<data_len; data_i++) {
        if ((display_i+3) >= LINE_WIDTH) {
            printf("\n                 ");
            display_i = 0;
        }
        printf("%02x:",data[data_i]);
        display_i += 3;
    }
    fflush(stdout);
    pthread_mutex_unlock(&(printlock));
}

#else

#define PRINT_QUEUE_SIZE 100

/*******************************************************
 * printer (repeatedly print characters in queue with time)
 ******************************************************/
static struct looper_t printer;

struct print_data_item {
    struct timeval now;
    char   data[LINE_WIDTH];
};

static struct print_data_item print_data[PRINT_QUEUE_SIZE];

static uint16_t head;
static uint16_t tail;

static void init_printer(void *d) {
    head = 0;
    tail = 0;
    printf("printing started\n");
}

static void repeat_printer_function(void *d) {
    while (head != tail) {
        assert(head < PRINT_QUEUE_SIZE);
        printf("%010ld.%06ld  ", print_data[head].now.tv_sec, print_data[head].now.tv_usec);
        printf("%s\n", print_data[head].data);
        fflush(stdout);
        if ((head+1)<PRINT_QUEUE_SIZE) head++;
        else head = 0;
    }
 }

static void final_printer(void *d) {
    printf("printing stopped\n");
}

void start_printing_worker() {
    printer.function_to_run_first = init_printer;
    printer.function_to_repeat = repeat_printer_function;
    printer.function_to_run_last = final_printer;
    looper_start(&printer);
}

void stop_printing_worker() {
    looper_stop(&printer);
}

uint8_t has_ending_null(char * s) {
  int i;
  for (i=0; i < LINE_WIDTH; i++) {
    if (s[i] ==0) return 1;
  }
  return 0;
}

// note that although looper handles mutual exclusion for the printer worker, we need to
// handle it explicitly here since this is outside of the looper abstraction
void q2print(const char *fmt, ...) {
    va_list args;
    pthread_mutex_lock(&(printer.lock));
    gettimeofday(&(print_data[tail].now), NULL);
    va_start(args, fmt);
    vsnprintf(print_data[tail].data, LINE_WIDTH, fmt, args);
    assert(has_ending_null(print_data[tail].data)==1);
    if ((tail+1) < PRINT_QUEUE_SIZE) tail++;
    else tail = 0;
    pthread_mutex_unlock(&(printer.lock));
}

static const char * hex = "0123456789ABCDEF";

void q2print_hex(uint8_t *data, int data_len) {
    int data_i, display_i;
    display_i = 0;
    for (data_i=0; data_i<data_len; data_i++) {
        if ((display_i+3) >= LINE_WIDTH) {
            // next line
            gettimeofday(&(print_data[tail].now), NULL);
            print_data[tail].data[display_i] = 0;
            if ((tail+1) < PRINT_QUEUE_SIZE) tail++;
            else tail = 0;
            display_i = 0;
        }
        assert (0<=((data[data_i]>>4)&0x0F) && ((data[data_i]>>4)&0x0F) <= 15);
        assert (0<=((data[data_i]>>4)&0x0F) && ((data[data_i])&0x0F) <= 15);
        print_data[tail].data[display_i] = hex[(data[data_i]>>4)&0x0F];
        print_data[tail].data[display_i+1] = hex[(data[data_i])&0x0F];
        print_data[tail].data[display_i+2] = ':';
        display_i += 3;
    }
    print_data[tail].data[display_i] = 0;
    gettimeofday(&(print_data[tail].now), NULL);
    if ((tail+1) < PRINT_QUEUE_SIZE) tail++;
    else tail = 0;
}

#endif

void q2print_fc_flags(frame_control_t *fc, char* type_msg, char *msg) {
    q2print("%-25s (%s)", type_msg, msg);
    q2print("v:%d, t: %02x, to:%d, From:%d, moref: %d, retry:%d, PS: %d, mored: %d, prtct:%d, ord:%d",
             fc->version, fc->type_subtype, fc->toDS, fc->FromDS, fc->morefrag, fc->retry, fc->PS, fc->moredata, fc->protected_MF, fc->order);
    q2print("dest: %02x:%02x:%02x:%02x:%02x:%02x", fc->dest[0], fc->dest[1], fc->dest[2], fc->dest[3], fc->dest[4], fc->dest[5]);
}

void q2print_data(uint8_t *wfpkt, uint32_t size) {
    unsigned int radiotap_len = wfpkt[2] + (wfpkt[3] << 8);
    q2print_hex(wfpkt+radiotap_len+24, size-radiotap_len-24);
}

void q2print_wftype(uint8_t *wfpkt, uint32_t size, char * msg) {
    frame_control_t fc;
    get_frame_control(&fc, wfpkt, size);
    switch(fc.type_subtype) {
        case ASSOCIATION_REQUEST:     q2print_fc_flags(&fc, "association request", msg); break;
        case ASSOCIATION_RESPONSE:    q2print_fc_flags(&fc, "association response", msg); break;
        case REASSOCIATION_REQUEST:   q2print_fc_flags(&fc, "reassociation request", msg); break;
        case REASSOCIATION_RESPONSE:  q2print_fc_flags(&fc, "reassociation response", msg); break;
        case PROBE_REQUEST:           q2print_fc_flags(&fc, "probe request", msg); break;
        case PROBE_RESPONSE:          q2print_fc_flags(&fc, "probe response", msg); break;
        case TIMING_ADVERTISEMENT:    q2print_fc_flags(&fc, "timing advertisement", msg); break;
        case BEACON:
        #ifdef Q2PRINT_BEACONS
        q2print_fc_flags(&fc, "beacon", msg);
        #endif
        break;
        case ATIM:                    q2print_fc_flags(&fc, "ATIM", msg); break;
        case DISASSOCIATION:          q2print_fc_flags(&fc, "disassocation", msg); break;
        case AUTHENTICATION:          q2print_fc_flags(&fc, "authentication", msg); break;
        case DEAUTHENTICATION:        q2print_fc_flags(&fc, "deauthentication", msg); break;
        case ACTION:                  q2print_fc_flags(&fc, "action", msg); break;
        case RESERVED0:
        case RESERVED1:
        case RESERVED2:
        case RESERVED3:               q2print_fc_flags(&fc, "reserved", msg); break;
        case BLOCK_ACK_REQUEST:       q2print_fc_flags(&fc, "block ack request", msg); break;
        case BLOCK_ACK:               q2print_fc_flags(&fc, "block ack", msg); break;
        case PS_POLL:                 q2print_fc_flags(&fc, "PS poll", msg); break;
        case RTS:                     q2print_fc_flags(&fc, "RTS", msg); break;
        case CTS:                     q2print_fc_flags(&fc, "CTS", msg); break;
        case ACK:                     q2print_fc_flags(&fc, "ACK", msg); break;
        case CF_END:                  q2print_fc_flags(&fc, "CF-end", msg); break;
        case CF_END_CF_ACK:           q2print_fc_flags(&fc, "CF-end+CF-ack", msg); break;
        case DATA:                    q2print_fc_flags(&fc, "Data", msg); break;
        case DATA_CF_ACK:             q2print_fc_flags(&fc, "Data+CF-ack", msg); break;
        case DATA_CF_POLL:            q2print_fc_flags(&fc, "Data+CF-poll", msg); break;
        case DATA_CF_ACK_CF_POLL:     q2print_fc_flags(&fc, "Data+CF-ack+CF-poll", msg); break;
        case NULL_FRAME:              q2print_fc_flags(&fc, "null", msg); break;
        case CF_ACK:                  q2print_fc_flags(&fc, "CF-ack", msg); break;
        case CF_POLL:                 q2print_fc_flags(&fc, "CF-poll", msg); break;
        case CF_ACK_CF_POLL:          q2print_fc_flags(&fc, "CF-ack+CF-poll", msg); break;
        case QOS_DATA:                q2print_fc_flags(&fc, "QOS data", msg); break;
        case QOS_DATA_CF_ACK:         q2print_fc_flags(&fc, "QOS data + CF-ack", msg); break;
        case QOS_DATA_CF_POLL:        q2print_fc_flags(&fc, "QOS data + CF-poll", msg); break;
        case QOS_DATA_CF_ACK_CF_POLL: q2print_fc_flags(&fc, "QOS data + CF-ack + CF-poll", msg); break;
        case QOS_NULL:                q2print_fc_flags(&fc, "QOS null", msg); break;
        case QOS_CF_POLL_NO_DATA:     q2print_fc_flags(&fc, "QOS +CF-poll (no data)", msg); break;
        case QOS_CF_ACK_NO_DATA:      q2print_fc_flags(&fc, "QOS +CF-ack (no data)", msg); break;
        case UNKNOWN_TYPE_SUBTYPE:    q2print("?????? unexpected type subtype %02X ???????? (%s)", fc.type_subtype, msg); break;
    }
    #ifdef Q2PRINT_GOING2SLEEP
    if (fc->PS) {
        q2print("Going to sleep (%02x)", flags);
    }
    else {
        q2print("Staying awake (%02x)", flags);
    }
    #endif
    #ifdef Q2PRINT_DATA
    if (0x20 <= fc.type_subtype && fc.type_subtype <= 0x2B) { /* some kind of data */
        q2print_data(wfpkt,size);
    }
    #endif
 }

 #endif // HIDE_ALL_Q2PRINT


void force_powersave_flag_off(uint8_t *wfpkt, uint32_t size) {
    unsigned int radiotap_len = wfpkt[2] + (wfpkt[3] << 8);
    clrb(wfpkt[radiotap_len+1],4);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////

static pthread_mutex_t fsmemlock;
static unsigned char fs_malloc_initialized = 0;
static unsigned char * fs_malloc_memory = NULL;
static unsigned char * fs_malloc_flags  = NULL;
static unsigned int fs_malloc_current;

/* get memory, initialize flags to "free", and mark as initialized, and put current at first element */
int fs_malloc_open() {
  unsigned int i;
  if (fs_malloc_initialized) fs_malloc_close();
  fs_malloc_memory = malloc(FS_MAX_SIZE * FS_MAX_NUM);
  if (!fs_malloc_memory) return -1;
  fs_malloc_flags = malloc(FS_MAX_NUM);
  if (!fs_malloc_flags) {
    free(fs_malloc_memory);
    return -2;
  }
  for (i=0; i<FS_MAX_NUM; i++) {
    fs_malloc_flags[i] = 0;
  }
  pthread_mutex_init(&fsmemlock, NULL);
  fs_malloc_current = 0;
  fs_malloc_initialized = 1;
  return 0;
}

/* free memory and mark as uninitialized */
void fs_malloc_close() {
    free(fs_malloc_memory);
    free(fs_malloc_flags);
    fs_malloc_initialized = 0;
}


void * fs_malloc(unsigned int size) {
    int i;
    void * return_ptr = NULL;
    pthread_mutex_lock(&(fsmemlock));
    if (!fs_malloc_initialized) return NULL;
    if (fs_malloc_current >= FS_MAX_NUM) return NULL;
    /* search for free element up to max */
    for (i=fs_malloc_current; i<FS_MAX_NUM && fs_malloc_flags[i]; i++);
    if (i<FS_MAX_NUM) {
        fs_malloc_flags[i] = 1;
        fs_malloc_current = i;
        return_ptr = (void *) (fs_malloc_memory + (i*FS_MAX_SIZE));
    }
    else {
        printf("failed to find element\n");
        /* search for free element starting at the bottom up to current */
        for (i=0; i<fs_malloc_current && fs_malloc_flags[i]; i++);
        if (i< fs_malloc_current) {
            fs_malloc_flags[i] = 1;
            fs_malloc_current = i;
            return_ptr = (void *) (fs_malloc_memory + (i*FS_MAX_SIZE));
        }
        else {
            printf("failed to find element anywhere\n");
            fs_malloc_current = FS_MAX_NUM;
            return_ptr = NULL;
        }
    }
    pthread_mutex_unlock(&(fsmemlock));
    return return_ptr;
    return NULL;
}

/* find element with this address and mark free */
/* since the assumption is that this was recently allocated, start at current and work back */
void fs_free(void * ptr) {
    int i;
    pthread_mutex_lock(&(fsmemlock));
    for (i=fs_malloc_current; i>=0 && (fs_malloc_memory + (i*FS_MAX_SIZE)) != ptr; i--);
    if (i>=0 && (fs_malloc_memory + (i*FS_MAX_SIZE)) == ptr) {
        fs_malloc_flags[i] = 0;
        pthread_mutex_unlock(&(fsmemlock));
        return;
    }
    else {
        for(i=FS_MAX_NUM-1; i>=fs_malloc_current && (fs_malloc_memory + (i*FS_MAX_SIZE)) != ptr; i--);
        if (i>=fs_malloc_current && (fs_malloc_memory + (i*FS_MAX_SIZE)) == ptr) {
            fs_malloc_flags[i] = 0;
            pthread_mutex_unlock(&(fsmemlock));
            return;
        }
    }
     pthread_mutex_unlock(&(fsmemlock));
     printf("ERROR: unable to find ptr to fs_free: %p\n", ptr);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

const uint32_t crctable[] = {
   0x00000000L, 0x77073096L, 0xee0e612cL, 0x990951baL, 0x076dc419L, 0x706af48fL, 0xe963a535L, 0x9e6495a3L,
   0x0edb8832L, 0x79dcb8a4L, 0xe0d5e91eL, 0x97d2d988L, 0x09b64c2bL, 0x7eb17cbdL, 0xe7b82d07L, 0x90bf1d91L,
   0x1db71064L, 0x6ab020f2L, 0xf3b97148L, 0x84be41deL, 0x1adad47dL, 0x6ddde4ebL, 0xf4d4b551L, 0x83d385c7L,
   0x136c9856L, 0x646ba8c0L, 0xfd62f97aL, 0x8a65c9ecL, 0x14015c4fL, 0x63066cd9L, 0xfa0f3d63L, 0x8d080df5L,
   0x3b6e20c8L, 0x4c69105eL, 0xd56041e4L, 0xa2677172L, 0x3c03e4d1L, 0x4b04d447L, 0xd20d85fdL, 0xa50ab56bL,
   0x35b5a8faL, 0x42b2986cL, 0xdbbbc9d6L, 0xacbcf940L, 0x32d86ce3L, 0x45df5c75L, 0xdcd60dcfL, 0xabd13d59L,
   0x26d930acL, 0x51de003aL, 0xc8d75180L, 0xbfd06116L, 0x21b4f4b5L, 0x56b3c423L, 0xcfba9599L, 0xb8bda50fL,
   0x2802b89eL, 0x5f058808L, 0xc60cd9b2L, 0xb10be924L, 0x2f6f7c87L, 0x58684c11L, 0xc1611dabL, 0xb6662d3dL,
   0x76dc4190L, 0x01db7106L, 0x98d220bcL, 0xefd5102aL, 0x71b18589L, 0x06b6b51fL, 0x9fbfe4a5L, 0xe8b8d433L,
   0x7807c9a2L, 0x0f00f934L, 0x9609a88eL, 0xe10e9818L, 0x7f6a0dbbL, 0x086d3d2dL, 0x91646c97L, 0xe6635c01L,
   0x6b6b51f4L, 0x1c6c6162L, 0x856530d8L, 0xf262004eL, 0x6c0695edL, 0x1b01a57bL, 0x8208f4c1L, 0xf50fc457L,
   0x65b0d9c6L, 0x12b7e950L, 0x8bbeb8eaL, 0xfcb9887cL, 0x62dd1ddfL, 0x15da2d49L, 0x8cd37cf3L, 0xfbd44c65L,
   0x4db26158L, 0x3ab551ceL, 0xa3bc0074L, 0xd4bb30e2L, 0x4adfa541L, 0x3dd895d7L, 0xa4d1c46dL, 0xd3d6f4fbL,
   0x4369e96aL, 0x346ed9fcL, 0xad678846L, 0xda60b8d0L, 0x44042d73L, 0x33031de5L, 0xaa0a4c5fL, 0xdd0d7cc9L,
   0x5005713cL, 0x270241aaL, 0xbe0b1010L, 0xc90c2086L, 0x5768b525L, 0x206f85b3L, 0xb966d409L, 0xce61e49fL,
   0x5edef90eL, 0x29d9c998L, 0xb0d09822L, 0xc7d7a8b4L, 0x59b33d17L, 0x2eb40d81L, 0xb7bd5c3bL, 0xc0ba6cadL,
   0xedb88320L, 0x9abfb3b6L, 0x03b6e20cL, 0x74b1d29aL, 0xead54739L, 0x9dd277afL, 0x04db2615L, 0x73dc1683L,
   0xe3630b12L, 0x94643b84L, 0x0d6d6a3eL, 0x7a6a5aa8L, 0xe40ecf0bL, 0x9309ff9dL, 0x0a00ae27L, 0x7d079eb1L,
   0xf00f9344L, 0x8708a3d2L, 0x1e01f268L, 0x6906c2feL, 0xf762575dL, 0x806567cbL, 0x196c3671L, 0x6e6b06e7L,
   0xfed41b76L, 0x89d32be0L, 0x10da7a5aL, 0x67dd4accL, 0xf9b9df6fL, 0x8ebeeff9L, 0x17b7be43L, 0x60b08ed5L,
   0xd6d6a3e8L, 0xa1d1937eL, 0x38d8c2c4L, 0x4fdff252L, 0xd1bb67f1L, 0xa6bc5767L, 0x3fb506ddL, 0x48b2364bL,
   0xd80d2bdaL, 0xaf0a1b4cL, 0x36034af6L, 0x41047a60L, 0xdf60efc3L, 0xa867df55L, 0x316e8eefL, 0x4669be79L,
   0xcb61b38cL, 0xbc66831aL, 0x256fd2a0L, 0x5268e236L, 0xcc0c7795L, 0xbb0b4703L, 0x220216b9L, 0x5505262fL,
   0xc5ba3bbeL, 0xb2bd0b28L, 0x2bb45a92L, 0x5cb36a04L, 0xc2d7ffa7L, 0xb5d0cf31L, 0x2cd99e8bL, 0x5bdeae1dL,
   0x9b64c2b0L, 0xec63f226L, 0x756aa39cL, 0x026d930aL, 0x9c0906a9L, 0xeb0e363fL, 0x72076785L, 0x05005713L,
   0x95bf4a82L, 0xe2b87a14L, 0x7bb12baeL, 0x0cb61b38L, 0x92d28e9bL, 0xe5d5be0dL, 0x7cdcefb7L, 0x0bdbdf21L,
   0x86d3d2d4L, 0xf1d4e242L, 0x68ddb3f8L, 0x1fda836eL, 0x81be16cdL, 0xf6b9265bL, 0x6fb077e1L, 0x18b74777L,
   0x88085ae6L, 0xff0f6a70L, 0x66063bcaL, 0x11010b5cL, 0x8f659effL, 0xf862ae69L, 0x616bffd3L, 0x166ccf45L,
   0xa00ae278L, 0xd70dd2eeL, 0x4e048354L, 0x3903b3c2L, 0xa7672661L, 0xd06016f7L, 0x4969474dL, 0x3e6e77dbL,
   0xaed16a4aL, 0xd9d65adcL, 0x40df0b66L, 0x37d83bf0L, 0xa9bcae53L, 0xdebb9ec5L, 0x47b2cf7fL, 0x30b5ffe9L,
   0xbdbdf21cL, 0xcabac28aL, 0x53b39330L, 0x24b4a3a6L, 0xbad03605L, 0xcdd70693L, 0x54de5729L, 0x23d967bfL,
   0xb3667a2eL, 0xc4614ab8L, 0x5d681b02L, 0x2a6f2b94L, 0xb40bbe37L, 0xc30c8ea1L, 0x5a05df1bL, 0x2d02ef8dL
};

uint32_t crc32(uint32_t bytes_sz, uint8_t *bytes)
{
   uint32_t crc = ~0;
   uint32_t i;
   for(i = 0; i < bytes_sz; ++i) {
      assert (0<= ((crc ^ bytes[i]) & 0xff) && ((crc ^ bytes[i]) & 0xff) <=255 );
      crc = crctable[(crc ^ bytes[i]) & 0xff] ^ (crc >> 8);
   }
   return ~crc;
}

uint32_t fcs(uint8_t *wfpkt, uint32_t size) {
    uint32_t data_start;
    uint32_t radiotap_len = wfpkt[2] + (wfpkt[3] << 8);
    uint32_t calculated_fcs, found_fcs;
    q2print("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
    q2print_hex(wfpkt, size);
    data_start = radiotap_len;
    fprintf(stderr, "data_start %08x, size: %08x\n", data_start, size);
    calculated_fcs = crc32(size-data_start, wfpkt+data_start);
    found_fcs = (wfpkt[size-1]<<24) + (wfpkt[size-2]<<16) + (wfpkt[size-3]<<8) + (wfpkt[size-4]);
    q2print("calculated FCS: %08x, found FCS: %08x", calculated_fcs, found_fcs);
    q2print(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
    return calculated_fcs;
}
