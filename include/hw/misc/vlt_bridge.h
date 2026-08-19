#ifndef HW_VLT_BRIDGE_H
#define HW_VLT_BRIDGE_H

#include "qom/object.h"

#include "hw/core/sysbus.h"
#include "chardev/char-fe.h"
#include "system/memory.h"

typedef struct VltBridgeState {
    SysBusDevice parent_obj;

    MemoryRegion iomem;   /* MMIO region forwarded to external model */
    CharFrontend chr;     /* chardev socket to Verilator */
    uint64_t size;        /* size of the MMIO window */
} VltBridgeState;

#define TYPE_VLT_BRIDGE "vlt_bridge"
OBJECT_DECLARE_SIMPLE_TYPE(VltBridgeState, VLT_BRIDGE)

#endif
