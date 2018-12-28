using namespace std;
#ifndef UPRIGHT_LZW_H
#define UPRIGHT_LZW_H
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <assert.h>
#define ATLASSERT assert



#define MIN_CODE_LEN    9
#define MAX_CODE_LEN    20
#define CURRENT_MAX_CODES(x)     (1UL << (x))

#define FIRST_CODE      (1 << CHAR_BIT)

#if (MIN_CODE_LEN <= CHAR_BIT)
#error Code words must be larger than 1 character
#endif

#if (MAX_CODE_LEN >= 25)
#endif

#define BITS 17
#define HASHING_SHIFT (BITS-8)
#define MAX_VALUE (1 << BITS) - 1
#define MAX_CODE MAX_VALUE - 1


#if BITS == 20
	#define TABLE_SIZE 1048583
#elif BITS == 19
	#define TABLE_SIZE 524309
#elif BITS == 18
	#define TABLE_SIZE 262147
#elif BITS == 17
	#define TABLE_SIZE 131101
#elif BITS == 16
	#define TABLE_SIZE 65543
#elif BITS == 15
	#define TABLE_SIZE 32797
#elif BITS == 14
  #define TABLE_SIZE 18041

#elif BITS == 13
  #define TABLE_SIZE 9029
#elif BITS == 12
  #define TABLE_SIZE 5021
#else
#error define smaller or bigger table sizes
#endif

#if (TABLE_SIZE <= MAX_VALUE)
#error your prime numbers need attention
#endif

#if (BITS > MAX_CODE_LEN)
#error BITS can only go up to a maximum
#endif


class CLZWImpl {
protected:
int *code_value;
unsigned int *prefix_code;
unsigned char *append_character;
unsigned char decode_stack[4000];
unsigned char CUR_BITS;

int input_bit_count, output_bit_count;
unsigned long input_bit_buffer, output_bit_buffer;

public:
CLZWImpl() {
	code_value = 0;
	prefix_code = 0;
	append_character = 0;
}

~CLZWImpl() {
	if(code_value)
		free(code_value);
	if(prefix_code)
		free(prefix_code);
	if(append_character)
		free(append_character);
}

int get_bits() { return CUR_BITS; }

protected:
int Init() {
	ATLASSERT(!code_value);

	code_value=(int*)malloc(TABLE_SIZE*sizeof(int));
	prefix_code=(unsigned int*)malloc(TABLE_SIZE*sizeof(unsigned int));
	append_character=(unsigned char*)malloc(TABLE_SIZE*sizeof(unsigned char));

	return code_value != 0 && prefix_code != 0 && append_character != 0;
}


virtual int getc_src() = 0;

virtual int getc_comp() = 0;

virtual int putc_comp(int ch) = 0;

virtual int putc_out(int ch) = 0;


void compress()
{
unsigned int next_code;
unsigned int character;
unsigned int string_code;
unsigned int index;
unsigned int bit_limit;
int i;

	ATLASSERT(code_value);

	CUR_BITS = MIN_CODE_LEN;
	bit_limit = CURRENT_MAX_CODES(CUR_BITS) - 1;
	output_bit_count=0;
	output_bit_buffer=0L;

	ATLASSERT(256==FIRST_CODE);
	next_code=FIRST_CODE;
	for (i=0;i<TABLE_SIZE;i++)
		code_value[i]=-1;

  string_code=getc_src();
  if(-1 == string_code)
	  return;


  while ((character=getc_src()) != -1)
  {
    index=find_match(string_code,character);
    if (code_value[index] != -1)
      string_code=code_value[index];
    else
    {
      if (next_code <= MAX_CODE)
      {
        code_value[index]=next_code++;
        prefix_code[index]=string_code;
        append_character[index]=character;
      }


		if(string_code >= bit_limit && CUR_BITS < BITS)
		{

			output_code(bit_limit);
			CUR_BITS++;
			bit_limit = (CURRENT_MAX_CODES(CUR_BITS) - 1);
		}

		ATLASSERT(string_code < bit_limit);

      output_code(string_code);
      string_code=character;
    }
  }
output_code(string_code);
  output_code(-1);
}


int find_match(unsigned int hash_prefix,unsigned int hash_character)
{
int index;
int offset;

  index = (hash_character << HASHING_SHIFT) ^ hash_prefix;
  if (index == 0)
    offset = 1;
  else
    offset = TABLE_SIZE - index;
  while (1)
  {
    if (code_value[index] == -1)
      return(index);
    if (prefix_code[index] == hash_prefix &&
        append_character[index] == hash_character)
      return(index);
    index -= offset;
    if (index < 0)
      index += TABLE_SIZE;
  }
}


void expand()
{
unsigned int next_code;
unsigned int new_code;
unsigned int old_code;
int character;
unsigned char *string;
unsigned int bit_limit;

	ATLASSERT(code_value);

	CUR_BITS = MIN_CODE_LEN;
	bit_limit = CURRENT_MAX_CODES(CUR_BITS) - 1;
	input_bit_count=0;
	input_bit_buffer=0L;



  next_code=FIRST_CODE;

  old_code=input_code();
  if(-1 == old_code)
	  return;
  character=old_code;
  if(putc_out(old_code)==-1)
	  return;

  while ((new_code=input_code()) != (-1))
  {

	  if(bit_limit == new_code && CUR_BITS < BITS)
	  {
		  CUR_BITS++;
		  bit_limit = CURRENT_MAX_CODES(CUR_BITS) - 1;

		  new_code=input_code();
		  ATLASSERT(new_code != -1);
		  if(new_code == -1)
			  break;
	  }

  		ATLASSERT(new_code < bit_limit);


    if (new_code>=next_code)
    {
      *decode_stack=character;
      string=decode_string(decode_stack+1,old_code);
    }

    else
      string=decode_string(decode_stack,new_code);

    character=*string;
    while (string >= decode_stack)
      putc_out(*string--);

    if (next_code <= MAX_CODE)
    {
      prefix_code[next_code]=old_code;
      append_character[next_code]=character;
      next_code++;
    }
    old_code=new_code;
  }
}


unsigned char *decode_string(unsigned char *buffer,unsigned int code)
{
int i;

  i=0;
  while (code >= FIRST_CODE)
  {
    *buffer++ = append_character[code];
    code=prefix_code[code];
	 i++;
	 ATLASSERT(i < sizeof(decode_stack));
  }
  *buffer=code;
  return(buffer);
}



unsigned int input_code()
{
int c;
unsigned int return_value;


  while (input_bit_count <= 24)
  {
		if ((c = getc_comp()) == -1)
			break;

    input_bit_buffer |=
        (unsigned long) c << (24-input_bit_count);
    input_bit_count += 8;
  }

  if(input_bit_count < CUR_BITS) {
	  ATLASSERT(!input_bit_buffer);
		return -1; /* EOF */
	}

  return_value=input_bit_buffer >> (32-CUR_BITS);
  input_bit_buffer <<= CUR_BITS;
  input_bit_count -= CUR_BITS;

  ATLASSERT(return_value < (1UL << CUR_BITS));
  return(return_value);
}

void output_code(unsigned int code)
{


	ATLASSERT(output_bit_count < 8);
	ATLASSERT(CUR_BITS + output_bit_count <= 32);


	if(-1 == code) {

		if(output_bit_count) {
			output_bit_buffer >>= 24;
			ATLASSERT((output_bit_buffer & 0xFF) == output_bit_buffer);
			putc_comp(output_bit_buffer);

			output_bit_count = 0;
			output_bit_buffer = 0;
		}

		return;
	}

	ATLASSERT(code < (1UL << CUR_BITS));


  output_bit_buffer |= (unsigned long) code << (32-CUR_BITS-output_bit_count);
  output_bit_count += CUR_BITS;
  while (output_bit_count >= 8)
  {

    putc_comp(output_bit_buffer >> 24);
    output_bit_buffer <<= 8;
    output_bit_count -= 8;
  }
}
};
class CLZWCompressFile : public CLZWImpl {
public:
	CLZWCompressFile() {
		io_file = 0;
		lzw_file = 0;
	};

