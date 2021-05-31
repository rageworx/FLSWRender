#include <sys/time.h>
#include <unistd.h>
#include <iostream>

#include "perfmon.h"

Perfmon::Perfmon()
 : perf_ms( 0 ),
   perf_ms_end( 0 )
{
}

Perfmon::~Perfmon()
{
}

void Perfmon::SetMonStart()
{
    perf_ms = gettimenow();
}

void Perfmon::SetMonStop()
{
    perf_ms_end = gettimenow();
}

unsigned long Perfmon::GetPerfMS()
{
    if ( ( perf_ms_end > perf_ms ) && ( perf_ms > 0 ) )
    {
        return perf_ms_end - perf_ms;
    }

    return 0;
}

unsigned long long Perfmon::gettimenow()
{
    timeval tv;
    gettimeofday(&tv, NULL);
    return (unsigned long long)(tv.tv_sec * 1000 + tv.tv_usec / 1000);
}
