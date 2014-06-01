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
    void checkError();
    void ref() { refcount++; }
    void unref() { if (!--refcount) { delete this; } }
    void schedule() {
        if (!entered) {
            scheduleNocheck();
        }
    }

private:
    void createCommands();
    void scheduleNocheck();

    bool peek(char *buf, size_t n);
    void consume(size_t n);

    /** Send queue - items to write to downstream */
    std::list<BufPair> sendQueue;
    Rope *rope;

    /** Parent object. Contains buffer pool, among other things */
    Daemon *parent;

    /** Socket descriptor */
    int sockfd;

    /** Reference count for client */
    size_t refcount;
    ev_io watcher;
    short watchedFor;
    struct sockaddr_in inaddr;
    std::string idstr;
    bool entered;
    bool closed;
};

}//namespace
#endif
