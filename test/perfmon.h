#ifndef __PERFMON_H__
#define __PERFMON_H__

class Perfmon
{
    public:
        Perfmon();
        ~Perfmon();

    public:
        void SetMonStart();
        void SetMonStop();

    public:
        unsigned long GetPerfMS();

    protected:
        unsigned long long gettimenow();

    private:
        unsigned long long perf_ms;
        unsigned long long perf_ms_end;
};

#endif // of __PERFMON_H__
