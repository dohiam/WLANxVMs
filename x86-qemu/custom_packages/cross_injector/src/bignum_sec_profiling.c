#include "bignum_sec_profiling.h"
#include <pthread.h>
#include <stdio.h>
#include <assert.h>
#include <inttypes.h>

//#define BIGNUM_DEBUG 1

#ifdef BIGNUM_DEBUG
static void bignum_sec_print(bignum_sec_t * a, char * msg) {
    printf("%s: ", msg);
    if (a->positive) printf("positive");
    else printf("negative");
    printf(" %#018" PRIx64 ".%#018" PRIx64 " ", a->sec, a->nsec);
    printf(" (%010ld.%06ld) \n", a->sec, a->nsec);
}
#endif // BIGNUM_DEBUG

/**********************************************************
 * unsigned arithmetic
 **********************************************************/

//Note: uint64_t can hold well over one billion (since 2**10 ~ 10**3, uint64_t max > 10**(3*6) which is 10~18, much like a billion billion) )
//      this means that borrowing and carry-over can be done in the straightforward way

#define one_billion 1000000000

// c = a + b
// note: c can also point to the same as a or b without messing things up (once c.nsec is set, neither a.nsec or b.nsec is referenced again)
static void bignum_sec_add_unsigned(bignum_sec_t *a, bignum_sec_t *b, bignum_sec_t *c) {
    c->nsec = a->nsec + b->nsec;
    if (c->nsec > one_billion) {
        c->nsec -= one_billion;
        c->sec = a->sec + b->sec + 1;
    }
    else c->sec = a->sec + b->sec;
}

// c = a - b; assume a > b
// note: c can also point to the same as a or b without messing things up (once c.nsec is set, neither a.nsec or b.nsec is referenced again)
static void bignum_sec_sub_unsigned(bignum_sec_t *a, bignum_sec_t *b, bignum_sec_t *c) {
    assert( (a->sec > b->sec) || ( (a->sec == b->sec) && (a->nsec > b->nsec)) );
    if (a->nsec < b->nsec) {
        c->nsec = (a->nsec + one_billion) - b->nsec;
        c->sec = (a->sec - 1) - b->sec;
    }
    else {
        c->nsec = a->nsec - b->nsec;
        c->sec = a->sec - b->sec;
    }
}

// c = a / b
//Note: a/b = a.sec + a.nsec/OB    where OB is one_billion
//            -----------------
//                    b
//
//          = (a.sec_even + a.sec_remainder)     a.nsec/OB    where a.sec_even is the part that evenly devices by b and
//            ------------------------------  +  ---------          a.sec_remainder is the remainder of a.sec / b
//                          b                        b
//
//          = a.sec_even     a.sec_remainder       a.nsec
//            ----------  +  ---------------  +  -----------
//                 b                b               b* OB
//
//          = a.sec_even     a.sec_remainder *OB + a.nsec
//            ----------  +  ----------------------------
//                 b                      b * OB
//
// example: 5.5    {5, 5*10**8} = {2, 1*10**9 + 5*10**8} = {2, 15*10**8) = (2, 7.5*10**8} = 2.75
//          --- =  ------------       ------------------       --
//           2          2                      2                2
static void bignum_sec_divs_unsigned(bignum_sec_t *a, uint32_t b, bignum_sec_t * c) {
    uint64_t remainder, carry;
    c->sec = a->sec / b;
    remainder = a->sec % b;
    c->nsec = (remainder * one_billion + a->nsec) / b;
    carry = c->nsec / one_billion;
    c->sec += carry;
    c->nsec -= carry * one_billion;
}

// true if a > b
static bool bignum_sec_greater_than_unsigned(bignum_sec_t *a, bignum_sec_t *b) {
    if (a->sec > b->sec) return true;
    if (a->sec < b->sec) return false;
    if (a->nsec > b->nsec) return true;
    else return false;
}

//true if a < b
static bool bignum_sec_less_than_unsigned(bignum_sec_t *a, bignum_sec_t *b) {
    if (a->sec < b->sec) return true;
    if (a->sec > b->sec) return false;
    if (a->nsec < b->nsec) return true;
    else return false;
}

