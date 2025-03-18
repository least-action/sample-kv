#include <stddef.h>
#include <stdlib.h>

#include "el_event_loop.h"
#if 1
    #include "el_mux_epoll.c"
#elif 0
    #include "el_mux_kqueue.c"
#endif


struct kv_event_loop* kv_el_create_event_loop ()
{
    struct kv_event_loop* el = (struct kv_event_loop *) malloc (sizeof (struct kv_event_loop));
    kv_el_mux_create (el);

    return el;
}

void kv_el_create_file_event (struct kv_event_loop* el, int fd, kv_task* handler, void* data, int mask)
{
    kv_el_mux_add (el, fd, mask);
    
    return;
}

void kv_el_delete_file_event (struct kv_event_loop* el, int fd, int mask)
{
    kv_el_mux_delete (el, fd);
    // todo
    return;
}

void kv_el_process_event_loop (struct kv_event_loop* el)
{   
    // todo
    while (1) {
        // kv_el_mux_mux ();
    }
}
