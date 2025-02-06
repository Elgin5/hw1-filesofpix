/**************************************************************
 *
 *                     readaline.c
 *
 *     Assignment: filesofpix
 *     Authors:  Oluwaponmile Fafowora, Elgin Zou
 *     Date:     01/29/2025
 *
 *     This program implements the readaline function which
 *     reads in characters from a file individually into
 *     an array and returns the number of bytes in the array.
 *
 **************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <readaline.h>
#include <string.h>
#include "mem.h"
#include "assert.h"

/*
 * name:      readaline
 * purpose:   to take in a line from a file and store it into
              an array
 * arguments: FILE *inputfd, a pointer to the file;
              char **datapp, a reference to the array
 * returns:   the size of the array after reading in the line
 * effects:   to take in a line from inputfd and store it into
              datapp
 */
size_t readaline(FILE *inputfd, char **datapp)
{
        int i = 0;
        int ch;
        *datapp = malloc(sizeof(char) * 1001);
    
        assert(*datapp != NULL);
        assert(inputfd != NULL);
        // If end of file set *datapp to NULL
        if (feof(inputfd) != 0){
                free(*datapp);
                *datapp = NULL;
                return 0;
        }
    
        while (((ch = fgetc(inputfd)) != '\n') && !(feof(inputfd))) {
                if (i > 1000) {
                        fprintf(stderr, "readaline: input line too long\n");
                        exit(4);
                }

                (*datapp)[i] = (char) ch;
                i++;
        }
    
        (*datapp)[i] = '\n';
        (*datapp)[i + 1] = '\0';

        return (i + 1);
}