/**********************************************************
 * signed arithmetic, using unsigned arithmetic
 **********************************************************/

//true if a == b
bool bignum_sec_equal(bignum_sec_t *a, bignum_sec_t *b) {
    if (a->positive != b->positive)  return false;
    if (a->sec != b->sec) return false;
    if (a->nsec != b->nsec) return false;
    return true;
}

// c = a + b  (where a and b could be either positive or negative numbers)
void bignum_sec_add(bignum_sec_t *a, bignum_sec_t *b, bignum_sec_t *c) {
    if (a->positive == b->positive) {
        // c will have the same sign as a and b and magnitude of a+b
        c->positive = a->positive;
        bignum_sec_add_unsigned(a,b,c);
        return;
    }
    if (a->positive && !b->positive) {
        // c = a + (-b) = a - b but unlike unsigned arithmetic, a could be less than b; in that case c = b-a with the negative sign of the dominant b
        if (bignum_sec_greater_than_unsigned(a,b)) {
            c->positive = true;
            bignum_sec_sub_unsigned(a,b,c);
        }
        else {
            c->positive = false;
            bignum_sec_sub_unsigned(b,a,c);
        }
    }
    else { /* a is negative and b is positive */
        // c = (-a) + b = b - a but unlike unsigned arithmetic, b could be less than a; in that case c = b-a with the negative sign of the dominant a
        if (bignum_sec_greater_than_unsigned(b,a)) {
            c->positive = true;
            bignum_sec_sub_unsigned(b,a,c);
        }
        else {
            c->positive = false;
            bignum_sec_sub_unsigned(a,b,c);
        }
    }
}

// c = a -b  (where a and b could be either positive or negative numbers)
void bignum_sec_sub(bignum_sec_t *a, bignum_sec_t *b, bignum_sec_t *c) {
    if (bignum_sec_equal(a,b)) {
        c->positive = true;
        c->sec = 0;
        c->nsec = 0;
    }
    if (a->positive && b->positive) {
        // c = a - b but unlike signed arithmetic, a could be less than b and in that case c is b-a with a negative sign
        if (bignum_sec_greater_than_unsigned(a,b)) {
            c->positive = true;
            bignum_sec_sub_unsigned(a,b,c);
            return;
        }
        else {
            c->positive = false;
            bignum_sec_sub_unsigned(b,a,c);
            return;
        }
    }
    if (a->positive && !b->positive) {
        // c = a - (-b) = c + b
        c->positive = true;
        bignum_sec_add_unsigned(a,b,c);
        return;
    }
    if (!a->positive && b->positive) {
        // c = -a - b = -(a+b)
        c->positive = false;
        bignum_sec_add_unsigned(a,b,c);
        return;
    }
    // both are negative, so it is the opposite situation of when they are both positive
    if (bignum_sec_greater_than_unsigned(b,a)) {
        c->positive = true;
        bignum_sec_sub_unsigned(b,a,c);
        return;
    }
    else {
        c->positive = false;
        bignum_sec_sub_unsigned(a,b,c);
        return;
    }
}

// c = a / b  (where a could be either positive or negative but b must be positive)
void bignum_sec_divs(bignum_sec_t *a, uint32_t b, bignum_sec_t * c) {
    c->positive = a->positive;
    bignum_sec_divs_unsigned(a,b,c);
}

// true if a > b  (where a and b could be either positive or negative numbers)
bool bignum_sec_greater_than(bignum_sec_t *a, bignum_sec_t *b) {
    if (bignum_sec_equal(a,b)) return false;
    if ( a->positive &&  b->positive) return bignum_sec_greater_than_unsigned(a,b);
    if ( a->positive && !b->positive) return true;   /* positive number always greater than negative number */
    if (!a->positive &&  b->positive) return false;  /* negative number is never greater than a positive number */
    if (!a->positive && !b->positive) return bignum_sec_less_than_unsigned(a,b);  /* less negative number is greater than a more negative number */
    return false;
}

