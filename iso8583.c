#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "iso8583.h"

static void hexdump(unsigned char *s, unsigned int l)
{
	int i;
	for (i = 0; i < l; i++)
		printf("%02x", s[i]);

	printf("\n");
}

static struct iso8583_field default_fields[129] = {
	{   4, '0', ISO8583_FIX   , ISO8583_R, ISO8583_Z },   // Message Type		   n	  4						
	{   1, 'D', ISO8583_FIX   , ISO8583_L, ISO8583_U },   // BIT MAP EXTENDED	   b	  1						
	{  19, 'F', ISO8583_LLVAR , ISO8583_L, ISO8583_Z },   // PRIMARY ACC. NUM	   n	 19 llvar				  
	{   6, '0', ISO8583_FIX   , ISO8583_R, ISO8583_Z },   // PROCESSING CODE		n	  6						
	{  12, '0', ISO8583_FIX   , ISO8583_R, ISO8583_Z },   // AMOUNT, TRANS.		 n	 12						
	{  12, '0', ISO8583_FIX   , ISO8583_R, ISO8583_Z },   // AMOUNT, SETTLEMENT	 n	 12						
	{  12, '0', ISO8583_FIX   , ISO8583_R, ISO8583_Z },   // AMOUNT,CardHolder bill n	 12						
	{  10, '0', ISO8583_FIX   , ISO8583_R, ISO8583_Z },   // TRANSMISSION D & T	 n	 10 mmddhhmmss			 
	{   8, '0', ISO8583_FIX   , ISO8583_R, ISO8583_Z },   // AMN., CH BILLING FEE   n	  8						
	{   8, '0', ISO8583_FIX   , ISO8583_R, ISO8583_Z },   // CONV RATE,SET'T		n	  8						
	{   8, '0', ISO8583_FIX   , ISO8583_R, ISO8583_Z },   // CONV RATE, CH billing  n	  8						
	{   6, '0', ISO8583_FIX   , ISO8583_R, ISO8583_Z },   // SYSTEM TRACE #		 n	  6						
	{   6, '0', ISO8583_FIX   , ISO8583_R, ISO8583_Z },   // TIME, LOCAL TRAN	   n	  6 hhmmss				 
	{   4, '0', ISO8583_FIX   , ISO8583_R, ISO8583_Z },   // DATE, LOCAL TRAN	   n	  4 mmdd				   
	{   4, '0', ISO8583_FIX   , ISO8583_R, ISO8583_Z },   // DATE, EXPIRATION	   n	  4 yymm				   
	{   4, '0', ISO8583_FIX   , ISO8583_R, ISO8583_Z },   // DATE, SETTLEMENT	   n	  4 mmdd				   
	{   4, '0', ISO8583_FIX   , ISO8583_R, ISO8583_Z },   // DATE, CONVERSION	   n	  4 mmdd				   
	{   4, '0', ISO8583_FIX   , ISO8583_R, ISO8583_Z },   // DATE, CAPTURE		  n	  4 mmdd				   
	{   4, '0', ISO8583_FIX   , ISO8583_R, ISO8583_Z },   // MERCHANT'S TYPE		n	  4						
	{   3, '0', ISO8583_FIX   , ISO8583_R, ISO8583_Z },   // AI COUNTRY CODE		n	  3						
	{   3, 'F', ISO8583_FIX   , ISO8583_L, ISO8583_Z },   // PAN EXT.,CO'Y CODE	 n	  3						
	{   3, '0', ISO8583_FIX   , ISO8583_R, ISO8583_Z },   // FI COUNTRY CODE		n	  3						
	{   3, 'D', ISO8583_FIX   , ISO8583_L, ISO8583_Z },   // POS ENTRY MODE		 n	  3						
	{   3, 'F', ISO8583_FIX   , ISO8583_L, ISO8583_Z },   // CARD SEQUECE NUM.	  n	  3						
	{   3, 'D', ISO8583_FIX   , ISO8583_L, ISO8583_Z },   // NETWORK INT'L ID	   n	  3						
	{   2, 'D', ISO8583_FIX   , ISO8583_R, ISO8583_Z },   // POS COND. CODE		 n	  2						
	{   2, 'D', ISO8583_FIX   , ISO8583_L, ISO8583_Z },   // POS PIN CAP. CODE	  n	  2						
	{   1, '0', ISO8583_FIX   , ISO8583_R, ISO8583_Z },   // AUTH ID RES. LEN	   n	  1						
	{   8, '0', ISO8583_FIX   , ISO8583_R, ISO8583_Z },   // AMT. TRANS FEE		 n	  8						
	{   8, '0', ISO8583_FIX   , ISO8583_R, ISO8583_Z },   // AMT. SETT.  FEE		n	  8						
	{   8, '0', ISO8583_FIX   , ISO8583_R, ISO8583_Z },   // AMT. TRAN PROC FEE	 n	  8						
	{   8, '0', ISO8583_FIX   , ISO8583_R, ISO8583_Z },   // AMT. SET PROC FEE	  n	  8						
	{  11, '0', ISO8583_LLVAR , ISO8583_L, ISO8583_Z },   // ACOUIR. INST. ID	   n	 11 llvar				  
	{  11, 'F', ISO8583_LLVAR , ISO8583_L, ISO8583_Z },   // FI ID				  n	 11 llvar				  
	{  11, 'F', ISO8583_LLVAR , ISO8583_L, ISO8583_Z },   // PAN EXTENDED		   n	 28 llvar				  
	{  37, 'F', ISO8583_LLVAR , ISO8583_L, ISO8583_Z },   // TRACK 2 DATA		   z	 37 llvar				  
	{ 104, 'F', ISO8583_LLLVAR, ISO8583_L, ISO8583_Z },   // TRACK 3 DATA		   z	104 lllvar				 
	{  12, 'D', ISO8583_FIX   , ISO8583_L, ISO8583_U },   // RETR. REF. NUM		 an	12						
	{   6, 'D', ISO8583_FIX   , ISO8583_R, ISO8583_U },   // AUTH. ID. RESP		 an	 6						
	{   2, 'D', ISO8583_FIX   , ISO8583_R, ISO8583_U },   // RESPONSE CODE		  an	 2						
	{   3, 'D', ISO8583_FIX   , ISO8583_L, ISO8583_U },   // SERV. REST'N CODE	  an	 3						
	{   8, 'D', ISO8583_FIX   , ISO8583_R, ISO8583_U },   // TERMINAL ID			ans	8						
	{  15, 'F', ISO8583_FIX   , ISO8583_L, ISO8583_U },   // CARD ACC. ID		   ans   15						
	{  40, ' ', ISO8583_FIX   , ISO8583_L, ISO8583_U },   // CARD ACC. NAME		 ans   40						
	{  25, '0', ISO8583_LLVAR , ISO8583_R, ISO8583_U },   // ADD. RESP DATA		 an	25 llvar				  
	{  76, 'F', ISO8583_LLVAR , ISO8583_L, ISO8583_U },   // TRACK 1 DATA		   an	76 llvar				  
	{ 999, 'F', ISO8583_LLLVAR, ISO8583_L, ISO8583_U },   // ADD. DATA - ISO		an   999 lllvar				 
	{ 999, 'F', ISO8583_LLLVAR, ISO8583_L, ISO8583_U },   // ADD. DATA - NATI.	  an   999 lllvar					  
	{ 999, 'F', ISO8583_LLLVAR, ISO8583_L, ISO8583_U },   // ADD. DATA - PRI.	   an   999 lllvar				 
	{   3, ' ', ISO8583_FIX   , ISO8583_L, ISO8583_U },   // CC, TRANSACTION		a	  3						
	{   3, '0', ISO8583_FIX   , ISO8583_L, ISO8583_U },   // CC, SETTLEMENT		 an	 3						
	{   3, '0', ISO8583_FIX   , ISO8583_L, ISO8583_U },   // CC, CH. BILLING		a	  3					   
	{   8, 'D', ISO8583_FIX   , ISO8583_R, ISO8583_U },   // PIN DATA			   b	  8						
	{  16, '0', ISO8583_FIX   , ISO8583_L, ISO8583_Z },   // SECU. CONT. INFO.	  n	 16						
	{ 120, 'F', ISO8583_LLLVAR, ISO8583_R, ISO8583_U },   // ADDITIONAL AMTS		an   120 LLLVAR				 
	{ 999, 'F', ISO8583_LLLVAR, ISO8583_L, ISO8583_U },   // REVERVED ISO		   ans  999 lllvar				 
	{ 999, 'F', ISO8583_LLLVAR, ISO8583_L, ISO8583_U },   // REVERVED ISO		   ans  999 lllvar				 
	{ 999, 'F', ISO8583_LLLVAR, ISO8583_L, ISO8583_U },   // REVERVED NATIONAL	  ans  999 lllvar				 
	{ 999, 'F', ISO8583_LLLVAR, ISO8583_L, ISO8583_U },   // REVERVED NATIONAL	  ans  999 lllvar				 
	{ 999, 'F', ISO8583_LLLVAR, ISO8583_L, ISO8583_U },   // REVERVED NATIONAL	  ans  999 lllvar
	{ 999, 'F', ISO8583_LLLVAR, ISO8583_L, ISO8583_Z },   // RESERVED - PRIV1	   ans  999 lllvar				 
	{ 999, 'F', ISO8583_LLLVAR, ISO8583_L, ISO8583_Z },   // RESERVED - PRIV2	   ans  999 lllvar				 
	{ 999, 'F', ISO8583_LLLVAR, ISO8583_L, ISO8583_U },   // RESERVED - PRIV2	   ans  999 lllvar				 
	{ 999, 'F', ISO8583_LLLVAR, ISO8583_L, ISO8583_U },   // RESERVED - PRIV3	   ans  999 lllvar				 
	{   8, 'D', ISO8583_FIX   , ISO8583_L, ISO8583_U },   // MSG. AUTH. CODE		b	  8						
	{   8, 'D', ISO8583_FIX   , ISO8583_L, ISO8583_U },   // BIT MAP, EXTENDED	  b	  8						
	{   1, 'D', ISO8583_FIX   , ISO8583_L, ISO8583_U },   // SETTLEMENT CODE		n	  1						
	{   2, 'D', ISO8583_FIX   , ISO8583_L, ISO8583_U },   // EXT. PAYMENT CODE	  n	  2						
	{   3, 'D', ISO8583_FIX   , ISO8583_L, ISO8583_U },   // RECE. INST. CN.		n	  3						
	{   3, 'D', ISO8583_FIX   , ISO8583_L, ISO8583_U },   // SETTLEMENT ICN.		n	  3						
	{   3, 'D', ISO8583_FIX   , ISO8583_L, ISO8583_U },   // NET MAN IC			 n	  3						
	{   4, 'D', ISO8583_FIX   , ISO8583_L, ISO8583_U },   // MESSAGE NUMBER		 n	  4						
	{   4, 'D', ISO8583_FIX   , ISO8583_L, ISO8583_U },   // MESSAGE NUM. LAST	  n	  4						
	{   6, 'D', ISO8583_FIX   , ISO8583_L, ISO8583_U },   // DATE, ACTION		   n	  6 yymmdd				 
	{  10, 'D', ISO8583_FIX   , ISO8583_L, ISO8583_U },   // CREDIT NUMBER		  n	 10						
	{  10, 'D', ISO8583_FIX   , ISO8583_L, ISO8583_U },   // CRED REVERSAL NUM	  n	 10						
	{  10, 'D', ISO8583_FIX   , ISO8583_L, ISO8583_U },   // DEBITS NUMBER		  n	 10						
	{  10, 'D', ISO8583_FIX   , ISO8583_L, ISO8583_U },   // DEBT REVERSAL NUM	  n	 10						
	{  10, 'D', ISO8583_FIX   , ISO8583_L, ISO8583_U },   // TRANSFER NUMBER		n	 10						
	{  10, 'D', ISO8583_FIX   , ISO8583_L, ISO8583_U },   // TRANS REVERSAL NUM	 n	 10						
	{  10, 'D', ISO8583_FIX   , ISO8583_L, ISO8583_U },   // INQUERIES NUMBER	   n	 10						
	{  10, 'D', ISO8583_FIX   , ISO8583_L, ISO8583_U },   // AUTHORIZE NUMBER	   n	 10						
	{  12, 'D', ISO8583_FIX   , ISO8583_L, ISO8583_U },   // CRED.PROC.FEE.AMT	  n	 12						
	{  12, 'D', ISO8583_FIX   , ISO8583_L, ISO8583_U },   // CRED.TRANS.FEE.AMT	 n	 12						
	{  12, 'D', ISO8583_FIX   , ISO8583_L, ISO8583_U },   // DEBT.PROC.FEE.AMT	  n	 12						
	{  12, 'D', ISO8583_FIX   , ISO8583_L, ISO8583_U },   // DEBT.TRANS.FEE.AMT	 n	 12						
	{  15, 'D', ISO8583_FIX   , ISO8583_L, ISO8583_U },   // CRED AMT			   n	 16						
	{  15, 'D', ISO8583_FIX   , ISO8583_L, ISO8583_U },   // CRED REVERSAL AMT	  n	 16						
	{  15, 'D', ISO8583_FIX   , ISO8583_L, ISO8583_U },   // DEBIT AMT			  n	 16						
	{  15, 'D', ISO8583_FIX   , ISO8583_L, ISO8583_U },   // DEBIT REVERSAL AMT	 n	 16						
	{  42, '0', ISO8583_FIX   , ISO8583_L, ISO8583_U },   // ORIGIN DATA ELEMNT	 n	 42						
	{   1, 'D', ISO8583_FIX   , ISO8583_L, ISO8583_U },   // FILE UPDATE CODE	   an	 1						
	{   2, 'D', ISO8583_FIX   , ISO8583_L, ISO8583_U },   // FILE SECURITY CODE	 n	  2						
	{   5, 'D', ISO8583_FIX   , ISO8583_L, ISO8583_U },   // RESPONSE INDICATOR	 n	  5						
	{   7, 'D', ISO8583_FIX   , ISO8583_L, ISO8583_U },   // SERVICE INDICATOR	  an	 7						
	{  42, 'D', ISO8583_FIX   , ISO8583_L, ISO8583_U },   // REPLACEMENT AMOUNT	 an	42						
	{   8, 'D', ISO8583_FIX   , ISO8583_L, ISO8583_U },   // MESSAGE SECUR CODE	 an	 8						
	{  16, 'D', ISO8583_FIX   , ISO8583_L, ISO8583_U },   // AMT.NET SETTLEMENT	 n	 16						
	{  25, 'D', ISO8583_FIX   , ISO8583_L, ISO8583_U },   // PAYEE				  ans   25						
	{  11, 'D', ISO8583_LLVAR , ISO8583_L, ISO8583_U },   // SETTLE.INST.IC		 n	 11 llvar				 
	{  11, 'D', ISO8583_LLVAR , ISO8583_L, ISO8583_U },   // RECE.INST.IC		   n	 11 llvar				  
	{  17, 'D', ISO8583_FIX   , ISO8583_L, ISO8583_U },   // FILE NAME			  ans   17						
	{  28, 'D', ISO8583_LLVAR , ISO8583_L, ISO8583_U },   // ACCOUNT ID 1		   ans   28 llvar				  
	{  28, 'D', ISO8583_LLVAR , ISO8583_L, ISO8583_U },   // ACCOUNT ID 2		   ans   28 llvar				  
	{ 100, 'D', ISO8583_LLLVAR, ISO8583_L, ISO8583_U },   // TRANS.DESCRIPTION	  ans  100 lllvar				 
	{ 999, 'D', ISO8583_LLLVAR, ISO8583_L, ISO8583_U },   // RESERVED FOR ISO	   ans  999 lllvar  
	{ 999, 'D', ISO8583_LLLVAR, ISO8583_L, ISO8583_U },   // RESERVED FOR ISO	   ans  999 lllvar				 
	{ 999, 'D', ISO8583_LLLVAR, ISO8583_L, ISO8583_U },   // RESERVED FOR ISO	   ans  999 lllvar				 
	{ 999, 'D', ISO8583_LLLVAR, ISO8583_L, ISO8583_U },   // RESERVED FOR ISO	   ans  999 lllvar				 
	{ 999, 'D', ISO8583_LLLVAR, ISO8583_L, ISO8583_U },   // RESERVED FOR ISO	   ans  999 lllvar				 
	{ 999, 'D', ISO8583_LLLVAR, ISO8583_L, ISO8583_U },   // RESERVED FOR ISO	   ans  999 lllvar				 
	{ 999, 'D', ISO8583_LLLVAR, ISO8583_L, ISO8583_U },   // RESERVED FOR ISO	   ans  999 lllvar				 
	{ 999, 'D', ISO8583_LLLVAR, ISO8583_L, ISO8583_U },   // RESERVED FOR NATIO	 ans  999 lllvar				 
	{ 999, 'D', ISO8583_LLLVAR, ISO8583_L, ISO8583_U },   // RESERVED FOR NATIO	 ans  999 lllvar				 
	{ 999, 'D', ISO8583_LLLVAR, ISO8583_L, ISO8583_U },   // RESERVED FOR NATIO	 ans  999 lllvar				 
	{ 999, 'D', ISO8583_LLLVAR, ISO8583_L, ISO8583_U },   // RESERVED FOR NATIO	 ans  999 lllvar				 
	{ 999, 'D', ISO8583_LLLVAR, ISO8583_L, ISO8583_U },   // RESERVED FOR NATIO	 ans  999 lllvar				 
	{ 999, 'D', ISO8583_LLLVAR, ISO8583_L, ISO8583_U },   // RESERVED FOR NATIO	 ans  999 lllvar				 
	{ 999, 'D', ISO8583_LLLVAR, ISO8583_L, ISO8583_U },   // RESERVED FOR NATIO	 ans  999 lllvar				 
	{ 999, 'D', ISO8583_LLLVAR, ISO8583_L, ISO8583_U },   // RESERVED FOR NATIO	 ans  999 lllvar				 
	{ 999, 'D', ISO8583_LLLVAR, ISO8583_L, ISO8583_U },   // RESERVED FOR PRIVA	 ans  999 lllvar				 
	{ 999, 'D', ISO8583_LLLVAR, ISO8583_L, ISO8583_U },   // RESERVED FOR PRIVA	 ans  999 lllvar				 
	{ 999, 'D', ISO8583_LLLVAR, ISO8583_L, ISO8583_U },   // RESERVED FOR PRIVA	 ans  999 lllvar				 
	{ 999, 'D', ISO8583_LLLVAR, ISO8583_L, ISO8583_U },   // RESERVED FOR PRIVA	 ans  999 lllvar				 
	{ 999, 'D', ISO8583_LLLVAR, ISO8583_L, ISO8583_U },   // RESERVED FOR PRIVA	 ans  999 lllvar				 
	{ 999, 'D', ISO8583_LLLVAR, ISO8583_L, ISO8583_U },   // RESERVED FOR PRIVA	 ans  999 lllvar				 
	{ 999, 'D', ISO8583_LLLVAR, ISO8583_L, ISO8583_U },   // RESERVED FOR PRIVA	 ans  999 lllvar				 
	{ 999, 'D', ISO8583_LLLVAR, ISO8583_L, ISO8583_U },	  // RESERVED FOR PRIVA	 ans  999 lllvar				 
	{   8, 'D', ISO8583_FIX,    ISO8583_L, ISO8583_U }	  // RESERVED FOR PRIVA	 ans  999 lllvar				 
};

