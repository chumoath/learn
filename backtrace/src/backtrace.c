#include <execinfo.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#define STACK_BUFFER 16

int get_data_from_awk(char *cmd, char *buf, int len)
{
    FILE* output;
    int ret = -1;
    output = popen(cmd, "r");
    if (output != NULL) {
        fgets(buf, len, output);
        ret = 0;
    }

    pclose(output);
    return ret;
}

void is_so(char * str)
{
    char file[1024];
    char offset_base[1024];
    char offset_now[1024];
    char offset[1024];
    char cmd[1024];
    char buf[1024];
    char so[1024];

    /* 1. 获取文件名 */
    memset(cmd, 0, sizeof(cmd));
    sprintf(cmd, "echo '%s' | awk -F'[( ]' '{printf $1}'", str);
    if (get_data_from_awk(cmd, file, sizeof(file)) != 0) {
        goto print_n;
    }

    /* 2. 判断文件是否是 so */
    memset(cmd, 0, sizeof(cmd));
    sprintf(cmd, "echo '%s' | awk '/\\.so/ {printf $1}'", file);
    if (get_data_from_awk(cmd, so, sizeof(so)) != 0) {
        goto print_n;
    }

    if (strcmp(so, file) != 0) {
        goto print_n;
    }


    /* 3. 拿到 offset_now */
    memset(cmd, 0, sizeof(cmd));
    sprintf(cmd, "echo '%s' | awk -F'[][]' '{printf $2}'", str);
    if (get_data_from_awk(cmd, offset_now, sizeof(offset_now)) != 0) {
        goto print_n;
    }

    /* 4. 拿到动态库加载的首地址 offset_base */
    // 获取 so，不包含 路径名
    memset(cmd, 0, sizeof(cmd));
    sprintf(cmd, "echo '%s' | awk -F'/' '{printf $NF}'", file);
    if (get_data_from_awk(cmd, so, sizeof(so)) != 0) {
        goto print_n;
    }

    // 获取 offset_base
    memset(cmd, 0, sizeof(cmd));
    sprintf(cmd, "cat /proc/%d/maps | grep -m 1 %s | awk -F'-' '{printf $1}'", getpid(), so);
    if (get_data_from_awk(cmd, offset_base, sizeof(offset_base)) != 0) {
        goto print_n;
    }

    // 获取 offset
    memset(cmd, 0, sizeof(cmd));
    sprintf(cmd, "echo '%s %s' | awk '{ printf(\"0x%%X\", strtonum($1) - strtonum(\"0x\" $2)) }'", offset_now, offset_base);
    if (get_data_from_awk(cmd, offset, sizeof(offset)) != 0) {
        goto print_n;
    }

    /* 5. 执行 addr2line */
    memset(cmd, 0, sizeof(cmd));
    sprintf(cmd, "addr2line -e %s %s", file, offset);
    if (get_data_from_awk(cmd, buf, sizeof(buf)) != 0) {
        goto print_n;
    }

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
//    sprintf(buf, "cat /proc/%d/maps", getpid());
//    system((const char *)buf);
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