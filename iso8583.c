#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "iso8583.h"

#define CHK_HANDLE_PTR(handle) do { if (!handle) return ISO8583_ENULL; } while (0)

#define CHK_INDEX(handle, index) do { \
	if (index > 128) { \
		snprintf(handle->error, ISO8583_ERROR_SIZE, "function %s: argument error! the index out of range! index = %u!", __func__, index); \
		return ISO8583_EINDEX; \
	} \
} while (0)

#define CHK_TYPE(handle, type) do { \
	if (type != ISO8583_FIX && type != ISO8583_LLVAR && type != ISO8583_LLLVAR && type != ISO8583_LLVAR_U && type != ISO8583_LLLVAR_U) { \
		snprintf(handle->error, ISO8583_ERROR_SIZE, "function %s: argument error! the type is invalid! type = %u!", __func__, type); \
		return ISO8583_ETYPE; \
	} \
} while (0)

#define CHK_ALIGN(handle, align) do { \
	if (align != ISO8583_L && align != ISO8583_R) { \
		snprintf(handle->error, ISO8583_ERROR_SIZE, "function %s: argument error! the align is invalid! align = %u!", __func__, align); \
		return ISO8583_EALIGN; \
	} \
} while (0)

#define CHK_FIELD0(handle) do { \
	if (handle->datas[0] == NULL) { \
		snprintf(handle->error, ISO8583_ERROR_SIZE, "function %s: field0 error! the data of field 0 must be exist!", __func__); \
		return ISO8583_EFIELD0; \
	} \
} while (0) 

#define CHK_PAD(handle, pad, compress) do {\
	if (compress && (hex_to_bin(pad) == -1)) { \
		snprintf(handle->error, ISO8583_ERROR_SIZE, "function %s: argument error! the pad char is not in 1-9/a-f/A-F! pad = %x!", __func__, pad); \
		return ISO8583_EHEXTOBIN; \
	} \
} while (0)

