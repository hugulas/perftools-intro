#include <stdlib.h>
#include <string.h>

int main()
{
        while(1)
        {
		// use 1MB memory
                void *m = malloc(1024*1024);
                memset(m,0,1024*1024);
		// sleep 1 second
		sleep(1000);
        }
        return 0;
}
