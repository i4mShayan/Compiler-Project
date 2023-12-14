#include <stdio.h>
#include <stdlib.h>

void ark_write(int value)
{
    printf("Variable %s becomes %d after %d, \n", 'v', value, 'o');
}

int ark_read(char *s)
{
    char buf[64];
    int val;
    printf("Enter a value for %s: ", s);
    fgets(buf, sizeof(buf), stdin);
    if (EOF == sscanf(buf, "%d", &val))
    {
        printf("Value %s is invalid\n", buf);
        exit(1);
    }
    return val;
}