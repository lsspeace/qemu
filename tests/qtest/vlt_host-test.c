#include "qemu/osdep.h"
#include "qemu/timer.h"
#include "hw/intc/riscv_aclint.h" 
#include "libqtest.h"


#define NS_PER_TICK  (NANOSECONDS_PER_SECOND/RISCV_ACLINT_DEFAULT_TIMEBASE_FREQ)

#define CLINT_BASE   0x2000000
#define MTIMECMP0    (CLINT_BASE + 0x4000 + 0x0)    /* mtimecmp of hart0 */
#define MTIME        (CLINT_BASE + 0x4000 + 0x7ff8) /* mtime  */

static void test_clint_timer(void)
{
    QTestState *qts = qtest_init("-machine vlt_host");
    uint64_t mtime;
    uint64_t tick_cnt = 0, tickt_wait = g_random_int_range(500, 5000);
    uint64_t deadline = tickt_wait * 2;
    bool irq7_flag;

    qtest_irq_intercept_in(qts, "/machine/cpus/harts[0]");

    mtime = qtest_readq(qts, MTIME);
    qtest_writeq(qts, MTIMECMP0, mtime + tickt_wait);

    do
    {
        tick_cnt++;
        qtest_clock_step(qts, NS_PER_TICK);
        irq7_flag = qtest_get_irq(qts, 7);

        if (irq7_flag)
            break;
    } while (tick_cnt < deadline);

    mtime = qtest_readq(qts, MTIME);
    g_test_message("tickt_wait:%ld, mtime:%ld, irq7_flag:%d",
                   tickt_wait, mtime, irq7_flag);
    g_assert_true(irq7_flag);

    qtest_quit(qts);
}

static void test_ram(void)
{
    QTestState *qts = qtest_init("-machine vlt_host");
    uint32_t ram_addr;
    uint32_t w_val, r_val;

    for (int i = 0; i < 1000; i++)
    {
        ram_addr = 0x80000000 + (g_random_int() & 0xffff);
        w_val = g_random_int_range(0, INT32_MAX);
        r_val = UINT32_MAX;

        qtest_writel(qts, ram_addr, w_val);
        r_val = qtest_readl(qts, ram_addr);

        //g_test_message("addr:0x%x, w:0x%x, r:0x%x", ram_addr, w_val, r_val);
        g_assert_cmphex(r_val, ==, w_val);
    }

    qtest_quit(qts);
}

int main(int argc, char **argv)
{
    g_test_init(&argc, &argv, NULL);
    qtest_add_func("/vlt_host/ram", test_ram);
    qtest_add_func("/vlt_host/clint", test_clint_timer);
    return g_test_run();
}