## 개요

1.  다음과 같은 lock 정책을 사용.
    ```
            read  |  write
    read    true  |   false
    write  false  |   false
    ```
    
2. rigorous 2PL(2 Phase Lock) 을 사용.
    * strict 2PL 과 차이점을 잘 이해하지 못하겠음.
    * 2PL 은 직렬성을 보장해줄 수 있음.
    * rigorous(or strict) 2PL 은 dirty read 를 방지하여 연쇄 복구를 방지하여 복구 과정을 간소화 할 수 있음.

3. UNDO 로그를 이용해 transaction rollback.

4. REDO 로그와 UNDO 로그를 이용해 복구.


## REDO log
서버가 처리는 했지만 영구 저장장치(ex. hdd)에 쓰기 전 시스템 종료된 상황에 복구하기 위한 로그 데이터

kvdb 파일에는 수행한 LSN 의 마지막 값을 포함하고 있음.
멱등성을 위해서 INC 명령 대신 WRITE 명령으로 작성.

### format
{8자리 LSN} {W | D} {2자리 key len} {2자리 val len} {key} {value}
```log
00000001 W 03 05 key value
00000002 D 03 00 key
00000003 W 03 06 key value1
00000004 W 04 07 name michale
```

## UNDO log

kvdb 파일에는 수행한 LSN 의 마지막 값을 포함하고 있음.
멱등성을 위해서 INC 명령 대신 WRITE 명령으로 작성.

### format
```log
00000001 T00000001 B 
00000002 T00000002 B
00000003 T00000003 B
00000004 T00000001 C
00000005 T00000003 W 04 05 06 name henry george
00000005 T00000002 A
```

## REDO/UNDO log
서버가 처리는 했지만 영구 저장장치(ex. hdd)에 쓰기 전 시스템 종료된 상황에 복구하기 위한 로그 데이터.
Transaction rollback 시 복구를 위한 로그 데이터.

kvdb 파일에는 수행한 LSN 의 마지막 값을 포함하고 있음.
멱등성을 위해서 INC 명령 대신 WRITE 명령으로 작성.

### format
{8자리 LSN} T{Transaction ID} {W | D} {2자리 key len} {2자리 val len} {2자리 old val len} {key} {new value} {old value}
```log
00000001 T00000001 B
00000002 T00000001 W 03 05 00 key old_value NULL
00000003 T00000001 C
00000004 T00000002 B
00000005 T00000002 D 03 05 key old_value
00000006 T00000003 D 03 00 name NULL
00000007 T00000002 A
00000008 T00000003 B
00000009 T00000003 W 04 07 00 name michale NULL
00000010 T00000003 W 03 09 00 key old_value NULL
00000011 T00000003 W 03 09 09 key new_value old_value
```


## in_prograss_transactions 파일
system crash 후 복구 과정에서 UNDO 여부를 확인해야하는 transaction 대상 확인용.
종료되지 않은 트랜잭션은 반드시 로그에 남아있어야 하지만, 서버 응답 속도 향상을 위해 종료된 트랜잭션이 파일에 남아있을 수도 있다.
이게 없으면 log 전체를 읽어서 종료하지 않은 transaction 이 있는지 확인해야 한다.

```log
T00000002
T00000003
```

## kvdb file

### format
LAST_LSN: {log LSN}  
{key1 len} {val1 len} {key1} {val1}  
{key2 len} {val2 len} {key2} {val2}  
...
```kvdb
LAST_LSN: 00000004
03 05 key value
04 07 name michale
```

## 복구 과정
(복구 과정 중에는 kvdb 를 업데이트하지 않는다. 이유는 복구 중에 실패 발생시 문제를 파악하기 쉬움.)

1. 종료되지 않은 트랜잭션들을 찾아 REDO/UNDO log 에 A 로그를 추가한다. (A 로그를 추가하면서 진행중인 트랜잭션 데이터에서 제거한다. 혹시 로그를 추가했지만 진행중인 트랜잭션에서 제거하지 못하고 crash 가 난 경우를 대비해 한 Transaction 에서 A 가 여러번 찍혀도 문제 없게 설계한다.)
2. kvdb 파일을 기반으로 memory 로 읽어온다.
3. log LSN 위치를 찾아 그 뒤부터 REDO 를 수행한다.

참고: 복구과정에서는 A 명령 수행시 특정 Transaction 이 수행한 LSN 들을 아래에서부터 B가 나올때 까지 훑어보지만, 서비스 중에 A 명령 수행시 메모리상에 저장해놓은 REDO/UNDO 파일의 position 목록을 보며 수행한다.