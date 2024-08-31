#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define	NROWS			2048
#define NUMCPLXVALS     (1 << 15) 
#define N               ((2*NUMCPLXVALS) * NROWS)
#define SCALEFAC        4580274307


int random_int(int min, int max);

int main(void) 
{
    const char* filename = "dblfile";
    FILE* fp;
    double temp;
    long long int i, tempint;

    srand(time(NULL));

    /* generate random data and put it into the file */
    //printf("Data to file:\n");
    fp = fopen(filename, "wb");
    if(fp == NULL) return -1;         /* error */ 
    for(i = 0; i < N; i++){
        tempint = random_int(5, 63);
        //temp = (double)SCALEFAC * (double)((unsigned long long)1 << tempint) * (double)rand()/((double)rand() + 1.0f);
        temp = (double)rand()/(RAND_MAX);
        if(fwrite(&temp, sizeof(double), 1, fp) != 1){
            fclose(fp);
            return -1;                /* error */
        }
        //printf("%e\n", temp);
    }
    fclose(fp);

    /* print the values stored */
    //printf("\nData from file:\n");
    fp = fopen(filename, "rb");
    if(fp == NULL) return -1;         /* error */ 
    for(i = 0; i < N; i++){
        if(fread(&temp, sizeof(double), 1, fp) != 1){
            fclose(fp);
            return -1;                /* error */
        }
        //printf("%e\n", temp);
    }
    fclose(fp);


    return 0;
}


int random_int(int min, int max)
{
   return min + rand() % (max+1 - min);
}

