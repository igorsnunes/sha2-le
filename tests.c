#include "sha2.h"

static int error_occured = 0;

void print_error(int line) {
	printf("ERROR on line %d!\n", line);
	error_occured = 1;
}

void print_error_arg(int line, size_t arg) {
	printf("ERROR on line %d! Output: %zu\n", line, arg);
	error_occured = 1;
}

int main () {
	if(rotate_right(1, 2) != 0x40000000) print_error(__LINE__);
	if(rotate_right(2, 2) != 0x80000000) print_error(__LINE__);
	if(calc_s0(0x80000) != 0x11002) print_error(__LINE__);
	if(calc_s1(0x800000) != 0x2050) print_error(__LINE__);
	if(calc_S0(0x8000000) != 0x2004020) print_error(__LINE__);
	if(calc_S1(0x8000000) != 0x210004) print_error(__LINE__);
	if(calc_ch(0xABABABAB,0xCDCDCDCD,0xEFEFEFEF) != 0xCDCDCDCD) print_error(__LINE__);
	if(calc_maj(0xABABABAB,0xCDCDCDCD,0xEFEFEFEF) != 0xEFEFEFEF) print_error(__LINE__);

	size_t size, padded_size;
	size =  3; calculate_padded_msg_size(size, &padded_size); if (size + padded_size !=  64) print_error_arg(__LINE__, padded_size);
	calculate_padded_msg_size_FIPS_180_4(size, &padded_size); if (size + padded_size !=  64) print_error_arg(__LINE__, padded_size);
	size = 63; calculate_padded_msg_size(size, &padded_size); if (size + padded_size != 128) print_error_arg(__LINE__, padded_size);
	calculate_padded_msg_size_FIPS_180_4(size, &padded_size); if (size + padded_size != 128) print_error_arg(__LINE__, padded_size);
	size = 64; calculate_padded_msg_size(size, &padded_size); if (size + padded_size != 128) print_error_arg(__LINE__, padded_size);
	calculate_padded_msg_size_FIPS_180_4(size, &padded_size); if (size + padded_size != 128) print_error_arg(__LINE__, padded_size);
	size = 65; calculate_padded_msg_size(size, &padded_size); if (size + padded_size != 128) print_error_arg(__LINE__, padded_size);
	calculate_padded_msg_size_FIPS_180_4(size, &padded_size); if (size + padded_size != 128) print_error_arg(__LINE__, padded_size);
	// testing around 5G
	size = 5368709119; calculate_padded_msg_size(size, &padded_size); if (size + padded_size != 5368709184) print_error_arg(__LINE__, padded_size);
	        calculate_padded_msg_size_FIPS_180_4(size, &padded_size); if (size + padded_size != 5368709184) print_error_arg(__LINE__, padded_size);
	size = 5368709120; calculate_padded_msg_size(size, &padded_size); if (size + padded_size != 5368709184) print_error_arg(__LINE__, padded_size);
	        calculate_padded_msg_size_FIPS_180_4(size, &padded_size); if (size + padded_size != 5368709184) print_error_arg(__LINE__, padded_size);
	size = 5368709121; calculate_padded_msg_size(size, &padded_size); if (size + padded_size != 5368709184) print_error_arg(__LINE__, padded_size);
	        calculate_padded_msg_size_FIPS_180_4(size, &padded_size); if (size + padded_size != 5368709184) print_error_arg(__LINE__, padded_size);

	return error_occured;
}

// if using vim with "set modeline" on your .vimrc, then this file will be
// automatically configured as:
// vim: noai:ts=4:sw=4:sts=4:noet :
