#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <ctype.h>


#define SIZE 128

typedef struct {
    size_t size;
    size_t used;
    char *data;
} bufferstr;

int bsInitialiser(bufferstr *bs, size_t size)
{
    bs->data = malloc(sizeof(char) * size);
    if (!bs->data) return 1;

    bs->size = size;
    bs->used   = 1;
    bs->data[0] = '\0';
    return 0;
}

void destroyBS(bufferstr *bs)
{
    free(bs->data);
}


int appendBS(bufferstr *bs, char letter)
{
    if (bs->used == bs->size) {
    size_t size = bs->size * 2;
    char *p = realloc(bs->data, sizeof(char) * size);
    if (!p) return 1;

    bs->data = p;
    bs->size = size;

    //if (deBUG) printf("Increased size to %lu\n", size);
    }

    bs->data[bs->used-1] = letter;
    bs->data[bs->used] = '\0';
    ++bs->used;

    return 0;
}

int concatenateBS(bufferstr* bslist, char* str){
    int iterator = 0;
    while(str[iterator] != '\0'){
        if(appendBS(bslist, str[iterator])){
            return 1;
        }
        iterator++;
    }
    return 0;
}

int wrap(int width, int inputReader, int outputWriter){
    char *bufferReader;
    
    bufferReader = (char*) calloc(SIZE, sizeof(char)); //caution attempt not to read all file at once

    bufferstr bsWord;
    bsWord.used = 1;//need to assign to use for else condition
    char prev[2] = " ";
    int currentLength = -1; //to allow the space for initial bsWord
    int newWord = 1; //vraible to check unique bsWord
    int firstWord = 1;// variable to check initial bsWord
    int error = 0;

    //reader
    while(read(inputReader,bufferReader,SIZE) != 0){
        for(int i = 0; i < SIZE; i++){ //run loop until '\n'
            char current = bufferReader[i];

            if(i > 0 && (bufferReader[i-1] == 0 && bufferReader[i] == 0)){
                if(currentLength > 0)
                    write(outputWriter,"\n",1);
                break;
            } 

            //checks if current is a space if is it it gets written and destroyed otherwise appended to the bsWord.
            if(!isspace(current) && current != '\0'){
                if(newWord) 
                    bsInitialiser(&bsWord,32);

                appendBS(&bsWord, current);
                newWord = 0;
            }
            else{
                    if(bsWord.used - 1 > width){
                        if(currentLength > 0)
                            write(outputWriter,"\n",1);
                        write(outputWriter,bsWord.data,bsWord.used-1);
                        destroyBS(&bsWord); // destroy to reset
                        newWord = 1;
                        currentLength += bsWord.used-1; //add current line with new size
                        bsWord.used = 1; //reinitialise values
                        error = 1;
                    }
                    else{
                        currentLength += bsWord.used; //increment for space
                        if(currentLength > width && bsWord.used > 1){ //look for blank spaces in paragraph
                                write(outputWriter,"\n",1); //append the line and reset
                                write(outputWriter,bsWord.data,bsWord.used-1);
                                destroyBS(&bsWord);
                                newWord = 1;
                                currentLength = bsWord.used-1;
                                bsWord.used = 1;
                                firstWord = 0;
                            
                        }
                        else if((prev[0] == '\n' && current == '\n') && currentLength > 0){ //paragraph has 1 or more bsWords so currentLength has to be > 0
                            write(outputWriter,"\n",1); //new line
                            write(outputWriter,"\n",1); //new line
                            firstWord = 1;
                            currentLength = -1; //removes any space 
                        }
                        else if(bsWord.used > 1){
                            if(!firstWord){
                                write(outputWriter," ",1);
                            }
                            write(outputWriter,bsWord.data,bsWord.used-1);
                            destroyBS(&bsWord);
                            bsWord.used = 1;
                            newWord = 1;
                            firstWord = 0;
                        }
                        else{
                            currentLength--;
                        }
                    }

            }
            prev[0] = current;
        }
        free(bufferReader);
        bufferReader = (char*) calloc(SIZE, sizeof(char));
    }
    free(bufferReader);
    if(error)
        return EXIT_FAILURE; // return incase of failure
    else
        return EXIT_SUCCESS; //return incase of success
}
int isdir(char *name) {
    struct stat data;
    int err = stat(name, &data);
    if (err == -1) {
        perror(name);  // display error
        return -1;
    }
    if (S_ISDIR(data.st_mode)) {
        return 1;
    } 
    else if (S_ISREG(data.st_mode)) {
        return 0;
    } 
    return -1;
}

int main(int argc, char* argv[]) {

   if(argc < 2) return EXIT_FAILURE; //if no argument
    int width = atoi(argv[1]);
    if(width < 1) return EXIT_FAILURE; //incase of invalid width
    int error = 0;
    if(argc == 2) {
         if(wrap(width, 0, 1))  // case 1: read from STDIN and print in STDOUT
            return EXIT_FAILURE;
         return EXIT_SUCCESS;
    }    
     
    //look for second argument if its a directory or file
    int check = isdir(argv[2]);
    if(check == 0) {
        int fd = open(argv[2], O_RDONLY);
        if(wrap(width, fd, 1)){ //case 2: the second argument is a file, read from file and then display in std output 
            close(fd);
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
        close(fd);
    } 
    else if(check == 1) { 
        DIR *parentDir = opendir(argv[2]);  // open the dir
        struct dirent *folder;
        while ((folder = readdir(parentDir))) {
            bufferstr source;
            bsInitialiser(&source, 32);
            concatenateBS(&source,argv[2]);
            appendBS(&source,'/');
            concatenateBS(&source,folder->d_name);
            int check2 = isdir(source.data);
            if(!check2){

                if(folder->d_name[0] == 'w' && folder->d_name[1] == 'r' && folder->d_name[2] == 'a' && folder->d_name[3] == 'p' && folder->d_name[4] == '.') {
                    destroyBS(&source);
                    continue;
                }
                //look for it starts with . then skip it
                if(folder->d_name[0] == '.') {
                    destroyBS(&source);
                    continue;
                }
                int fd2 = open(source.data, O_RDONLY);
                //create a new file
                bufferstr out_source;
                bsInitialiser(&out_source, 32);
                concatenateBS(&out_source,argv[2]);
                appendBS(&out_source,'/');
                concatenateBS(&out_source,"wrap.");
                concatenateBS(&out_source, folder->d_name);
                //create and write the file
                int fd3 = open(out_source.data, O_WRONLY|O_CREAT|O_TRUNC, 0600);
                if(wrap(width, fd2, fd3))
                    error = 1;
                close(fd3);
                close(fd2);
                destroyBS(&out_source);
                destroyBS(&source);
            }
            else if(check2 == 1){ 
                destroyBS(&source);
                continue;
            }
            else{
                error = 1;
                destroyBS(&source);
                continue;
            }
            
        }
        closedir(parentDir); // should check for failure
    }
    else{
        return EXIT_FAILURE;
    }
    if(error)
        return EXIT_FAILURE;
    return EXIT_SUCCESS;
}
