#ifndef __KV_RECOVERY_H__
#define __KV_RECOVERY_H__

/*
  * check 랑 같이 하는게 좋은가? 아닌가?
  * 1. fork() 하기만 하면 됨. 오래 안걸림(?)
  * 2. 아래와 같은 복잡한 프로세스 대신
  *     1) dump 에서 LSN 확인(혹은 filename with pos)
  *     2) 해당 LSN 부터 redo 및 undo 하기
  * 3. 단점은
  *     1) 딱히 없나?
  *     2) LSN lock 을 건 상태에서 fork() 가 완료 되고 lock 을 해제해야 해당 LSN 기준으로 dump 생성됨을 보장받을 수 있음.
  *     3) 진행중인 tx 도 lock 을 건 상태에서 로그에 찍어야 함.
  *     4) 굳이 시간이 걸린다면 tx 리스트 찍는게 더 오래걸려서 그냥 같이 해도 별 문제 없을거 같음.
  *     5) 오히려 프로세스 간단해져서 편할듯
  * 
  * check 랑 dump 랑 분리되어있을때 프로세스
  * 1. dump 를 불러오기
  * 2. dump 기준으로 먼저 발생한 것중 가장 마지막 check 찾기
  * 3. check 에서 진행중인 tx 보고 dump 까지 가면서 완료된 tx 제거
  * 4. dump 부터 redo 하기.
  * 5. redu 끝나면 미완료된 tx 여부 확인하고 있으면 redu process 밟기(abort > clr > end)
  */


/*
 * file format
 * LSN      PrevLSN  TxID      Type    l1 l2 l3 Key BeforeImage AfterImage
 * 00000001 00000000 T00000001 BEGIN__
 * 00000002 00000001 T00000001 UPDATE_ 03 03 00 key old
 * 00000003 00000000 T00000002 BEGIN__
 * 00000004 00000003 T00000002 UPDATE_ 05 06 08 key00 oldval newvalue
 * 00000005 00000000 000000000 CHECK__ T00000001 T00000002
 * 00000006 00000002 T00000001 COMMIT_
 * 00000007 00000000 000000000 CHECK__ T00000002
 * 00000008 00000004 T00000002 ABORT__ 
 * 00000009 00000005 T00000002 CLR____ 00000005 05 06 key00 oldval
 * 
 * 012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678|0123456789012345678901234567
 */

#include <stdint.h>
#include <stddef.h>

#define KV_RECOVERY_MAX_LINE_LEN 128


typedef uint32_t lsn_id;

enum kv_recovery_log_type {
    KV_REC_BEGIN,
    KV_REC_COMMIT,
    KV_REC_UPDATE,
    KV_REC_ABORT,
    KV_REC_CLR,
    KV_REC_CHECK,
    KV_REC_END,
};

struct kv_recovery_log_line {
    uint32_t lsn;
    uint32_t prev_lsn;
    uint32_t tx_id;
    enum kv_recovery_log_type log_type;
    char *key;
    size_t key_len;
    char *old_val;
    size_t old_len;
    char *new_val;
    size_t new_len;
};

int kv_recovery_lock ();
int kv_recovery_unlock ();

int kv_recovery_recover ();

int kv_recovery_init ();
int kv_recovery_destroy ();

int kv_recovery_begin_log (lsn_id prev_id, uint32_t tx_id);
int kv_recovery_commit_log (lsn_id prev_id, uint32_t tx_id);
int kv_recovery_update_log (lsn_id prev_id, uint32_t tx_id, char *key, size_t key_len, char *old_val, size_t old_len, char *new_val, size_t new_len);
int kv_recovery_abort_log (lsn_id prev_id, uint32_t tx_id);
int kv_recovery_clr_log (lsn_id prev_id, uint32_t tx_id, uint32_t undo_next_lsn, char *key, size_t key_len, char *cur_val, size_t cur_len, char *prev_val, size_t prev_len);
int kv_recovery_check_log (uint32_t *tx_id_list, size_t list_size);
int kv_recovery_end_log (lsn_id prev_id, uint32_t tx_id);

struct kv_recovery_log_line* kv_recovery_get_log (lsn_id lsn);
int kv_recovery_destroy_log_line (struct kv_recovery_log_line *log);

 
#endif
