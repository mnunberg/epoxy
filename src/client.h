#ifndef LCBPROXY_CLIENT_H
#define LCBPROXY_CLIENT_H
#include "common.h"
#include <list>
#include <ev.h>
#include <arpa/inet.h>
#include <netinet/in.h>

namespace Epoxy {
class EnteredContext {
public:
    EnteredContext(Client*);
    ~EnteredContext();
private:
    Client *client;
};

class Client {
public:
    friend class EnteredContext;

    /**
     * Create a new client object
     * @param d parent daemon object
     * @param fd Socket descriptor
     */
    Client(Daemon *d, int fd);
    ~Client();

    /**
     * Process the response for a command
     * @param cmd The command received
     * @param resp The response structure
     */
    void gotResponse(Command *cmd, lcb_PKTFWDRESP* resp);
    void gotResponse(Command *cmd, lcb_error_t err);

    /** Drain all outstanding items to the socket */
    void drain();

    /** Read relevant commands and dispatch them */
    void slurp();
    void checkError(int rv, int errcur);
    void ref() { refcount++; }
    void unref() { if (!--refcount) { delete this; } }
    void schedule() { if (!entered) { scheduleNocheck(); } }

private:
    void createCommands();
    void scheduleNocheck();
    bool peek(char *buf, size_t n);
    void consume(size_t n);
    void closeSock();

    std::list<BufPair> sendQueue; /** Send queue - items to write to downstream */
    Rope *rope; /**< Buffer used for incoming data */
    Daemon *parent; /**< Parent object. Contains buffer pool, among other things */
    int sockfd; /**< Socket descriptor */
    ev_io watcher; /**< Watcher for I/O */
    short watchedFor; /**< Current watcher flags */
    size_t refcount; /**< Reference count for client. Closed when 0 */
    bool entered; /**< Inside an event handler? */
    bool closed; /**< Whether this socket has an error */
};

}//namespace
#endif
