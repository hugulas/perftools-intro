#include <stdio.h>

int number(int);
int ifif();
int main()
{
    number(20000000);
    printf("\n");

    printf("start sleep\n");
    sleep(1);
    printf("stop sleep\n");

    number(20000001);
    printf("\n");

    printf("start sleep\n");
    sleep(1);
    printf("stop sleep\n");

    ifif();
    return 0;
}

// 1st function which is very hot
int number(int input)
{
    int i;
    int j;

    if (input % 2 == 1)
    {
        for (i = 2; i < input; i++)
        {
            for (j = 100000; j > 0; j--) /*for test*/
            {                            /*for test*/
                i++;                     /*for test*/
                i = i * 1;               /*for test*/
            }
        }
    }

    if (input % 2 == 0)
    {
        for (i = 2; i < input; i++)
        {

            for (j = 100000; j > 0; j--)
            {
                /*for test
                                                  *for test
                                                 */
                i++;
                i = i * 1;
            }
        }
    }

    return 1;
}

int ifif()
{
    int i = 1;
    int j = 2;
    int k = 1;
    int l = 1;
    if (i == j)
    {
        number(1000000);
    }
    else
    {
        number(1000001);
    }
    if (i < j)
    {
        number(1000000);
    }
    else
    {
        number(1000001);
    }
    j = 1;
    if (i == j)
    {
        number(1000000);
    }
    else
    {
        number(1000001);
    }
    if (i < j)
    {
        number(1000000);
    }
    else
    {
        number(1000001);
    }

    for (k = 10000000; k > 0; k--)
    {
        l++;
        l = k * 1;
    }
}