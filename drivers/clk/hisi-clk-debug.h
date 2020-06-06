

#include <linux/version.h>
#include <linux/clk-provider.h>
#include <linux/fs.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0))
#include <linux/clk.h>

#include <linux/seq_file.h>
#define CLK_ADDR_HIGH_MASK	(0xFFFFFFFFFFFFF000)
#define CLK_ADDR_LOW_MASK	(0xFFF)
extern struct hlist_head *clk_all_lists_debug[];
extern char *hs_base_addr_transfer(long unsigned int base_addr);
void clk_tree_dump_reg(struct seq_file *s, struct clk_core *clk_core);
void clk_base_addr_print(struct seq_file *s);

extern struct clk *clk_get_parent_by_index(struct clk *clk, u8 index);
#endif
int hisi_clk_debug_init(void);
void clk_list_add(struct clk_core *clk);
struct dentry *debugfs_create_clkfs(struct clk_core *clk);

#ifdef CONFIG_HISI_CLK_TRACE
void track_clk_enable(struct clk *clk);
void track_clk_set_freq(struct clk *clk, u32 freq);
void track_clk_set_dvfs(struct clk *clk, u32 freq);
#endif
