#include <linux/kernel.h>
#include <linux/string.h>

#include <asm/system_info.h>
#include <soc/qcom/lge/board_lge.h>
#include <linux/of.h>
#include <linux/of_fdt.h>
#include <linux/platform_device.h>
#include <asm/system_misc.h>

#ifdef CONFIG_LGE_USB_FACTORY
#include <linux/platform_data/lge_android_usb.h>
#endif

static enum hw_rev_type lge_bd_rev = HW_REV_MAX;
static bool vdd_always_on = false;
#if defined(CONFIG_MACH_MSM8937_PH2_GLOBAL_COM) || defined(CONFIG_MACH_MSM8937_PH2_CMO_CN)
char *rev_str[] = {"rev_0", "rev_a", "rev_b", "rev_c", "rev_10", "rev_11", "rev_12",
	"reserved"};
#elif defined(CONFIG_MACH_MSM8937_PH2N_TMO_US) || defined(CONFIG_MACH_MSM8937_L5_DCM_JP) || defined(CONFIG_MACH_MSM8937_JSG_KDDI_JP)\
	  || defined(CONFIG_MACH_MSM8937_PH2N_MPCS_US) || defined(CONFIG_MACH_MSM8937_PH2N_GLOBAL_CA)
char *rev_str[] = {"hdk_a", "hdk_b", "rdk", "rev_a", "rev_b", "rev_c",
	"rev_10", "rev_11", "revserved"};
#else
char *rev_str[] = {"evb1", "evb2", "evb3", "rev_0", "rev_01", "rev_f", "rev_b", "rev_c",
	"rev_d", "rev_e", "rev_a", "rev_10", "rev_11", "rev_12",
	"reserved"};
#endif

static int __init board_revno_setup(char *rev_info)
{
	int i;

	for (i = 0; i < HW_REV_MAX; i++) {
		if (!strncmp(rev_info, rev_str[i], 6)) {
			lge_bd_rev = i;
			system_rev = lge_bd_rev;
			break;
		}
	}
	pr_info("BOARD: LGE %s\n", rev_str[lge_bd_rev]);

	return 1;
}
__setup("lge.rev=", board_revno_setup);

enum hw_rev_type lge_get_board_revno(void)
{
	return lge_bd_rev;
}

static int lge_boot_reason = -1;

static int __init lge_check_bootreason(char *reason)
{
	int ret = 0;

	/* handle corner case of kstrtoint */
	if (!strcmp(reason, "0xffffffff")) {
		lge_boot_reason = 0xffffffff;
		return 1;
	}

	ret = kstrtoint(reason, 16, &lge_boot_reason);
	if (!ret)
		pr_info("LGE BOOT REASON: 0x%x\n", lge_boot_reason);
	else
		pr_info("LGE BOOT REASON: Couldn't get bootreason - %d\n", ret);

	return 1;
}
__setup("lge.bootreasoncode=", lge_check_bootreason);

int lge_get_bootreason(void)
{
	return lge_boot_reason;
}

int on_hidden_reset;

static int __init lge_check_hidden_reset(char *reset_mode)
{
	if (!strncmp(reset_mode, "on", 2))
		on_hidden_reset = 1;

	return 1;
}
__setup("lge.hreset=", lge_check_hidden_reset);

/*byungyong.hwang*/
static int __init emmc_vdd_always_on_setup(char *str)
{
	vdd_always_on = true;
	pr_info("vdd_always_on=%d\n", (int)vdd_always_on);

	return 1;
}
__setup("emmc-vdd-always-on=", emmc_vdd_always_on_setup);

bool lge_get_vdd_always_on(void)
{
	return vdd_always_on;
}
/*byungyong.hwang*/

#ifdef CONFIG_LGE_USB_FACTORY
/* get boot mode information from cmdline.
 * If any boot mode is not specified,
 * boot mode is normal type.
 */
static enum lge_boot_mode_type lge_boot_mode = LGE_BOOT_MODE_NORMAL;
int __init lge_boot_mode_init(char *s)
{
	if (!strcmp(s, "charger"))
		lge_boot_mode = LGE_BOOT_MODE_CHARGER;
	else if (!strcmp(s, "chargerlogo"))
		lge_boot_mode = LGE_BOOT_MODE_CHARGERLOGO;
	else if (!strcmp(s, "qem_56k"))
		lge_boot_mode = LGE_BOOT_MODE_QEM_56K;
	else if (!strcmp(s, "qem_130k"))
		lge_boot_mode = LGE_BOOT_MODE_QEM_130K;
	else if (!strcmp(s, "qem_910k"))
		lge_boot_mode = LGE_BOOT_MODE_QEM_910K;
	else if (!strcmp(s, "pif_56k"))
		lge_boot_mode = LGE_BOOT_MODE_PIF_56K;
	else if (!strcmp(s, "pif_130k"))
		lge_boot_mode = LGE_BOOT_MODE_PIF_130K;
	else if (!strcmp(s, "pif_910k"))
		lge_boot_mode = LGE_BOOT_MODE_PIF_910K;
	/* LGE_UPDATE_S for MINIOS2.0 */
	else if (!strcmp(s, "miniOS"))
		lge_boot_mode = LGE_BOOT_MODE_MINIOS;
	pr_info("ANDROID BOOT MODE : %d %s\n", lge_boot_mode, s);
	/* LGE_UPDATE_E for MINIOS2.0 */

	return 1;
}
__setup("androidboot.mode=", lge_boot_mode_init);

enum lge_boot_mode_type lge_get_boot_mode(void)
{
	return lge_boot_mode;
}

