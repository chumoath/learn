#include <execinfo.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#define STACK_BUFFER 16

void is_so(char * str)
{
    char file[1024];
    char offset[1024];
    char cmd[1024];
    char buf[1024];
    char so[1024];

    /* 1. 获取文件名 */
    memset(cmd, 0, sizeof(cmd));
    sprintf(cmd, "echo '%s' | awk -F'[( ]' '{printf $1}'", str);
    FILE* output = popen(cmd, "r");
    if (output != NULL) {
        fgets(file, sizeof(file), output);
//        printf ("file = %s\n", file);
    } else {
        goto print_n;
    }

    /* 2. 判断文件是否是 so */
    memset(cmd, 0, sizeof(cmd));
    sprintf(cmd, "echo '%s' | awk '/\\.so/ {printf $1}'", file);
    output = popen(cmd, "r");
    if (output == NULL) {
        goto print_n;
    } else {
        fgets(so, sizeof(so), output);
        if (strcmp(so, file) != 0) {
            goto print_n;
        } else {
//            printf("%s contains .so\n", file);
        }
    }

    /* 3. 拿到 offset */
    memset(cmd, 0, sizeof(cmd));
    sprintf(cmd, "echo '%s' |  awk -F'[()]' '{split($2, a, \"+\"); printf a[2]}'", str);
    output = popen(cmd, "r");
    if (output == NULL) {
        goto print_n;
    } else {
        fgets(offset, sizeof(offset), output);
//        printf ("offset = %s\n", offset);
    }

    /* 4. 执行 addr2line */
    memset(cmd, 0, sizeof(cmd));
    sprintf(cmd, "addr2line -e %s %s", file, offset);
//    printf ("cmd = %s\n", cmd);
    output = popen(cmd, "r");
    if (output == NULL) {
        goto print_n;
    } else {
        fgets(buf, sizeof(buf), output);
//        printf ("line = %s\n", buf);
    }

//    printf ("%s\n", buf);
    // 输出结果自动带 换行符
    printf ("%s", buf);
    return;

print_n:
    printf("\n");
}

void dump(void)
{
    char buf[1024];
    void *buffer[STACK_BUFFER];
    char **string;
    int nptrs;

    nptrs = backtrace(buffer, STACK_BUFFER);
    string = backtrace_symbols(buffer, nptrs);
    if (string == NULL) {
        printf ("dump: string is NULL\n");
        return;
    }

    for (int i = 0; i < nptrs; i++) {
        printf ("%s ", string[i]);
        // 判断是否是动态库
        is_so(string[i]);
    }

    // 打印 当前进程的 maps，通过 实际地址 - 动态库加载首地址，即可获取实际偏移
    // add2line -e lib*.so 0x...
    sprintf(buf, "cat /proc/%d/maps", getpid());
    system((const char *)buf);
}


void sig_handler(int signo)
{
    printf ("=============>>> start dump <<<=================\n");
    dump();
    printf ("=============>>> start dump <<<=================\n");

    /* 设为默认处理 */
    signal(signo, SIG_DFL);
    /* 再次触发该信号 */
    raise(signo);
}