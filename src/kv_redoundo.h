#ifndef __KV_REDOUNDO_H__
#define __KV_REDOUNDO_H__

#include "kv_hash.h"

#include <stdint.h>

enum kv_ru_type {
    KV_RU_BEGIN,
    KV_RU_ABORT,
    KV_RU_COMMIT,
    KV_RU_WRITE,
    KV_RU_DELETE,
};

void kv_ru_init ();
void kv_ru_destroy ();
void kv_ru_add (int tx_id, enum kv_ru_type ru_type, char *key, char *value, char *old_value);
void kv_ru_redo (struct kv_ht *ht);
void kv_ru_undo (int tx_id);


#endif
