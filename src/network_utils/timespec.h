#ifndef TIMESPEC_H
#define TIMESPEC_H

// Source: https://stackoverflow.com/questions/68804469/subtract-two-timespec-objects-find-difference-in-time-or-duration

// C Standard Library
#include <time.h>
#include <stdio.h>

/**
 * @brief Normalize a tv_sec, tv_nsec pair into a timespec such that tv_nsec is in the range [0, 999999999]
 *
 * Per the timespec specification, tv_nsec should be positive, in the range 0 to 999,999,999. It is always
 * added to tv_sec (even when tv_sec is negative).
 * For this reason, if t is the time in secs, tv_sec is actually floor(t), and nsec is (t - floor(t)) * 1.0e9.
 *
 * @param tv_sec   (long) time in seconds. Msy be negative.
 * @param tv_nsec  (long) time in nanoseconds to be added to tv_sec. May be negative or >= 1000000000.
 *
 * @return timespec  Normalized timespec value with tv_nsec in the range [0, 999999999]
 */
inline static struct timespec normalize_timespec(long tv_sec, long tv_nsec) {
    long const billion = 1000000000;
    struct timespec t;
    t.tv_sec = (tv_nsec >= 0 ? tv_nsec : tv_nsec - (billion-1)) / billion;
    t.tv_nsec = tv_nsec - t.tv_sec * billion;
    t.tv_sec += tv_sec;
    return t;
}

/**
 * @brief Subtract two timespec values
 *
 * @param time1 Ending time
 * @param time0 Starting time
 * @return timespec  (time1 - time0)
 */
inline static struct timespec timespec_subtract(const struct timespec* time1, const struct timespec* time0) {
    return normalize_timespec(time1->tv_sec - time0->tv_sec, time1->tv_nsec - time0->tv_nsec);
}

inline static void timespec_fprintf (FILE* stream, const struct timespec* time)
{
	fprintf (stream, "%ld.%09ld", time->tv_sec, time->tv_nsec);
}

#endif // TIMESPEC_H