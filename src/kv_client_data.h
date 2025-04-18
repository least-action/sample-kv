#ifndef __KV_CLIENT_DATA_H__
#define __KV_CLIENT_DATA_H__

#include "transaction.h"

struct kv_client_data {
    struct kv_tx *tx;
};

#endif