	~CLZWCompressFile() {
		ATLASSERT(!io_file);
		ATLASSERT(!lzw_file);
	};

	int AnyIOErrors() {return io_error; }
	unsigned int Compress(char* input_file_name, char* to_name)
	{
		ATLASSERT(input_file_name && *input_file_name);
		ATLASSERT(to_name && *to_name);
		ATLASSERT(strcmp(to_name, input_file_name));

		io_error = 1;

		if(!code_value)
			if(!Init())
				return 0;

		u_comp = 0;
		u_io = 0;
		io_file=fopen(input_file_name,"rb");
		if(io_file) {
			lzw_file=fopen(to_name,"wb");
			if(lzw_file) {

				putc('L', lzw_file);
				if(putc(MIN_CODE_LEN, lzw_file) == MIN_CODE_LEN) {
					compress();
					io_error = ferror(lzw_file) || ferror(io_file);
					if(!io_error)
						ATLASSERT(u_comp <= u_io);
				}
				fclose(lzw_file);
				lzw_file = 0;
			}

			fclose(io_file);
			io_file = 0;
		}

		return u_comp;
	}

	unsigned int Expand(char* lzw_name, char* to_name)
	{
		ATLASSERT(lzw_name && *lzw_name);
		ATLASSERT(to_name && *to_name);
		ATLASSERT(strcmp(to_name, lzw_name));

		io_error = 1;

		if(!code_value)
			if(!Init())
				return 0;

		u_comp = 0;
		u_io = 0;
		lzw_file=fopen(lzw_name,"rb");
		if(lzw_file) {

			int ch1 = getc(lzw_file);
			int ch2 = getc(lzw_file);
			if('L' == ch1 && MIN_CODE_LEN==ch2) {
				io_file=fopen(to_name,"wb");
				if(io_file) {
					expand();
					io_error = ferror(lzw_file) || ferror(io_file);

					fclose(io_file);
					io_file = 0;
				}
			}

			fclose(lzw_file);
			lzw_file = 0;
		}

		return u_io;
	}

protected:

	virtual int getc_src() {
		ATLASSERT(io_file);
		int ch = getc(io_file);
		if(EOF == ch)
			return -1;

		u_io++;
		return ch;
	}
	virtual int getc_comp() {
		ATLASSERT(lzw_file);
		int ch = getc(lzw_file);
		if(EOF == ch)
			return -1;

		u_comp++;
		return ch;
	}
	virtual int putc_comp(int ch) {
		ATLASSERT(lzw_file);
		ATLASSERT(ch >= 0 && ch < 256);
		int ret = putc(ch, lzw_file);

		if(ret != EOF) {
			ATLASSERT(ret == ch);
			u_comp++;
		}
		else
			ret = -1;

		return ret;
	}
	virtual int putc_out(int ch) {
		ATLASSERT(io_file);
		ATLASSERT(ch >= 0 && ch < 256);
		int ret = putc(ch, io_file);

		if(ret != EOF)
			u_io++;
		else
			ret = -1;

		return ret;
	}

	FILE* io_file;
	FILE *lzw_file;
	int io_error;
public:
	unsigned long u_io, u_comp;
};
#endif
