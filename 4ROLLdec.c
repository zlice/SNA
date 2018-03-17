#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
  table 'key' has 4 bases and 24 total size with permutations
  4 bases = 1byte (8bits)
  dencode base and 'roll' table
  roll = 0-23 table (24 values)
       each roll adds 6 (changes dencoding)
       roll depending on last base dencoded
       (^NOTE: may want to change to non-data related roll to avoid massive errors when decoding)

dencode method:
dencoded byte 00101111 (0x2F in hex, or '/' in ascii)
break binary into sections 00-10-11-11
use table to dencode
ACGT
A = 00
C = 01
G = 10
T = 11

00-10-11-11
^start here

A = 00
add '0' to output (no nothing)
'roll' through table

00-10-11-11
   ^now here

repeat

exceptions: homopolymer nucleotide repeats
when HOMO_REPEATS is hit, ignore bs (bit shift, bullshit, whichever) in buffer

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

int g_tbl_pos=0, g_tbl_add = 0;//g_tbl_pos should be 0-23

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

     //printf("new g_tbl_pos is %d - output was %c\n", g_tbl_pos, p_last);
}//tblSet


int main() {

  char * file_buf;
  u_char * out_buf, lst_nuc=0, dt_dec;
  long long file_sz=0, i=0, k=0, n_cnt=1, bs_offset=0;
  FILE *read_file,*out_file;

/*
  printf("using table\n");
  for(i=0;i<24;i++) {
   printf("%lld = %c%c%c%c\n", i, G4ROLL[i]>>24, G4ROLL[i]>>16, G4ROLL[i]>>8, G4ROLL[i]);
  }//for 24 (i)
*/

  read_file = fopen("/tmp/test.dec", "r");
  fseek(read_file, 0L, SEEK_END);
  file_sz = ftell(read_file);
  rewind(read_file);

  file_buf = malloc(file_sz+1);
  out_buf  = malloc(file_sz/4+4);//not always even but more is fine
  memset(file_buf, 0x0, file_sz+1);
  memset(out_buf, 0x0, file_sz/4+4);
  fread(file_buf, file_sz, 1, read_file);

  //printf("%s\n---EOF---\n", file_buf);

  for(i=0; (i+(bs_offset/4))<(file_sz/4); i++) {
     for(k=0; k<4; k++) {

          dt_dec = file_buf[i*4+k+bs_offset];
          if(n_cnt >= HOMO_REPEAT) {
            lst_nuc = dt_dec;
            tblSet(lst_nuc);
            bs_offset++;
            n_cnt = 1;
            k--;     //go back a chunk
            continue;//skip the bs value
          }

          //printf("data read...%c\n", dt_dec);
          if(lst_nuc == dt_dec)
            n_cnt++;
          else
            n_cnt = 1;
          lst_nuc = dt_dec;

          //           k == 0- 1- 2- 3
          //decode ACGT to 00-00-00-00 byte, chunk by chunk
          if(dt_dec == (u_char)((G4ROLL[g_tbl_pos])) )
            out_buf[i] += (u_char)(3<<(2*(3-k)));//192,48,12,3
          else if(dt_dec == (u_char)((G4ROLL[g_tbl_pos])>>8) )
            out_buf[i] += (u_char)(2<<(2*(3-k)));//128,32,8,2
          else if(dt_dec == (u_char)((G4ROLL[g_tbl_pos])>>16) )
            out_buf[i] += (u_char)(1<<(2*(3-k)));//64,16,4,1
          if(n_cnt < HOMO_REPEAT)
            tblSet(lst_nuc);
     }//k

     //same roll 'randomness' as encode
     if(i % 24)
       g_tbl_pos += 7;
     if(g_tbl_pos>23)
       g_tbl_pos -= 24;

  };//i, file

  printf("outputting file to /tmp/output with size %lld\n", file_sz/4);
  printf("had %lld junk nucleotides\n", bs_offset);
  out_file = fopen("/tmp/output", "wb");
  fwrite(out_buf, 1, (file_sz/4) - (bs_offset/4), out_file);
  //                               ^okay if this hits x.25 ?
  free(file_buf);
  fclose(read_file);
  free(out_buf);
  fclose(out_file);
  return 0;

}//main