static inline unsigned bcd2bin(unsigned char val)
{
	return (val & 0x0f) + (val >> 4) * 10;
}

static inline unsigned char bin2bcd(unsigned val)
{
	return ((val / 10) << 4) + val % 10;
}

static inline int hex_to_bin(char ch)
{
	if ((ch >= '0') && (ch <= '9'))
		return ch - '0';
	ch = tolower(ch);
	if ((ch >= 'a') && (ch <= 'f'))
		return ch - 'a' + 10;
	return -1;
}

static inline char bin_to_hex(int n)
{
	char *hex = "0123456789abcdef";

	if (n < 0 || n > 15)
		return -1;

	return hex[n];
}

static inline int check_index(unsigned int index)
{
	if (index > 128)		
		return ISO8583_FAILED;
	
	return ISO8583_OK;
}

static inline int check_pad(char pad, unsigned int compress)
{
	if (compress && (hex_to_bin(pad) == -1))
			return ISO8583_FAILED;
			
	return ISO8583_OK;
}

static inline int to_hex(unsigned char *data, unsigned char *bin, unsigned int size)
{
	int i;
	int begin = 0;

	for (i = 0; i < size; i++) {
		unsigned int byte = bin[i / 2];

		byte = (i % 2) ? byte & 0x0f : byte >> 4;

		data[i] = bin_to_hex(byte);
	}

	return ISO8583_OK;	
}

