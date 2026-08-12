/*
 * QEMU RISC-V Verilator Host Board
 *
 * This provides a RISC-V Board with the following devices:
 *
 * 0)
 * 1)
 * 2) Verilator bridge

 */

/* base tools */
#include "qemu/osdep.h"
#include "qemu/error-report.h"
#include "qemu/log.h"
#include "qemu/units.h"
#include "qapi/error.h"

/* qom/qdev */
#include "hw/riscv/vlt_host.h"
#include "hw/riscv/machines-qom.h"
#include "hw/misc/sifive_test.h"

/* riscv */
#include "target/riscv/cpu.h"

/* system */
#include "system/address-spaces.h"

static const MemMapEntry vlt_host_map[] = {
    [VLT_HOST_DEV_TEST] = {        0x0, 0x1000 },

    [VLT_HOST_DEV_DRAM] = { 0x80000000, 0x0    }
};

static void vlt_host_board_init(MachineState *machine)
{
    VltHostState *s = VLT_HOST_MACHINE(machine);
    const MemMapEntry *memmap = vlt_host_map;

    /* Initialize CPU */
    info_report("cpu_type=%s", machine->cpu_type ? machine->cpu_type : " ");
    object_initialize_child(OBJECT(machine), "cpus", &s->cpus, 
                            TYPE_RISCV_HART_ARRAY);
    object_property_set_str(OBJECT(&s->cpus), "cpu-type",
                            machine->cpu_type, &error_abort);
    sysbus_realize(SYS_BUS_DEVICE(&s->cpus), &error_fatal);

    /* Create SiFive Test MMIO device */
    sifive_test_create(memmap[VLT_HOST_DEV_TEST].base);


    /* Register system main memory */
    // [TODO]

    info_report("vlt_host board init, now void");
}

static void vlt_host_machine_instance_init(Object *obj)
{
    (void)obj;
    qemu_log("vlt_host instance init, do nothing\n");
}

static void vlt_host_machine_class_init(ObjectClass *oc, const void *data)
{
    MachineClass *mc = MACHINE_CLASS(oc);
    mc->desc = "Minimal RISC-V SoC (CPU+RAM) for Verilator co-simulation";
    mc->init = vlt_host_board_init;
    mc->default_cpu_type = TYPE_RISCV_CPU_MAX;
}

static const TypeInfo vlt_host_machine_type_info = {
    .name       = TYPE_VLT_HOST_MACHINE,
    .parent     = TYPE_MACHINE,
    .class_init = vlt_host_machine_class_init,
    .instance_init = vlt_host_machine_instance_init,
    .instance_size = sizeof(VltHostState),
    .interfaces = riscv64_machine_interfaces,
};

static void vlt_host_machine_init_register_types(void)
{
    type_register_static(&vlt_host_machine_type_info);
}

type_init(vlt_host_machine_init_register_types)