#ifndef BIGNUM_SEC_PROFILING_H
#define BIGNUM_SEC_PROFILING_H

#include <stdint.h>
#include <stdbool.h>

typedef struct bignum_sec_s {
    uint64_t sec;
    uint64_t nsec;
    bool     positive;
} bignum_sec_t;

// c = a + b  (where a and b could be either positive or negative numbers)
void bignum_sec_add(bignum_sec_t *a, bignum_sec_t *b, bignum_sec_t *c);

// c = a - b  (where a and b could be either positive or negative numbers)
void bignum_sec_sub(bignum_sec_t *a, bignum_sec_t *b, bignum_sec_t *c);

// c = a / b  (where a could be either positive or negative but b must be positive)
void bignum_sec_divs(bignum_sec_t *a, uint32_t b, bignum_sec_t * c);

// true if a == b
bool bignum_sec_equal(bignum_sec_t *a, bignum_sec_t *b);

// true if a > b  (where a and b could be either positive or negative numbers)
bool bignum_sec_greater_than(bignum_sec_t *a, bignum_sec_t *b);

//true if a < b  (where a and b could be either positive or negative numbers)
bool bignum_sec_less_than(bignum_sec_t *a, bignum_sec_t *b);

void bignum_sec_assigns(bignum_sec_t *a, bool positive, uint64_t sec, uint64_t nsec);
void bignum_sec_assign(bignum_sec_t *a, bignum_sec_t * b);

void profiling_init_lag_time_sum();

void profiling_update_lag_time(bignum_sec_t * a);

void profiling_get_lag_time_avg(bignum_sec_t * avg, uint32_t num);
void profiling_get_lag_time_min(bignum_sec_t * min);
void profiling_get_lag_time_max(bignum_sec_t * max);


#endif // BIGNUM_SEC_PROFILING_H
