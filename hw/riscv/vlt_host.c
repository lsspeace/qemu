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
#include "hw/riscv/boot.h"
#include "hw/riscv/fdt-common.h"
#include "hw/misc/sifive_test.h"
#include "hw/intc/riscv_aclint.h"

/* riscv */
#include "target/riscv/cpu.h"

/* system */
#include "system/address-spaces.h"

static const MemMapEntry vlt_host_map[] = {
    [VLT_HOST_DEV_TEST]     = {        0x0, 0x1000  },
    [VLT_HOST_DEV_MROM]     = {     0x1000, 0xf000  },
    [VLT_HOST_DEV_CLINT]    = {  0x2000000, 0x10000 },
    [VLT_HOST_DEV_DRAM]     = { 0x80000000, 0x0     },
};

static void create_fdt(VltHostState* s)
{
    MachineState *ms = MACHINE(s);
    int fdt_size;
    ms->fdt = create_board_device_tree("vlt_host", "vl_host_dev", &fdt_size);

    /* do nothing */
}

static void vlt_host_board_init(MachineState *machine)
{
    VltHostState *s = VLT_HOST_MACHINE(machine);
    MemoryRegion* sys_mem = get_system_memory();
    RISCVBootInfo boot_info;
    uint64_t fdt_load_addr;
    const MemMapEntry *memmap = vlt_host_map;

    /* Initialize CPU */
    object_initialize_child(OBJECT(machine), "cpus", &s->cpus, 
                            TYPE_RISCV_HART_ARRAY);
    object_property_set_str(OBJECT(&s->cpus), "cpu-type",
                            machine->cpu_type, &error_fatal);
    sysbus_realize(SYS_BUS_DEVICE(&s->cpus), &error_fatal);

    /* Create SiFive Test MMIO device */
    sifive_test_create(memmap[VLT_HOST_DEV_TEST].base);

    /* Register Boot ROM (filled later by riscv_setup_rom_reset_vec) */
    memory_region_init_rom(&s->mrom, NULL, "riscv.vlt_host.mrom",
                           memmap[VLT_HOST_DEV_MROM].size, &error_fatal);
    memory_region_add_subregion(sys_mem, memmap[VLT_HOST_DEV_MROM].base, 
                                &s->mrom);

    /* Configure CLINT
     * SWI @0x2000000, MTIMER @0x2004000 (10MHz, IRQ7 timer / IRQ3 soft) 
     */
    riscv_aclint_swi_create(memmap[VLT_HOST_DEV_CLINT].base, 
                            0,  machine->smp.cpus, false);
    riscv_aclint_mtimer_create(memmap[VLT_HOST_DEV_CLINT].base 
                                + RISCV_ACLINT_SWI_SIZE,
                               RISCV_ACLINT_DEFAULT_MTIMER_SIZE,
                               0, machine->smp.cpus, 
                               RISCV_ACLINT_DEFAULT_MTIMECMP,
                               RISCV_ACLINT_DEFAULT_MTIME,
                               RISCV_ACLINT_DEFAULT_TIMEBASE_FREQ, true);

    /* Register system main memory */
    memory_region_add_subregion(sys_mem, memmap[VLT_HOST_DEV_DRAM].base, 
                                machine->ram);

    /* Ready to boot */
    riscv_boot_info_init(&boot_info, &s->cpus);
    
    if (machine->kernel_filename)
    {
        riscv_load_kernel(machine, &boot_info, memmap[VLT_HOST_DEV_DRAM].base,
                          false, NULL);
    }

    create_fdt(s);
    fdt_load_addr = riscv_compute_fdt_addr(memmap[VLT_HOST_DEV_DRAM].base,
                                           memmap[VLT_HOST_DEV_DRAM].size,
                                           machine, &boot_info);
    riscv_load_fdt(fdt_load_addr, machine->fdt);

    /* Jumping to the kernel with a0=hartid, a1=FDT */
    riscv_setup_rom_reset_vec(machine, &s->cpus, memmap[VLT_HOST_DEV_DRAM].base,
                              memmap[VLT_HOST_DEV_MROM].base,
                              memmap[VLT_HOST_DEV_MROM].size,
                              boot_info.image_low_addr, fdt_load_addr);
}

static void vlt_host_machine_instance_init(Object *obj)
{
    (void)obj;
}

static void vlt_host_machine_class_init(ObjectClass *oc, const void *data)
{
    MachineClass *mc = MACHINE_CLASS(oc);
    mc->desc = "Minimal RISC-V SoC (CPU+RAM) for Verilator co-simulation";
    mc->init = vlt_host_board_init;
    mc->default_cpu_type = TYPE_RISCV_CPU_MAX;
    mc->default_ram_id   = "riscv.vlt_host.ram";
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