static struct iso8583_field default_fields[129] = {
	{   4, '0', ISO8583_FIX     , ISO8583_R, ISO8583_U },   // 0.   Message Type             n    4						
	{   1, 'D', ISO8583_FIX     , ISO8583_L, ISO8583_U },   // 1.   BIT MAP EXTENDED         b    1						
	{  19, 'F', ISO8583_LLVAR_U , ISO8583_L, ISO8583_U },   // 2.   PRIMARY ACC. NUM         n    19   llvar				  
	{   6, '0', ISO8583_FIX     , ISO8583_L, ISO8583_U },   // 3.   PROCESSING CODE          n    6						
	{  12, '0', ISO8583_FIX     , ISO8583_R, ISO8583_U },   // 4.   AMOUNT, TRANS.           n    12						
	{  12, '0', ISO8583_FIX     , ISO8583_R, ISO8583_U },   // 5.   AMOUNT, SETTLEMENT       n    12						
	{  12, '0', ISO8583_FIX     , ISO8583_R, ISO8583_U },   // 6.   AMOUNT,CardHolder bill   n    12						
	{  10, '0', ISO8583_FIX     , ISO8583_R, ISO8583_U },   // 7.   TRANSMISSION D & T       n    10   mmddhhmmss			 
	{   8, '0', ISO8583_FIX     , ISO8583_R, ISO8583_U },   // 8.   AMN., CH BILLING FEE     n    8						
	{   8, '0', ISO8583_FIX     , ISO8583_R, ISO8583_U },   // 9.   CONV RATE,SET'T          n    8						
	{   8, '0', ISO8583_FIX     , ISO8583_R, ISO8583_U },   // 10.  CONV RATE, CH billing    n    8						
	{   6, '0', ISO8583_FIX     , ISO8583_L, ISO8583_U },   // 11.  SYSTEM TRACE #           n    6						
	{   6, '0', ISO8583_FIX     , ISO8583_L, ISO8583_U },   // 12.  TIME, LOCAL TRAN         n    6    hhmmss				 
	{   4, '0', ISO8583_FIX     , ISO8583_L, ISO8583_U },   // 13.  DATE, LOCAL TRAN         n    4    mmdd				   
	{   4, '0', ISO8583_FIX     , ISO8583_R, ISO8583_U },   // 14.  DATE, EXPIRATION         n    4    yymm				   
	{   4, '0', ISO8583_FIX     , ISO8583_L, ISO8583_U },   // 15.  DATE, SETTLEMENT         n    4    mmdd				   
	{   4, '0', ISO8583_FIX     , ISO8583_R, ISO8583_U },   // 16.  DATE, CONVERSION         n    4    mmdd				   
	{   4, '0', ISO8583_FIX     , ISO8583_R, ISO8583_U },   // 17.  DATE, CAPTURE            n    4    mmdd				   
	{   4, '0', ISO8583_FIX     , ISO8583_R, ISO8583_U },   // 18.  MERCHANT'S TYPE          n    4						
	{   3, '0', ISO8583_FIX     , ISO8583_R, ISO8583_U },   // 19.  AI COUNTRY CODE          n    3						
	{   3, 'F', ISO8583_FIX     , ISO8583_R, ISO8583_U },   // 20.  PAN EXT.,CO'Y CODE       n    3						
	{   3, '0', ISO8583_FIX     , ISO8583_R, ISO8583_U },   // 21.  FI COUNTRY CODE          n    3						
	{   3, 'D', ISO8583_FIX     , ISO8583_R, ISO8583_U },   // 22.  POS ENTRY MODE           n    3						
	{   3, 'F', ISO8583_FIX     , ISO8583_R, ISO8583_U },   // 23.  CARD SEQUECE NUM.        n    3						
	{   3, 'D', ISO8583_FIX     , ISO8583_R, ISO8583_U },   // 24.  NETWORK INT'L ID         n    3						
	{   2, 'D', ISO8583_FIX     , ISO8583_R, ISO8583_U },   // 25.  POS COND. CODE           n    2						
	{   2, 'D', ISO8583_FIX     , ISO8583_R, ISO8583_U },   // 26.  POS PIN CAP. CODE        n    2						
	{   1, '0', ISO8583_FIX     , ISO8583_R, ISO8583_U },   // 27.  AUTH ID RES. LEN         n    1						
	{   8, '0', ISO8583_FIX     , ISO8583_R, ISO8583_U },   // 28.  AMT. TRANS FEE           n    8						
	{   8, '0', ISO8583_FIX     , ISO8583_R, ISO8583_U },   // 29.  AMT. SETT.  FEE          n    8						
	{   8, '0', ISO8583_FIX     , ISO8583_R, ISO8583_U },   // 30.  AMT. TRAN PROC FEE       n    8						
	{   8, '0', ISO8583_FIX     , ISO8583_R, ISO8583_U },   // 31.  AMT. SET PROC FEE        n    8						
	{  11, '0', ISO8583_LLVAR_U , ISO8583_L, ISO8583_U },   // 32.  ACOUIR. INST. ID         n    11   llvar				  
	{  11, 'F', ISO8583_LLVAR_U , ISO8583_L, ISO8583_U },   // 33.  FI ID                    n    11   llvar				  
	{  11, 'F', ISO8583_LLVAR_U , ISO8583_L, ISO8583_U },   // 34.  PAN EXTENDED             n    8    llvar				  
	{  37, 'F', ISO8583_LLVAR_U , ISO8583_L, ISO8583_U },   // 35.  TRACK 2 DATA             z    37   llvar				  
	{ 104, 'F', ISO8583_LLLVAR_U, ISO8583_L, ISO8583_U },   // 36.  TRACK 3 DATA             z    104  lllvar				 
	{  12, 'D', ISO8583_FIX     , ISO8583_L, ISO8583_U },   // 37.  RETR. REF. NUM           an   12						
	{   6, 'D', ISO8583_FIX     , ISO8583_L, ISO8583_U },   // 38.  AUTH. ID. RESP           an   6						
	{   2, 'D', ISO8583_FIX     , ISO8583_L, ISO8583_U },   // 39.  RESPONSE CODE            an   2						
	{   3, 'D', ISO8583_FIX     , ISO8583_L, ISO8583_U },   // 40.  SERV. REST'N CODE        an   3						
	{   8, 'D', ISO8583_FIX     , ISO8583_L, ISO8583_U },   // 41.  TERMINAL ID              ans  8						
	{  15, 'F', ISO8583_FIX     , ISO8583_L, ISO8583_U },   // 42.  CARD ACC. ID             ans  15						
	{  40, ' ', ISO8583_FIX     , ISO8583_L, ISO8583_U },   // 43.  CARD ACC. NAME           ans  40						
	{  25, '0', ISO8583_LLVAR_U , ISO8583_L, ISO8583_U },   // 44.  ADD. RESP DATA           an   25   llvar				  
	{  76, 'F', ISO8583_LLVAR_U , ISO8583_L, ISO8583_U },   // 45.  TRACK 1 DATA             an   76   llvar				  
	{ 999, 'F', ISO8583_LLLVAR_U, ISO8583_L, ISO8583_U },   // 46.  ADD. DATA - ISO          an   999  lllvar				 
	{ 999, 'F', ISO8583_LLLVAR_U, ISO8583_L, ISO8583_U },   // 47.  ADD. DATA - NATI.        an   999  lllvar					  
	{ 999, 'F', ISO8583_LLLVAR_U, ISO8583_L, ISO8583_U },   // 48.  ADD. DATA - PRI.         an   999  lllvar				 
	{   3, ' ', ISO8583_FIX     , ISO8583_R, ISO8583_U },   // 49.  CC, TRANSACTION          a    3						
	{   3, '0', ISO8583_FIX     , ISO8583_R, ISO8583_U },   // 50.  CC, SETTLEMENT           an   3						
	{   3, '0', ISO8583_FIX     , ISO8583_R, ISO8583_U },   // 51.  CC, CH. BILLING          a    3					   
	{   8, 'D', ISO8583_FIX     , ISO8583_L, ISO8583_U },   // 52.  PIN DATA                 b    8						
	{  16, '0', ISO8583_FIX     , ISO8583_R, ISO8583_U },   // 53.  SECU. CONT. INFO.        n    16						
	{ 120, 'F', ISO8583_LLLVAR_U, ISO8583_L, ISO8583_U },   // 54.  ADDITIONAL AMTS          an   120  lllvar			 
	{ 999, 'F', ISO8583_LLLVAR_U, ISO8583_L, ISO8583_U },   // 55.  REVERVED ISO             ans  999  lllvar				 
	{ 999, 'F', ISO8583_LLLVAR_U, ISO8583_L, ISO8583_U },   // 56.  REVERVED ISO             ans  999  lllvar				 
	{ 999, 'F', ISO8583_LLLVAR_U, ISO8583_L, ISO8583_U },   // 57.  REVERVED NATIONAL        ans  999  lllvar				 
	{ 999, 'F', ISO8583_LLLVAR_U, ISO8583_L, ISO8583_U },   // 58.  REVERVED NATIONAL        ans  999  lllvar				 
	{ 999, 'F', ISO8583_LLLVAR_U, ISO8583_L, ISO8583_U },   // 59.  REVERVED NATIONAL        ans  999  lllvar
	{ 100, 'F', ISO8583_LLLVAR_U, ISO8583_L, ISO8583_U },   // 60.  RESERVED - PRIV1         ans  999  lllvar				 
	{ 200, 'F', ISO8583_LLLVAR_U, ISO8583_L, ISO8583_U },   // 61.  RESERVED - PRIV2         ans  999  lllvar				 
	{ 200, 'F', ISO8583_LLLVAR_U, ISO8583_L, ISO8583_U },   // 62.  RESERVED - PRIV2         ans  999  lllvar				 
	{ 200, 'F', ISO8583_LLLVAR_U, ISO8583_L, ISO8583_U },   // 63.  RESERVED - PRIV3         ans  999  lllvar				 
	{   8, 'D', ISO8583_FIX     , ISO8583_L, ISO8583_U },   // 64.  MSG. AUTH. CODE          b    8						
	{   8, 'D', ISO8583_FIX     , ISO8583_L, ISO8583_U },   // 65.  BIT MAP, EXTENDED        b    8						
	{   1, 'D', ISO8583_FIX     , ISO8583_L, ISO8583_U },   // 66.  SETTLEMENT CODE          n    1						
	{   2, 'D', ISO8583_FIX     , ISO8583_L, ISO8583_U },   // 67.  EXT. PAYMENT CODE        n    2						
	{   3, 'D', ISO8583_FIX     , ISO8583_L, ISO8583_U },   // 68.  RECE. INST. CN.          n    3						
	{   3, 'D', ISO8583_FIX     , ISO8583_L, ISO8583_U },   // 69.  SETTLEMENT ICN.          n    3						
	{   3, 'D', ISO8583_FIX     , ISO8583_L, ISO8583_U },   // 70.  NET MAN IC               n    3						
	{   4, 'D', ISO8583_FIX     , ISO8583_L, ISO8583_U },   // 71.  MESSAGE NUMBER           n    4						
	{   4, 'D', ISO8583_FIX     , ISO8583_L, ISO8583_U },   // 72.  MESSAGE NUM. LAST        n    4						
	{   6, 'D', ISO8583_FIX     , ISO8583_L, ISO8583_U },   // 73.  DATE, ACTION             n    6    yymmdd				 
	{  10, 'D', ISO8583_FIX     , ISO8583_L, ISO8583_U },   // 74.  CREDIT NUMBER            n    10						
	{  10, 'D', ISO8583_FIX     , ISO8583_L, ISO8583_U },   // 75.  CRED REVERSAL NUM        n    10						
	{  10, 'D', ISO8583_FIX     , ISO8583_L, ISO8583_U },   // 76.  DEBITS NUMBER            n    10						
	{  10, 'D', ISO8583_FIX     , ISO8583_L, ISO8583_U },   // 77.  DEBT REVERSAL NUM        n    10						
	{  10, 'D', ISO8583_FIX     , ISO8583_L, ISO8583_U },   // 78.  TRANSFER NUMBER          n    10						
	{  10, 'D', ISO8583_FIX     , ISO8583_L, ISO8583_U },   // 79.  TRANS REVERSAL NUM       n    10						
	{  10, 'D', ISO8583_FIX     , ISO8583_L, ISO8583_U },   // 80.  INQUERIES NUMBER         n    10						
	{  10, 'D', ISO8583_FIX     , ISO8583_L, ISO8583_U },   // 81.  AUTHORIZE NUMBER         n    10						
	{  12, 'D', ISO8583_FIX     , ISO8583_L, ISO8583_U },   // 82.  CRED.PROC.FEE.AMT        n    12						
	{  12, 'D', ISO8583_FIX     , ISO8583_L, ISO8583_U },   // 83.  CRED.TRANS.FEE.AMT       n    12						
	{  12, 'D', ISO8583_FIX     , ISO8583_L, ISO8583_U },   // 84.  DEBT.PROC.FEE.AMT        n    12						
	{  12, 'D', ISO8583_FIX     , ISO8583_L, ISO8583_U },   // 85.  DEBT.TRANS.FEE.AMT       n    12						
	{  15, 'D', ISO8583_FIX     , ISO8583_L, ISO8583_U },   // 86.  CRED AMT                 n    16						
	{  15, 'D', ISO8583_FIX     , ISO8583_L, ISO8583_U },   // 87.  CRED REVERSAL AMT        n    16						
	{  15, 'D', ISO8583_FIX     , ISO8583_L, ISO8583_U },   // 88.  DEBIT AMT                n    16						
	{  15, 'D', ISO8583_FIX     , ISO8583_L, ISO8583_U },   // 89.  DEBIT REVERSAL AMT       n    16						
	{  42, '0', ISO8583_FIX     , ISO8583_L, ISO8583_U },   // 90.  ORIGIN DATA ELEMNT       n    42						
	{   1, 'D', ISO8583_FIX     , ISO8583_L, ISO8583_U },   // 91.  FILE UPDATE CODE         an   1						
	{   2, 'D', ISO8583_FIX     , ISO8583_L, ISO8583_U },   // 92.  FILE SECURITY CODE       n    2						
	{   5, 'D', ISO8583_FIX     , ISO8583_L, ISO8583_U },   // 93.  RESPONSE INDICATOR       n    5						
	{   7, 'D', ISO8583_FIX     , ISO8583_L, ISO8583_U },   // 94.  SERVICE INDICATOR        an   7						
	{  42, 'D', ISO8583_FIX     , ISO8583_L, ISO8583_U },   // 95.  REPLACEMENT AMOUNT       an   42						
	{   8, 'D', ISO8583_FIX     , ISO8583_L, ISO8583_U },   // 96.  MESSAGE SECUR CODE       an   8						
	{  16, 'D', ISO8583_FIX     , ISO8583_L, ISO8583_U },   // 97.  AMT.NET SETTLEMENT       n    16						
	{  25, 'D', ISO8583_FIX     , ISO8583_L, ISO8583_U },   // 98.  PAYEE                    ans  25						
	{  11, 'D', ISO8583_LLVAR_U , ISO8583_L, ISO8583_U },   // 99.  SETTLE.INST.IC           n    11   llvar				 
	{  11, 'D', ISO8583_LLVAR_U , ISO8583_L, ISO8583_U },   // 100. RECE.INST.IC             n    11   llvar				  
	{  17, 'D', ISO8583_FIX     , ISO8583_L, ISO8583_U },   // 101. FILE NAME                ans  17						
	{  28, 'D', ISO8583_LLVAR_U , ISO8583_L, ISO8583_U },   // 102. ACCOUNT ID 1             ans  28   llvar				  
	{  28, 'D', ISO8583_LLVAR_U , ISO8583_L, ISO8583_U },   // 103. ACCOUNT ID 2             ans  28   llvar				  
	{ 100, 'D', ISO8583_LLLVAR_U, ISO8583_L, ISO8583_U },   // 104. TRANS.DESCRIPTION        ans  100  lllvar				 
	{ 999, 'D', ISO8583_LLLVAR_U, ISO8583_L, ISO8583_U },   // 105. RESERVED FOR ISO         ans  999  lllvar  
	{ 999, 'D', ISO8583_LLLVAR_U, ISO8583_L, ISO8583_U },   // 106. RESERVED FOR ISO         ans  999  lllvar				 
	{ 999, 'D', ISO8583_LLLVAR_U, ISO8583_L, ISO8583_U },   // 107. RESERVED FOR ISO         ans  999  lllvar				 
	{ 999, 'D', ISO8583_LLLVAR_U, ISO8583_L, ISO8583_U },   // 108. RESERVED FOR ISO         ans  999  lllvar				 
	{ 999, 'D', ISO8583_LLLVAR_U, ISO8583_L, ISO8583_U },   // 109. RESERVED FOR ISO         ans  999  lllvar				 
	{ 999, 'D', ISO8583_LLLVAR_U, ISO8583_L, ISO8583_U },   // 110. RESERVED FOR ISO         ns   999  lllvar				 
	{ 999, 'D', ISO8583_LLLVAR_U, ISO8583_L, ISO8583_U },   // 111. RESERVED FOR ISO         ans  999  lllvar				 
	{ 999, 'D', ISO8583_LLLVAR_U, ISO8583_L, ISO8583_U },   // 112. RESERVED FOR NATIO       ans  999  lllvar				 
	{ 999, 'D', ISO8583_LLLVAR_U, ISO8583_L, ISO8583_U },   // 113. RESERVED FOR NATIO       ans  999  lllvar				 
	{ 999, 'D', ISO8583_LLLVAR_U, ISO8583_L, ISO8583_U },   // 114. RESERVED FOR NATIO       ans  999  lllvar				 
	{ 999, 'D', ISO8583_LLLVAR_U, ISO8583_L, ISO8583_U },   // 115. RESERVED FOR NATIO       ans  999  lllvar				 
	{ 999, 'D', ISO8583_LLLVAR_U, ISO8583_L, ISO8583_U },   // 116. RESERVED FOR NATIO       ans  999  lllvar				 
	{ 999, 'D', ISO8583_LLLVAR_U, ISO8583_L, ISO8583_U },   // 117. RESERVED FOR NATIO       ans  999  lllvar				 
	{ 999, 'D', ISO8583_LLLVAR_U, ISO8583_L, ISO8583_U },   // 118. RESERVED FOR NATIO       ans  999  lllvar				 
	{ 999, 'D', ISO8583_LLLVAR_U, ISO8583_L, ISO8583_U },   // 119. RESERVED FOR NATIO       ans  999  lllvar				 
	{ 999, 'D', ISO8583_LLLVAR_U, ISO8583_L, ISO8583_U },   // 120. RESERVED FOR PRIVA       ans  999  lllvar				 
	{ 999, 'D', ISO8583_LLLVAR_U, ISO8583_L, ISO8583_U },   // 121. RESERVED FOR PRIVA       ans  999  lllvar				 
	{ 999, 'D', ISO8583_LLLVAR_U, ISO8583_L, ISO8583_U },   // 122. RESERVED FOR PRIVA       ans  999  lllvar				 
	{ 999, 'D', ISO8583_LLLVAR_U, ISO8583_L, ISO8583_U },   // 123. RESERVED FOR PRIVA       ans  999  lllvar				 
	{ 999, 'D', ISO8583_LLLVAR_U, ISO8583_L, ISO8583_U },   // 124. RESERVED FOR PRIVA       ans  999  lllvar				 
	{ 999, 'D', ISO8583_LLLVAR_U, ISO8583_L, ISO8583_U },   // 125. RESERVED FOR PRIVA       ans  999  lllvar				 
	{ 999, 'D', ISO8583_LLLVAR_U, ISO8583_L, ISO8583_U },   // 126. RESERVED FOR PRIVA       ans  999  lllvar				 
	{ 999, 'D', ISO8583_LLLVAR_U, ISO8583_L, ISO8583_U },   // 127. RESERVED FOR PRIVA       ans  999  lllvar				 
	{   8, 'D', ISO8583_FIX     , ISO8583_L, ISO8583_U }    // 128. RESERVED FOR PRIVA       ans  999  lllvar				 
};

