#include "SystemHelper.h"
#include "common/WorkerThread.h"
#include "common/Utils.h"

#include <QBuffer>
#include <QMutex>
#include <QWaitCondition>

#if defined(Q_OS_WIN)
#include <Windows.h>
#include <Winternl.h>
#include <Psapi.h>
__int64 CompareFileTime(FILETIME time1, FILETIME time2)
{
    LARGE_INTEGER l1;
    l1.LowPart = time1.dwLowDateTime;
    l1.HighPart = time1.dwHighDateTime;

    LARGE_INTEGER l2;
    l2.LowPart = time2.dwLowDateTime;
    l2.HighPart = time2.dwHighDateTime;

    return (l2.QuadPart - l1.QuadPart);
}

#elif defined(Q_OS_LINUX)
#include <unistd.h>
#endif

/*

/proc/stat
  cpu: 提供系统所有 CPU 的汇总统计。每个数字代表特定类型的时间，单位为 jiffies
       在 Linux 中，1 jiffy 是内核的一个时钟滴答，具体长度依赖于系统配置，通常为 1/100 或 1/1000 秒
    user: 用户态花费的时间，不包括 nice 时间。
    nice: 调整为 nice 的用户态进程花费的时间。
    system: 内核态花费的时间。
    idle: 空闲时间，没有任何任务执行也没有等待 I/O。
    iowait: 等待 I/O 完成的时间。
    irq: 处理硬件中断的时间。
    softirq: 处理软件中断的时间。
    steal: 在虚拟环境中，等待其他虚拟 CPU 运行的时间。
    guest: 运行虚拟 CPU 的时间。
    guest_nice: 调整为 nice 的运行虚拟 CPU 的时间。
  cpuN（例如 cpu0, cpu1）：这是每个 CPU 核心的统计，格式与 cpu 行相同。
  ctxt: 上下文切换的次数。
  btime: 自系统启动以来的秒数。
  processes: 自系统启动以来创建的进程数。
  procs_running: 当前运行队列里的任务数。
  procs_blocked: 当前被阻塞等待 I/O 完成的任务数。

/proc/[pid]/stat
   0 pid: 进程 ID
   1 comm: 进程的命令名称（可执行文件名）
   2 state: 进程状态，如 R（运行）、S（睡眠）、Z（僵死）等
   3 ppid: 父进程 ID
   4 pgrp: 进程组 ID
   5 session: 会话 ID
   6 tty_nr: 控制终端的次设备号
   7 tpgid: 前台进程组 ID
   8 flags: 进程标志位
   9 minflt: 次缺页错误数（不需要从磁盘加载页面）
  10 cminflt: 进程的子进程的次缺页错误数
  11 majflt: 主缺页错误数（需要从磁盘加载页面）
  12 cmajflt: 进程的子进程的主缺页错误数
  13 utime: 用户态运行时间（时钟滴答数）
  14 stime: 内核态运行时间（时钟滴答数）
  15 cutime: 子进程用户态运行时间
  16 cstime: 子进程内核态运行时间
  17 priority: 动态优先级
  18 nice: 静态优先级
  19 num_threads: 线程数
  20 itrealvalue: 当前的时间片计数器的值
  21 starttime: 进程启动时间（自系统启动以来的时钟滴答数）
  22 vsize: 虚拟内存大小（字节）
  23 rss: 常驻内存大小（页数）
  24 rlim: 当前进程的地址空间限制
  25 startcode, endcode: 可执行代码的开始和结束地址
  26 startstack: 栈的开始地址
  27 kstkesp, kstkeip: 当前栈指针和栈指令指针
  28 signal: 当前挂起的信号
  29 blocked: 当前阻塞的信号
  30 sigignore: 当前忽略的信号
  31 sigcatch: 当前捕获的信号
  32 wchan: 进程等待的事件的地址
  33 nswap: 从文件系统交换到磁盘的页面数
  34 cnswap: 进程的子进程的交换到磁盘的页面数
  35 exit_signal: 用于杀死进程的信号
  36 processor: 运行进程的 CPU 编号
  37 rt_priority: 实时进程的优先级
  38 policy: 实时进程的策略
  39 delayacct_blkio_ticks: 自系统启动以来块 I/O 的滴答数
  40 guest_time: 虚拟进程的用户态运行时间
  41 cguest_time: 虚拟进程的子进程的用户态运行时间
  42 start_data, end_data: 数据段的开始和结束地址
  43 start_brk, arg_start, arg_end: 堆、命令行参数的开始和结束地址
  44 env_start, env_end: 环境变量的开始和结束地址
  45 exit_code: 退出状态码

/proc/[pid]/statm: 单位是页（通常是 4KB）
  size：进程的总虚拟内存大小（包含代码、数据、堆和栈）。
  resident：进程的常驻集大小（实际物理内存）。
  share：共享内存大小（与其他进程共享的内存）。
  text：可执行代码的大小（代码段）。
  lib：库的大小（加载的共享库）。此字段通常为零，已弃用。
  data：数据段的大小（包含堆）。
  dt：脏页的数量（已弃用）。

*/

