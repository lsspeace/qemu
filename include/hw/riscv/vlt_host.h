#ifndef HW_RISCV_VLT_HOST
#define HW_RISCV_VLT_HOST

#include "hw/core/boards.h"
#include "hw/riscv/riscv_hart.h"

typedef struct VltHostState {
    /*< private >*/
    DeviceState parent_obj;

    /*< public >*/
    RISCVHartArrayState cpus;
} VltHostState;

enum {
    VLT_HOST_DEV_DEBUG,
    VLT_HOST_DEV_MROM,
    VLT_HOST_DEV_CLINT,
    VLT_HOST_DEV_HTIF,
    VLT_HOST_DEV_VL_BRG,
    VL_HOST_DRAM
};

#endif