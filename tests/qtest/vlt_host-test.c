#include "qemu/osdep.h"
#include "libqtest.h"

static void test_ram(void)
{
    QTestState *qts = qtest_init("-machine vlt_host");

    for (int i = 0; i < 1000; i++)
    {
        uint32_t ram_addr = 0x80000000 + (g_random_int() & 0xffff);
        uint32_t w_value = g_random_int_range(0, INT32_MAX);
        uint32_t r_value = UINT32_MAX;

        qtest_writel(qts, ram_addr, w_value);
        r_value = qtest_readl(qts, ram_addr);

        g_test_message("addr=0x%x, w:0x%x, r:0x%x", ram_addr, w_value, r_value);
        g_assert_cmphex(r_value, ==, w_value);
    }

    qtest_quit(qts);
}

int main(int argc, char **argv)
{
    g_test_init(&argc, &argv, NULL);
    qtest_add_func("/vlt_host/demo", test_ram);
    return g_test_run();
}