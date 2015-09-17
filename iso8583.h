#ifndef _ISO8583_H_
#define _ISO8583_H_

#define ISO8583_OK         0
#define ISO8583_EINDEX     1
#define ISO8583_EHEXTOBIN  2
#define ISO8583_EBINTOHEX  3
#define ISO8583_EALIGN     4
#define ISO8583_ETYPE      5
#define ISO8583_EMALLOC    6
#define ISO8583_ESIZE      7

#define ISO8583_FAILED -1

#define ISO8583_L 0
#define ISO8583_R 1

#define ISO8583_FIX    0
#define ISO8583_LLVAR  1
#define ISO8583_LLLVAR 2

#define ISO8583_U 0
#define ISO8583_Z 1

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
};

struct iso8583 *iso8583_create();

int iso8583_define(struct iso8583 *handle, unsigned int index, unsigned int size, char pad, unsigned int type, unsigned int align, unsigned int compress);
int iso8583_set(struct iso8583 *handle, unsigned int index, const unsigned char *data, unsigned int size);
int iso8583_get(struct iso8583 *handle, unsigned int index, const unsigned char **data, unsigned int *size);
int iso8583_pack(struct iso8583 *handle, unsigned char *data, unsigned int *size);
int iso8583_unpack(struct iso8583 *handle, unsigned char *data, unsigned int *size);
int iso8583_clear_datas(struct iso8583 *handle);
int iso8583_clear_fields(struct iso8583 *handle);
int iso8583_clear(struct iso8583 *handle);
int iso8583_destroy(struct iso8583 *handle);

#endif
