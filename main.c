#include "common.h"
#include "syt.h"

int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        fprintf(stderr, "Usage: ./<executable> <ncol> <nrow>\n");
        return -1;
    }

    int m = atoi(argv[1]);
    int n = atoi(argv[2]);

    int status = syt_compute(m, n);

    return status;
}
