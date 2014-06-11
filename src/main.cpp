#define CLIOPTS_ENABLE_CXX
#include "common.h"
#include "daemon.h"
#include "cliopts.h"
using namespace Epoxy;

int main(int argc, char **argv)
{
    cliopts::StringOption optDsn('u', "uri", "couchbase://", "Upstream URI");
    cliopts::UIntOption optPort('p', "port", 4444, "Port to listen on");
    cliopts::Parser p("Epoxy");

    p.addOption(&optDsn);
    p.addOption(&optPort);
    p.parse(argc, argv);

    epoxy_yolog_init(NULL);
    Daemon d(optDsn.result(), optPort.result());
    d.run();
    return 0;
}
