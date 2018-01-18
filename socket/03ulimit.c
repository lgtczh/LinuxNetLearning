#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/resource.h>

#include "../err_exit.h"

int main(void)
{
    struct rlimit rl;
    if (getrlimit(RLIMIT_NOFILE, &rl) < 0)
        ERR_EXIT("getrlimit");

    printf("cur_limit = %ld\n", (rl.rlim_cur));
    printf("max_limit = %ld\n", (rl.rlim_max));
    rl.rlim_cur = 2048;
    // 仅修改当前进程及其子进程的值
    if (setrlimit(RLIMIT_NOFILE, &rl) < 0)
        ERR_EXIT("setrlimit");

    printf("set nofile successfully %ld\n", rl.rlim_cur);

    return 0;
}

