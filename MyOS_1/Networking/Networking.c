#include "Ethernet.h"
#include <stdint.h>
#include <stdbool.h>

// HACKY, temporary code ensues. Will only handle one NIC, which for now is the rtl8139

bool NetworkInterfaceCardPresent = false;