static inline int correct_size(unsigned int *size, unsigned int type, unsigned int fix_size)
{
	switch (type) {
	case ISO8583_FIX:
		*size = (*size > fix_size) ? fix_size : *size;
		break;
	case ISO8583_LLVAR:
		*size = (*size > 99) ? 99 : *size;
		break;
	case ISO8583_LLLVAR:
		*size = (*size > 9999) ? 9999 : *size;
		break;
	default:
		return ISO8583_FAILED;
	}   

	return ISO8583_OK;
}

static inline int get_8583size_from_userdata(unsigned int *iso8583_size, unsigned int user_size, 
								unsigned int type, unsigned int fix_size, unsigned int compress)
{
	switch (type) {
	case ISO8583_FIX:
		*iso8583_size = compress ? (fix_size + 1) / 2 : user_size;
		break;
	case ISO8583_LLVAR:
		*iso8583_size = compress ? (user_size + 1) / 2 + 1 : user_size + 1;
		break;
	case ISO8583_LLLVAR:
		*iso8583_size = compress ? (user_size + 1) / 2 + 2 : user_size + 2;
		break;
	}

	return ISO8583_OK;
}

static inline int str_compress_pad(unsigned char *dst, unsigned int dst_size,
										unsigned char *src, unsigned int src_size, 
										unsigned int align, char pad)
{
	int i;

	memset(dst, 0, dst_size);

	if (align == ISO8583_L) {

		int bit4, bit8;
		
		for (i = 0; i < src_size; i++) {
			if ((bit4 = hex_to_bin(src[i])) == -1)
				return ISO8583_FAILED;

			dst[i / 2] |= (i % 2) ? bit4 : bit4 << 4;
		}

		if ((bit4 = hex_to_bin(pad)) == -1)
			return ISO8583_FAILED;

		if (src_size % 2) 
			dst[src_size / 2] |= bit4;

		if ((src_size + 1) / 2 < dst_size) {
			bit8 = bit4 << 4 | bit4;
			for (i = (src_size + 1) / 2; i < dst_size; i++)	
				dst[i] = bit8;
		}

	} else {

		int bit4, bit8;
		int pad_len = dst_size * 2 - src_size;

		if ((bit4 = hex_to_bin(pad)) == -1)
			return ISO8583_FAILED;

		if (pad_len > 1) {
			bit8 = bit4 << 4 | bit4;
			for (i = 0; i < pad_len / 2; i++)
				dst[i] = bit8;
		}

		if (pad_len % 2)
			dst[pad_len / 2] |= bit4 << 4;
			
		for (i = pad_len; i < dst_size * 2; i++, src++) {
			if ((bit4 = hex_to_bin(*src)) == -1)
				return ISO8583_FAILED;

			dst[i / 2] |= (i % 2) ? bit4 : bit4 << 4;
		}
	}

	return ISO8583_OK;
}

