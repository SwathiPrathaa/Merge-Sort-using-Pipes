#define LEN 11
#define MAXEL 20
#define LEN_DEF 8
#define LEN_MAX 256

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

void print_list(int * list, int len)
{
        /*Print all the ints of a list in brackets followed by a newline*/
        int i;
        printf("[ ");
        for (i = 0; i < len; i++)
        {
                printf("%d ", list[i]);
        }
	printf("]\n\n");
}



void merge(int * list,int * l_list,int l_size,int * rlist,int r_size){

        int r = 0; //right_half index
        int l = 0; //left_half index
        int i = 0; //list index

        //merge left_half and right_half back into list
        for ( i =0, r = 0, l = 0; l < l_size && r < r_size; i++)
        {
                if ( l_list[l] < rlist[r] ) {
                                list[i] = l_list[l++];
                        } else {
                                list[i] = rlist[r++];
                        }
        }

        //Copy over leftovers of whichever temporary list hasn't finished

	for ( ; l < l_size; i++, l++) {
                list[i] = l_list[l];
        }

        for ( ; r < r_size; i++, r++) {
                list[i] = rlist[r];
        }
}


void forking_mergesort(int * list,int n) {

        //termination condition
        if (n <= 1) {
                return;
        }

        int l_size=(n/2);
        int r_size=(n/2+n%2);

        int i,r,l;
	int fd1[2],fd2[2],fd3[2],fd4[2];
        int pid1,pid2;
        int stat;

        //declare temporary lists
        int l_list[l_size];
        int rlist[r_size];

        for (i = 0,l=0; i<l_size; i++, l++)
        {
                l_list[l] = list[i];
        }

        for (i =l_size,r=0; i < n; i++, r++)
        {
                rlist[r] = list[i];
        }


        //pipes creating
        if(pipe(fd1) == -1){
	       perror("Pipe error");
                exit(-1);
        }

        if(pipe(fd2) == -1){
                perror("Pipe error");
                exit(-1);
        }

        if(pipe(fd3) == -1){
                perror("Pipe error");
                exit(-1);
        }

        if(pipe(fd4) == -1){
                perror("Pipe error");
                exit(-1);
        }


        //fork using
	pid1=fork();
        switch(pid1){
                case -1://fork error
                        perror("fork error\n");
                        exit (-1);
                case 0://left_child process
                        close(fd1[1]);  //Close writing end of first pipe
                        close(fd2[0]);  //Clsoe reading end of second pipe
                        //read data from parent
                        while(read(fd1[0],&l_list,sizeof(int)*(l_size)) !=0)

                        //l_list handling with fork_mergesort recursive calling
                        forking_mergesort(l_list,l_size);

                        //return data of a left child to parent
                write(fd2[1],&l_list, sizeof(int)*(l_size));
                close(fd2[1]);

                exit(0);
        default://parent process
                //right
		  close(fd1[0]);  //Close reading end of the first pipe
                close(fd2[1]);  //Close writing end of second pipe
                //send data to the left child
                write(fd1[1],&l_list, sizeof(int)*(l_size));
                close(fd1[1]);  //Close writing end of first pipe

                //read data from left child
                while(read(fd2[0],&l_list,sizeof(int)*(l_size)) !=0)
                        wait(NULL);
                        close(fd2[0]); //Close reading end of second pipe

                        pid2=fork();
                        switch(pid2){
                                case -1://fork error
                                        perror("fork error\n");
                                        exit(-1);
                                case 0://right child process
                                        close(fd3[1]);
                                        close(fd4[0]);
                                        //read data from parent
                                        while(read(fd3[0],rlist,sizeof(int)*(r_size)) !=0)
					//l_list handling with fork_mergesort recursive calling
                                                forking_mergesort(rlist,r_size);

                                        //return data of right child to parent
                                        write(fd4[1],rlist,sizeof(int)*(r_size));
                                        close(fd4[1]);
                                        exit(0);
                                default:
                                        //right
                                        close(fd3[0]);
                                        close(fd4[1]);
                                        //send data to the right child
                                        write(fd3[1],&rlist,sizeof(int)*(r_size));
                                        close(fd3[1]);

                                        //read data from right child
                                                while(read(fd4[0],&rlist,sizeof(int)*(r_size)) !=0)
                                                wait(NULL);
                                                close(fd4[0]);

                                                //merge function calling
						  merge(list,l_list,l_size,rlist,r_size);
                        }
        }
}


//Main//////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[]) {

          char *fileName = NULL;
          FILE * filePtr = NULL;
          int i;
          char array[LEN_MAX];

          if(argc>1){

          //Test if argv[1] =-f////////////////////////////////////////////////////////////////////////////////////
          if(strcmp(argv[1],"-f")== 0){
            int size=0;
            int array1[LEN_MAX];
            fileName=argv[2];
	    filePtr = fopen("data.txt", "rt");

            if(filePtr == NULL) {
              printf("File opening error\n");
              exit(-1);
            }

            while (fgets(array,LEN_MAX, filePtr) != NULL){
              //printf("%d \n", atoi(array));
              array1[size]=atoi(array);
              size++;
            }

            //printing before sort
                printf("\nArray of integers generated from the data file %s\n",fileName);
                print_list(array1,size);

                forking_mergesort(array1,size);

                //printing after sort
                printf("Mergesort result:\n");
		  print_list(array1,size);
          }

        //Test if argv [1] = null (default array1le of 11 ints is created)////////////////////////////////////////////
        if(argv[1]==NULL){
                int field[LEN];
                srand(time(NULL));

                // fill field with random values 0...30
                for (i=0; i<LEN; i++){
                        field[i] = rand() % 30;
                }
		//printing before sort
                printf("\nArray of integers generated by default\n");
                print_list(field,LEN);
                forking_mergesort(field,LEN);
                //printing after sort
                printf("Mergesort result:\n");
                print_list(field,LEN);

        }

return 0;

}
