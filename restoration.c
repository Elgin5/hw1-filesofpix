/**************************************************************
 *
 *                     restoration.c
 *
 *     Assignment: filesofpix
 *     Authors:  Oluwaponmile Fafowora, Elgin Zou
 *     Date:     01/29/2025
 *
 *     This program restores a corrupted pgm file and prints
 *     it out in raw format
 *
 **************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <readaline.h>
#include "table.h"
#include "seq.h"
#include "atom.h"
#include <string.h>
#include "mem.h"
#include "pnmrdr.h"
#include "assert.h"

/*
 * name:      addChar()
 * purpose:   adds a char to the end of a string
 * arguments: char **s, a reference to the string;
              char c, the char to be added to the end
              of the string
 * returns:   none
 * effects:   adds char c to the end of s
 */
void addChar(char **s, char c) 
{
        // Get the current length of the string
        int l = strlen(*s);  
  
        // Add the new character at the end
        (*s)[l] = c;

        // Add the null terminator
        (*s)[l + 1] = '\0';
}

/*
 * name:      spliceSeq()
 * purpose:   separates the digits from the non digits within the
              fileLine into a Hanson sequence and Hanson atom
              respectively
 * arguments: char **fileLine, a reference to the file line;
              Seq_T currLine, a sequence where the digits will be stored;
              int lineSize, the length of the file line;
              char **wSpace, a reference to a char pointer that points to
              white space
 * returns:   returns a pointer to a const char containing the separated
              injection sequence
 * effects:   Separates the digits in the file line into currLine and
              places the characters of the injection sequence into a
              Hanson atom and returns it
 */
const char *spliceSeq(char **fileLine, Seq_T currLine,
                                                 int lineSize, char **wSpace) 
{
        char *injectionSeqChar = malloc(lineSize + 1);
        assert(injectionSeqChar != NULL);
        injectionSeqChar[0] = '\0';
        injectionSeqChar[lineSize - 1] = '\0';
        // Seperates the digits and the non digits
        for (int i = 0; i < lineSize; i++) {
                if (((*fileLine)[i] <= 57 && (*fileLine)[i] >= 48)) {
                        Seq_addhi(currLine, &(*fileLine)[i]);
                } else {
                        Seq_addhi(currLine, (*wSpace));
                        char g = (*fileLine)[i];
                        addChar(&injectionSeqChar, g);
                }
        }
   
        const char *injectionSeqStr = injectionSeqChar;
        const char *injectionSeq = Atom_string(injectionSeqStr);
        free (injectionSeqChar);
        return injectionSeq;
}

/*
 * name:      printRawPgm
 * purpose:   prints the raw format of the uncorrupted pgm to 
              standard output
 * arguments: Seq_T pgmSeq, a sequence containing sequences of
              the original digits of the pgm as rows
 * returns:   none
 * effects:   prints the raw format of the uncorrupted pgm to
              standard output
 */
void printRawPgm(Seq_T pgmSeq) 
{
        int width = 0;
        bool foundDigit = false;
        bool wSpaceBreak = true;
        // counts the width of the sequence
        for (int i = 0; i < Seq_length(Seq_get(pgmSeq, 0)); i++) {
                char *t = Seq_get(Seq_get(pgmSeq, 0), i);
                // checks when current character is a digit and when
                // previous character was whitespace
                if ((!(*t > 57 || *t < 48)) && wSpaceBreak) {
                        width++;
                        foundDigit = true;
                        wSpaceBreak = false;
                }
                if ((*t > 57 || *t < 48) && foundDigit) {
                        foundDigit = false;
                        wSpaceBreak = true;
                }
        }    
        printf("P5\n");
        printf("%d ", width);
        printf("%d\n", Seq_length(pgmSeq));
        printf("255\n");
    
        //              Prints the Char with its ASCII values
        bool prevDigit = false;
        int num = 0;
        for (int j = 0; j < Seq_length(pgmSeq); j++) {
                for (int i = 0; i < Seq_length((Seq_T)Seq_get(pgmSeq, j));
                                                                         i++) {
                        char *t = Seq_get(Seq_get(pgmSeq, j), i);
                        if ((*t > 57 || *t < 48)) {
                                if (prevDigit) {
                                        printf("%c", num);
                                        num = 0;
                                        prevDigit = false;
                                }
                        } 
                        else {
                                num = num * 10 + (*t - '0');
                                prevDigit = true;
                        }
                }
        }
}