static inline int str_pad(unsigned char *dst, unsigned int dst_size,
								unsigned char *src, unsigned int src_size, 
								unsigned int align, char pad)
{
	int i;

	if (align == ISO8583_L) {
		for (i = 0; i < src_size; i++)
			dst[i] = src[i];
		for (i = src_size; i < dst_size; i++)
			dst[i] = pad;
	} else {
		int pad_len = dst_size - src_size;

		for (i = 0; i < pad_len; i++)
			dst[i] = pad;
		for (i = pad_len; i < dst_size; i++, src++)
			dst[i] = *src;
	}

	return ISO8583_OK;
}

static inline int to_fix_8583data(unsigned char *iso8583_data, unsigned int iso8583_size, 
									unsigned char *user_data, unsigned int user_size, 
									unsigned int align, char pad, unsigned int compress)
{
	if (compress)
		return str_compress_pad(iso8583_data, iso8583_size, user_data, user_size, align, pad);
	else
		return str_pad(iso8583_data, iso8583_size, user_data, user_size, align, pad);
}

static inline int to_var_8583data(unsigned char *iso8583_data, unsigned int iso8583_size,
							unsigned char *user_data, unsigned int user_size,
							unsigned int align, char pad, unsigned int compress, unsigned int headlen)
{
	int i;
	unsigned int size = user_size;

	for (i = 0; i < headlen; i++) {
		iso8583_data[headlen - 1 - i] = bin2bcd(size % 100);
		size /= 100;   
	}

	return to_fix_8583data(iso8583_data + headlen, iso8583_size - headlen, user_data, user_size, align, pad, compress);
}

