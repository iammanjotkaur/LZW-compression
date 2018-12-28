/*
C++ mini project
Submitted by-
Manjot kaur
sem 2
01211604417
Implementation of LZW algorithm to compress and decompress text files
Input file is data.txt
Compressed file is test.lzw
decompressed file is test.o
*/

using namespace std;
#include<iostream>
#include <time.h>
#include "lzw.h"
main(int argc, char *argv[])
{
clock_t timer;
CLZWCompressFile lzw;

argv[1]="data.txt";
  cout<<"Compressing ..."<< argv[1];
/*
 Compress the file.
*/
  timer = clock();
  int crunch = lzw.Compress(argv[1], "test.lzw");
  timer = clock() - timer; //CLOCKS_PER_SEC
  //cout<<"compress time="<<timer<<"ms"<<" encoding="<<lzw.get_bits()<<" size= "<<crunch;
  cout<<"\nFile compressed";
  int filesize = lzw.u_io;
  printf(" (ratio=%d%%)\n", filesize ? (filesize-crunch)*100/filesize : 0);
  if(lzw.AnyIOErrors())
	  cout<<"***I/O ERROR***\n";

/*
 Expand the file.
*/
   timer = clock();
  int orig = lzw.Expand("test.lzw", "test.out");
  timer = clock() - timer;
  //cout<<"expand time= "<<timer<<" ms"<<"encoding= "<<lzw.get_bits();
  cout<<"File decompressed";
	if(lzw.AnyIOErrors())
	  cout<<"***I/O ERROR***\n";

  ATLASSERT(filesize == orig);
  return 0;
}
