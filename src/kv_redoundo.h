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


enum UndoType {
    UNDO_SET,
    UNDO_DEL,
};

int kv_undo_current_id (void);

void kv_undo_add (enum UndoType undo_type, char *key, char *value);

char* kv_undo_undo (char *key, char *value, int tx_id);

#endif

