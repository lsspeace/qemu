#ifndef HW_RISCV_VLT_HOST
#define HW_RISCV_VLT_HOST

#include "hw/core/boards.h"
#include "hw/riscv/riscv_hart.h"

#include "system/memory.h"


typedef struct VltHostState {
    /*< private >*/
    MachineState parent_obj;

    /*< public >*/
    RISCVHartArrayState cpus;
    MemoryRegion        mrom;
} VltHostState;

#define TYPE_VLT_HOST_MACHINE   MACHINE_TYPE_NAME("vlt_host")
DECLARE_INSTANCE_CHECKER(VltHostState, VLT_HOST_MACHINE, TYPE_VLT_HOST_MACHINE)

enum {
    VLT_HOST_DEV_TEST,
    VLT_HOST_DEV_MROM,
    VLT_HOST_DEV_CLINT,
    VLT_HOST_DEV_VL_BRG,
    VLT_HOST_DEV_DRAM
};

#endif