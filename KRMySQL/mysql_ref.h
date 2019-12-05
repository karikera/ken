#pragma once

#define __MYSQL_REFERENCE_H

#ifndef _mysql_h

typedef struct st_net NET;
typedef struct st_mysql_stmt MYSQL_STMT;
typedef char		my_bool;
typedef struct st_mysql MYSQL;
typedef struct st_mysql_field MYSQL_FIELD;
typedef struct st_mysql_res MYSQL_RES;
typedef char** MYSQL_ROW;

enum enum_field_types { MYSQL_TYPE_DECIMAL, MYSQL_TYPE_TINY,
                        MYSQL_TYPE_SHORT,  MYSQL_TYPE_LONG,
                        MYSQL_TYPE_FLOAT,  MYSQL_TYPE_DOUBLE,
                        MYSQL_TYPE_NULL,   MYSQL_TYPE_TIMESTAMP,
                        MYSQL_TYPE_LONGLONG,MYSQL_TYPE_INT24,
                        MYSQL_TYPE_DATE,   MYSQL_TYPE_TIME,
                        MYSQL_TYPE_DATETIME, MYSQL_TYPE_YEAR,
                        MYSQL_TYPE_NEWDATE, MYSQL_TYPE_VARCHAR,
                        MYSQL_TYPE_BIT,
                        MYSQL_TYPE_NEWDECIMAL=246,
                        MYSQL_TYPE_ENUM=247,
                        MYSQL_TYPE_SET=248,
                        MYSQL_TYPE_TINY_BLOB=249,
                        MYSQL_TYPE_MEDIUM_BLOB=250,
                        MYSQL_TYPE_LONG_BLOB=251,
                        MYSQL_TYPE_BLOB=252,
                        MYSQL_TYPE_VAR_STRING=253,
                        MYSQL_TYPE_STRING=254,
                        MYSQL_TYPE_GEOMETRY=255,
                        MAX_NO_FIELD_TYPES /* Should always be last */
};

typedef struct st_mysql_bind
{
  unsigned long	*length;          /* output length pointer */
  my_bool       *is_null;	  /* Pointer to null indicator */
  void		*buffer;	  /* buffer to get/put data */
  /* set this if you want to track data truncations happened during fetch */
  my_bool       *error;
  unsigned char *row_ptr;         /* for the current data position */
  void (*store_param_func)(NET *net, struct st_mysql_bind *param);
  void (*fetch_result)(struct st_mysql_bind *, MYSQL_FIELD *,
                       unsigned char **row);
  void (*skip_result)(struct st_mysql_bind *, MYSQL_FIELD *,
		      unsigned char **row);
  /* output buffer length, must be set when fetching str/binary */
  unsigned long buffer_length;
  unsigned long offset;           /* offset position for char/binary fetch */
  unsigned long	length_value;     /* Used if length is 0 */
  unsigned int	param_number;	  /* For null count and error messages */
  unsigned int  pack_length;	  /* Internal length for packed data */
  enum enum_field_types buffer_type;	/* buffer type */
  my_bool       error_value;      /* used if error is 0 */
  my_bool       is_unsigned;      /* set if integer type is unsigned */
  my_bool	long_data_used;	  /* If used with mysql_send_long_data */
  my_bool	is_null_value;    /* Used if is_null is 0 */
  void *extension;
} MYSQL_BIND;

#endif