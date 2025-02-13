/* Automatically generated nanopb header */
/* Generated by nanopb-1.0.0-dev */

#ifndef PB_MAIN_PB_H_INCLUDED
#define PB_MAIN_PB_H_INCLUDED
#include <pb.h>
#include "sys/types.h"
#include "dirent.h"

#if PB_PROTO_HEADER_VERSION != 40
#error Regenerate this file with the current version of nanopb generator.
#endif

/* Struct definitions */
typedef struct _get_device_infoRequest {
    char dummy_field;
} get_device_infoRequest;

typedef struct _get_device_infoResponse {
    char type[40];
    char product[40];
    char ip[40];
} get_device_infoResponse;

typedef struct _claimRequest {
    char dummy_field;
} claimRequest;

typedef struct _claimResponse {
    char token[40];
} claimResponse;

typedef struct _reclaimRequest {
    char token[40];
} reclaimRequest;

typedef struct _unclaimRequest {
    char token[40];
} unclaimRequest;

typedef struct _set_smartledRequest {
    char token[40];
    int32_t id;
    uint32_t color;
} set_smartledRequest;


#ifdef __cplusplus
extern "C" {
#endif

/* Initializer values for message structs */
#define get_device_infoRequest_init_default      {0}
#define get_device_infoResponse_init_default     {"", "", ""}
#define claimRequest_init_default                {0}
#define claimResponse_init_default               {""}
#define reclaimRequest_init_default              {""}
#define unclaimRequest_init_default              {""}
#define set_smartledRequest_init_default         {"", 0, 0}
#define get_device_infoRequest_init_zero         {0}
#define get_device_infoResponse_init_zero        {"", "", ""}
#define claimRequest_init_zero                   {0}
#define claimResponse_init_zero                  {""}
#define reclaimRequest_init_zero                 {""}
#define unclaimRequest_init_zero                 {""}
#define set_smartledRequest_init_zero            {"", 0, 0}

/* Field tags (for use in manual encoding/decoding) */
#define get_device_infoResponse_type_tag         1
#define get_device_infoResponse_product_tag      2
#define get_device_infoResponse_ip_tag           3
#define claimResponse_token_tag                  1
#define reclaimRequest_token_tag                 1
#define unclaimRequest_token_tag                 1
#define set_smartledRequest_token_tag            1
#define set_smartledRequest_id_tag               2
#define set_smartledRequest_color_tag            3

/* Struct field encoding specification for nanopb */
#define get_device_infoRequest_FIELDLIST(X, a) \

#define get_device_infoRequest_CALLBACK NULL
#define get_device_infoRequest_DEFAULT NULL

#define get_device_infoResponse_FIELDLIST(X, a) \
X(a, STATIC,   REQUIRED, STRING,   type,              1) \
X(a, STATIC,   REQUIRED, STRING,   product,           2) \
X(a, STATIC,   REQUIRED, STRING,   ip,                3)
#define get_device_infoResponse_CALLBACK NULL
#define get_device_infoResponse_DEFAULT NULL

#define claimRequest_FIELDLIST(X, a) \

#define claimRequest_CALLBACK NULL
#define claimRequest_DEFAULT NULL

#define claimResponse_FIELDLIST(X, a) \
X(a, STATIC,   REQUIRED, STRING,   token,             1)
#define claimResponse_CALLBACK NULL
#define claimResponse_DEFAULT NULL

#define reclaimRequest_FIELDLIST(X, a) \
X(a, STATIC,   REQUIRED, STRING,   token,             1)
#define reclaimRequest_CALLBACK NULL
#define reclaimRequest_DEFAULT NULL

#define unclaimRequest_FIELDLIST(X, a) \
X(a, STATIC,   REQUIRED, STRING,   token,             1)
#define unclaimRequest_CALLBACK NULL
#define unclaimRequest_DEFAULT NULL

#define set_smartledRequest_FIELDLIST(X, a) \
X(a, STATIC,   REQUIRED, STRING,   token,             1) \
X(a, STATIC,   REQUIRED, INT32,    id,                2) \
X(a, STATIC,   REQUIRED, UINT32,   color,             3)
#define set_smartledRequest_CALLBACK NULL
#define set_smartledRequest_DEFAULT NULL

extern const pb_msgdesc_t get_device_infoRequest_msg;
extern const pb_msgdesc_t get_device_infoResponse_msg;
extern const pb_msgdesc_t claimRequest_msg;
extern const pb_msgdesc_t claimResponse_msg;
extern const pb_msgdesc_t reclaimRequest_msg;
extern const pb_msgdesc_t unclaimRequest_msg;
extern const pb_msgdesc_t set_smartledRequest_msg;

/* Defines for backwards compatibility with code written before nanopb-0.4.0 */
#define get_device_infoRequest_fields &get_device_infoRequest_msg
#define get_device_infoResponse_fields &get_device_infoResponse_msg
#define claimRequest_fields &claimRequest_msg
#define claimResponse_fields &claimResponse_msg
#define reclaimRequest_fields &reclaimRequest_msg
#define unclaimRequest_fields &unclaimRequest_msg
#define set_smartledRequest_fields &set_smartledRequest_msg

/* Maximum encoded size of messages (where known) */
#define MAIN_PB_H_MAX_SIZE                       get_device_infoResponse_size
#define claimRequest_size                        0
#define claimResponse_size                       41
#define get_device_infoRequest_size              0
#define get_device_infoResponse_size             123
#define reclaimRequest_size                      41
#define set_smartledRequest_size                 58
#define unclaimRequest_size                      41

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
