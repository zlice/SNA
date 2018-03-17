#include <stdlib.h>
#include <stdio.h>
#include <string.h>


/*
  table 'key' has 4 bases and 24 total size with permutations
  4 bases = 1byte (8bits)
  encode base and 'roll' table
  roll = 0-23 table (24 values)
       each roll adds 6 (changes encoding)
       roll depending on last base encoded
       (^NOTE: may want to change to non-data related roll to avoid massive errors when decoding)

encode method:
encoded byte 00101111 (0x2F in hex, or '/' in ascii)
break binary into sections 00-10-11-11
use table to encode
ACGT
A = 00
C = 01
G = 10
T = 11

00-10-11-11
^start here

A = 00
add 'A' to output
'roll' through table

00-10-11-11
   ^now here

repeat

exceptions: homopolymer nucleotide repeats
when HOMO_REPEATS is hit, add bs (bit shift, bullshit, whichever) to buffer

*/

//32bit x 24 array
const u_int G4ROLL [24] = {
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
const u_char HOMO_REPEAT = 3;

int g_tbl_pos=0, g_tbl_add = 0;//gTblPos should be 0-23

void tblSet(u_char p_last) { //time for dinner!
     g_tbl_pos += 6;
     if(g_tbl_pos > 23)
       g_tbl_pos -= 24;

     if      (p_last == (u_char) (G4ROLL[g_tbl_pos]) )
       g_tbl_add=3;
     else if(p_last == (u_char) (G4ROLL[g_tbl_pos])>>8)
       g_tbl_add=1;
     else if(p_last == (u_char) (G4ROLL[g_tbl_pos])>>16)
       g_tbl_add=2;
     else if(p_last == (u_char) (G4ROLL[g_tbl_pos])>>24)
       g_tbl_add=6;

     g_tbl_pos += g_tbl_add;
     if(g_tbl_pos > 23)
       g_tbl_pos -= 24;
     if(g_tbl_pos > 23) {
       printf("ERROR! table pos outside bounds!\nEXITING!\n");
       return;
     }//if

     //printf("new g_tbl_pos is %d - output was %c\n", gTblPos, p_last);
}//tblSet


int main() {

  char * file_buf;
  u_char * out_buf, lst_nuc=0, b1=0, b2=0, b3=0;//b4=0;
  long long file_sz=0, i=0, k=0, n_cnt=1, bs_offset=0, shift=0;
  u_char dt_enc = 0;//read byte by byte
  FILE *read_file;

/*
  printf("using table\n");
  for(i=0;i<24;i++) {
   printf("%d = %c%c%c%c\n", i, G4ROLL[i]>>24, G4ROLL[i]>>16, G4ROLL[i]>>8, G4ROLL[i]);
  }//for 24 (i)
*/

  read_file = fopen("/tmp/test", "r");
  fseek(read_file, 0L, SEEK_END);
  file_sz = ftell(read_file);
  rewind(read_file);

  file_buf = malloc(file_sz+1);
  out_buf  = malloc(file_sz*5+1);
  //*5 for bs_offset safety
  //unlikely to be full of XXX repeats
  //should only need 25% increase in size
  //max 30%(?) XXXYYYZZZ
  //           rrrGrrGrr (G=garbage,r=real)
  memset(file_buf, 0x0, file_sz+1);
  memset(out_buf, 0x0, file_sz*5+1);
  fread(file_buf, file_sz, 1, read_file);

  for(i=0; i<file_sz; i++) {
     //printf("filePos is %d\n", i);
     dt_enc = file_buf[i];

     for(k=0; k<4; k++) {
          //00-00-00-00 byte encoded
          b1 = (3<<(2*(3-k)));// 11
          b2 = (2<<(2*(3-k)));// 10
          b3 = (1<<(2*(3-k)));// 01
          //b4 = 0;              00
          //  k == 0 -  1 -  2 -  3
          //take  00 - 10 - 11 - 11
          //check 00, add nucleotide
          //check 10, add nuc...
          if(dt_enc >= b1 ) {      //>= 192,48,12,3
            dt_enc -= b1;
            //shift = 0;
          }
          else if(dt_enc >= b2 ) { //>=  128,32,8,2
            dt_enc -= b2;
            shift = 8;
          }
          else if(dt_enc >= b3 ) { //>=  64,16,4,1
            dt_enc -= b3;
            shift = 16;
          }
          else { //00b
            shift = 24;
          }//else
          out_buf[i*4+k+bs_offset] = (u_char)((G4ROLL[g_tbl_pos])>>shift);
          //printf("coded %c from TBLSET = %d\n", out_buf[i*4+k+bs_offset], g_tbl_pos);
          if(lst_nuc == out_buf[i*4+k+bs_offset])
            n_cnt++;
          else
            n_cnt = 1;
          if(n_cnt >= HOMO_REPEAT) {
            if(shift>7) shift -= 8;
            else        shift += 8;
            bs_offset++;
            out_buf[i*4+k+bs_offset] = (u_char)((G4ROLL[g_tbl_pos])>>shift);
            //^add bs value, problem is this rolls the table next
            //could lead to huge errors from corrupt input in decode
            //ALSO: using non-data value for 'roll' could allow for multi-threading
            n_cnt = 1;
          }
          lst_nuc = out_buf[i*4+k+bs_offset];
          tblSet(lst_nuc);
          shift = 0;
     }//k

     //just some more rolling 'randomness'
     if(i % 24)
       g_tbl_pos += 7;
     if(g_tbl_pos>23)
       g_tbl_pos -= 24;

  };//i, file

//printf("DNA ENCODED DATA:\n%s\n---EOF---\n", out_buf);
  printf("%s",out_buf);
//printf("\nbs_offset = %d\n", bs_offset);
  free(file_buf);
  free(out_buf);
  fclose(read_file);
  return 0;

}//main

