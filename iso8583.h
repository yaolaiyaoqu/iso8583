#ifndef _ISO8583_H_
#define _ISO8583_H_

// error code

enum {
	ISO8583_OK,
	ISO8583_EINDEX,
	ISO8583_EHEXTOBIN,
	ISO8583_EBINTOHEX,
	ISO8583_EALIGN,
	ISO8583_ETYPE,
	ISO8583_EMALLOC,
	ISO8583_ESIZE,
	ISO8583_ENULL,
	ISO8583_EFIELD0
};

#define ISO8583_FAILED -1

// algin 
enum {
	ISO8583_L,
	ISO8583_R
};

// type
enum {
	ISO8583_FIX,
	ISO8583_LLVAR,
	ISO8583_LLLVAR
};

// compress
enum {
	ISO8583_U,
	ISO8583_Z
};

// varlentype
enum {
        VARLEN_BCD,
        VARLEN_ASC
};

#define ISO8583_ERROR_SIZE 128

struct iso8583_field {
	unsigned int size;
	char pad;
	unsigned type:3;    
	unsigned align:1;
	unsigned compress:1;
};

struct iso8583_data {
	unsigned int size;
	unsigned char *data;
};

struct iso8583 {
	struct iso8583_field *fields[129];
	struct iso8583_data *datas[129];
	char error[ISO8583_ERROR_SIZE];
	unsigned varlentype:1;
};

struct iso8583 *iso8583_create();

int iso8583_define(struct iso8583 *handle, unsigned int index, unsigned int size, char pad, unsigned int type, unsigned int align, unsigned int compress);
int iso8583_set(struct iso8583 *handle, unsigned int index, const unsigned char *data, unsigned int size);
int iso8583_get(struct iso8583 *handle, unsigned int index, const unsigned char **data, unsigned int *size);
int iso8583_pack(struct iso8583 *handle, unsigned char *data, unsigned int *size);
int iso8583_unpack(struct iso8583 *handle, unsigned char *data, unsigned int *size, unsigned int maxfield);
int iso8583_clear_datas(struct iso8583 *handle);
int iso8583_clear_fields(struct iso8583 *handle);
int iso8583_clear(struct iso8583 *handle);
int iso8583_destroy(struct iso8583 *handle);

#endif
