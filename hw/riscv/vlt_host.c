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

/* qom/qdev */
#include "hw/riscv/vlt_host.h"
#include "hw/riscv/machines-qom.h"

/* riscv */
#include "target/riscv/cpu.h"

static void vl_host_board_init(MachineState *machine)
{
    (void)machine;

    info_report("vlt_host board init, now void");
}

static void vlt_host_machine_instance_init(Object *obj)
{
    (void)obj;

    info_report("vlt_host instance init, now void");
}

static void vlt_host_machine_class_init(ObjectClass *oc, const void *data)
{
    qemu_log("vlt_host class init");

    MachineClass *mc = MACHINE_CLASS(oc);
    mc->desc = "Minimal RISC-V SoC (CPU+RAM) for Verilator co-simulation";
    mc->init = vl_host_board_init;
}

static const TypeInfo vlt_host_machine_type_info = {
    .name       = MACHINE_TYPE_NAME("vlt_host"),
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



