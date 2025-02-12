#ifndef __KV_REDOUNDO_H__
#define __KV_REDOUNDO_H__

#include "kv_hash.h"

enum RedoType {
    REDO_SET,
    REDO_DEL,
};

void kv_redo_init (void);

void kv_redo_add (enum RedoType redo_type, char *key, char *value);

void kv_redo_redo (struct kv_ht *ht);

void kv_redo_terminate (void);

#endif