int lge_get_factory_boot(void)
{
	int res;

	/*   if boot mode is factory,
	 *   cable must be factory cable.
	 */
	switch (lge_boot_mode) {
	case LGE_BOOT_MODE_QEM_56K:
	case LGE_BOOT_MODE_QEM_130K:
	case LGE_BOOT_MODE_QEM_910K:
	case LGE_BOOT_MODE_PIF_56K:
	case LGE_BOOT_MODE_PIF_130K:
	case LGE_BOOT_MODE_PIF_910K:
	case LGE_BOOT_MODE_MINIOS:
		res = 1;
		break;
	default:
		res = 0;
		break;
	}
	return res;
}

int get_factory_cable(void)
{
	int res = 0;

	/* if boot mode is factory, cable must be factory cable. */
	switch (lge_boot_mode) {
	case LGE_BOOT_MODE_QEM_56K:
	case LGE_BOOT_MODE_PIF_56K:
		res = LGEUSB_FACTORY_56K;
		break;

	case LGE_BOOT_MODE_QEM_130K:
	case LGE_BOOT_MODE_PIF_130K:
		res = LGEUSB_FACTORY_130K;
		break;

	case LGE_BOOT_MODE_QEM_910K:
	case LGE_BOOT_MODE_PIF_910K:
		res = LGEUSB_FACTORY_910K;
		break;

	default:
		res = 0;
		break;
	}

	return res;
}

struct lge_android_usb_platform_data lge_android_usb_pdata = {
	.vendor_id = 0x1004,
	.factory_pid = 0x6000,
	.iSerialNumber = 0,
	.product_name = "LGE Android Phone",
	.manufacturer_name = "LG Electronics Inc.",
	.factory_composition = "acm,diag",
	.get_factory_cable = get_factory_cable,
};

static struct platform_device lge_android_usb_device = {
	.name = "lge_android_usb",
	.id = -1,
	.dev = {
		.platform_data = &lge_android_usb_pdata,
	},
};

static int __init lge_android_usb_devices_init(void)
{
	return platform_device_register(&lge_android_usb_device);
}
arch_initcall(lge_android_usb_devices_init);
#endif

#ifdef CONFIG_LGE_USB_DIAG_LOCK
static struct platform_device lg_diag_cmd_device = {
	.name = "lg_diag_cmd",
	.id = -1,
	.dev    = {
		.platform_data = 0, /* &lg_diag_cmd_pdata */
	},
};

static int __init lge_diag_devices_init(void)
{
	return platform_device_register(&lg_diag_cmd_device);
}
arch_initcall(lge_diag_devices_init);
#endif

#ifdef CONFIG_LGE_QFPROM_INTERFACE
static struct platform_device qfprom_device = {
	.name = "lge-qfprom",
	.id = -1,
};

static int __init lge_add_qfprom_devices(void)
{
	return platform_device_register(&qfprom_device);
}

arch_initcall(lge_add_qfprom_devices);
#endif

#ifdef CONFIG_LGE_USB_G_LAF
static enum lge_laf_mode_type lge_laf_mode = LGE_LAF_MODE_NORMAL;

int __init lge_laf_mode_init(char *s)
{
	if (strcmp(s, "") && strcmp(s, "MID"))
		lge_laf_mode = LGE_LAF_MODE_LAF;

	return 1;
}
__setup("androidboot.laf=", lge_laf_mode_init);

enum lge_laf_mode_type lge_get_laf_mode(void)
{
	return lge_laf_mode;
}
#endif

static bool is_mfts_mode = 0;
static int __init lge_mfts_mode_init(char *s)
{
	if(strncmp(s,"1",1) == 0)
		is_mfts_mode = 1;
	return 0;
}
__setup("mfts.mode=", lge_mfts_mode_init);

bool lge_get_mfts_mode(void)
{
		return is_mfts_mode;
}

#ifdef CONFIG_LGE_DISPLAY_BL_DIMMING
int lge_get_bootreason_with_lcd_dimming(void)
{
	int ret = 0;

	if (lge_get_bootreason() == 0x77665560)
		ret = 1;
	else if (lge_get_bootreason() == 0x77665561)
		ret = 2;
	else if (lge_get_bootreason() == 0x77665562)
		ret = 3;
	return ret;
}
#endif

#if IS_ENABLED(CONFIG_LGE_DISPLAY_COMMON)
struct panel_lookup_entry {
	char name[64];
	enum lge_panel_type type;
};

struct panel_lookup_entry panel_lookup_table[] = {
	{"sharp_nt35596_fhd_video", PH2_SHARP},
	{"jdi_nt35596_fhd_video", PH2_JDI},
	{"lgd_incell_db7400_hd_video", PH2_LGD_DB7400},
	{"lgd_ft8707_fhd_video_1_0", JAPAN_LGD_FT8707_1_0},
	{"lgd_ft8707_fhd_video_1_1", JAPAN_LGD_FT8707_1_1},
	{"lgd_td4300_fhd_video", JAPAN_LGD_TD4300},
};

static enum lge_panel_type panel_type;
static int __init lge_set_panel_type(char *s)
{
	int i;
	int size = ARRAY_SIZE(panel_lookup_table);

	for (i = 0; i < size; ++i) {
		if (!strcmp(s, panel_lookup_table[i].name)) {
			panel_type = panel_lookup_table[i].type;
			pr_info("%s: panel type = %d\n", __func__, panel_type);
			return 1;
		}
	}
	pr_err("%s: panel name is not in lookup table: %s\n", __func__, s);

	return 1;
}

__setup("lge.panel_type=", lge_set_panel_type);

enum lge_panel_type lge_get_panel_type(void)
{
	return panel_type;
}
#endif