static inline int to_8583data(struct iso8583 *handle, unsigned int index, unsigned char *data, unsigned int *size)
{
	unsigned int iso8583_size;
	struct iso8583_field *field;
	struct iso8583_data *userdata;
	int ret;

	userdata = handle->datas[index];

	if (userdata == NULL) {
		*size = 0;	
		return ISO8583_OK;
	}

	field = handle->fields[index] ? handle->fields[index] : &default_fields[index];

	if (field->type > ISO8583_LLLVAR) {
		snprintf(handle->error, ISO8583_ERROR_SIZE, "wrong type! index = %d, type = %u!", index, field->type);
		return ISO8583_FAILED;
	}

	get_8583size_from_userdata(&iso8583_size, userdata->size, field->type, field->size, field->compress);

	if (iso8583_size > *size) {
		snprintf(handle->error, ISO8583_ERROR_SIZE, 
					"to_8583data() error! not enough size! index = %d, size = %u, iso8583_size = %u!", index, *size, iso8583_size);
		return ISO8583_FAILED;
	}

	switch (field->type) {
	case ISO8583_FIX:
		ret = to_fix_8583data(data, iso8583_size, userdata->data, userdata->size, field->align, field->pad, field->compress);
		break;
	case ISO8583_LLVAR:
		ret = to_var_8583data(data, iso8583_size, userdata->data, userdata->size, field->align, field->pad, field->compress, 1);
		break;
	case ISO8583_LLLVAR:
		ret = to_var_8583data(data, iso8583_size, userdata->data, userdata->size, field->align, field->pad, field->compress, 2);
		break;
	}

	if (ret != ISO8583_OK) {
		snprintf(handle->error, ISO8583_ERROR_SIZE, 
					"to_8583data() error! convert to 8583data failed! index = %d, size = %u, iso8583_size = %u!", index, *size, iso8583_size);
		return ISO8583_FAILED;
	}

	*size = iso8583_size;

	return ISO8583_OK;
}

