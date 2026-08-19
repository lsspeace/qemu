/*
 * vlt_bridge.c - MMIO bridge device.
 *
 * Guest MMIO access is forwarded over a chardev (TCP socket)
 * to an external Verilator model.
 *
 * Protocol (little-endian):
 *   request 21 bytes: [addr:8][size:4][is_write:1][data:8]
 *   response 9 bytes: [err:1][data:8]
 */

#include "qemu/osdep.h"
#include "qapi/error.h"
#include "hw/core/sysbus.h"
#include "hw/core/qdev-properties-system.h"
#include "qemu/units.h"

#include "hw/misc/vlt_bridge.h"

#define vlt_BRIDGE_REQUEST_LEN 21
#define vlt_BRIDGE_RESPONSE_LEN 9

/* Send one request, wait for one response. */
static void vlt_bridge_do_transaction(VltBridgeState *s, hwaddr addr,
                                     uint32_t size, bool is_write,
                                     uint64_t *data, Error **errp)
{
    uint8_t req[vlt_BRIDGE_REQUEST_LEN] = {0};
    uint8_t rsp[vlt_BRIDGE_RESPONSE_LEN] = {0};

    stq_le_p(&req[0], addr);
    stl_le_p(&req[8], size);
    req[12] = is_write ? 1 : 0;
    if (is_write) {
        stq_le_p(&req[13], *data);
    }

    if (qemu_chr_fe_write_all(&s->chr, req, sizeof(req)) != sizeof(req)) {
        error_setg(errp, "vl-bridge: failed to send request");
        return;
    }
    if (qemu_chr_fe_read_all(&s->chr, rsp, sizeof(rsp)) != sizeof(rsp)) {
        error_setg(errp, "vl-bridge: failed to receive response");
        return;
    }
    if (rsp[0] != 0) {
        error_setg(errp, "vl-bridge: remote error %d", rsp[0]);
        return;
    }
    if (!is_write) {
        *data = ldq_le_p(&rsp[1]);
    }
}

/* MMIO read: forward to remote, return its data. */
static uint64_t vlt_bridge_read(void *opaque, hwaddr addr, unsigned int size)
{
    VltBridgeState *s = VLT_BRIDGE(opaque);
    Error *err = NULL;
    uint64_t data = 0;

    vlt_bridge_do_transaction(s, addr, size, false, &data, &err);
    if (err) {
        error_report_err(err);
    }
    return data;
}

/* MMIO write: forward to remote. */
static void vlt_bridge_write(void *opaque, hwaddr addr, uint64_t data,
                            unsigned int size)
{
    VltBridgeState *s = VLT_BRIDGE(opaque);
    Error *err = NULL;

    vlt_bridge_do_transaction(s, addr, size, true, &data, &err);
    if (err) {
        error_report_err(err);
    }
}

static const MemoryRegionOps vlt_bridge_ops = {
    .read = vlt_bridge_read,
    .write = vlt_bridge_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
    .impl.min_access_size = 1,
    .impl.max_access_size = 8,
};

static void vlt_bridge_realize(DeviceState *dev, Error **errp)
{
    VltBridgeState *s = VLT_BRIDGE(dev);
    SysBusDevice *sbd = SYS_BUS_DEVICE(dev);

    if (!qemu_chr_fe_backend_connected(&s->chr)) {
        error_setg(errp, "vl-bridge: chardev is not set");
        return;
    }
    qemu_chr_fe_set_handlers(&s->chr, NULL, NULL, NULL, NULL, s, NULL, true);

    /* Create the MMIO window. */
    memory_region_init_io(&s->iomem, OBJECT(dev), &vlt_bridge_ops, s,
                          "vl-bridge", s->size);
    sysbus_init_mmio(sbd, &s->iomem);
}

static Property vlt_bridge_properties[] = {
    DEFINE_PROP_CHR("chardev", VltBridgeState, chr),
    DEFINE_PROP_UINT64("size", VltBridgeState, size, 4 * KiB),
};

static void vlt_bridge_class_init(ObjectClass *klass, const void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);

    dc->realize = vlt_bridge_realize;
    dc->desc = "MMIO bridge forwarding to an external model (Verilator)";
    device_class_set_props(dc, vlt_bridge_properties);
}

static const TypeInfo vlt_bridge_type_info = {
    .name = TYPE_VLT_BRIDGE,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(VltBridgeState),
    .class_init = vlt_bridge_class_init,
};

static void vlt_bridge_register_types(void)
{
    type_register_static(&vlt_bridge_type_info);
}

type_init(vlt_bridge_register_types)
