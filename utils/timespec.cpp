#include "utils/timespec.h"
//----------------------------------------------------------------------------------------------------------------------
long timespec_getdiff_ms(struct timespec &tp_old, struct timespec &tp_new)
{
    return ((tp_new.tv_sec - tp_old.tv_sec) * (long)(ONE_BILLION) +
        tp_new.tv_nsec - tp_old.tv_nsec) / ONE_MILLION;
}
//----------------------------------------------------------------------------------------------------------------------