class SystemHelperPrivate
{
public:
    bool abort = true;
    WorkerThread worker_thread;

    float cpu_usage = 0;
    float pcpu_usage = 0;

    int64_t mem_total = 0;
    int64_t mem_avail = 0;
    int64_t pmem_used = 0;

    QMutex mutex;
    QWaitCondition cond;

    void run();
};

void SystemHelperPrivate::run()
{
#if defined(Q_OS_WIN)
    bool first = true;
    FILETIME last_cpu_idle;
    FILETIME last_cpu_kernal;
    FILETIME last_cpu_user;
    FILETIME cur_cpu_idle;
    FILETIME cur_cpu_kernal;
    FILETIME cur_cpu_user;

    FILETIME last_creation_time;
    FILETIME last_exit_time;
    FILETIME last_kernel_time;
    FILETIME last_user_time;
    FILETIME last_sys_time;
    FILETIME cur_creation_time;
    FILETIME cur_exit_time;
    FILETIME cur_kernel_time;
    FILETIME cur_user_time;
    FILETIME cur_sys_time;

    SYSTEM_INFO si;
    GetSystemInfo(&si);

    DWORD pid = GetCurrentProcessId();
    HANDLE handle = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);

    while (!abort) {
        {
            MEMORYSTATUSEX statex;

            statex.dwLength = sizeof(statex);

            GlobalMemoryStatusEx(&statex);

            mem_total = statex.ullTotalPhys;
            mem_avail = statex.ullAvailPhys;

            // 该方式获取的内存情况和任务管理器对不上
            // PROCESS_MEMORY_COUNTERS pmc;
            // GetProcessMemoryInfo(handle, &pmc, sizeof(pmc));
            // pmem_used = pmc.WorkingSetSize;

            PSAPI_WORKING_SET_INFORMATION workSet;
            memset(&workSet, 0, sizeof(workSet));
            BOOL bOk = QueryWorkingSet(handle, &workSet, sizeof(workSet));
            if (bOk || (!bOk && GetLastError() == ERROR_BAD_LENGTH))
            {
                int nSize = sizeof(workSet.NumberOfEntries) + workSet.NumberOfEntries*sizeof(workSet.WorkingSetInfo);
                char* pBuf = new char[nSize];
                if (pBuf)
                {
                    QueryWorkingSet(handle, pBuf, nSize);
                    PSAPI_WORKING_SET_BLOCK* pFirst = (PSAPI_WORKING_SET_BLOCK*)(pBuf + sizeof(workSet.NumberOfEntries));
                    DWORD dwMem = 0;
                    for (ULONG_PTR nMemEntryCnt = 0; nMemEntryCnt < workSet.NumberOfEntries; nMemEntryCnt++, pFirst++)
                    {
                        if (pFirst->Shared == 0)
                        {
                            dwMem += si.dwPageSize;
                        }
                    }
                    delete [] pBuf;
                    pBuf = NULL;
                    if (workSet.NumberOfEntries > 0 && dwMem > 0)
                    {
                        pmem_used = dwMem;
                    }
                }
            }
        }

        int64_t total_diff = 0;
        {
            GetSystemTimes(&cur_cpu_idle, &cur_cpu_kernal, &cur_cpu_user);
            if (!first) {
                __int64 idle = CompareFileTime(last_cpu_idle, cur_cpu_idle);
                __int64 kernel = CompareFileTime(last_cpu_kernal, cur_cpu_kernal);
                __int64 user = CompareFileTime(last_cpu_user, cur_cpu_user);

                total_diff = kernel + user;

                float cpu = 100.0*(kernel+user-idle)/(kernel+user);
                if (0 <= cpu && cpu <= 100)
                    cpu_usage = cpu;
            }

            last_cpu_idle = cur_cpu_idle;
            last_cpu_kernal = cur_cpu_kernal;
            last_cpu_user = cur_cpu_user;
        }

        {
            GetSystemTimeAsFileTime(&cur_sys_time);
            GetProcessTimes(handle, &cur_creation_time, &cur_exit_time, &cur_kernel_time, &cur_user_time);
            if (!first) {
                __int64 kernel = CompareFileTime(last_kernel_time, cur_kernel_time);
                __int64 user = CompareFileTime(last_user_time, cur_user_time);

                float cpu = 100.0*(kernel+user)/total_diff;
                if (0 <= cpu && cpu <= 100)
                    pcpu_usage = cpu;
            }

            last_creation_time = cur_creation_time;
            last_exit_time = cur_exit_time;
            last_kernel_time = cur_kernel_time;
            last_user_time = cur_user_time;
        }

        // LOGD("mem: %.1f%% %s/%s %s", 100.0*(mem_total-mem_avail)/mem_total,
        //      qUtf8Printable(Utils::formatFileSize(mem_total-mem_avail)),
        //      qUtf8Printable(Utils::formatFileSize(mem_total)),
        //      qUtf8Printable(Utils::formatFileSize(pmem_used)));
        // LOGD("cpu: %.1f%% %.1f%%", cpu_usage, pcpu_usage);

        first = false;
        mutex.lock();
        if (!abort) cond.wait(&mutex, 1000);
        mutex.unlock();
    }

    CloseHandle(handle);

