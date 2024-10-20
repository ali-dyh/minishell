#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>



int main(int ac, char **av)
{
    av++;
    char *val = getenv(*av);
    printf("%s: %s\n", *av, val);
    return(0);
}