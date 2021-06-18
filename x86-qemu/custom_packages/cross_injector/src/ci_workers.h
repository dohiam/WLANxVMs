/*!
 * @file src/ci_workers.h
 * @brief defines the workers to move packets from capture to send and from receive to inject, and the listen socket for a server
 * @details
 * Includes functions to start, stop, pause, and resume the workers. A "production" application generally only needs
 * to start them and leave them running.
 *
 * fine-print: Copyright (c) 2020-2021, David Hamilton <david@davidohamilton.com>. This software may be freely copied and used under GPLv2 (see LICENSE.txt in root directory).
 */

#ifndef CI_WORKERS_H
#define CI_WORKERS_H

#include "lisa.h"

 void start_listener(lisa *lp);


#endif // QUEUE_WORKERS

