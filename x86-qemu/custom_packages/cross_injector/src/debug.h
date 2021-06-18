/*!
 * @file src/debug.h
 * @brief defines several different areas of debug messages and some behavior modifying options
 * @details
 * Debug Options:
 * * When ***HIDE_ALL_Q2PRINT*** is set, all debugging functions are compiled out.
 * * When ***Q2PRINT_BEACONS*** is set, messages will be printed for beacons
 * * When ***Q2PRINT_SENDING*** is set, messages will be printed for all packets sent (except possibly beacons)
 * * When ***Q2PRINT_RECEIVING*** is set to 1, messages will be printed for all packets received (except possibly beacons)
 * * When ***Q2PRINT_RECEIVING*** is set to 2, messages will be printed for all packets sent, including beacons (this second level includes processing when the packet type is not yet known)
 * * When ***Q2PRINT_PROCESSING*** is set to 1, messages will be printed for packets before they are sent or received (when the packet type can be determined)
 * * When ***Q2PRINT_PROCESSING*** is set to 2, messages will be printed for packets before they are sent or received even when the packet type is not known (so will always include beacons)
 * * When ***Q2PRINT_CAPTURED*** is set, messages will be printed for all packets captured (except possibly beacons)
 * * When ***Q2PRINT_INJECTED*** is set, messages will be printed for all packets injected
 * * When ***Q2PRINT_GOING2SLEEP*** is set, messages will be printed to flag when a STA is going to sleep (based on the PS flag)
 * * When ***Q2PRINT_DATA*** is set, the contents of data packages will be printed in hex
 * * When ***Q2PRINT_MOVING*** is set, messages will be printed when a packet is moved from one queue to another
 *
 * Behavior Modifying Options:
 * * When ***FORCE_POWERSAVE_OFF*** is set, each packet's PS flag will be set to off (note that checksum is not altered as with hwsim, it isn't captured)
 * * When ***SKIP_ACKS*** is set, acks captured will be ignored (as they are sent internally in hwsim)
 * * When ***QUEUED_PRINT*** is set, debug messages will not block tasks but instead will be printed on a separate thread (in order received)
 *
 *
 * fine-print: Copyright (c) 2020-2021, David Hamilton <david@davidohamilton.com>. This software may be freely copied and used under GPLv2 (see LICENSE.txt in root directory).
 */

#define Q2PRINT_BEACONS 0
#define Q2PRINT_PROCESSING 0
#define Q2PRINT_SENDING 0
#define Q2PRINT_RECEIVING 0
#define Q2PRINT_CAPTURED 0
#define Q2PRINT_INJECTED 0
#define Q2PRINT_GOING2SLEEP 0
#define Q2PRINT_DATA 0
//#define DEBUG 1
//#define HIDE_ALL_Q2PRINT 1
//#define Q2PRINT_BEACONS 1
//#define Q2PRINT_PROCESSING 1
//#define Q2PRINT_SENDING 2
//#define Q2PRINT_RECEIVING 2
//#define Q2PRINT_CAPTURED 1
//#define Q2PRINT_INJECTED 1
//#define Q2PRINT_GOING2SLEEP 1
//#define Q2PRINT_DATA 1
//#define FORCE_POWERSAVE_OFF 1
//#define QUEUED_PRINT 1
#define SKIP_ACKS
