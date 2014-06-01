#define CLIOPTS_ENABLE_CXX
#include "common.h"
#include "daemon.h"
#include "cliopts.h"
using namespace Epoxy;

int main(int argc, char **argv)
{
    cliopts::StringOption optBucket('b', "bucket", "default", "Bucket to service");
    cliopts::UIntOption optPort('p', "port", 4444, "Port to listen on");
    cliopts::Parser p("Epoxy");

    p.addOption(&optBucket);
    p.addOption(&optPort);
    p.parse(argc, argv);

    Daemon d(optBucket.result(), optPort.result());
    d.run();
    return 0;
}