/*
 * name:      vfree()
 * purpose:   serves as the free function for Table_map
 * arguments: const void *key, a void pointer to the key of
              the table;
              void **value, a reference to the value to the
              corresponding key;
              void *cl, a void pointer representing the stopping
              point for the function
 * returns:   none
 * effects:   frees each sequence (value) inside of the table
 */
void vfree(const void *key, void **value, void *cl) 
{
        (void) key;
        (void) cl;
        Seq_T val = *value;
        Seq_free(&val);
}

/*
 * name:      main()
 * purpose:   uncorrupts the pgm file passed in as an argument
              and calls printRawPgm to print out its raw format
 * arguments: int argc, an integer representing the number of
              arguments passed in by the user;
              char *argv[], a char array containing the arguments
              the user passed in
 * returns:   none
 * effects:   uncorrupts the pgm file passed in as an argument
              and calls printRawPgm to print out its raw format
 */
int main(int argc, char *argv[]) 
{
        char *fileLine;
        char *wSpace = " ";
        Table_T file_table = Table_new(0, NULL, NULL);
        // sequence containing the correct non-digit sequences
        Seq_T pgmSeq = Seq_new(0);
        // string containing correct injection sequence
        const char *interjectedSeq = "";
        FILE *fp;
        bool found = false;
        Seq_T correctLine = NULL;
        Seq_T fileLines = Seq_new(0);
        
        assert(argc <= 2);
        assert(argc >= 1);

        if (argc == 1) {
                fp = stdin;
        } else {
                fp = fopen(argv[1], "r");
        }

        assert(fp != NULL);

        while (true) {
                // gets the line from the file
                int lineSize = readaline(fp, &fileLine);
                Seq_addhi(fileLines, fileLine);
                if (lineSize < 1) {
                        free(fileLine);
                        break;
                }
                // the line without non digits
                Seq_T currLine = Seq_new(lineSize);
                // the injection sequence
                const char *currSeq = spliceSeq(&(fileLine), currLine, 
                                                            lineSize, &wSpace); 
        
                if (!found) {
                        // stores the previous value into correctLine if the
                        // keys are the same
                        correctLine = Table_put(file_table, currSeq, currLine);

                        if (correctLine != NULL) {
                                found = true;
                                // adds correctLine and currLine 
                                // into the sequence of correct digits
                                Seq_addhi(pgmSeq, correctLine);
                                Seq_addhi(pgmSeq, currLine);
                                // keeps track of the repeating injection 
                                // sequence
                                interjectedSeq = currSeq;
                        }
                } 
                else {
                        // if interjectedSeq matches current seq 
                        //add it to pgmSeq else next line
                        if (interjectedSeq == currSeq) {
                                Seq_addhi(pgmSeq, currLine);
                        } else {
                                Seq_free(&currLine);
                        }  
                }
        }
    
        printRawPgm(pgmSeq);

        // frees each fileLine
        for (int i = 0; i < Seq_length(fileLines); i++) {
                char *tempFileLine = Seq_get(fileLines, i);
                free(tempFileLine);
        }
        
        // frees each sequence in pgmSeq
        for (int j = 0; j < Seq_length(pgmSeq); j++) {
                if (j != 1) {
                        Seq_T temp = Seq_get(pgmSeq, j);
                        Seq_free(&temp);
                }
        }
    
        // frees the table, remaining sequences, and closes the file
        Table_map(file_table, vfree, NULL);
        Table_free(&file_table);
        Seq_free(&pgmSeq);
        Seq_free(&fileLines);
        fclose(fp);
        exit(EXIT_SUCCESS);
}   
