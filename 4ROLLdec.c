#include <stdlib.h>
#include <stdio.h>

/*
  table 'key' has 4 bases - 6 perms = 24 total size
  4 bases = 1byte (8bits)
  encode base and 'roll'
  roll = 0-23 table (24 values)
       each roll adds 6 (changes 'low' encoding bit)
       add 1-4 depending on last base encoded
       add determined from 'key table' where the +6 landed

 base4 (data to encode is 43dec (0x2B) )
 table 'key'
     : ACGT - A=0, C=1, G=2, T=3
 encoded A for high end of data (0 decimal. 3000base4 is 192base10)
 table jump 6 ahead (lands between 0-23)
 tmp table says
     : C=0, A=1, T=2, G=3
 last data encoded was A, so add 1 more
 table 'key'
     : C=0, G=1, T=2, A=3
 data : AT__ (02__)
 table jump 6 ahead
 tmp table says
     : G=0, A=1, T=2, C=3
 last data encoded was T, so add 2 more
 next encode will be the format of
 table 'key'
     : G=0, C=1, T=2, A=3
 data : ATT_ (022_)
 table jump 6 ahead (back to [0])
 tmp table says
     : T=0, C=1, A=2, G=3
 last data encoded was T, so add 1 more
 next encode will be the format of
 table 'key'
     : T=0, G=1, C=2, A=3
 data : ATTA (0223)
 final data: ATTA (0223) or 43dec or '+' or 0x2B :)
*/
//32bit x 24 array
const u_int g4ROLL [24] = {
(u_int)( (u_char)'A'<<24 | (u_char)'C'<<16 | (u_char)'G'<<8 | (u_char)'T'),
(u_int)( (u_char)'A'<<24 | (u_char)'C'<<16 | (u_char)'T'<<8 | (u_char)'G'),
(u_int)( (u_char)'A'<<24 | (u_char)'G'<<16 | (u_char)'C'<<8 | (u_char)'T'),
(u_int)( (u_char)'A'<<24 | (u_char)'G'<<16 | (u_char)'T'<<8 | (u_char)'C'),
(u_int)( (u_char)'A'<<24 | (u_char)'T'<<16 | (u_char)'C'<<8 | (u_char)'G'),
(u_int)( (u_char)'A'<<24 | (u_char)'T'<<16 | (u_char)'G'<<8 | (u_char)'C'),
(u_int)( (u_char)'C'<<24 | (u_char)'A'<<16 | (u_char)'T'<<8 | (u_char)'G'),
(u_int)( (u_char)'C'<<24 | (u_char)'A'<<16 | (u_char)'G'<<8 | (u_char)'T'),
(u_int)( (u_char)'C'<<24 | (u_char)'G'<<16 | (u_char)'T'<<8 | (u_char)'A'),
(u_int)( (u_char)'C'<<24 | (u_char)'G'<<16 | (u_char)'A'<<8 | (u_char)'T'),
(u_int)( (u_char)'C'<<24 | (u_char)'T'<<16 | (u_char)'G'<<8 | (u_char)'A'),
(u_int)( (u_char)'C'<<24 | (u_char)'T'<<16 | (u_char)'A'<<8 | (u_char)'G'),
(u_int)( (u_char)'G'<<24 | (u_char)'A'<<16 | (u_char)'C'<<8 | (u_char)'T'),
(u_int)( (u_char)'G'<<24 | (u_char)'A'<<16 | (u_char)'T'<<8 | (u_char)'C'),
(u_int)( (u_char)'G'<<24 | (u_char)'C'<<16 | (u_char)'A'<<8 | (u_char)'T'),
(u_int)( (u_char)'G'<<24 | (u_char)'C'<<16 | (u_char)'T'<<8 | (u_char)'A'),
(u_int)( (u_char)'G'<<24 | (u_char)'T'<<16 | (u_char)'A'<<8 | (u_char)'C'),
(u_int)( (u_char)'G'<<24 | (u_char)'T'<<16 | (u_char)'C'<<8 | (u_char)'A'),
(u_int)( (u_char)'T'<<24 | (u_char)'A'<<16 | (u_char)'G'<<8 | (u_char)'C'),
(u_int)( (u_char)'T'<<24 | (u_char)'A'<<16 | (u_char)'C'<<8 | (u_char)'G'),
(u_int)( (u_char)'T'<<24 | (u_char)'C'<<16 | (u_char)'G'<<8 | (u_char)'A'),
(u_int)( (u_char)'T'<<24 | (u_char)'C'<<16 | (u_char)'A'<<8 | (u_char)'G'),
(u_int)( (u_char)'T'<<24 | (u_char)'G'<<16 | (u_char)'C'<<8 | (u_char)'A'),
(u_int)( (u_char)'T'<<24 | (u_char)'G'<<16 | (u_char)'A'<<8 | (u_char)'C')
};

int gTblPos=0, gTblAdd = 0;//gTblPos should be 0-23, gTblAdd should be 1-4