static inline int get_8583size_from_8583data(unsigned int *size, unsigned char *iso8583_data, unsigned int iso8583_size,
								unsigned int compress, unsigned int type, unsigned int fix_length)
{
	unsigned int l;

	switch (type) {
	case ISO8583_FIX:
		*size = compress ? (fix_length + 1) / 2 : fix_length;
		break;
	case ISO8583_LLVAR:
		if (iso8583_size < 1)
			return ISO8583_FAILED;

		l = bcd2bin(iso8583_data[0]);	

		*size = compress ? 1 + (l + 1) / 2 : 1 + l;
		break;
	case ISO8583_LLLVAR:
		if (iso8583_size < 2)	
			return ISO8583_FAILED;

		l = bcd2bin(iso8583_data[0]) * 100 + bcd2bin(iso8583_data[1]);

		*size = compress ? 2 + (l + 1) / 2 : 2 + l;
		break;
	}

	if (*size > iso8583_size)
		return ISO8583_FAILED;	

	return ISO8583_OK;
}

static inline int get_usersize_from_8583size(unsigned int *size, unsigned int iso8583_size, 
												unsigned int compress, unsigned int type)
{
	switch (type) {
	case ISO8583_FIX:
		*size = compress ? iso8583_size * 2 : iso8583_size;
		break;
	case ISO8583_LLVAR:
		*size = compress ? (iso8583_size - 1) * 2 : iso8583_size - 1;	
		break;
	case ISO8583_LLLVAR:
		*size = compress ? (iso8583_size - 2) * 2 : iso8583_size - 2;
		break;
	}

	return ISO8583_OK;
}

static inline int to_userdata_fix(unsigned char *user_data, unsigned char *iso8583_data, 
									unsigned int user_size, unsigned int compress)
{
	if (compress) 
		to_hex(user_data, iso8583_data, user_size);
	else
		memcpy(user_data, iso8583_data, user_size);

	return ISO8583_OK;
}

static inline int to_userdata(struct iso8583 *handle, unsigned int index, unsigned char *data, unsigned int *size)
{
	struct iso8583_field *field = handle->fields[index] ? handle->fields[index] : &default_fields[index];		
	struct iso8583_data *user_data;
	unsigned int iso8583_size, user_size;
	int ret;

	ret = get_8583size_from_8583data(&iso8583_size, data, *size, field->compress, field->type, field->size);				

	if (ret != ISO8583_OK) {
		snprintf(handle->error, ISO8583_ERROR_SIZE, 
			"to_userdata() error! get_8583size_from_8583data error! compress = %u, type = %u, field->size = %u, size = %u!", 
			field->compress, field->type, field->size, *size);
		return ISO8583_FAILED;
	}

	get_usersize_from_8583size(&user_size, iso8583_size, field->compress, field->type);

	user_data = handle->datas[index];	

	if (user_data) {
		free(user_data->data);
	} else {
		user_data = (struct iso8583_data *)malloc(sizeof(struct iso8583_data));

		if (user_data == NULL) {
			snprintf(handle->error, ISO8583_ERROR_SIZE, "to_userdata() alloc iso8583_data memory failed!");
			return ISO8583_FAILED;
		}
	}

	user_data->data = (unsigned char *)malloc(user_size);

	if (user_data->data == NULL) {
		snprintf(handle->error, ISO8583_ERROR_SIZE, "to_userdata() alloc user_data memory failed!");
		free(user_data);
		handle->datas[index] = NULL;
		return ISO8583_FAILED;
	}

	switch (field->type) {
	case ISO8583_FIX:
		to_userdata_fix(user_data->data, data, user_size, field->compress);
		break;
	case ISO8583_LLVAR:
		to_userdata_fix(user_data->data, data + 1, user_size, field->compress);
		break;
	case ISO8583_LLLVAR:
		to_userdata_fix(user_data->data, data + 2, user_size, field->compress);
		break;
	}

	user_data->size = user_size;
	handle->datas[index] = user_data;

	*size = iso8583_size;

	return ISO8583_OK;
}

static inline unsigned int bitmap_get_bit(unsigned char *data, unsigned int index)
{
	int i = (index - 1) / 8;
	int j = (index - 1) % 8;

	return data[i] & (1 << (7 - j)) ? 1 : 0;
}

