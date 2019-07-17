/* uSockets is entierly opaque so we can use the real header straight up */
#include "../uSockets/src/libusockets.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdalign.h>

struct us_loop_t {

    /* We only support one listen socket */
    alignas(16) struct us_listen_socket_t *listen_socket;

    /* The list of closed sockets */
    struct us_socket_t *close_list;
};

struct us_loop_t *us_create_loop(void *hint, void (*wakeup_cb)(struct us_loop_t *loop), void (*pre_cb)(struct us_loop_t *loop), void (*post_cb)(struct us_loop_t *loop), unsigned int ext_size) {
    struct us_loop_t *loop = (struct us_loop_t *) malloc(sizeof(struct us_loop_t) + ext_size);

    loop->listen_socket = 0;
    loop->close_list = 0;

    return loop;
}

void us_loop_free(struct us_loop_t *loop) {
    free(loop);
}

void *us_loop_ext(struct us_loop_t *loop) {
    return loop + 1;
}

void us_loop_run(struct us_loop_t *loop) {

}

struct us_socket_context_t {
    alignas(16) struct us_loop_t *loop;

    struct us_socket_t *(*on_open)(struct us_socket_t *s, int is_client, char *ip, int ip_length);
    struct us_socket_t *(*on_close)(struct us_socket_t *s);
    struct us_socket_t *(*on_data)(struct us_socket_t *s, char *data, int length);
    struct us_socket_t *(*on_writable)(struct us_socket_t *s);
    struct us_socket_t *(*on_timeout)(struct us_socket_t *s);
    struct us_socket_t *(*on_end)(struct us_socket_t *s);
};

struct us_socket_context_t *us_create_socket_context(int ssl, struct us_loop_t *loop, int ext_size, struct us_socket_context_options_t options) {
    struct us_socket_context_t *socket_context = (struct us_socket_context_t *) malloc(sizeof(struct us_socket_context_t) + ext_size);

    socket_context->loop = loop;

    //printf("us_create_socket_context: %p\n", socket_context);

    return socket_context;
}

void us_socket_context_free(int ssl, struct us_socket_context_t *context) {
    //printf("us_socket_context_free: %p\n", context);
    free(context);
}

void us_socket_context_on_open(int ssl, struct us_socket_context_t *context, struct us_socket_t *(*on_open)(struct us_socket_t *s, int is_client, char *ip, int ip_length)) {
    context->on_open = on_open;
}

void us_socket_context_on_close(int ssl, struct us_socket_context_t *context, struct us_socket_t *(*on_close)(struct us_socket_t *s)) {
    context->on_close = on_close;
}

void us_socket_context_on_data(int ssl, struct us_socket_context_t *context, struct us_socket_t *(*on_data)(struct us_socket_t *s, char *data, int length)) {
    context->on_data = on_data;
}

void us_socket_context_on_writable(int ssl, struct us_socket_context_t *context, struct us_socket_t *(*on_writable)(struct us_socket_t *s)) {
    context->on_writable = on_writable;
}

void us_socket_context_on_timeout(int ssl, struct us_socket_context_t *context, struct us_socket_t *(*on_timeout)(struct us_socket_t *s)) {
    context->on_timeout = on_timeout;
}

void us_socket_context_on_end(int ssl, struct us_socket_context_t *context, struct us_socket_t *(*on_end)(struct us_socket_t *s)) {
    context->on_end = on_end;
}

void *us_socket_context_ext(int ssl, struct us_socket_context_t *context) {
    return context + 1;
}

struct us_listen_socket_t {
    int socket_ext_size;
    struct us_socket_context_t *context;
};

struct us_listen_socket_t *us_socket_context_listen(int ssl, struct us_socket_context_t *context, const char *host, int port, int options, int socket_ext_size) {
    struct us_listen_socket_t *listen_socket = (struct us_listen_socket_t *) malloc(sizeof(struct us_listen_socket_t));

