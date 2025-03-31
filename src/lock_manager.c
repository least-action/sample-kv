#include "lock_manager.h"

struct kv_lm
{
    char _reserved;
};

struct kv_lm* kv_lm_create (void)
{
    struct kv_lm *lm = (struct kv_lm *) malloc (sizeof (struct kv_lm));
    return lm;
}

int kv_lm_destruct (struct kv_lm *lm)
{
    free (lm);
    return 0;
}

struct kv_rwl* kv_lm_get_rwlock (struct kv_lm *lm, char *key, size_t key_len)
{
    // todo
    return NULL;
}