void tblSet(u_char pLast) { //time for dinner!
     gTblPos += 6;
     if(gTblPos > 23)
       gTblPos -= 24;

     if      (pLast == (u_char) (g4ROLL[gTblPos]) )
       gTblAdd=1;
     else if(pLast == (u_char) (g4ROLL[gTblPos])>>8)
       gTblAdd=2;
     else if(pLast == (u_char) (g4ROLL[gTblPos])>>16)
       gTblAdd=3;
     else if(pLast == (u_char) (g4ROLL[gTblPos])>>24)
       gTblAdd=4;

     gTblPos += gTblAdd;
     if(gTblPos > 23)
       gTblPos -= 24;
     if(gTblPos > 23) {
       printf("ERROR! table pos outside bounds!\nEXITING!\n");
       return 0;
     }//if

     //printf("new gTblPos is %d - output was %c\n", gTblPos, pLast);
}//tblSet


int main() {

  char * fileBuf;
  u_char * outBuf;
  int fileSz=0, outCnt=0, i=0;
  u_char dtDec[4]={0,0,0,0};//form byte by byte
  FILE *readFile,*outFile;

/*
  printf("using table\n");
  for(i=0;i<24;i++) {
   printf("%d = %c%c%c%c\n", i, g4ROLL[i]>>24, g4ROLL[i]>>16, g4ROLL[i]>>8, g4ROLL[i]);
  }//for 24 (i)
*/

  readFile = fopen("/tmp/test.dec", "r");
  fseek(readFile, 0L, SEEK_END);
  fileSz = ftell(readFile);
  rewind(readFile);

  fileBuf = malloc(fileSz+1);
  outBuf  = malloc(fileSz/4+4);//not always even but more is fine
  memset(fileBuf, 0x0, fileSz+1);
  memset(outBuf, 0x0, fileSz/4+4);
  fread(fileBuf, fileSz, 1, readFile);

  //printf("%s\n---EOF---\n", fileBuf);

  for(i=0; i<fileSz/4; i++) {
     //printf("filePos 'real byte' is %d\n", i);
     dtDec[0] = fileBuf[i*4];
     dtDec[1] = fileBuf[i*4+1];
     dtDec[2] = fileBuf[i*4+2];
     dtDec[3] = fileBuf[i*4+3];
     //printf("data read...%c%c%c%c\n", dtDec[0],dtDec[1],dtDec[2],dtDec[3] );

     //block 1 (high) (0xFF, 0x00, 0x00, 0x00)
     if(dtDec[0] == (u_char)((g4ROLL[gTblPos])) )
       outBuf[outCnt] += 192;
     else if(dtDec[0] == (u_char)((g4ROLL[gTblPos])>>8) )
       outBuf[outCnt] += 128;
     else if(dtDec[0] == (u_char)((g4ROLL[gTblPos])>>16) )
       outBuf[outCnt] += 64;
     tblSet(dtDec[0]);


     //block 2 (high) (0x00, 0xFF, 0x00, 0x00)
     if(dtDec[1] == (u_char)((g4ROLL[gTblPos])) )
       outBuf[outCnt] += 48;
     else if(dtDec[1] == (u_char)((g4ROLL[gTblPos])>>8) )
       outBuf[outCnt] += 32;
     else if(dtDec[1] == (u_char)((g4ROLL[gTblPos])>>16) )
       outBuf[outCnt] += 16;
     tblSet(dtDec[1]);


     //block 3 (low) (0x00, 0x00, 0xFF, 0x00)
     if(dtDec[2] == (u_char)((g4ROLL[gTblPos])) )
       outBuf[outCnt] += 12;
     else if(dtDec[2] == (u_char)((g4ROLL[gTblPos])>>8) )
       outBuf[outCnt] += 8;
     else if(dtDec[2] == (u_char)((g4ROLL[gTblPos])>>16) )
       outBuf[outCnt] += 4;
     tblSet(dtDec[2]);


     //block 4 (low) (0x00, 0x00, 0x00, 0xFF)
     if(dtDec[3] == (u_char)((g4ROLL[gTblPos])) )
       outBuf[outCnt] += 3;
     else if(dtDec[3] == (u_char)((g4ROLL[gTblPos])>>8) )
       outBuf[outCnt] += 2;
     else if(dtDec[3] == (u_char)((g4ROLL[gTblPos])>>16) )
       outBuf[outCnt] += 1;

     //printf("last output formatted = %c =\n",outBuf[outCnt] );
     /*
      this should make data repeats longer for DNA structure.
      if needed, you could shuffle the 'key' table for even
      more repeat avoidance, while being able to encode/decode.

      i looked at another 'proven' method and there are repeats
      like this produces in the same encoded data.
      shuffle is likely a waste

      mathmatical probablity could produce repeats
     */

     outCnt++;
     if(outCnt % 24) //4 * 6 = 24 bases
       gTblPos += 7;
     if(gTblPos>23)
       gTblPos -= 24;

     tblSet(dtDec[3]);

  };//i, file

  printf("outputting file to /tmp/output with size %d\n", fileSz/4);
  outFile = fopen("/tmp/output", "wb");
  fwrite(outBuf, 1, fileSz/4, outFile);

  free(fileBuf);
  fclose(readFile);
  free(outBuf);
  fclose(outFile);
  return 0;

}//main

