#include "kernel/types.h"
#include "user/user.h"

int
main(void) {
    exec_mem();
    printf("If you see this we might be good");
    exit(0);
}