static inline void bitmap_set_bit(unsigned char *data, unsigned int index, unsigned int value)
{
	int i = (index - 1) / 8;
	int j = (index - 1) % 8;

	data[i] |= (value ? 1 : 0) << (7 - j);
}

struct iso8583 *iso8583_create()
{
	struct iso8583 *handle= malloc(sizeof(struct iso8583));

	if (!handle)
		return NULL;

	memset(handle, 0, sizeof(struct iso8583));

	return handle;
}

int iso8583_define(struct iso8583 *handle, unsigned int index, unsigned int size, char pad, unsigned int type, unsigned int align, unsigned int compress)
{
	struct iso8583_field *field;

	if (!handle) {
		snprintf(handle->error, ISO8583_ERROR_SIZE, "iso8583_define() argument error! the ptr of handle is null!");
		return ISO8583_FAILED;
	}

	if (check_index(index) != ISO8583_OK) {
		snprintf(handle->error, ISO8583_ERROR_SIZE, "iso8583_define() argument error! the index out of range! index = %d!", index);
		return ISO8583_FAILED;
	}

	if (check_pad(pad, compress) != ISO8583_OK)	{
		snprintf(handle->error, ISO8583_ERROR_SIZE, "iso8583_define() argument error! the pad can't be compressed! pad = %x!", pad);
		return ISO8583_FAILED;
	}

	if (align != ISO8583_L && align != ISO8583_R) {
		snprintf(handle->error, ISO8583_ERROR_SIZE, "iso8583_define() argument error! the align is invalid! align = %ud!", align);
		return ISO8583_FAILED;
	}

	if (type != ISO8583_FIX && type != ISO8583_LLVAR && type != ISO8583_LLLVAR) {
		snprintf(handle->error, ISO8583_ERROR_SIZE, "iso8583_define() argument error! the type is invalid! type = %ud!", type);
		return ISO8583_FAILED;
	}

	field = handle->fields[index];

	if (!field) {
		field = (struct iso8583_field *)malloc(sizeof(struct iso8583_field));
		if (!field) {
			snprintf(handle->error, ISO8583_ERROR_SIZE, "iso8583_define() argument error! alloc field memory failed!");
			return ISO8583_FAILED;
		}
		handle->fields[index] = field;		
	}

	field->size		= size;
	field->pad		= pad;
	field->type		= type;
	field->align	= align;
	field->compress = compress;

	return ISO8583_OK;
}

int iso8583_set(struct iso8583 *handle, unsigned int index, const unsigned char *data, unsigned int size)
{
	struct iso8583_field *field;
	struct iso8583_data *userdata;
	
	if (!handle) {
		snprintf(handle->error, ISO8583_ERROR_SIZE, "iso8583_set() argument error! the ptr of handle is null!");
		return ISO8583_FAILED;
	}

	if (check_index(index) != ISO8583_OK) {
		snprintf(handle->error, ISO8583_ERROR_SIZE, "iso8583_set() argument error! the index out of range! index = %d!", index);
		return ISO8583_FAILED;
	}

	userdata = handle->datas[index];

	if (!data || !size) {
		if (userdata) {
			free(userdata->data);
			free(userdata);
			handle->datas[index] = NULL;
		}
		return ISO8583_OK;
  	} 

	field = handle->fields[index] ? handle->fields[index] : &default_fields[index];

	if (correct_size(&size, field->type, field->size) != ISO8583_OK) {
		snprintf(handle->error, ISO8583_ERROR_SIZE, "iso8583_set() correct size error!");
		return ISO8583_FAILED;
	}

	if (userdata) {
		free(userdata->data);
	} else {
		userdata = (struct iso8583_data *)malloc(sizeof(struct iso8583_data));

		if (userdata == NULL) {
			snprintf(handle->error, ISO8583_ERROR_SIZE, "iso8583_set() alloc memory failed!");
			return ISO8583_FAILED;
		}
	}
		
	userdata->data = (unsigned char *)malloc(size);

	if (userdata->data == NULL) {
		snprintf(handle->error, ISO8583_ERROR_SIZE, "iso8583_set() alloc memory failed!");
		free(userdata);
		handle->datas[index] = NULL;
		return ISO8583_FAILED;
	}

	memcpy(userdata->data, data, size);

	userdata->size = size;	
	handle->datas[index] = userdata;

	return ISO8583_OK;
}

int iso8583_get(struct iso8583 *handle, unsigned int index, const unsigned char **data, unsigned int *size)
{
	struct iso8583_data *userdata;

	if (!handle) {
		snprintf(handle->error, ISO8583_ERROR_SIZE, "iso8583_get() argument error! the ptr of handle is null!");
		return ISO8583_FAILED;
	}

	if (check_index(index) != ISO8583_OK) {
		snprintf(handle->error, ISO8583_ERROR_SIZE, "iso8583_get() argument error! the index out of range! index = %d!", index);
		return ISO8583_FAILED;
	}

	userdata = handle->datas[index];

	*data = userdata ? userdata->data : NULL;
	*size = userdata ? userdata->size : 0;

	return ISO8583_OK;
}

