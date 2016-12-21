// pkt_receive4.cc
#include <hooks/hooks.h>
#include <dhcp/pkt4.h>
#include "library_common.h"
#include <string>
using namespace isc::dhcp;
using namespace isc::hooks;
using namespace std;
extern "C" {
// This callout is called at the "pkt4_receive" hook.
int pkt4_receive(CalloutHandle& handle) {
    // A pointer to the packet is passed to the callout via a "boost" smart
    // pointer. The include file "pkt4.h" typedefs a pointer to the Pkt4
    // object as Pkt4Ptr.  Retrieve a pointer to the object.
    Pkt4Ptr query4_ptr;
    handle.getArgument("query4", query4_ptr);
    interesting << "new request\n";
    interesting << query4_ptr->toText() << "\n";
    flush(interesting);
    // Point to the hardware address.
    HWAddrPtr hwaddr_ptr = query4_ptr->getHWAddr();
    // The hardware address is held in a public member variable. We'll classify
    // it as interesting if the sum of all the bytes in it is divisible by 4.
    //  (This is a contrived example after all!)
    long sum = 0;
    for (int i = 0; i < hwaddr_ptr->hwaddr_.size(); ++i) {
        sum += hwaddr_ptr->hwaddr_[i];
    }
    // Classify it.
    if (1 == 1) {
        // Store the text form of the hardware address in the context to pass
        // to the next callout.
        string hwaddr = hwaddr_ptr->toText(false);
        handle.setContext("hwaddr", hwaddr);
    }
    return (0);
};
}
