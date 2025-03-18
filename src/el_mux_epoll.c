#include <sys/epoll.h>
#include <stdlib.h>

#define MAX_EVENTS 64


int kv_el_mux_create (struct kv_event_loop* el)
{
    int epfd;
    struct epoll_event set_event;
    struct epoll_event *events;
    int e_count;

    epfd = epoll_create1(EPOLL_CLOEXEC);
    if (epfd < 0) {
        perror ("epoll_create1 error");
        exit (EXIT_FAILURE);
    }
    el->mux_fd = epfd;

    events = malloc (sizeof(struct epoll_event) * MAX_EVENTS);
    if (!events) {
        perror ("epoll malloc");
        free (events);
        close (epfd);
        exit (EXIT_FAILURE);
    }
    
    return 1;
}

void kv_el_mux_add (struct kv_event_loop* el, int fd, int mask)
{
    struct epoll_event set_event;

    set_event.data.fd = fd;
    set_event.data.ptr = 

    set_event.events = 0;
    if (mask & EL_READABLE)
        set_event.events |= EPOLLIN;
    if (mask & EL_WRITABLE)
        set_event.events |= EPOLLOUT;

    // handler
    epoll_ctl (el->mux_fd, EPOLL_CTL_ADD, fd, &set_event);

    return;
}

void kv_el_mux_delete (struct kv_event_loop* el, int fd)
{
    epoll_ctl (el->epfd, EPOLL_CTL_DEL, fd, NULL);
}

void kv_el_mux_mux (struct kv_event_loop* el)
{
    struct epoll_event *events;
    int e_count;
    struct epoll_event event;
    
    events = malloc (sizeof(struct epoll_event) * MAX_EVENTS);
    
    e_count = epoll_wait (el->mux_fd, events, MAX_EVENTS, -1);
}