    listen_socket->socket_ext_size = socket_ext_size;
    listen_socket->context = context;

    context->loop->listen_socket = listen_socket;

    return listen_socket;
}

void us_listen_socket_close(int ssl, struct us_listen_socket_t *ls) {
    free(ls);
}

struct us_socket_t {
    alignas(16) struct us_socket_context_t *context;

    int closed;
    int shutdown;

    //struct us_socket_t *next;
};

struct us_socket_t *us_socket_context_connect(int ssl, struct us_socket_context_t *context, const char *host, int port, int options, int socket_ext_size) {
    //printf("us_socket_context_connect\n");

    return 0;
}

struct us_loop_t *us_socket_context_loop(int ssl, struct us_socket_context_t *context) {
    return context->loop;
}

struct us_socket_t *us_socket_context_adopt_socket(int ssl, struct us_socket_context_t *context, struct us_socket_t *s, int ext_size) {
    struct us_socket_t *new_s = (struct us_socket_t *) realloc(s, sizeof(struct us_socket_t) + ext_size);
    new_s->context = context;

    //printf("us_socket_context_adopt_socket: %p till %p\n", s, new_s);
    //printf("new context is: %p\n", new_s->context);

    return new_s;
}

struct us_socket_context_t *us_create_child_socket_context(int ssl, struct us_socket_context_t *context, int context_ext_size) {
    /* We simply create a new context in this mock */
    struct us_socket_context_options_t options = {};
    struct us_socket_context_t *child_context = us_create_socket_context(ssl, context->loop, context_ext_size, options);

    //printf("us_create_child_socket_context: %p\n", child_context);

    return child_context;
}

int us_socket_write(int ssl, struct us_socket_t *s, const char *data, int length, int msg_more) {
    return length;
}

void us_socket_timeout(int ssl, struct us_socket_t *s, unsigned int seconds) {

}

void *us_socket_ext(int ssl, struct us_socket_t *s) {
    return s + 1;
}

struct us_socket_context_t *us_socket_context(int ssl, struct us_socket_t *s) {
    return s->context;
}

void us_socket_flush(int ssl, struct us_socket_t *s) {

}

void us_socket_shutdown(int ssl, struct us_socket_t *s) {
    s->shutdown = 1;
}

int us_socket_is_shut_down(int ssl, struct us_socket_t *s) {

    //printf("us_socket_is_shut_down: %d\n", s->shutdown);

    return s->shutdown;
}

int us_socket_is_closed(int ssl, struct us_socket_t *s) {

    //printf("us_socket_is_closed: %d\n", s->closed);

    return s->closed;
}

struct us_socket_t *us_socket_close(int ssl, struct us_socket_t *s) {
    s->closed = 1;

    //printf("us_socket_close\n");

    /* Add us to the close list */

    return s;
}

void us_socket_remote_address(int ssl, struct us_socket_t *s, char *buf, int *length) {
    printf("us_socket_remote_address\n");
}

/* We expose this function to let fuzz targets push data to uSockets */
void us_loop_read_mocked_data(struct us_loop_t *loop, char *data, unsigned int size) {

    //printf("us_loop_read_mocked_data\n");

    /* We are unwound so let's free all closed polls here */


    /* We have one listen socket */
    int socket_ext_size = loop->listen_socket->socket_ext_size;

    /* Create a socket with information from the listen socket */
    struct us_socket_t *s = (struct us_socket_t *) malloc(sizeof(struct us_socket_t) + socket_ext_size);
    s->context = loop->listen_socket->context;
    s->closed = 0;
    s->shutdown = 0;

    /* Emit open event */
    s = s->context->on_open(s, 0, 0, 0);
    if (!us_socket_is_closed(0, s)) {
        /* Emit a bunch of data events here */
        s = s->context->on_data(s, data, size);
        if (!us_socket_is_closed(0, s)) {
            /* Emit close event */
            s = s->context->on_close(s);
        }
    }

    /* Free the socket */
    free(s);
}
