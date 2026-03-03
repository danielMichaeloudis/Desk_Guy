#include <stdio.h>

extern int rust_main(void);

void app_main(void)
{
    printf("Hello world from C!\n");

    int result = rust_main();

    printf("Rust returned code: %d\n", result);
}