//true if a < b  (where a and b could be either positive or negative numbers)
bool bignum_sec_less_than(bignum_sec_t *a, bignum_sec_t *b) {
    if (bignum_sec_equal(a,b)) return false;
    if ( a->positive &&  b->positive) return bignum_sec_less_than_unsigned(a,b);
    if ( a->positive && !b->positive) return false;
    if (!a->positive &&  b->positive) return true;
    if (!a->positive && !b->positive) return bignum_sec_greater_than_unsigned(a,b); /* more negative number less than a less negative number */
    return false;
}

void bignum_sec_assigns(bignum_sec_t *a, bool positive, uint64_t sec, uint64_t nsec) {
    a->positive = positive;
    a->sec = sec;
    a->nsec = nsec;
}

void bignum_sec_assign(bignum_sec_t *a, bignum_sec_t * b) {
    a->positive = b->positive;
    a->sec = b->sec;
    a->nsec = b->nsec;
}

bool bignum_sec_is_zero(bignum_sec_t * a) {
    if ( (a->sec == 0) && (a->nsec == 0) ) return true;
    else return false;
}

static bignum_sec_t lag_time_sum;
static bignum_sec_t lag_time_min;
static bignum_sec_t lag_time_max;
pthread_mutex_t lag_time_lock;

void profiling_init_lag_time_sum() {
    pthread_mutex_init(&lag_time_lock, NULL);
    bignum_sec_assigns(&lag_time_sum, true, 0, 0);
    bignum_sec_assigns(&lag_time_min, true, 0, 0);
    bignum_sec_assigns(&lag_time_max, true, 0, 0);
}

void profiling_update_lag_time(bignum_sec_t * a) {
    struct timespec now_ts;
    bignum_sec_t now;
    bignum_sec_t delta;
    bignum_sec_t current_sum;
    if (clock_gettime(CLOCK_REALTIME, &now_ts)<0) {
        printf("unable to get stop time, not processing lag time\n");
    }
    else {
        bignum_sec_assigns(&now, true, now_ts.tv_sec, now_ts.tv_nsec);
        bignum_sec_sub(&now, a, &delta);
        pthread_mutex_lock(&lag_time_lock);
        bignum_sec_assign(&current_sum, &lag_time_sum);
        bignum_sec_add(&current_sum, &delta, &lag_time_sum);
        if ( bignum_sec_is_zero(&lag_time_min) || (bignum_sec_less_than(&delta, &lag_time_min)) ) bignum_sec_assign(&lag_time_min, &delta);
        if ( bignum_sec_is_zero(&lag_time_max) || (bignum_sec_greater_than(&delta, &lag_time_max)) ) bignum_sec_assign(&lag_time_max, &delta);
        #ifdef BIGNUM_DEBUG
        bignum_sec_print(a, "a");
        bignum_sec_print(&now, "now");
        bignum_sec_print(&delta, "delta");
        bignum_sec_print(&lag_time_sum, "lag_time_sum");
        bignum_sec_print(&lag_time_min, "lag_time_min");
        bignum_sec_print(&lag_time_max, "lag_time_max");
        #endif // BIGNUM_DEBUG
        pthread_mutex_unlock(&lag_time_lock);
    }
}

void profiling_get_lag_time_avg(bignum_sec_t * avg, uint32_t num) {
    pthread_mutex_lock(&lag_time_lock);
    if ( (lag_time_sum.sec == 0) && (lag_time_sum.nsec == 0)) {
        avg->sec = avg->nsec = 0;
    }
    else {
        bignum_sec_divs(&lag_time_sum, num, avg);
    }
    pthread_mutex_unlock(&lag_time_lock);
}

void profiling_get_lag_time_min(bignum_sec_t * min) {
    pthread_mutex_lock(&lag_time_lock);
    bignum_sec_assign(min, &lag_time_min);
    pthread_mutex_unlock(&lag_time_lock);
}

void profiling_get_lag_time_max(bignum_sec_t * max) {
    pthread_mutex_lock(&lag_time_lock);
    bignum_sec_assign(max, &lag_time_max);
    pthread_mutex_unlock(&lag_time_lock);
}