int iso8583_pack(struct iso8583 *handle, unsigned char *data, unsigned int *size)
{
	unsigned int left_size, packed_size;
	unsigned int bitmap_n = 64;			  /* default */
	unsigned char *bitmap;
	unsigned int i;

	if (!handle) {
		snprintf(handle->error, ISO8583_ERROR_SIZE, "iso8583_pack() argument error! the ptr of handle is null!");
		return ISO8583_FAILED;
	}

	if (handle->datas[0] == NULL) {
		snprintf(handle->error, ISO8583_ERROR_SIZE, "iso8583_pack() error! the data of field 0 must be exist!");
		return ISO8583_FAILED;
	}

	left_size = *size;
	packed_size = 0;

	if (to_8583data(handle, 0, data, &left_size) != ISO8583_OK)
		return ISO8583_FAILED;

	packed_size += left_size;
	left_size = *size - packed_size;

	if (left_size < 8) {
		snprintf(handle->error, ISO8583_ERROR_SIZE, 
			"iso8583_pack() error! no enough size to pack bitmap! left_size = %u, left_size < 8!", left_size);
		return ISO8583_FAILED;
	}

	bitmap = data + packed_size;

	if (handle->datas[1] != NULL && handle->datas[1]->data[0] == '1') {
		bitmap_n = 16;	
		bitmap_set_bit(bitmap, 1, 1);
	} else {
		bitmap_n = 8;
		bitmap_set_bit(bitmap, 1, 0);
	}

	if (left_size < bitmap_n) {
		snprintf(handle->error, ISO8583_ERROR_SIZE, 
			"iso8583_pack() error! no enough size to pack bitmap! left_size = %u, bitmap_n = %u!", left_size, bitmap_n);
		return ISO8583_FAILED;
	}

	packed_size += bitmap_n;	
	left_size = *size - packed_size;
		
	for (i = 2; i <= bitmap_n * 8; i++)	
		if (handle->datas[i]) {
			if (to_8583data(handle, i, data + packed_size, &left_size) != ISO8583_OK)
				return ISO8583_FAILED;
	
			packed_size += left_size;
			left_size = *size - packed_size;

			bitmap_set_bit(bitmap, i, 1);
		}

	*size = packed_size;	
	
	return ISO8583_OK;
}

int iso8583_unpack(struct iso8583 *handle, unsigned char *data, unsigned int *size)
{
	unsigned int i;
	unsigned int unpacked_size, left_size;
	unsigned char *bitmap;
	unsigned int bitmap_n = 64;			  /* default */
	unsigned int bit;

	if (!handle) {
		snprintf(handle->error, ISO8583_ERROR_SIZE, "iso8583_unpack() argument error! the ptr of handle is null!");
		return ISO8583_FAILED;
	}

	iso8583_clear_datas(handle);

	unpacked_size = 0;
	left_size = *size;

	if (to_userdata(handle, 0, data, &left_size) != ISO8583_OK)
		return ISO8583_FAILED;	

	unpacked_size += left_size;	
	left_size = *size - unpacked_size;

	if (left_size < 8) {
		snprintf(handle->error, ISO8583_ERROR_SIZE, 
			"iso8583_unpack() error! no enough size to pack bitmap! left_size = %u, left_size < 8!", left_size);
		return ISO8583_FAILED;
	}

	bitmap = data + unpacked_size;

	if (bitmap_get_bit(bitmap, 1)) {
		bitmap_n = 16;	
		iso8583_set(handle, 1, "1", 1);
	} else {
		bitmap_n = 8;
		iso8583_set(handle, 1, "0", 1);
	}

	if (left_size < bitmap_n) {
		snprintf(handle->error, ISO8583_ERROR_SIZE, 
			"iso8583_pack() error! no enough size to pack bitmap! left_size = %u, bitmap_n = %u!", left_size, bitmap_n);
		return ISO8583_FAILED;
	}

	unpacked_size += bitmap_n;	
	left_size = *size - unpacked_size;
		
	for (i = 2; i <= bitmap_n * 8; i++)
		if (bitmap_get_bit(bitmap, i)) {
			if (to_userdata(handle, i, data + unpacked_size, &left_size) != ISO8583_OK)
				return ISO8583_FAILED;
	
			unpacked_size += left_size;
			left_size = *size - unpacked_size;
		}

	*size = unpacked_size;	
	
	return ISO8583_OK;	
}

int iso8583_clear_datas(struct iso8583 *handle)
{
	unsigned int i;

	for (i = 0; i < 129; i++) 
		if (handle->datas[i]) {
			free(handle->datas[i]->data);
			free(handle->datas[i]);
			handle->datas[i] = NULL;
		}

	return ISO8583_OK;
}

int iso8583_clear_fields(struct iso8583 *handle)
{
	unsigned int i;

	for (i = 0; i < 129; i++) {
		free(handle->fields[i]);
		handle->fields[i] = NULL;
	}

	return ISO8583_OK;
}

int iso8583_clear(struct iso8583 *handle)
{
	unsigned int i;

	for (i = 0; i < 129; i++) {

		free(handle->fields[i]);
		handle->fields[i] = NULL;

		if (handle->datas[i]) {
			free(handle->datas[i]->data);
			free(handle->datas[i]);
			handle->datas[i] = NULL;
		}
	}

	return ISO8583_OK;
}

int iso8583_destroy(struct iso8583 *handle)
{
	unsigned int i;

	for (i = 0; i < 129; i++) {
		free(handle->fields[i]);
		if (handle->datas[i])
			free(handle->datas[i]->data);
		free(handle->datas[i]);
	}

	free(handle);

	return ISO8583_OK;
}