#elif defined(Q_OS_LINUX)
    pid_t pid = getpid();

    int64_t last_cpu_total = INT64_MAX;
    int64_t last_cpu_idle  = INT64_MAX;
    int64_t last_pcpu_use  = INT64_MAX;
    int64_t cur_cpu_total  = INT64_MAX;
    int64_t cur_cpu_idle   = INT64_MAX;
    int64_t cur_pcpu_use   = INT64_MAX;

    while (!abort) {

        // 获取系统内存情况
        {
            /*
             * MemTotal:        2018700 kB
             * MemFree:          300888 kB
             * MemAvailable:    1504260 kB
             * Buffers:          187752 kB
             * Cached:          1039364 kB
             * SwapCached:            0 kB
             * Active:           626660 kB
             * ...
            */
            QByteArray data = Utils::execute("cat", { QString("/proc/meminfo") });
            QBuffer buffer(&data);
            buffer.open(QIODevice::ReadOnly);

            while (!buffer.atEnd()) {
                QString line = buffer.readLine().trimmed();
                if (line.startsWith("MemTotal", Qt::CaseInsensitive)) {
                    QStringList list = line.split(" ", QString::SkipEmptyParts);
                    if (list.size() == 3) {
                        mem_total = list.at(1).toLongLong()*1024LL;
                    }
                }
                else if (line.startsWith("MemAvailable", Qt::CaseInsensitive)) {
                    QStringList list = line.split(" ", QString::SkipEmptyParts);
                    if (list.size() == 3) {
                        mem_avail = list.at(1).toLongLong()*1024LL;
                    }
                }
            }
        }

        // 获取进程内存情况
        {
            /*
             * 164453 75422 44529 646 0 103566 0
            */
            QString data = Utils::execute("cat", { QString("/proc/%1/statm").arg(pid) }).trimmed();
            QStringList list = data.split(" ", QString::SkipEmptyParts);
            if (list.size() == 7) {
                pmem_used = list.at(1).toLongLong()*4096LL;
            }
        }

        // 统计系统CPU信息
        int64_t total_diff = INT64_MAX;
        int64_t idle_diff = INT64_MAX;
        {
            /*
             * cpu  59115408 0 29479507 367555186 124706 0 9203131 0 0 0
             * ...
            */
            QByteArray data = Utils::execute("cat", { QString("/proc/stat") });
            QBuffer buffer(&data);
            buffer.open(QIODevice::ReadOnly);

            while (!buffer.atEnd()) {
                QString line = buffer.readLine().trimmed();
                if (line.startsWith("cpu ", Qt::CaseInsensitive)) {
                    QStringList list = line.split(" ", QString::SkipEmptyParts);
                    if (list.size() >= 4) {
                        int user_time = list.at(1).toLongLong();
                        int nice_time = list.at(2).toLongLong();
                        int sys_time = list.at(3).toLongLong();
                        int idle_time = list.at(4).toLongLong();

                        cur_cpu_total = user_time + nice_time + sys_time + idle_time;
                        cur_cpu_idle = idle_time;
                    }

                    break;
                }
            }

            if (last_cpu_total != INT64_MAX && last_cpu_idle != INT64_MAX) {
                total_diff = cur_cpu_total - last_cpu_total;
                idle_diff = cur_cpu_idle - last_cpu_idle;
                cpu_usage = 100.0*(total_diff-idle_diff)/total_diff;
            }

            last_cpu_total = cur_cpu_total;
            last_cpu_idle = cur_cpu_idle;
        }

        //
        {
            QString data = Utils::execute("cat", { QString("/proc/%1/stat").arg(pid) }).trimmed();
            QStringList list = QString(data).split(" ");
            if (list.size() > 16) {
                int64_t utime = list.at(13).toLongLong();
                int64_t stime = list.at(14).toLongLong();
                int64_t cutime = list.at(15).toLongLong();
                int64_t cstime = list.at(16).toLongLong();

                cur_pcpu_use = utime + stime + cutime + cstime;
            }

            if (last_pcpu_use != INT64_MAX && total_diff != INT64_MAX) {
                int64_t use_diff = cur_pcpu_use - last_pcpu_use;
                pcpu_usage = 100.0*use_diff/total_diff;
            }

            last_pcpu_use = cur_pcpu_use;
        }

        // LOGD("mem: %.1f%% %s/%s %s", 100.0*(mem_total-mem_avail)/mem_total,
        //      qUtf8Printable(Utils::formatFileSize(mem_total-mem_avail)),
        //      qUtf8Printable(Utils::formatFileSize(mem_total)),
        //      qUtf8Printable(Utils::formatFileSize(pmem_used)));
        // LOGD("cpu: %.1f%% %.1f%%", cpu_usage, pcpu_usage);

        mutex.lock();
        if (!abort) cond.wait(&mutex, 1000);
        mutex.unlock();
    }
