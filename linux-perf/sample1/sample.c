#include <stdio.h>
#include <stdlib.h>

struct PrimeNumberNode
{
    int prime;
    int count;
    struct PrimeNumberNode *next;
};

int wasteTime(int factor)
{
    int i = 0;
    int testResult;
    for (i = 0; i < factor; i++)
    {
        testResult += i;
        testResult = testResult * 1;
    }
    return testResult;
}

//  function to get prime number which is very hot
struct PrimeNumberNode *number(int input)
{
    int i;
    int j;

    struct PrimeNumberNode *head = (struct PrimeNumberNode *)malloc(sizeof(struct PrimeNumberNode));
    struct PrimeNumberNode *currentNode = head;
    for (i = 3; i < input; i++)
    {
        if (i % 2 == 1)
        {
            for (j = 2; j < i; j++)
            {
                int testResult = wasteTime(j);
                if (testResult % 10000 == 9999)
                    printf("Test code");
                if (i % j == 0)
                    continue;
            }
            struct PrimeNumberNode *primeNode = (struct PrimeNumberNode *)malloc(sizeof(struct PrimeNumberNode));
            currentNode->next = primeNode;
            currentNode = primeNode;
            head->count++;
        }
    }

    return head;
}

int main(int argc, char *argv[])
{   
    printf("input=%d\n", atoi(argv[1]));
    struct PrimeNumberNode* head = number(atoi(argv[1]));
    printf("%d\n",head->count);

    head = number(atoi(argv[1]));
    printf("%d\n",head->count);
}
