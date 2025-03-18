#ifndef __KV_EL_EVENT_LOOP_H__
#define __KV_EL_EVENT_LOOP_H__


#define EL_READABLE 1
#define EL_WRITABLE 2

struct kv_event_loop;

typedef void kv_task (struct kv_event_loop* el, int fd, void* data);

struct kv_file_event {
    int fd;
    kv_task *call_back;
    void *data;
};

struct kv_event_loop {
    int mux_fd;
    struct kv_file_event *events;
};

struct kv_event_loop* kv_el_create_event_loop ();

void kv_el_create_file_event (struct kv_event_loop* el, int fd, kv_task* handler, void* data, int mask);

void kv_el_delete_file_event (struct kv_event_loop* el, int fd, int mask);
void kv_el_process_event_loop (struct kv_event_loop* el);

#endif