#endif
}

SystemHelper::SystemHelper(QObject *parent)
    : QObject(parent)
{
    d.reset(new SystemHelperPrivate);
    d->worker_thread.setObjectName("SystemHelper");
}

SystemHelper::~SystemHelper()
{
    stop();
}

void SystemHelper::start()
{
    d->abort = false;
    d->worker_thread.runOnWorkerThread(std::bind(&SystemHelperPrivate::run, d.get()));
}

void SystemHelper::stop()
{
    d->abort = true;

    d->mutex.lock();
    d->cond.wakeAll();
    d->mutex.unlock();

    d->worker_thread.waitForDone();
}

float SystemHelper::getCpuUsage() const
{
    return d->cpu_usage;
}

float SystemHelper::getCpuUsageSelf() const
{
    return d->pcpu_usage;
}

float SystemHelper::getMemUsage() const
{
    if (d->mem_total == 0 || d->mem_avail == 0)
        return 0.0;
    else
        return 100.0*(d->mem_total-d->mem_avail)/d->mem_total;
}

int64_t SystemHelper::getMemTotal() const
{
    return d->mem_total;
}

int64_t SystemHelper::getMemUsed() const
{
    return d->mem_total - d->mem_avail;
}

int64_t SystemHelper::getMemUsedSelf() const
{
    return d->pmem_used;
}
