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
  u_char dtEnc = 0;//read byte by byte
  FILE *readFile;

/*
  printf("using table\n");
  for(i=0;i<24;i++) {
   printf("%d = %c%c%c%c\n", i, g4ROLL[i]>>24, g4ROLL[i]>>16, g4ROLL[i]>>8, g4ROLL[i]);
  }//for 24 (i)
*/

  readFile = fopen("/tmp/test", "r");
  fseek(readFile, 0L, SEEK_END);
  fileSz = ftell(readFile);
  rewind(readFile);

  fileBuf = malloc(fileSz+1);
  outBuf  = malloc(fileSz*4+1);
  memset(fileBuf, 0x0, fileSz+1);
  memset(outBuf, 0x0, fileSz*4+1);
  fread(fileBuf, fileSz, 1, readFile);

  //printf("%s\n---EOF---\n", fileBuf);

  for(i=0; i<fileSz; i++) {
     //printf("filePos is %d\n", i);
     dtEnc = fileBuf[i];
     //block 1 (high) (0xFF, 0x00, 0x00, 0x00)
     if(dtEnc > 191) {
       outBuf[outCnt*4] = (u_char)((g4ROLL[gTblPos]));
       dtEnc -= 192;
     }
     else if(dtEnc > 127) {
       outBuf[outCnt*4] = (u_char)((g4ROLL[gTblPos])>>8);
       dtEnc -= 128;
     }
     else if(dtEnc > 63) {
       outBuf[outCnt*4] = (u_char)((g4ROLL[gTblPos])>>16);
       dtEnc -= 64;
     }
     else {
       outBuf[outCnt*4] = (u_char)((g4ROLL[gTblPos])>>24);
     }//else
     tblSet(outBuf[outCnt*4]);


     //block 2 (high) (0x00, 0xFF, 0x00, 0x00)
     if(dtEnc > 47) {
       outBuf[outCnt*4+1] = (u_char)((g4ROLL[gTblPos]));
       dtEnc -= 48;
     }
     else if(dtEnc > 31) {
       outBuf[outCnt*4+1] = (u_char)((g4ROLL[gTblPos])>>8);
       dtEnc -= 32;
     }
     else if(dtEnc > 15) {
       outBuf[outCnt*4+1] = (u_char)((g4ROLL[gTblPos])>>16);
       dtEnc -= 16;
     }
     else {
       outBuf[outCnt*4+1] = (u_char)((g4ROLL[gTblPos])>>24);
     }//else
     tblSet(outBuf[outCnt*4+1]);


     //block 3 (low) (0x00, 0x00, 0xFF, 0x00)
     if(dtEnc > 11) {
       outBuf[outCnt*4+2] = (u_char)((g4ROLL[gTblPos]));
       dtEnc -= 12;
     }
     else if(dtEnc > 7) {
       outBuf[outCnt*4+2] = (u_char)((g4ROLL[gTblPos])>>8);
       dtEnc -= 8;
     }
     else if(dtEnc > 3) {
       outBuf[outCnt*4+2] = (u_char)((g4ROLL[gTblPos])>>16);
       dtEnc -= 4;
     }
     else {
       outBuf[outCnt*4+2] = (u_char)((g4ROLL[gTblPos])>>24);
     }//else
     tblSet(outBuf[outCnt*4+2]);


     //block 4 (low) (0x00, 0x00, 0x00, 0xFF)
     if(dtEnc > 2) {
       outBuf[outCnt*4+3] = (u_char)((g4ROLL[gTblPos]));
       //dtEnc -= 3;
     }
     else if(dtEnc > 1) {
       outBuf[outCnt*4+3] = (u_char)((g4ROLL[gTblPos])>>8);
       //dtEnc -= 2;
     }
     else if(dtEnc > 0) {
       outBuf[outCnt*4+3] = (u_char)((g4ROLL[gTblPos])>>16);
       //dtEnc -= 1;
     }
     else {
       outBuf[outCnt*4+3] = (u_char)((g4ROLL[gTblPos])>>24);
     }//else

     /*
      this should make data repeats longer for DNA structure.
      if needed, you could shuffle the 'key' table for even
      more repeat avoidance, while being able to encode/decode.

      i looked at another 'proven' method and there are repeats
      like this produces in the same encoded data.
      shuffle is likely a waste

      mathmatical probablity could produce repeats
     */
     tblSet(outBuf[outCnt*4+3]);

     outCnt++;
     if(outCnt % 24)
       gTblPos += 7;
     if(gTblPos>23)
       gTblPos -= 24;

  };//i, file

//printf("DNA ENCODED DATA:\n%s\n---EOF---\n", outBuf);
  printf("%s",outBuf);
  free(fileBuf);
  free(outBuf);
  fclose(readFile);
  return 0;

}//main