static inline unsigned bcd2bin(unsigned char val)
{
	return (val & 0x0f) + (val >> 4) * 10;
}

static inline unsigned char bin2bcd(unsigned val)
{
	return ((val / 10) << 4) + val % 10;
}

static inline unsigned asc2bin(unsigned char *val, int len)
{
	unsigned char *p = val;
	unsigned int res = 0;
	for (int i=0; i<len; i++){
	    res = res * 10 + *p - '0';
    	    p++;
	}
	return res;
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

static inline int correct_size(unsigned int *size, unsigned int type, unsigned int fix_size)
{
	switch (type) {
	case ISO8583_FIX:
		*size = (*size > fix_size) ? fix_size : *size;
		break;
	case ISO8583_LLVAR:
	case ISO8583_LLVAR_U:
		*size = (*size > 99) ? 99 : *size;
		break;
	case ISO8583_LLLVAR:
	case ISO8583_LLLVAR_U:
		*size = (*size > 9999) ? 9999 : *size;
		break;
	default:
		return ISO8583_ETYPE;
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
		*iso8583_size = (compress ? (user_size + 1) / 2 : user_size) + 1;
		break;
	case ISO8583_LLLVAR:
		*iso8583_size = (compress ? (user_size + 1) / 2 : user_size) + 2; 
		break;
	case ISO8583_LLVAR_U:
		*iso8583_size = (compress ? (user_size + 1) / 2 : user_size) + 2;
		break;
	case ISO8583_LLLVAR_U:
		*iso8583_size = (compress ? (user_size + 1) / 2 : user_size) + 3;
		break;
	default:
		return ISO8583_ETYPE;
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
				return ISO8583_EHEXTOBIN;

			dst[i / 2] |= (i % 2) ? bit4 : bit4 << 4;
		}

		if ((bit4 = hex_to_bin(pad)) == -1)
			return ISO8583_EHEXTOBIN;

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
			return ISO8583_EHEXTOBIN;

		if (pad_len > 1) {
			bit8 = bit4 << 4 | bit4;
			for (i = 0; i < pad_len / 2; i++)
				dst[i] = bit8;
		}

		if (pad_len % 2)
			dst[pad_len / 2] |= bit4 << 4;
			
		for (i = pad_len; i < dst_size * 2; i++, src++) {
			if ((bit4 = hex_to_bin(*src)) == -1)
				return ISO8583_EHEXTOBIN;

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
					unsigned int align, char pad, unsigned int compress, 
					unsigned int prefixlen, unsigned int prefixcompress)
{
	int i;
	unsigned int size = user_size;

        if ( prefixcompress ) {
	    for (i = 0; i < prefixlen; i++) {
		iso8583_data[prefixlen - 1 - i] = bin2bcd(size % 100);
		size /= 100;   
	    }
	} else {
	    for (i = 0; i < prefixlen; i++) {
		iso8583_data[prefixlen - 1 - i] = size % 10 + '0';
		size /= 10;   
	    }
        }

	return to_fix_8583data(iso8583_data + prefixlen, iso8583_size - prefixlen, user_data, user_size, align, pad, compress);
}

static inline int to_8583data(struct iso8583 *handle, unsigned int index, unsigned char *data, unsigned int *size)
{
	unsigned int iso8583_size = 0;
	struct iso8583_field *field;
	struct iso8583_data *userdata;
	int ret;

	userdata = handle->datas[index];

	if (userdata == NULL) {
		*size = 0;	
		return ISO8583_OK;
	}

	field = handle->fields[index] ? handle->fields[index] : &default_fields[index];

	if (field->type > ISO8583_LLLVAR_U) {
		snprintf(handle->error, ISO8583_ERROR_SIZE, "wrong type! index = %d, type = %u!", index, field->type);
		return ISO8583_ETYPE;
	}

	get_8583size_from_userdata(&iso8583_size, userdata->size, field->type, field->size, field->compress);

	if (data == NULL) {
		*size = iso8583_size;
		return ISO8583_OK;
	}

	if (iso8583_size > *size) {
		snprintf(handle->error, ISO8583_ERROR_SIZE, 
			"to_8583data() error! not enough size! index = %d, size = %u, iso8583_size = %u!", index, *size, iso8583_size);
		return ISO8583_ESIZE;
	}


	switch (field->type) {
	case ISO8583_FIX:
		ret = to_fix_8583data(data, iso8583_size, userdata->data, userdata->size, field->align, field->pad, field->compress);
		break;
	case ISO8583_LLVAR:
		ret = to_var_8583data(data, iso8583_size, userdata->data, userdata->size, field->align, field->pad, field->compress, 1, ISO8583_Z);
		break;
	case ISO8583_LLLVAR:
		ret = to_var_8583data(data, iso8583_size, userdata->data, userdata->size, field->align, field->pad, field->compress, 2, ISO8583_Z);
		break;
	case ISO8583_LLVAR_U:
		ret = to_var_8583data(data, iso8583_size, userdata->data, userdata->size, field->align, field->pad, field->compress, 2, ISO8583_U);
		break;
	case ISO8583_LLLVAR_U:
		ret = to_var_8583data(data, iso8583_size, userdata->data, userdata->size, field->align, field->pad, field->compress, 3, ISO8583_U);
		break;
	}

	if (ret != ISO8583_OK) {
		snprintf(handle->error, ISO8583_ERROR_SIZE, 
			"to_8583data() error! convert to 8583data failed! index = %d, size = %u, iso8583_size = %u!", index, *size, iso8583_size);
		return ret;
	}

	*size = iso8583_size;

	return ISO8583_OK;
}


static inline int get_size_from_8583data(unsigned int *iso8583_size, unsigned int *user_size,
						unsigned char *iso8583_data, unsigned int data_size,
						unsigned int compress, unsigned int type, unsigned int fix_length)

{
	unsigned int l;

	switch (type) {
	case ISO8583_FIX:
		*iso8583_size = compress ? (fix_length + 1) / 2 : fix_length;
		*user_size = fix_length;
		break;
	case ISO8583_LLVAR:
		if (data_size < 1)
			return ISO8583_ESIZE;

		l = bcd2bin(iso8583_data[0]);	
		*iso8583_size = (compress ? (l + 1) / 2 : l) + 1;
		*user_size = l;
		break;
	case ISO8583_LLLVAR:
		if (data_size < 2)	
			return ISO8583_ESIZE;

 	        l = bcd2bin(iso8583_data[0]) * 100 + bcd2bin(iso8583_data[1]);
		*iso8583_size = (compress ? (l + 1) / 2 : l) + 2;
		*user_size = l;	
		break;
	case ISO8583_LLVAR_U:
		if (data_size < 2)
			return ISO8583_ESIZE;

		l = asc2bin(iso8583_data, 2);
		*iso8583_size = (compress ? (l + 1) / 2 : l) + 2;
		*user_size = l;
		break;
	case ISO8583_LLLVAR_U:
		if (data_size < 3)	
			return ISO8583_ESIZE;

		l = asc2bin(iso8583_data, 3);
		*iso8583_size = compress ? (l + 1) / 2 + 3 : l + 3;
		*user_size = l;	
                break;
	default:
		return ISO8583_ETYPE;
	}
	if (*iso8583_size > data_size) 
		return ISO8583_ESIZE;	

	return ISO8583_OK;
}

static inline int to_hex_from_right(unsigned char *data, unsigned char *bin, unsigned int user_size)
{
	int i;
	int size = user_size;

	unsigned char *b = bin - 1;
	unsigned char *d = data - 1;

	for (i = 0; i > -size; i--) {

		unsigned int byte = b[i / 2];

		byte = (i % 2) ? byte >> 4 : byte & 0x0f;

		d[i] = bin_to_hex(byte);
	}

	return ISO8583_OK;
}

static inline int to_hex_from_left(unsigned char *data, unsigned char *bin, unsigned int user_size)
{
	int i;

	for (i = 0; i < user_size; i++) {
		unsigned int byte = bin[i / 2 ];

		byte = (i % 2) ? byte & 0x0f : byte >> 4;

		data[i] = bin_to_hex(byte);
	}

	return ISO8583_OK;
}

static inline int to_userdata_fix(unsigned char *user_data, unsigned char *iso8583_data, 
					unsigned int iso8583_size, unsigned int user_size, 
					unsigned int compress, unsigned int align)
{
	if (compress) {
		if (align == ISO8583_L)
			to_hex_from_left(user_data, iso8583_data, user_size);
		else
			to_hex_from_right(user_data + user_size, iso8583_data + iso8583_size, user_size);	
	} else {
		memcpy(user_data, iso8583_data, user_size);
	}

	return ISO8583_OK;
}

static inline int to_userdata(struct iso8583 *handle, unsigned int index, unsigned char *data, unsigned int *size)
{
	struct iso8583_field *field = handle->fields[index] ? handle->fields[index] : &default_fields[index];		
	struct iso8583_data *user_data;
	unsigned int iso8583_size, user_size;

	int ret = get_size_from_8583data(&iso8583_size, &user_size, data, *size, field->compress, field->type, field->size);

	if (ret != ISO8583_OK) {
		snprintf(handle->error, ISO8583_ERROR_SIZE, 
			"to_userdata() error! get_size_from_8583 data error! index = %u, compress = %u, type = %u, field->size = %u, size = %u", 
			index, field->compress, field->type, field->size, *size);
		return ret;
	}

	user_data = handle->datas[index];	

	if (user_data) {
		free(user_data->data);
	} else {
		user_data = (struct iso8583_data *)malloc(sizeof(struct iso8583_data));

		if (user_data == NULL) {
			snprintf(handle->error, ISO8583_ERROR_SIZE, "to_userdata() alloc iso8583_data memory failed!");
			return ISO8583_EMALLOC;
		}
	}

	user_data->data = (unsigned char *)malloc(user_size);

	if (user_data->data == NULL) {
		snprintf(handle->error, ISO8583_ERROR_SIZE, "to_userdata() alloc user_data memory failed!");
		free(user_data);
		handle->datas[index] = NULL;
		return ISO8583_EMALLOC;
	}


	switch (field->type) {
	case ISO8583_FIX:
		to_userdata_fix(user_data->data, data, iso8583_size, user_size, field->compress, field->align);
		break;
	case ISO8583_LLVAR:
		to_userdata_fix(user_data->data, data + 1, iso8583_size - 1, user_size, field->compress, field->align);
		break;
	case ISO8583_LLLVAR:
		to_userdata_fix(user_data->data, data + 2, iso8583_size - 2, user_size, field->compress, field->align);
		break;
	case ISO8583_LLVAR_U:
		to_userdata_fix(user_data->data, data + 2, iso8583_size - 2, user_size, field->compress, field->align);
		break;
	case ISO8583_LLLVAR_U:
		to_userdata_fix(user_data->data, data + 3, iso8583_size - 3, user_size, field->compress, field->align);
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

	CHK_HANDLE_PTR(handle);
	CHK_INDEX(handle, index);
	CHK_TYPE(handle, type);
	CHK_ALIGN(handle, align);
	CHK_PAD(handle, pad, compress);

	field = handle->fields[index];

	if (!field) {
		field = (struct iso8583_field *)malloc(sizeof(struct iso8583_field));
		if (!field) {
			snprintf(handle->error, ISO8583_ERROR_SIZE, "iso8583_define() error! alloc field memory failed!");
			return ISO8583_EMALLOC;
		}
		handle->fields[index] = field;		
	}

	field->size     = size;
	field->pad      = pad;
	field->type     = type;
	field->align    = align;
	field->compress = compress;

	return ISO8583_OK;
}

int iso8583_set(struct iso8583 *handle, unsigned int index, const unsigned char *data, unsigned int size)
{
	struct iso8583_field *field;
	struct iso8583_data *userdata;

	CHK_HANDLE_PTR(handle);
	CHK_INDEX(handle, index);

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
		snprintf(handle->error, ISO8583_ERROR_SIZE, "iso8583_set() correct size error! field type error! type = %u!", field->type);
		return ISO8583_ESIZE;
	}

	if (userdata) {
		free(userdata->data);
	} else {
		userdata = (struct iso8583_data *)malloc(sizeof(struct iso8583_data));

		if (userdata == NULL) {
			snprintf(handle->error, ISO8583_ERROR_SIZE, "iso8583_set() alloc memory failed!");
			return ISO8583_EMALLOC;
		}
	}
		
	userdata->data = (unsigned char *)malloc(size);

	if (userdata->data == NULL) {
		snprintf(handle->error, ISO8583_ERROR_SIZE, "iso8583_set() alloc memory failed!");
		free(userdata);
		handle->datas[index] = NULL;
		return ISO8583_EMALLOC;
	}

	memcpy(userdata->data, data, size);

	userdata->size = size;	
	handle->datas[index] = userdata;

	return ISO8583_OK;
}

int iso8583_get(struct iso8583 *handle, unsigned int index, const unsigned char **data, unsigned int *size)
{
	struct iso8583_data *userdata;

	CHK_HANDLE_PTR(handle);
	CHK_INDEX(handle, index);

	userdata = handle->datas[index];

	*data = userdata ? userdata->data : NULL;
	*size = userdata ? userdata->size : 0;

	return ISO8583_OK;
}

int iso8583_pack(struct iso8583 *handle, unsigned char *data, unsigned int *size)
{
	unsigned int left_size, packed_size;
	unsigned int bitmap_n;
	unsigned char *bitmap;
	unsigned int i;
	int ret;

	CHK_HANDLE_PTR(handle);
	CHK_FIELD0(handle);

	left_size = *size;
	packed_size = 0;

	ret = to_8583data(handle, 0, data, &left_size);

	if (ISO8583_OK != ret)
		return ret;

	packed_size += left_size;
	left_size = *size - packed_size;

	if (left_size < 8) {
		snprintf(handle->error, ISO8583_ERROR_SIZE, 
			"iso8583_pack() error! no enough size to pack bitmap! left_size = %u, left_size < 8!", left_size);
		return ISO8583_ESIZE;
	}

	bitmap = data + packed_size;

	if (handle->datas[1] != NULL && handle->datas[1]->data[0] == '1') {
		bitmap_n = 16;	
		memset(bitmap, 0, bitmap_n);
		bitmap_set_bit(bitmap, 1, 1);
	} else {
		bitmap_n = 8;
		memset(bitmap, 0, bitmap_n);
		bitmap_set_bit(bitmap, 1, 0);
	}

	if (left_size < bitmap_n) {
		snprintf(handle->error, ISO8583_ERROR_SIZE, 
			"iso8583_pack() error! no enough size to pack bitmap! left_size = %u, bitmap_n = %u!", left_size, bitmap_n);
		return ISO8583_ESIZE;
	}

	packed_size += bitmap_n;	
	left_size = *size - packed_size;
		
	for (i = 2; i <= bitmap_n * 8; i++)	
		if (handle->datas[i]) {
            ret = to_8583data(handle, i, data + packed_size, &left_size);
			if (ISO8583_OK != ret)
				return ret;
	
			packed_size += left_size;
			left_size = *size - packed_size;

			bitmap_set_bit(bitmap, i, 1);
		}

	*size = packed_size;
	
	return ISO8583_OK;
}

int iso8583_size(struct iso8583 *handle, unsigned int *size)
{
	unsigned int data_size;
	unsigned int bitmap_n;
	unsigned int i;
	int ret;

	CHK_HANDLE_PTR(handle);
	CHK_FIELD0(handle);

	*size = 0;

	ret = to_8583data(handle, 0, NULL, &data_size);

	if (ISO8583_OK != ret)
        	return ret;

	*size += data_size;

	if (handle->datas[1] != NULL && handle->datas[1]->data[0] == '1') {
		bitmap_n = 16;
	} else {
		bitmap_n = 8;
	}

	*size += bitmap_n;

	for (i = 2; i <= bitmap_n * 8; i++)	
		if (handle->datas[i]) {
			ret = to_8583data(handle, i, NULL, &data_size);
			if (ISO8583_OK != ret)
				return ret;
			*size += data_size;
		}

	return ISO8583_OK;
}

int iso8583_unpack(struct iso8583 *handle, unsigned char *data, unsigned int *size, unsigned int maxfield)
{
	unsigned int i;
	unsigned int unpacked_size, left_size;
	unsigned char *bitmap;
	unsigned int bitmap_n;

	CHK_HANDLE_PTR(handle);
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
		iso8583_set(handle, 1, (unsigned char *)"1", 1);
	} else {
		bitmap_n = 8;
		iso8583_set(handle, 1, (unsigned char *)"0", 1);
	}

	if (left_size < bitmap_n) {
		snprintf(handle->error, ISO8583_ERROR_SIZE, 
			"iso8583_pack() error! no enough size to pack bitmap! left_size = %u, bitmap_n = %u!", left_size, bitmap_n);
		return ISO8583_FAILED;
	}

	unpacked_size += bitmap_n;	
	left_size = *size - unpacked_size;
		
	for (i = 2; i <= bitmap_n * 8 && i<= maxfield ; i++)
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

