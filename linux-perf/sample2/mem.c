#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
	int n = atoi(argv[1]);
	int enough = 1;
        while(1)
        {
		
		// use 1MB memory
        	if (enough) {        
			void *m = malloc(1024*1024);
                	if (!m) {
	           		enough = 0;
		   		printf("No more memory to use");
			}
			else {
		   		memset(m,0,1024*1024);
			}
		}
		// sleep n micro seconds
		usleep(n);
        }
        return 0;
}
