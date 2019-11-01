#define MODULE_TAG	"<atmf04>"

#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/input.h>
#if 0
#include <linux/wakelock.h>
#else
#include <linux/pm_wakeup.h>
#endif
#include <linux/unistd.h>
#include <linux/async.h>
#include <linux/in.h>

#include <linux/syscalls.h>
#include <linux/uaccess.h>
#include <linux/sort.h>

#include <linux/firmware.h>

#ifdef CONFIG_OF
#include <linux/regulator/consumer.h>
#include <linux/of_gpio.h>
#endif

//#include <mach/board_lge.h>
#include <soc/qcom/lge/board_lge.h>

#include "bs_log.h"

#include "atmf04.h"

#if defined (CONFIG_LGE_USE_SAR_CONTROLLER)
#define CONFIG_LGE_SAR_CONTROLLER_NEW_ALGORITHM
#define CONFIG_LGE_SAR_CONTROLLER_AUTO_CAL_ALGORITHM
#define CONFIG_LGE_SAR_CONTROLLER_IGNORE_INT_ON_PROBE
//#define CONFIG_LGE_SAR_CONTROLLER_UPDATE_SENSITIVITY
#define CONFIG_LGE_SAR_CONTROLLER_CHECK_FIRMWARE_STATUS
#endif

#if defined SAR_ATMF04_2ND
#define CONFIG_LGE_SAR_CONTROLLER_BOOTING_TIME_IMPROVEMENT
#endif

#define ATMF04_DRV_NAME     "lge_sar_rf"
#define ATMF04_DRV_NAME2     "lge_sar_rf2"

#define SAR_INPUT      "/dev/input/sar_dev"
#define SAR2_INPUT      "/dev/input/sar2_dev"

#define CNT_MAX_CH		3
#define CH1_FAR         2
#define CH2_FAR        (2<<2)
#define CH1_NEAR        1
#define CH2_NEAR       (1<<2)

/* I2C Suspend Check */
#define ATMF04_STATUS_RESUME        0
#define ATMF04_STATUS_SUSPEND       1
#define ATMF04_STATUS_QUEUE_WORK    2

/* Calibration Check */

#if defined(CONFIG_LGE_USE_SAR_CONTROLLER) // Device tuning value
// Each operator use different CS/CR duty range
#define ATMF04_CR_DUTY_LOW        300 // CR range MIN
#define ATMF04_CR_DUTY_HIGH       2900 // CR Range MAX
#define ATMF04_CS_DUTY_LOW        300 // CS Range MIN
#define ATMF04_CS_DUTY_HIGH       2900 // CS Range MAX
#define ATMF04_CSCR_RESULT          -2
#endif

/* I2C Register */
#if defined(CONFIG_LGE_SAR_CONTROLLER_NEW_ALGORITHM)
#if defined(CONFIG_LGE_ATMF04_2CH)
#define I2C_ADDR_SSTVT_H            0x01
#define I2C_ADDR_SSTVT_L            0x02
#define I2C_ADDR_SSTVT_CH2_H        0x03
#define I2C_ADDR_SSTVT_CH2_L        0x04
#define I2C_ADDR_SAFE_DUTY_CHK      0x1C
#define I2C_ADDR_SYS_CTRL           0x1F
#define I2C_ADDR_SYS_STAT           0x20
#define I2C_ADDR_CR_DUTY_H          0x26
#define I2C_ADDR_CR_DUTY_L          0x27
#define I2C_ADDR_CS_DUTY_H          0x28
#define I2C_ADDR_CS_DUTY_L          0x29
#define I2C_ADDR_CS_DUTY_CH2_H      0x2A
#define I2C_ADDR_CS_DUTY_CH2_L      0x2B
#define I2C_ADDR_PER_H              0x22
#define I2C_ADDR_PER_L              0x23
#define I2C_ADDR_PER_CH2_H          0x24
#define I2C_ADDR_PER_CH2_L          0x25
#define I2C_ADDR_TCH_OUTPUT         0x21
#define I2C_ADDR_PGM_VER_MAIN       0x71
#define I2C_ADDR_PGM_VER_SUB        0x72
#endif
#endif

#if defined(CONFIG_LGE_SAR_CONTROLLER_NEW_ALGORITHM)
//Calibration Data Backup/Restore
#define I2C_ADDR_CMD_OPT 					0x7E
#define I2C_ADDR_COMMAND 					0x7F
#define I2C_ADDR_REQ_DATA					0x80
#define CMD_R_CD_DUTY						0x04		//Cal Data Duty Read
#define CMD_R_CD_REF						0x05		//Cal Data Ref Read
#define CMD_W_CD_DUTY						0x84		//Cal Data Duty Read
#define CMD_W_CD_REF						0x85		//Cal Data Ref Read
#define SZ_CALDATA_UNIT 					24
static int CalData[6][SZ_CALDATA_UNIT];
#endif

#define BIT_PERCENT_UNIT            8.192
#define MK_INT(X, Y)                (((int)X << 8)+(int)Y)

#define ENABLE_controller_PINS          0
#define DISABLE_controller_PINS         1

#define ON_controller                   1
#define OFF_controller                  2
#define PATH_SAR_CONTROLLER_CAL  "/vendor/sns/sar_controller_cal.dat"

#define CH_1                        1
#define CH_2                        2

static struct i2c_driver atmf04_driver;
static struct workqueue_struct *atmf04_workqueue;

static unsigned char fuse_data[SZ_PAGE_DATA];

#ifdef CONFIG_OF
enum controller_dt_entry_status {
  DT_REQUIRED,
  DT_SUGGESTED,
  DT_OPTIONAL,
};

enum controller_dt_entry_type {
  DT_U32,
  DT_GPIO,
  DT_BOOL,
  DT_STRING,
};

struct controller_dt_to_pdata_map {
  const char			*dt_name;
  void				*ptr_data;
  enum controller_dt_entry_status status;
  enum controller_dt_entry_type	type;
  int				default_val;
};
#endif

static struct i2c_client
*atmf04_i2c_client; /* global i2c_client to support ioctl */

struct atmf04_platform_data {
  int (*init)(struct i2c_client *client);
  void (*exit)(struct i2c_client *client);
  unsigned int irq_gpio;
  unsigned int irq2_gpio;
  unsigned long chip_enable;
#if defined (CONFIG_LGE_SAR_CONTROLLER_CURRENT_ISSUE) || defined (CONFIG_LGE_SAR_CONTROLLER_BOOTING_TIME_IMPROVEMENT)
  unsigned long chip_enable2;
#endif
  int (*power_on)(struct i2c_client *client, bool on);
  u32 irq_gpio_flags;

  bool i2c_pull_up;

  struct regulator *vcc_ana;
  struct regulator *vcc_dig;
  struct regulator *vcc_i2c;

  u32 vdd_ana_supply_min;
  u32 vdd_ana_supply_max;
  u32 vdd_ana_load_ua;

  u32 input_pins_num; // not include ref sar controller pin
  const char *fw_name;
};

struct atmf04_data {
  int (*get_nirq_low)(void);
  struct i2c_client *client;
  struct mutex update_lock;
  struct mutex enable_lock;
  struct delayed_work	dwork;		/* for PS interrupt */
  struct input_dev *input_dev_sar;
  struct input_dev *input_dev_sar2;
#ifdef CONFIG_OF
  struct atmf04_platform_data *platform_data;
  int irq;
  int irq2;
#endif
  unsigned int enable;
  unsigned int sw_mode;
  atomic_t i2c_status;

  unsigned int sar_detection;
  int touch_out;
  atomic_t lge_sar_rf_enable;
  atomic_t lge_sar_rf2_enable;
  bool wake_irq_enabled;
  bool wake_irq2_enabled;
};

static bool on_controller = false;
static bool check_allnear = false;
static bool cal_result = false; // debugging calibration paused

#if defined(CONFIG_LGE_SAR_CONTROLLER_IGNORE_INT_ON_PROBE)
static bool probe_end_flag = false;
#endif

static int get_bit(unsigned short x, int n);
//static short get_abs(short x);
#if defined(CONFIG_LGE_SAR_CONTROLLER_CHECK_FIRMWARE_STATUS)
static void check_firmware_ready(struct i2c_client *client);
static void check_init_touch_ready(struct i2c_client *client);
#endif
s32 atmf04_i2c_smbus_write_byte_data(const struct i2c_client *client, u8 command,
                u8 value)
{
  usleep_range(50,60);
  return i2c_smbus_write_byte_data(client, command, value);
}
s32 atmf04_i2c_smbus_read_byte_data(const struct i2c_client *client, u8 command)
{
  usleep_range(50,60);
  return i2c_smbus_read_byte_data(client, command);
}
static void chg_mode(unsigned char flag, struct i2c_client *client)
{
  if(flag == ON) {
    atmf04_i2c_smbus_write_byte_data(client, ADDR_EFLA_STS, 0x80);
    PINFO("change_mode : %d\n",atmf04_i2c_smbus_read_byte_data(client, ADDR_EFLA_STS));
  }
  else {
    atmf04_i2c_smbus_write_byte_data(client, ADDR_EFLA_STS, 0x00);
    PINFO("change_mode : %d\n",atmf04_i2c_smbus_read_byte_data(client, ADDR_EFLA_STS));
  }
  mdelay(1);
}

#if defined(CONFIG_LGE_SAR_CONTROLLER_NEW_ALGORITHM)
static int Backup_CalData(struct i2c_client *client)
{
  int loop, dloop;
  int ret;

  for(loop = 0 ; loop < CNT_MAX_CH ; loop++)
  {
    ret = atmf04_i2c_smbus_write_byte_data(client,I2C_ADDR_CMD_OPT, loop);
    if (ret) {
      PINFO("atmf04: i2c_write_fail \n");
      return ret;
    }
    ret = atmf04_i2c_smbus_write_byte_data(client,I2C_ADDR_COMMAND, CMD_R_CD_DUTY);
    if (ret) {
      PINFO("atmf04: i2c_write_fail \n");
      return ret;
    }

    mdelay(1); 		//1 ms Delay

    for(dloop = 0; dloop < SZ_CALDATA_UNIT ; dloop++)
      CalData[loop][dloop] = atmf04_i2c_smbus_read_byte_data(client,I2C_ADDR_REQ_DATA + dloop);
  }

  for(loop = 0 ; loop < CNT_MAX_CH ; loop++)
  {
    ret = atmf04_i2c_smbus_write_byte_data(client,I2C_ADDR_CMD_OPT, loop);
    if (ret) {
      PINFO("atmf04: i2c_write_fail \n");
      return ret;
    }
    ret = atmf04_i2c_smbus_write_byte_data(client,I2C_ADDR_COMMAND, CMD_R_CD_REF);
    if (ret) {
      PINFO("atmf04: i2c_write_fail \n");
      return ret;
    }

    mdelay(1); 		//1 ms Delay

    for(dloop = 0; dloop < SZ_CALDATA_UNIT ; dloop++)
      CalData[CNT_MAX_CH+loop][dloop] = atmf04_i2c_smbus_read_byte_data(client,I2C_ADDR_REQ_DATA + dloop);
  }
  if(CalData[0][0] == 0xFF || (CalData[0][0] == 0x00 && CalData[0][1] == 0x00))
  {
    PINFO("atmf04: Invalid cal data, Not back up this value.");
    return -1;
  }

  //============================================================//
  //[20180327] ADS Change
  //[START]=====================================================//
  //for(loop =0;loop<3;loop++)
  for(loop =0;loop<6;loop++)
  //[END]======================================================//
  {
    for(dloop=0;dloop < SZ_CALDATA_UNIT ; dloop++)
      PINFO("atmf04: backup_caldata data[%d][%d] : %d \n",loop,dloop,CalData[loop][dloop]);
  }

  PINFO("atmf04 backup_cal success");
  return 0;
}

static int Write_CalData(struct i2c_client *client)
{
  int loop, dloop;
  int ret;

  for(loop = 0 ; loop < CNT_MAX_CH ; loop++)
  {
    ret = atmf04_i2c_smbus_write_byte_data(client,I2C_ADDR_CMD_OPT, loop);
    if (ret) {
      PINFO("atmf04: i2c_write_fail \n");
      return ret;
    }
    ret =atmf04_i2c_smbus_write_byte_data(client,I2C_ADDR_COMMAND, CMD_W_CD_DUTY);
    if (ret) {
      PINFO("atmf04: i2c_write_fail \n");
      return ret;
    }

    mdelay(1); 		//1 ms Delay

    for(dloop = 0; dloop < SZ_CALDATA_UNIT ; dloop++)
    {
      ret = atmf04_i2c_smbus_write_byte_data(client,I2C_ADDR_REQ_DATA + dloop, CalData[loop][dloop]);
      if (ret) {
        PINFO("atmf04: i2c_write_fail \n");
        return ret;
      }

    }
  }

  for(loop = 0 ; loop < CNT_MAX_CH ; loop++)
  {
    ret = atmf04_i2c_smbus_write_byte_data(client,I2C_ADDR_CMD_OPT, loop);
    if (ret) {
      PINFO("atmf04: i2c_write_fail \n");
      return ret;
    }
    ret = atmf04_i2c_smbus_write_byte_data(client,I2C_ADDR_COMMAND, CMD_W_CD_REF);
    if (ret) {
      PINFO("atmf04: i2c_write_fail \n");
      return ret;
    }

    mdelay(1); 		//1 ms Delay

    for(dloop = 0; dloop < SZ_CALDATA_UNIT ; dloop++)
    {
      ret = atmf04_i2c_smbus_write_byte_data(client,I2C_ADDR_REQ_DATA + dloop, CalData[3+loop][dloop]);
      if (ret) {
        PINFO("atmf04: i2c_write_fail \n");
        return ret;
      }

    }
  }
  return 0;
}

static int RestoreProc_CalData(struct atmf04_data *data, struct i2c_client *client)
{
  int loop;
  int ret;
  //Power On
  gpio_set_value(data->platform_data->chip_enable, 0);
  mdelay(450);


  //Calibration data write
  ret = Write_CalData(client);
  if(ret)
    return ret;

  //Initial code write
  for(loop = 0 ; loop < CNT_INITCODE ; loop++) {
    ret = atmf04_i2c_smbus_write_byte_data(client, InitCodeAddr[loop], InitCodeVal[loop]);
    if (ret) {
      PINFO("i2c_write_fail[0x%x]",InitCodeAddr[loop]);
    }
    PINFO("Restore##[0x%x][0x%x]##", InitCodeAddr[loop], atmf04_i2c_smbus_read_byte_data(client, InitCodeAddr[loop]));
  }

  //============================================================//
  //[20180327] ADS Change
  //[START]=====================================================//
  ////E-flash Data Save Command old version: 0x80, 2ch version : 0x0c
  ////atmf04_i2c_smbus_write_byte_data(client, I2C_ADDR_SYS_CTRL, 0x80);
  //atmf04_i2c_smbus_write_byte_data(client, I2C_ADDR_SYS_CTRL, 0x0c);
  //mdelay(800);		//50ms delay
  //check_firmware_ready(client);
  check_firmware_ready(client);
  atmf04_i2c_smbus_write_byte_data(client, I2C_ADDR_SYS_CTRL, 0x80);
  check_firmware_ready(client);
  //[END]======================================================//

  //Software Reset2
  atmf04_i2c_smbus_write_byte_data(client,I2C_ADDR_SYS_CTRL, 0x02);
  PINFO("atmf04 restore_cal success");
  return 0;
}

#endif

#if defined(CONFIG_LGE_SAR_CONTROLLER_UPDATE_SENSITIVITY)
static int Update_Sensitivity(struct atmf04_data *data, struct i2c_client *client)
{
  int ret = 0;
  char sstvt_h,sstvt_l;
#ifdef CONFIG_LGE_ATMF04_2CH
  char sstvt_ch2_h,sstvt_ch2_l;
#endif
  unsigned char loop;

  PINFO("Update_Sensitivity Check");

  sstvt_h = atmf04_i2c_smbus_read_byte_data(client, I2C_ADDR_SSTVT_H);
  if (ret) {
    PINFO("i2c_read_fail[0x%x]",I2C_ADDR_SSTVT_H);
    goto i2c_fail;
  }

  sstvt_l = atmf04_i2c_smbus_read_byte_data(client, I2C_ADDR_SSTVT_L);
  if (ret) {
    PINFO("i2c_read_fail[0x%x]",I2C_ADDR_SSTVT_L);
    goto i2c_fail;
  }

#ifdef CONFIG_LGE_ATMF04_2CH
  sstvt_ch2_h = atmf04_i2c_smbus_read_byte_data(client, I2C_ADDR_SSTVT_CH2_H);
  if (ret) {
    PINFO("i2c_read_fail[0x%x]",I2C_ADDR_SSTVT_CH2_H);
    goto i2c_fail;
  }

  sstvt_ch2_l = atmf04_i2c_smbus_read_byte_data(client, I2C_ADDR_SSTVT_CH2_L);
  if (ret) {
    PINFO("i2c_read_fail[0x%x]",I2C_ADDR_SSTVT_CH2_L);
    goto i2c_fail;
  }
#endif
  PINFO(" > sstvt_h : 0x%02x, sstvt_l : 0x%02x", sstvt_h, sstvt_l);

  if(sstvt_h != InitCodeVal[0] || sstvt_l != InitCodeVal[1]) {

    mdelay(200);
#if defined(CONFIG_LGE_SAR_CONTROLLER_CHECK_FIRMWARE_STATUS)
    check_firmware_ready(client);
#endif

    PINFO("Update_Sensitivity Start");
    for(loop = 0 ; loop < CNT_INITCODE ; loop++) {
      ret = atmf04_i2c_smbus_write_byte_data(client, InitCodeAddr[loop], InitCodeVal[loop]);
      if (ret) {
        PINFO("i2c_write_fail[0x%x]",InitCodeAddr[loop]);
        return ret;
      }
      PINFO("##[0x%x][0x%x]##", InitCodeAddr[loop], atmf04_i2c_smbus_read_byte_data(client, InitCodeAddr[loop]));
    }

    //E-flash Data Save Command old version: 0x80, 2ch version : 0x0c
    //ret = atmf04_i2c_smbus_write_byte_data(client, I2C_ADDR_SYS_CTRL, 0x80);
    ret = atmf04_i2c_smbus_write_byte_data(client, I2C_ADDR_SYS_CTRL, 0x04); 
    if (ret) {
      PINFO("i2c_write_fail[0x%x]",I2C_ADDR_SYS_CTRL);
      goto i2c_fail;
    }
    mdelay(50);		//50ms delay
    PINFO("Update_Sensitivity Finished");
  }

  return 0;

i2c_fail:
  return ret;
}
#endif

static unsigned char chk_done(unsigned int wait_cnt, struct i2c_client *client)
{
  unsigned int trycnt = 0;
  unsigned char rtn;

  do
  {
    if(++trycnt > wait_cnt) {
      PINFO("RTN_TIMEOUT");
      return RTN_TIMEOUT;
    }
    mdelay(1);
    rtn = atmf04_i2c_smbus_read_byte_data(client, ADDR_EFLA_STS);
  }while((rtn & FLAG_DONE) != FLAG_DONE);

  return RTN_SUCC;
}

static unsigned char chk_done_erase(unsigned int wait_cnt, struct i2c_client *client)
{
  unsigned int trycnt = 0;
  unsigned char rtn;

  do
  {
    if(++trycnt > wait_cnt) return RTN_TIMEOUT;

	//============================================================//
    //[20180327] ADS Change
    //[START]=====================================================//
    //mdelay(1);
	mdelay(5);
	//[END]======================================================//
    rtn = atmf04_i2c_smbus_read_byte_data(client, ADDR_EFLA_STS);
  }while((rtn & FLAG_DONE_ERASE) != FLAG_DONE_ERASE);

  return RTN_SUCC;
}

static unsigned char erase_eflash(struct i2c_client *client)
{
  atmf04_i2c_smbus_write_byte_data(client, ADDR_EFLA_CTRL, CMD_EFL_ERASE_ALL);

  if(chk_done_erase(FL_EFLA_TIMEOUT_CNT, client) == RTN_TIMEOUT)
    return RTN_TIMEOUT; //timeout

  return RTN_SUCC;
}

static unsigned char write_eflash_page(unsigned char flag, unsigned char * page_addr, unsigned char * wdata, struct i2c_client *client)
{
  unsigned char paddr[2];

  if(flag != FLAG_FUSE)
  {
    paddr[0] = page_addr[1];
    paddr[1] = page_addr[0];
  }
  else	//Extra User Memory
  {
    paddr[0] = 0x00;
    paddr[1] = 0x00;
  }

  if(flag != FLAG_FUSE)
  {
    atmf04_i2c_smbus_write_byte_data(client, ADDR_EFLA_CTRL, CMD_EFL_L_WR);
  }
  else
  {
    //Erase
    atmf04_i2c_smbus_write_byte_data(client, ADDR_EFLA_CTRL, CMD_EUM_ERASE);
	if(chk_done_erase(FL_EFLA_TIMEOUT_CNT, client) == RTN_TIMEOUT) return RTN_TIMEOUT;
	//Write
    atmf04_i2c_smbus_write_byte_data(client, ADDR_EFLA_CTRL, CMD_EUM_WR);
  }

  if(chk_done(FL_EFLA_TIMEOUT_CNT, client) == RTN_TIMEOUT) return RTN_TIMEOUT;

  return RTN_SUCC;
}

static unsigned char read_eflash_page(unsigned char flag, unsigned char * page_addr, unsigned char * rdata, struct i2c_client *client)
{
  unsigned char paddr[2];

  if(flag != FLAG_FUSE)
  {
    paddr[0] = page_addr[1];
    paddr[1] = page_addr[0];
  }
  else	//Extra User Memory
  {
    paddr[0] = 0x00;
    paddr[1] = 0x00;
  }

  if(flag != FLAG_FUSE)
    atmf04_i2c_smbus_write_byte_data(client, ADDR_EFLA_CTRL, CMD_EFL_RD);
  else
    atmf04_i2c_smbus_write_byte_data(client, ADDR_EFLA_CTRL, CMD_EUM_RD);

  if(chk_done(FL_EFLA_TIMEOUT_CNT, client) == RTN_TIMEOUT) return RTN_TIMEOUT;

  return RTN_SUCC;
}

static void onoff_controller(struct atmf04_data *data, int onoff_mode, int ch_num)
{
  int nparse_mode;

  nparse_mode = onoff_mode;
  PINFO("onoff_controller: nparse_mode [%d], ch_num:%d",nparse_mode, ch_num);

  if (nparse_mode == ENABLE_controller_PINS) {
#if 1 // fixed bug to support sar controller HAL
    if(ch_num == CH_1)
    {
      input_report_abs(data->input_dev_sar, ABS_DISTANCE, 3);/* INITIALIZING INPUT EVENT */
      input_sync(data->input_dev_sar);
    }
    else if(ch_num == CH_2)
    {
      input_report_abs(data->input_dev_sar2, ABS_DISTANCE, 3);/* INITIALIZING INPUT EVENT */
      input_sync(data->input_dev_sar2);

    }
#endif
    gpio_set_value(data->platform_data->chip_enable, 0);    /*chip_en pin - low : on, high : off*/
    mdelay(500);
    if (!on_controller)
    {
      if(ch_num == CH_1)
      {
        enable_irq_wake(data->irq);
        data->wake_irq_enabled = true;
        PINFO("enable irq ch1");
      }
      else if(ch_num == CH_2)
      {
        enable_irq_wake(data->irq2);
        data->wake_irq2_enabled = true;
        PINFO("enable irq ch2");
      }
    }
    on_controller = true;
#if 0 // fixed bug to support sar controller HAL
    if (gpio_get_value(data->platform_data->irq_gpio)) {
      input_report_abs(data->input_dev_sar, ABS_DISTANCE, 1);/* force FAR detection */
      input_sync(data->input_dev_sar);
    }
#endif
  }
  if (nparse_mode == DISABLE_controller_PINS) {
    gpio_set_value(data->platform_data->chip_enable, 1);    /*chip_en pin - low : on, high : off*/
    if (on_controller)
    {
      if(ch_num == CH_1)
      {
        if(data->wake_irq_enabled)
        {
          disable_irq_wake(data->irq);
          PINFO("disable irq ch1");
        }
      }
      else if(ch_num == CH_2)
      {
        if(data->wake_irq_enabled)
        {
          disable_irq_wake(data->irq2);
          PINFO("disable irq ch2");
        }
      }
    }
    on_controller = false;
  }
  PINFO("get_gpio value(0:on/1:off) :[%s]",gpio_get_value(data->platform_data->chip_enable) == 0 ? "0:on" : "1:off");


}

static unsigned char load_firmware(struct atmf04_data *data, struct i2c_client *client, const char *name)
{
  const struct firmware *fw = NULL;
  unsigned char rtn;
  int ret, i, count = 0;
  int max_page;
  unsigned short main_version, sub_version;
  unsigned char page_addr[2];
  unsigned char fw_version, ic_fw_version, page_num;
  int version_addr;
  int chip_id = -1;
#if defined (CONFIG_LGE_SAR_CONTROLLER_NEW_ALGORITHM)
  int restore = 0;
#endif
  //============================================================//
  //[20180320] ADS Add
  //[START]=====================================================//
  unsigned char sys_status = 1;
  //[END]=======================================================//

  PINFO("Load Firmware Entered!!");

  //============================================================//
  //[20180320] ADS Change
  //[START]=====================================================//
  //gpio_set_value(data->platform_data->chip_enable, 0);
  gpio_set_value(data->platform_data->chip_enable, 1);
  mdelay(100);
  gpio_set_value(data->platform_data->chip_enable, 0);
  //[END]=======================================================//

  ret = request_firmware(&fw, name, &data->client->dev);
  if (ret) {
    PINFO("Unable to open bin [%s]  ret %d", name, ret);
    if (fw)
      release_firmware(fw);
    return 1;
  } else {
    PINFO("Open bin [%s] ret : %d ", name, ret);
  }

  max_page = (fw->size)/SZ_PAGE_DATA;
  version_addr = (fw->size)-SZ_PAGE_DATA;
  fw_version = MK_INT(fw->data[version_addr], fw->data[version_addr+1]);
  page_num = fw->data[version_addr+3];
  PINFO("###########fw version : %d.%d, fw_version : %d, page_num : %d###########", fw->data[version_addr], fw->data[version_addr+1], fw_version, page_num);

  //============================================================//
  //[20180320] ADS Change
  //[START]=====================================================//
  //mdelay(800);
  //check_firmware_ready(client); //just for test
  mdelay(10);
  //[END]=======================================================//

  //check IC as reading register 0x00
  PINFO("SAR controller ATMF04 chip_id read check start");
  chip_id= atmf04_i2c_smbus_read_byte_data(client, 0x00);
  for(i=0 ; i < IC_TIMEOUT_CNT; i++)
  {
    if(chip_id >=  0)
    {
      PINFO("SAR controller ATMF04 chip_id:0x%x , cnt:%d", chip_id, i);
      break;
    }
    chip_id= atmf04_i2c_smbus_read_byte_data(client, 0x00);
    if( i == IC_TIMEOUT_CNT-1)
    {
      PINFO("SAR controller ATMF04 chip_id read fail ");
      if (fw)
        release_firmware(fw);
      return 1;
    }
  }
  //check IC as reading register 0x00

  main_version = atmf04_i2c_smbus_read_byte_data(client, I2C_ADDR_PGM_VER_MAIN);
  sub_version = atmf04_i2c_smbus_read_byte_data(client, I2C_ADDR_PGM_VER_SUB);
  ic_fw_version = MK_INT(main_version, sub_version);
  PINFO("###########ic version : %d.%d, ic_fw_version : %d###########", main_version, sub_version, ic_fw_version);

  //============================================================//
  //[20180320] ADS Change
  //[START]=====================================================//
  //if (fw_version > ic_fw_version || fw->data[version_addr] > main_version) {
  if( (fw->data[version_addr] != main_version) || (fw->data[version_addr+1] != sub_version)) {
  //[END]=======================================================//

    //============================================================//
    //[20180320] ADS Add
    //[START]=====================================================//
    mdelay(300);
    check_firmware_ready(client); //just for test
    sys_status = atmf04_i2c_smbus_read_byte_data(client, I2C_ADDR_SYS_STAT);
    //[END]=======================================================//


    //mdelay(200);
#if defined(CONFIG_LGE_SAR_CONTROLLER_NEW_ALGORITHM)
    //============================================================//
    //[20180320] ADS Change
    //[START]=====================================================//
    /*if (ic_fw_version == 0){
      restore = 0;
    }
    else {
      if (Backup_CalData(client) < 0){
        restore = 0;
      }
      else {
        restore = 1;
      }
    }*/
    if ((ic_fw_version == 0) || ((sys_status & 0x06) == 0)) {
      restore = 0;
    }
    else {
      if (Backup_CalData(client) < 0){
        restore = 0;
      }
      else {
        restore = 1;
      }
    }
	//[END]=======================================================//
#endif
    /* IC Download Mode Change */
    chg_mode(ON, client);

    /* fuse data process */
    page_addr[0] = 0x00;
    page_addr[1] = 0x00;

    rtn = read_eflash_page(FLAG_FUSE, page_addr, fuse_data, client);
    if (rtn != RTN_SUCC) {
      PERR("read eflash page fail!");
      if (fw)
        release_firmware(fw);
      return rtn;		/* fuse read fail */
    }

    fuse_data[51] |= 0x80;

    rtn = write_eflash_page(FLAG_FUSE, page_addr, fuse_data, client);
    if (rtn != RTN_SUCC) {
      PERR("write eflash page fail!");
      if (fw)
        release_firmware(fw);
      return rtn;		/* fuse write fail */
    }

    /* firmware write process */
    rtn = erase_eflash(client);
    if(rtn != RTN_SUCC) {
      PINFO("earse fail\n");
      if (fw)
        release_firmware(fw);
      return rtn;		//earse fail
    }

    while(count < page_num) {
      //PINFO("%d\n",count);
      for(i=0; i < SZ_PAGE_DATA; i++) {
        atmf04_i2c_smbus_write_byte_data(client, i, fw->data[i + (count*SZ_PAGE_DATA)]);
        //PINFO("%d : %x ",i + (count*SZ_PAGE_DATA),fw->data[i + (count*SZ_PAGE_DATA)]);
      }
      //PINFO("\n");
      page_addr[1] = (unsigned char)((count & 0xFF00) >> 8);
      page_addr[0] = (unsigned char)(count & 0x00FF);

      atmf04_i2c_smbus_write_byte_data(client, ADDR_EFLA_PAGE_L, page_addr[0]);
      atmf04_i2c_smbus_write_byte_data(client, ADDR_EFLA_PAGE_H, page_addr[1]);

      /*Eflash write command 0xFC -> 0x01 Write*/
      atmf04_i2c_smbus_write_byte_data(client, ADDR_EFLA_CTRL, CMD_EFL_L_WR);

      if(chk_done(FL_EFLA_TIMEOUT_CNT, client) == RTN_TIMEOUT)
      {
        if (fw)
          release_firmware(fw);
        return RTN_TIMEOUT;
      }

      count++;
    }
  }
  else {
    PINFO("Not update firmware. Firmware version is lower than ic.(Or same version)");
  }
  chg_mode(OFF, client);
  //Debug ****************
  mdelay(300);
  main_version = atmf04_i2c_smbus_read_byte_data(client, I2C_ADDR_PGM_VER_MAIN);
  sub_version = atmf04_i2c_smbus_read_byte_data(client, I2C_ADDR_PGM_VER_SUB);
  ic_fw_version = MK_INT(main_version, sub_version);
  PINFO("Read ---> ic version : %d.%d, ic_fw_version : %d", main_version, sub_version, ic_fw_version);
  // **********************

#if defined(CONFIG_LGE_SAR_CONTROLLER_UPDATE_SENSITIVITY)
  Update_Sensitivity(data, client);
#endif

  gpio_set_value(data->platform_data->chip_enable, 1);

#if defined(CONFIG_LGE_SAR_CONTROLLER_NEW_ALGORITHM)
  mdelay(10);
  if(restore)
  {
    ret = RestoreProc_CalData(data, client);
    if(ret)
      PINFO("restore fail ret: %d",ret);
  }
#endif
  PINFO("disable ok");

  release_firmware(fw);

  return 0;
}

static int write_initcode(struct i2c_client *client)
{
  struct atmf04_data *data = i2c_get_clientdata(client);
  unsigned char loop;
  int en_state;
  int ret = 0;

  PINFO("write_initcode entered[%d]",data->platform_data->irq_gpio);

  en_state = gpio_get_value(data->platform_data->chip_enable);

  if (en_state)
    gpio_set_value(data->platform_data->chip_enable, 0);
#ifdef CONFIG_MACH_MSM8916_E7IILTE_SPR_US
  mdelay(350);
#else
  mdelay(450);
#endif

#if defined(CONFIG_LGE_SAR_CONTROLLER_CHECK_FIRMWARE_STATUS)
  check_firmware_ready(client);
#endif

  for(loop = 0 ; loop < CNT_INITCODE ; loop++) {
    ret = atmf04_i2c_smbus_write_byte_data(client, InitCodeAddr[loop], InitCodeVal[loop]);
    if (ret) {
      PINFO("i2c_write_fail[0x%x]",InitCodeAddr[loop]);
      return ret;
    }
    PINFO("##[0x%x][0x%x]##", InitCodeAddr[loop], atmf04_i2c_smbus_read_byte_data(client, InitCodeAddr[loop]));
  }

#if defined(CONFIG_LGE_SAR_CONTROLLER_UPDATE_SENSITIVITY)
  for(loop = 0 ; loop < 2 ; loop++) {
    ret = atmf04_i2c_smbus_write_byte_data(client, InitCodeAddr[loop], InitCodeVal[loop+8]); // write sensing percent value to autocal range
    if (ret) {
      PINFO("i2c_write_fail[0x%x]",InitCodeAddr[loop]);
      return ret;
    }
    PINFO("##FORCE SET[0x%x][0x%x]##", InitCodeAddr[loop], atmf04_i2c_smbus_read_byte_data(client, InitCodeAddr[loop]));
  }
#endif
  return 0;
}

static bool valid_multiple_input_pins(struct atmf04_data *data)
{
  if (data->platform_data->input_pins_num > 1)
    return true;

  return false;

}

static int write_calibration_data(struct atmf04_data *data, char *filename)
{
#if 1
  int fd = 0;

  char file_result[2]; // debugging calibration paused
  mm_segment_t old_fs = get_fs();

  PINFO("write_calibration_data Entered[%d]",data->platform_data->irq_gpio);
  set_fs(KERNEL_DS);
  fd = sys_open(filename, O_WRONLY|O_CREAT, 0664);

  if (fd >= 0) {
#if 1 // debugging calibration paused
    if(cal_result) {
      file_result[0] = SAR_CAL_RESULT_PASS;
      file_result[1] = '\0';
      sys_write(fd, file_result, 1);
    } else {
      strncpy(file_result, SAR_CAL_RESULT_FAIL, strlen(SAR_CAL_RESULT_FAIL));
      file_result[1] = '\0';
      sys_write(fd, file_result, 1);
    }
    PINFO("%s: write [%s] to %s", __FUNCTION__, file_result, filename);
    sys_close(fd);
    set_fs(old_fs);
#else
    sys_write(fd,0,sizeof(int));
    sys_close(fd);
#endif
  } else {
    PINFO("%s: %s open failed [%d]......", __FUNCTION__, filename, fd);
  }

  //PINFO("sys open to save cal.dat");
#endif

  return 0;
}

#if defined(CONFIG_LGE_SAR_CONTROLLER_CHECK_FIRMWARE_STATUS)
static void check_firmware_ready(struct i2c_client *client)
{
  unsigned char sys_status = 1;
  int i;

  sys_status = atmf04_i2c_smbus_read_byte_data(client, I2C_ADDR_SYS_STAT);
  for(i = 0 ; i < 100 ; i++) {
    if (get_bit(sys_status, 0) == 1) {
      PINFO("%s: Firmware is busy now.....[%d] sys_status = [0x%x]", __FUNCTION__, i, sys_status);
      mdelay(10);
      sys_status = atmf04_i2c_smbus_read_byte_data(client, I2C_ADDR_SYS_STAT);
    } else {
      break;
    }
  }
  PINFO("%s: sys_status = [0x%x]", __FUNCTION__, sys_status);

  return;

}

static void check_init_touch_ready(struct i2c_client *client)
{
  unsigned char init_touch_md_check = 0;
  int i;

  init_touch_md_check = atmf04_i2c_smbus_read_byte_data(client, I2C_ADDR_SYS_STAT);
  for(i = 0 ; i < 50 ; i++) {
    if ((get_bit(init_touch_md_check, 2) == 0) && (get_bit(init_touch_md_check, 1) == 0)) {
      PINFO("%s: Firmware init touch is not yet ready.....[%d] sys_statue = [0x%x]", __FUNCTION__, i, init_touch_md_check);
      mdelay(10);
      init_touch_md_check = atmf04_i2c_smbus_read_byte_data(client, I2C_ADDR_SYS_STAT);
    } else {
      break;
    }
  }
  PINFO("%s: sys_status = [0x%x]", __FUNCTION__, init_touch_md_check);

  return;

}
#endif
static ssize_t atmf04_show_reg(struct device *dev,
    struct device_attribute *attr, char *buf)
{
  struct i2c_client *client = to_i2c_client(dev);
  struct atmf04_data *data = i2c_get_clientdata(client);
  int loop;

  int ret = 0;
  client = data->client;

  for (loop = 0; loop < CNT_INITCODE; loop++) {
    PINFO("###### [0x%x][0x%x]###", InitCodeAddr[loop], atmf04_i2c_smbus_read_byte_data(client, InitCodeAddr[loop]));
  }
  return ret;
}

static ssize_t atmf04_store_read_reg(struct device *dev,
    struct device_attribute *attr, const char *buf, size_t count)
{
  struct i2c_client *client = to_i2c_client(dev);
  struct atmf04_data *data = i2c_get_clientdata(client);
  int loop;
  int ret = 0;
  unsigned long val;
  bool flag_match = false;

  client = data->client;

  PINFO("input :%s", buf);

  if (kstrtoul(buf, 0, &val))
    return -EINVAL;

  for (loop = 0; loop < CNT_INITCODE; loop++) {
    PINFO("###### loop:%d [0x%x][0x%x]###", loop,InitCodeAddr[loop], atmf04_i2c_smbus_read_byte_data(client, InitCodeAddr[loop]));
    if( val ==  InitCodeAddr[loop])
    {
      PINFO("match Addr :%s", buf);
      flag_match = true;
      break;
    }
  }

  if(flag_match)
  {
    ret = atmf04_i2c_smbus_read_byte_data(client, InitCodeAddr[loop]); 
    PINFO("read register [0x%x][0x%x]",InitCodeAddr[loop],atmf04_i2c_smbus_read_byte_data(client, InitCodeAddr[loop]));
  }
  else
  {
      PINFO("match Addr fail");
  }


  return count;
}

static ssize_t atmf04_store_write_reg(struct device *dev,
    struct device_attribute *attr, const char *buf, size_t count)
{
  struct i2c_client *client = to_i2c_client(dev);
  struct atmf04_data *data = i2c_get_clientdata(client);
  unsigned char loop;
  bool flag_match = false;
  unsigned int addr, val;
  client = data->client;

  PINFO("input :%s", buf);

  if (sscanf(buf, "%x %x", &addr, &val) <= 0)
    return count;

  for (loop = 0; loop < CNT_INITCODE; loop++) {
    PINFO("###### [0x%x][0x%x]###", InitCodeAddr[loop], atmf04_i2c_smbus_read_byte_data(client, InitCodeAddr[loop]));
    if( addr ==  InitCodeAddr[loop])
    {
      PINFO("match Addr :%s", buf);
      flag_match = true;
      break;
    }
  }



  if(flag_match)
  {
    atmf04_i2c_smbus_write_byte_data(client, InitCodeAddr[loop], val);
    PINFO("write register ##[0x%x][0x%x]##", InitCodeAddr[loop], val);
    atmf04_i2c_smbus_write_byte_data(client, I2C_ADDR_SYS_CTRL, 0x04);
    mdelay(50);		//50ms delay
    atmf04_i2c_smbus_write_byte_data(client, I2C_ADDR_SYS_CTRL, 0x01);
    // return sprintf(buf,"0x%02x\n",atmf04_i2c_smbus_read_byte_data(client, InitCodeAddr[loop]));
    return count;
  }
  else
    return count;
}

static ssize_t atmf04_show_regproxctrl0(struct device *dev,
    struct device_attribute *attr, char *buf)
{
  PINFO("atmf04_show_regproxctrl0: %d\n",on_controller);
  if(on_controller==true)
    return sprintf(buf,"0x%02x\n",0x0C);
  return sprintf(buf,"0x%02x\n",0x00);
}

static ssize_t atmf04_store_reg(struct device *dev,
    struct device_attribute *attr, const char *buf, size_t count)
{
  struct i2c_client *client = to_i2c_client(dev);
  struct atmf04_data *data = i2c_get_clientdata(client);
  unsigned char loop;
  client = data->client;

  for (loop = 0; loop < CNT_INITCODE; loop++) {
    atmf04_i2c_smbus_write_byte_data(client, InitCodeAddr[loop], InitCodeVal[loop]);
    PINFO("##[0x%x][0x%x]##", InitCodeAddr[loop], InitCodeVal[loop]);
  }
  atmf04_i2c_smbus_write_byte_data(client, I2C_ADDR_SYS_CTRL, 0x01);
  return count;
}

static ssize_t atmf04_show_proxstatus(struct device *dev,
    struct device_attribute *attr, char *buf)
{
  int ret ;
  struct atmf04_data *data = dev_get_drvdata(dev);
  struct input_dev *input_dev = to_input_dev(dev);

  PINFO("proxstatus name :%s", input_dev->name);

  if(strcmp(input_dev->name, ATMF04_DRV_NAME) == 0)
  {
    ret = gpio_get_value(data->platform_data->irq_gpio);
  }
  else
  {
    ret = gpio_get_value(data->platform_data->irq2_gpio);
  }
  /////
  return sprintf(buf, "%d\n", ret);
}


static ssize_t atmf04_store_onoffcontroller(struct device *dev,
    struct device_attribute *attr,
    const char *buf, size_t count)
{
  struct atmf04_data *data = dev_get_drvdata(dev);
  struct input_dev *input_dev = to_input_dev(dev);
  unsigned long val;
  int first_value;
  int irq_state, irq2_state;
  struct i2c_client *client = to_i2c_client(dev);

  client = data->client;

  if (kstrtoul(buf, 0, &val))
    return -EINVAL;

  PINFO("Store ON/OFF_controller name :%s", input_dev->name);

  if(strcmp(input_dev->name, ATMF04_DRV_NAME) == 0)
  {
    PINFO("Store ON/OFF_controller name :ATMF04_DRV_NAME");
    if(val == ON_controller) {
      mutex_lock(&data->enable_lock);
      if(atomic_read(&data->lge_sar_rf_enable) == 1 || atomic_read(&data->lge_sar_rf2_enable) == 1)
      {
        PINFO("already enabled! ");
        input_report_abs(data->input_dev_sar, ABS_DISTANCE, 3);/* INITIALIZING INPUT EVENT */
        input_sync(data->input_dev_sar);

        irq_state = gpio_get_value(data->platform_data->irq_gpio);
        first_value = atmf04_i2c_smbus_read_byte_data(client, I2C_ADDR_TCH_OUTPUT);
        PINFO("touch_out(%d), irq_state[%d]", first_value, irq_state);
        if(first_value < 0)
        {
          PINFO("ATMF04 DEFECT !!!");
          mutex_unlock(&data->enable_lock);
          return -EINVAL;
        }

        if(irq_state == 0 && ((first_value & CH1_FAR) == CH1_FAR)){ // far
          input_report_abs(data->input_dev_sar, ABS_DISTANCE, 1);/* NEAR-to-FAR detection */
          input_sync(data->input_dev_sar);
          PINFO("[%d]CH_1 FAR ",data->platform_data->irq_gpio);
        }
        else
        {
          input_report_abs(data->input_dev_sar, ABS_DISTANCE, 0);/* FAR-to-NEAR detection */
          input_sync(data->input_dev_sar);
          PINFO("[%d]CH_1 NEAR ",data->platform_data->irq_gpio);
        }

        enable_irq_wake(data->irq);
        data->wake_irq_enabled = true;
        PINFO("enable irq ch1");
        atomic_set(&data->lge_sar_rf_enable, 1);

        mutex_unlock(&data->enable_lock);
        return count;
      }
      atomic_set(&data->lge_sar_rf_enable, 1);
      onoff_controller(data,ENABLE_controller_PINS, CH_1);
      first_value = atmf04_i2c_smbus_read_byte_data(client, I2C_ADDR_TCH_OUTPUT);
      PINFO("touch_out(%d)", first_value);
      if(first_value < 0)
      {
        PINFO("ATMF04 DEFECT !!!");
        mutex_unlock(&data->enable_lock);
        return -EINVAL;
      }
      mutex_unlock(&data->enable_lock);
    }
    else if (val == OFF_controller) {
      mutex_lock(&data->enable_lock);
      if(atomic_read(&data->lge_sar_rf_enable) == 0 && atomic_read(&data->lge_sar_rf2_enable) == 0)
      {
        PINFO("already disabled! ");

        if(data->wake_irq_enabled)
        {
          disable_irq_wake(data->irq);
          PINFO("disable irq ch1");
        }

        atomic_set(&data->lge_sar_rf_enable, 0);
        mutex_unlock(&data->enable_lock);
        return count;
      }
      if(atomic_read(&data->lge_sar_rf2_enable) == 0)
      {
        onoff_controller(data,DISABLE_controller_PINS, CH_1);
      }
      atomic_set(&data->lge_sar_rf_enable, 0);
      mutex_unlock(&data->enable_lock);
    }
  }
  else if(strcmp(input_dev->name, ATMF04_DRV_NAME2) == 0)
  {
    PINFO("Store ON/OFF_controller name :ATMF04_DRV_NAME2");
    if(val == ON_controller) {
      mutex_lock(&data->enable_lock);
      if(atomic_read(&data->lge_sar_rf_enable) == 1 || atomic_read(&data->lge_sar_rf2_enable) == 1)
      {
        PINFO("already enabled! ");
        input_report_abs(data->input_dev_sar2, ABS_DISTANCE, 3);/* INITIALIZING INPUT EVENT */
        input_sync(data->input_dev_sar2);

        irq2_state = gpio_get_value(data->platform_data->irq2_gpio);
        first_value = atmf04_i2c_smbus_read_byte_data(client, I2C_ADDR_TCH_OUTPUT);

        PINFO("touch_out(%d), irq2_state[%d]", first_value, irq2_state);

        if(first_value < 0)
        {
          PINFO("ATMF04 DEFECT !!!");
          mutex_unlock(&data->enable_lock);
          return -EINVAL;
        }

        if(irq2_state == 0 && ((first_value & CH2_FAR) == CH2_FAR)){ // far

          input_report_abs(data->input_dev_sar2, ABS_DISTANCE, 1);/* NEAR-to-FAR detection */
          input_sync(data->input_dev_sar2);
          PINFO("[%d]CH_2 FAR ",data->platform_data->irq2_gpio);
        }
        else
        {
          input_report_abs(data->input_dev_sar2, ABS_DISTANCE, 0);/* FAR-to-NEAR detection */
          input_sync(data->input_dev_sar2);
          PINFO("[%d]CH_2 NEAR ",data->platform_data->irq2_gpio);
        }

        enable_irq_wake(data->irq2);
        data->wake_irq2_enabled = true;
        PINFO("enable irq ch2");

        atomic_set(&data->lge_sar_rf2_enable, 1);
        mutex_unlock(&data->enable_lock);
        return count;
      }
      atomic_set(&data->lge_sar_rf2_enable, 1);
      onoff_controller(data,ENABLE_controller_PINS, CH_2);
      first_value = atmf04_i2c_smbus_read_byte_data(client, I2C_ADDR_TCH_OUTPUT);
      PINFO("touch_out(%d)", first_value);
      if(first_value < 0)
      {
        PINFO("ATMF04 DEFECT !!!");
        mutex_unlock(&data->enable_lock);
        return -EINVAL;
      }
      mutex_unlock(&data->enable_lock);
    }
    else if (val == OFF_controller) {
      mutex_lock(&data->enable_lock);
      if(atomic_read(&data->lge_sar_rf_enable) == 0 && atomic_read(&data->lge_sar_rf2_enable) == 0)
      {
        PINFO("already disabled! ");

        if(data->wake_irq2_enabled)
        {
          disable_irq_wake(data->irq2);
          PINFO("disable irq ch2");
        }

        atomic_set(&data->lge_sar_rf2_enable, 0);
        mutex_unlock(&data->enable_lock);
        return count;
      }
      if(atomic_read(&data->lge_sar_rf_enable) == 0)
      {
        onoff_controller(data,DISABLE_controller_PINS, CH_2);
      }
      atomic_set(&data->lge_sar_rf2_enable, 0);
      mutex_unlock(&data->enable_lock);
    }
  }
  return count;
}

static ssize_t atmf04_store_sar_hard(struct device *dev,
    struct device_attribute *attr,
    const char *buf, size_t count)
{
  struct atmf04_data *data = dev_get_drvdata(dev);
  struct input_dev *input_dev = to_input_dev(dev);
  unsigned long val;

  if (kstrtoul(buf, 0, &val))
    return -EINVAL;

  PINFO("Store ON/OFF_controller name :%s", input_dev->name);

  if(strcmp(input_dev->name, ATMF04_DRV_NAME) == 0)
  {
    PINFO("Store hard name :ATMF04_DRV_NAME");
    if(val == 0 ) {
      PINFO("set near! ");
      input_report_abs(data->input_dev_sar, ABS_DISTANCE, 0);/* FAR-to-NEAR detection */
      input_sync(data->input_dev_sar);
    }
    else if (val == 1) {
      PINFO("set far ");
      input_report_abs(data->input_dev_sar, ABS_DISTANCE, 1);/* NEAR-to-FAR detection */
      input_sync(data->input_dev_sar);
    }
  }
  else if(strcmp(input_dev->name, ATMF04_DRV_NAME2) == 0)
  {
    PINFO("Store ON/OFF_controller name :ATMF04_DRV_NAME2");
    if(val == 0 ) {
      PINFO("set near! ");
      input_report_abs(data->input_dev_sar2, ABS_DISTANCE, 0);/* FAR-to-NEAR detection */
      input_sync(data->input_dev_sar2);
    }
    else if (val == 1) {
      PINFO("set far ");
      input_report_abs(data->input_dev_sar2, ABS_DISTANCE, 1);/* NEAR-to-FAR detection */
      input_sync(data->input_dev_sar2);
    }
  }
  return count;
}
static ssize_t atmf04_show_docalibration(struct device *dev,
    struct device_attribute *attr, char *buf)
{
  struct i2c_client *client = to_i2c_client(dev);
  struct atmf04_data *data = i2c_get_clientdata(client);
  unsigned char  safe_duty;
  client = data->client;

  /* check safe duty for validation of cal*/
  safe_duty = atmf04_i2c_smbus_read_byte_data(client, I2C_ADDR_SAFE_DUTY_CHK);

  return sprintf(buf, "%d\n", get_bit(safe_duty, 7));
}

static ssize_t atmf04_store_docalibration(struct device *dev,
    struct device_attribute *attr, const char *buf, size_t count)
{
  struct i2c_client *client = to_i2c_client(dev);
  struct atmf04_data *data = i2c_get_clientdata(client);
  int ret, init_state;
  unsigned char  safe_duty;
  client = data->client;

  PINFO("atmf04_store_docalibration[%d]",data->platform_data->irq_gpio);
  init_state = write_initcode(client);
  PINFO("write_initcode End[%d]",data->platform_data->irq_gpio);

#if 1 // debugging calibration paused
  if(init_state) {
    PINFO("%s: write_initcode result is failed.....\n", __FUNCTION__);
    cal_result = false;
    write_calibration_data(data, PATH_SAR_CONTROLLER_CAL);
    return count;
  } else {
    PINFO("%s: write_initcode result is successful..... continue next step!!!\n", __FUNCTION__);
  }
#else
  if(init_state)
    return count;
#endif

  mutex_lock(&data->update_lock);
  ret = atmf04_i2c_smbus_write_byte_data(client, I2C_ADDR_SYS_CTRL, 0x0C);
  mutex_unlock(&data->update_lock);
  if(ret)
    PINFO("i2c_write_fail\n");

  mdelay(800);
  check_firmware_ready(client);

  safe_duty = atmf04_i2c_smbus_read_byte_data(client, I2C_ADDR_SAFE_DUTY_CHK);

  PINFO("[%u]safe_duty : %d", data->platform_data->irq_gpio, get_bit(safe_duty, 7));

#if 1 // debugging calibration paused
  if(ret || (get_bit(safe_duty, 7) != 1)) {
    // There is i2c failure or safe duty bit is not 1
    PINFO("%s: calibration result is failed.....\n", __FUNCTION__);
    cal_result = false;
  } else {
    PINFO("%s: calibration result is successful!!!!!\n", __FUNCTION__);
    cal_result = true;
  }
#endif

  write_calibration_data(data, PATH_SAR_CONTROLLER_CAL);

  return count;
}

static ssize_t atmf04_store_regreset(struct device *dev,
    struct device_attribute *attr,
    const char *buf, size_t count)
{
  struct i2c_client *client = to_i2c_client(dev);
  struct atmf04_data *data = i2c_get_clientdata(client);
  short tmp;
  short cs_per[2], cs_per_result;
  short cr_duty[2], cs_duty[2], cr_duty_val, cs_duty_val;
#ifdef CONFIG_LGE_ATMF04_2CH	
  short cs_per_ch2[2], cs_per_result_ch2;
  short cs_duty_ch2[2], cs_duty_val_ch2;
#endif
  int ret;
  client = data->client;

#if defined(CONFIG_LGE_SAR_CONTROLLER_CHECK_FIRMWARE_STATUS)
  check_firmware_ready(client);
#endif

#if 1 // debugging calibration paused
  // Whether cal is pass or fail, make it cal_result true to check raw data/CS/CR/count in bypass mode of AAT
  cal_result = true;
  write_calibration_data(data, PATH_SAR_CONTROLLER_CAL);
#endif

  ret = atmf04_i2c_smbus_write_byte_data(client, I2C_ADDR_SYS_CTRL, 0x02);
  if(ret)
    PINFO("[%d]i2c_write_fail\n",data-> platform_data->irq_gpio);

  // CR Duty value 
  cr_duty[1] = atmf04_i2c_smbus_read_byte_data(client, I2C_ADDR_CR_DUTY_H);
  cr_duty[0] = atmf04_i2c_smbus_read_byte_data(client, I2C_ADDR_CR_DUTY_L);
  cr_duty_val = MK_INT(cr_duty[1], cr_duty[0]);

  // Ch1 Per value
  cs_per[0] = atmf04_i2c_smbus_read_byte_data(client,I2C_ADDR_PER_H);
  cs_per[1] = atmf04_i2c_smbus_read_byte_data(client,I2C_ADDR_PER_L);
  tmp = MK_INT(cs_per[0], cs_per[1]);
  cs_per_result = tmp / 8;    // BIT_PERCENT_UNIT;

  // Ch1 CS Duty value
  cs_duty[1] = atmf04_i2c_smbus_read_byte_data(client, I2C_ADDR_CS_DUTY_H);
  cs_duty[0] = atmf04_i2c_smbus_read_byte_data(client, I2C_ADDR_CS_DUTY_L);
  cs_duty_val = MK_INT(cs_duty[1], cs_duty[0]);

  PINFO("[%d] Result(ch1): %2d %6d %6d", data-> platform_data->irq_gpio, cs_per_result, cr_duty_val, cs_duty_val);

#ifdef CONFIG_LGE_ATMF04_2CH
  // Ch2 Per value
  cs_per_ch2[0] = atmf04_i2c_smbus_read_byte_data(client,I2C_ADDR_PER_CH2_H);
  cs_per_ch2[1] = atmf04_i2c_smbus_read_byte_data(client,I2C_ADDR_PER_CH2_L);
  tmp = MK_INT(cs_per_ch2[0], cs_per_ch2[1]);
  cs_per_result_ch2 = tmp / 8;    // BIT_PERCENT_UNIT;

  // Ch2 CS Duty value
  cs_duty_ch2[1] = atmf04_i2c_smbus_read_byte_data(client, I2C_ADDR_CS_DUTY_CH2_H);
  cs_duty_ch2[0] = atmf04_i2c_smbus_read_byte_data(client, I2C_ADDR_CS_DUTY_CH2_L);
  cs_duty_val_ch2 = MK_INT(cs_duty_ch2[1], cs_duty_ch2[0]);

  PINFO("[%d] Result(ch2): %2d %6d %6d", data-> platform_data->irq_gpio, cs_per_result_ch2, cr_duty_val, cs_duty_val_ch2);
#endif

  return count;
}

static int get_bit(unsigned short x, int n) {
  return (x & (1 << n)) >> n;
}
/*
static short get_abs(short x) {
  return ((x >= 0) ? x : -x);
}
*/
static ssize_t atmf04_show_regproxdata(struct device *dev,
    struct device_attribute *attr, char *buf)
{
  struct i2c_client *client = to_i2c_client(dev);
  struct atmf04_data *data = i2c_get_clientdata(client);
  short cs_per[2], cs_per_result;
  short cr_duty[2], cs_duty[2], cr_duty_val, cs_duty_val;
#ifdef CONFIG_LGE_ATMF04_2CH
  short cs_per_ch2[2], cs_per_result_ch2;
  short cs_duty_ch2[2], cs_duty_val_ch2;
  short cap_value_ch2;
#endif
  short tmp, cap_value;
  int nlength = 0;
  char buf_regproxdata[256] = "";
  char buf_line[256] = "";
  unsigned char init_touch_md;
#if defined(CONFIG_LGE_SAR_CONTROLLER_AUTO_CAL_ALGORITHM)	 /*auto calibration FW*/
  int check_mode;
#endif
  client = data->client;
  memset(buf_line, 0, sizeof(buf_line));
  memset(buf_regproxdata, 0, sizeof(buf_regproxdata));

  init_touch_md = atmf04_i2c_smbus_read_byte_data(client, I2C_ADDR_SYS_STAT);

  cs_per[0] = atmf04_i2c_smbus_read_byte_data(client,I2C_ADDR_PER_H);
  cs_per[1] = atmf04_i2c_smbus_read_byte_data(client,I2C_ADDR_PER_L);
  tmp = MK_INT(cs_per[0], cs_per[1]);
  cs_per_result = tmp / 8;    // BIT_PERCENT_UNIT;

  cr_duty[1] = atmf04_i2c_smbus_read_byte_data(client, I2C_ADDR_CR_DUTY_H);
  cr_duty[0] = atmf04_i2c_smbus_read_byte_data(client, I2C_ADDR_CR_DUTY_L);
  cr_duty_val = MK_INT(cr_duty[1], cr_duty[0]);

  cs_duty[1] = atmf04_i2c_smbus_read_byte_data(client, I2C_ADDR_CS_DUTY_H);
  cs_duty[0] = atmf04_i2c_smbus_read_byte_data(client, I2C_ADDR_CS_DUTY_L);
  cs_duty_val = MK_INT(cs_duty[1], cs_duty[0]);

  cap_value = (int)cs_duty_val * (int)cs_per_result;

#ifdef CONFIG_LGE_ATMF04_2CH
  // Ch2 Per value
  cs_per_ch2[0] = atmf04_i2c_smbus_read_byte_data(client,I2C_ADDR_PER_CH2_H);
  cs_per_ch2[1] = atmf04_i2c_smbus_read_byte_data(client,I2C_ADDR_PER_CH2_L);
  tmp = MK_INT(cs_per_ch2[0], cs_per_ch2[1]);
  cs_per_result_ch2 = tmp / 8;    // BIT_PERCENT_UNIT;

  // Ch2 CS Duty value
  cs_duty_ch2[1] = atmf04_i2c_smbus_read_byte_data(client, I2C_ADDR_CS_DUTY_CH2_H);
  cs_duty_ch2[0] = atmf04_i2c_smbus_read_byte_data(client, I2C_ADDR_CS_DUTY_CH2_L);
  cs_duty_val_ch2 = MK_INT(cs_duty_ch2[1], cs_duty_ch2[0]);

  cap_value_ch2 = (int)cs_duty_val_ch2 * (int)cs_per_result_ch2;


#if defined(CONFIG_LGE_SAR_CONTROLLER_AUTO_CAL_ALGORITHM)	/*auto calibration FW*/
  check_mode = get_bit(init_touch_md, 2);

  PINFO("Enable CH2 check_mode[%d]", check_mode);

  /* Normal Mode */
  if (check_mode) {
    PINFO("Enable CH2 check_mode[%d]", check_mode);
    sprintf(buf_line, "[R]%6d %6d %6d %6d %6d %6d %6d %6d\n",
        get_bit(init_touch_md,2), cs_per_result, cs_per_result_ch2, cr_duty_val, cs_duty_val, cs_duty_val_ch2, cap_value, cap_value_ch2);
  }
  /* Init Touch Mode */
  else {
#endif
    PINFO("Enable CH2 check_mode[%d]", check_mode);
    sprintf(buf_line, "[R]%6d %6d %6d %6d %6d %6d %6d %6d\n", 
        get_bit(init_touch_md,1), cs_per_result, cs_per_result_ch2, cr_duty_val, cs_duty_val, cs_duty_val_ch2, cap_value, cap_value_ch2);
  }
#else
  // printk("H: %x L:%x H:%x L:%x\n",cr_duty[1] ,cr_duty[0], cs_duty[1], cs_duty[0]);
#if defined(CONFIG_LGE_SAR_CONTROLLER_AUTO_CAL_ALGORITHM)	/*auto calibration FW*/
  check_mode = get_bit(init_touch_md, 2);

  if (check_mode)		/* Normal Mode */
    sprintf(buf_line, "[R]%6d %6d %6d %6d %6d\n", get_bit(init_touch_md,2), cs_per_result, cr_duty_val, cs_duty_val, cap_value);
  else		/* Init Touch Mode */
#endif
    sprintf(buf_line, "[R]%6d %6d %6d %6d %6d\n", get_bit(init_touch_md,1), cs_per_result, cr_duty_val, cs_duty_val, cap_value);
#endif
  nlength = strlen(buf_regproxdata);
  strcpy(&buf_regproxdata[nlength], buf_line);

  return sprintf(buf, "%s", buf_regproxdata);
}

static ssize_t atmf04_store_checkallnear(struct device *dev,
    struct device_attribute *attr,
    const char *buf, size_t count)
{
  unsigned long val;

  if (kstrtoul(buf, 0, &val))
    return -EINVAL;

  if (val == 0)
    check_allnear = false;
  else if (val == 1)
    check_allnear = true;

  printk("atmf04_store_checkallnear %d\n",check_allnear);
  return count;
}

static ssize_t atmf04_show_count_inputpins(struct device *dev,
    struct device_attribute *attr, char *buf)
{
  int count_inputpins = 0;

  struct atmf04_data *data = dev_get_drvdata(dev);

  count_inputpins = data->platform_data->input_pins_num;
  if (count_inputpins > 1) {
    if (valid_multiple_input_pins(data) == false)
      count_inputpins = 1;
  }
  return sprintf(buf, "%d\n", count_inputpins);
}

static ssize_t atmf04_store_firmware(struct device *dev,
    struct device_attribute *attr,
    const char *buf, size_t count)
{
  const char *fw_name = NULL;
  struct atmf04_data *data = dev_get_drvdata(dev);
  struct i2c_client *client = to_i2c_client(dev);
  client = data->client;

  fw_name = data->platform_data->fw_name;
  load_firmware(data, client, fw_name);
  return count;
}

static ssize_t atmf04_show_version(struct device *dev,
    struct device_attribute *attr, char *buf)
{
  unsigned short main_version, sub_version;
  unsigned char ic_fw_version;
  struct i2c_client *client = to_i2c_client(dev);
  struct atmf04_data *data = dev_get_drvdata(dev);
  char buf_line[64] = "";
  int nlength = 0;
  char buf_regproxdata[256] = "";
  client = data->client;

  memset(buf_line, 0, sizeof(buf_line));
  onoff_controller(data,ENABLE_controller_PINS, CH_1);

  mdelay(200);

  main_version = atmf04_i2c_smbus_read_byte_data(client, I2C_ADDR_PGM_VER_MAIN);
  sub_version = atmf04_i2c_smbus_read_byte_data(client, I2C_ADDR_PGM_VER_SUB);
  ic_fw_version = MK_INT(main_version, sub_version);
  PINFO("###########ic version : %d.%d, ic_fw_version : %d###########", main_version, sub_version, ic_fw_version);

  sprintf(buf_line, "%d.%d\n",main_version, sub_version);
  nlength = strlen(buf_regproxdata);
  strcpy(&buf_regproxdata[nlength], buf_line);

  return sprintf(buf,"%s", buf_regproxdata);
}

static ssize_t atmf04_show_check_far(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct atmf04_data *data = i2c_get_clientdata(client);
	unsigned char init_touch_md_check;
	short tmp, cs_per[2], cs_per_result;
	short cr_duty[2], cs_duty[2], cr_duty_val, cs1_duty_val, cs2_duty_val;
	int bit_mask = 1; //bit for reading of I2C_ADDR_SYS_STAT
	unsigned int check_result = 0;
	int poor_contact = 0;

    client = data->client;

	mutex_lock(&data->update_lock);

#if defined(CONFIG_LGE_SAR_CONTROLLER_CHECK_FIRMWARE_STATUS)
	check_init_touch_ready(client);
#endif

	cs_per[0] = atmf04_i2c_smbus_read_byte_data(client,I2C_ADDR_PER_H);
	cs_per[1] = atmf04_i2c_smbus_read_byte_data(client,I2C_ADDR_PER_L);
	tmp = MK_INT(cs_per[0], cs_per[1]);
	cs_per_result = tmp / 8;    // BIT_PERCENT_UNIT;

	cr_duty[1] = atmf04_i2c_smbus_read_byte_data(client, I2C_ADDR_CR_DUTY_H);
	cr_duty[0] = atmf04_i2c_smbus_read_byte_data(client, I2C_ADDR_CR_DUTY_L);
	cr_duty_val = MK_INT(cr_duty[1], cr_duty[0]);

	cs_duty[1] = atmf04_i2c_smbus_read_byte_data(client, I2C_ADDR_CS_DUTY_H);
	cs_duty[0] = atmf04_i2c_smbus_read_byte_data(client, I2C_ADDR_CS_DUTY_L);
	cs1_duty_val = MK_INT(cs_duty[1], cs_duty[0]);

    cs_duty[1] = atmf04_i2c_smbus_read_byte_data(client, I2C_ADDR_CS_DUTY_CH2_H);
	cs_duty[0] = atmf04_i2c_smbus_read_byte_data(client, I2C_ADDR_CS_DUTY_CH2_L);
	cs2_duty_val = MK_INT(cs_duty[1], cs_duty[0]);

	init_touch_md_check = i2c_smbus_read_byte_data(client, I2C_ADDR_SYS_STAT);
#if defined(CONFIG_LGE_SAR_CONTROLLER_AUTO_CAL_ALGORITHM) /*auto calibration FW*/
	if(get_bit(init_touch_md_check, 2))
		bit_mask = 2;  /* Normal Mode */
	else
		bit_mask = 1;  /* Init Touch Mode */
#endif

/* Check poor contact */
    if (get_bit(init_touch_md_check, 4) == 1)
	{
		poor_contact = 1;
		check_result += 1;
	}

/* Calibration Check */
    if (get_bit(init_touch_md_check, bit_mask) != 1)
       check_result += 1;

/* CR Duty Range Check */
    if ((cr_duty_val < ATMF04_CR_DUTY_LOW) || (cr_duty_val > ATMF04_CR_DUTY_HIGH))
       check_result += 1;

/* CS Duty Rnage Check */
    if ((cs1_duty_val < ATMF04_CS_DUTY_LOW) || (cs1_duty_val > ATMF04_CS_DUTY_HIGH) || (cs2_duty_val < ATMF04_CS_DUTY_LOW) || (cs1_duty_val > ATMF04_CS_DUTY_HIGH))
       check_result += 1;

  /* IRQ status Check (High-Near/Low-Far) */
	if((gpio_get_value(data->platform_data->irq_gpio) == 1) || (gpio_get_value(data->platform_data->irq2_gpio) == 1))
	{
		PINFO("Check_Far IRQ Status IRQ1[%d]",gpio_get_value(data->platform_data->irq_gpio));
        PINFO("Check_Far IRQ Status IRQ2[%d]",gpio_get_value(data->platform_data->irq2_gpio));
		check_result += 1;
	}

    if (check_result != 0)
    {
		PINFO("[fail] 1.cal: %d, cr: %d, cs_ch1: %d, cs_ch2: %d, poor_contact: %d",
                get_bit(init_touch_md_check, bit_mask), cr_duty_val, cs1_duty_val, cs2_duty_val, poor_contact);
		mutex_unlock(&data->update_lock);
		return sprintf(buf,"%d",0);
    }
    else
    {
		PINFO("[PASS] 2.cal: %d, cr: %d, cs_ch1: %d, cs_ch2: %d, poor_contact: %d",
                get_bit(init_touch_md_check, bit_mask), cr_duty_val, cs1_duty_val, cs2_duty_val, poor_contact);
		mutex_unlock(&data->update_lock);
		return sprintf(buf,"%d",1);
    }
}

static ssize_t atmf04_show_check_mid(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct atmf04_data *data = i2c_get_clientdata(client);
	unsigned char init_touch_md_check;
	short tmp, cs_per[2], cs_per_result;
	short cr_duty[2], cs_duty[2], cr_duty_val, cs1_duty_val, cs2_duty_val;
	int bit_mask = 1; //bit for reading of I2C_ADDR_SYS_STAT
    unsigned int check_result = 0;
	int poor_contact = 0;

    client = data->client;

	mutex_lock(&data->update_lock);

#if defined(CONFIG_LGE_SAR_CONTROLLER_CHECK_FIRMWARE_STATUS)
	check_init_touch_ready(client);
#endif

	cs_per[0] = atmf04_i2c_smbus_read_byte_data(client,I2C_ADDR_PER_H);
	cs_per[1] = atmf04_i2c_smbus_read_byte_data(client,I2C_ADDR_PER_L);
	tmp = MK_INT(cs_per[0], cs_per[1]);
	cs_per_result = tmp / 8;    // BIT_PERCENT_UNIT;

	cr_duty[1] = atmf04_i2c_smbus_read_byte_data(client, I2C_ADDR_CR_DUTY_H);
	cr_duty[0] = atmf04_i2c_smbus_read_byte_data(client, I2C_ADDR_CR_DUTY_L);
	cr_duty_val = MK_INT(cr_duty[1], cr_duty[0]);

	cs_duty[1] = atmf04_i2c_smbus_read_byte_data(client, I2C_ADDR_CS_DUTY_H);
	cs_duty[0] = atmf04_i2c_smbus_read_byte_data(client, I2C_ADDR_CS_DUTY_L);
	cs1_duty_val = MK_INT(cs_duty[1], cs_duty[0]);

    cs_duty[1] = atmf04_i2c_smbus_read_byte_data(client, I2C_ADDR_CS_DUTY_CH2_H);
	cs_duty[0] = atmf04_i2c_smbus_read_byte_data(client, I2C_ADDR_CS_DUTY_CH2_L);
	cs2_duty_val = MK_INT(cs_duty[1], cs_duty[0]);

	init_touch_md_check = i2c_smbus_read_byte_data(client, I2C_ADDR_SYS_STAT);
#if defined(CONFIG_LGE_SAR_CONTROLLER_AUTO_CAL_ALGORITHM) /*auto calibration FW*/
	if(get_bit(init_touch_md_check, 2))
		bit_mask = 2;  /* Normal Mode */
	else
		bit_mask = 1;  /* Init Touch Mode */
#endif

/* Calibration Check */
    if (get_bit(init_touch_md_check, bit_mask) != 1)
    {
       check_result += 1;
    }
/* CR Duty Range Check */
    if ((cr_duty_val < ATMF04_CR_DUTY_LOW) || (cr_duty_val > ATMF04_CR_DUTY_HIGH))
    {
       check_result += 1;
    }
/* CS Duty Rnage Check */
    if ((cs1_duty_val < ATMF04_CS_DUTY_LOW) || (cs1_duty_val > ATMF04_CS_DUTY_HIGH) || (cs2_duty_val < ATMF04_CS_DUTY_LOW) || (cs1_duty_val > ATMF04_CS_DUTY_HIGH))
    {
       check_result += 1;
    }

/* Check poor contact */
    if (get_bit(init_touch_md_check, 4) == 1)
	{
		poor_contact = 1;
		check_result += 1;
	}

    if (check_result != 0)
    {
		PINFO("[fail] 1.cal: %d, cr: %d, cs_ch1: %d, cs_ch2: %d, poor_contact: %d",
                get_bit(init_touch_md_check, bit_mask), cr_duty_val, cs1_duty_val, cs2_duty_val, poor_contact);
		mutex_unlock(&data->update_lock);
		return sprintf(buf,"%d",0);
    }
    else
    {
		PINFO("[PASS] 2.cal: %d, cr: %d, cs_ch1: %d, cs_ch2: %d, poor_contact: %d",
                get_bit(init_touch_md_check, bit_mask), cr_duty_val, cs1_duty_val, cs2_duty_val, poor_contact);
		mutex_unlock(&data->update_lock);
		return sprintf(buf,"%d",1);
    }

}

static DEVICE_ATTR(onoff,        0664, NULL, atmf04_store_onoffcontroller);
static DEVICE_ATTR(proxstatus,   0664, atmf04_show_proxstatus, NULL);
static DEVICE_ATTR(docalibration,0664, atmf04_show_docalibration, atmf04_store_docalibration);
static DEVICE_ATTR(reg_ctrl,     0664, atmf04_show_reg, atmf04_store_reg);
static DEVICE_ATTR(write_reg,     0664, NULL, atmf04_store_write_reg);
static DEVICE_ATTR(read_reg,     0664, NULL, atmf04_store_read_reg);
static DEVICE_ATTR(regproxdata,  0664, atmf04_show_regproxdata, NULL);
static DEVICE_ATTR(regreset,     0664, NULL, atmf04_store_regreset);
static DEVICE_ATTR(checkallnear, 0664, NULL, atmf04_store_checkallnear);
static DEVICE_ATTR(cntinputpins, 0664, atmf04_show_count_inputpins, NULL);
static DEVICE_ATTR(regproxctrl0, 0664, atmf04_show_regproxctrl0, NULL);
static DEVICE_ATTR(download,     0664, NULL, atmf04_store_firmware);
static DEVICE_ATTR(version,      0664, atmf04_show_version, NULL);
static DEVICE_ATTR(check_far,    0664, atmf04_show_check_far, NULL);
static DEVICE_ATTR(check_mid,    0664, atmf04_show_check_mid, NULL);
static DEVICE_ATTR(sar_hard,   0664, NULL, atmf04_store_sar_hard);


static struct attribute *atmf04_attributes[] = {
  &dev_attr_onoff.attr,
  &dev_attr_docalibration.attr,
  &dev_attr_proxstatus.attr,
  &dev_attr_reg_ctrl.attr,
  &dev_attr_write_reg.attr,
  &dev_attr_read_reg.attr,
  &dev_attr_regproxdata.attr,
  &dev_attr_regreset.attr,
  &dev_attr_checkallnear.attr,
  &dev_attr_cntinputpins.attr,
  &dev_attr_regproxctrl0.attr,
  &dev_attr_download.attr,
  &dev_attr_version.attr,
  &dev_attr_check_far.attr,
  &dev_attr_check_mid.attr,
  &dev_attr_sar_hard.attr,
  NULL,
};

static struct attribute_group atmf04_attr_group = {
  .attrs = atmf04_attributes,
};

static void atmf04_reschedule_work(struct atmf04_data *data,
    unsigned long delay)
{
  int ret;
  struct i2c_client *client = data->client;
  /*
   * If work is already scheduled then subsequent schedules will not
   * change the scheduled time that's why we have to cancel it first.
   */
  if (gpio_get_value(data->platform_data->chip_enable) == 0) { // if power on
    dev_err(&client->dev, "atmf04_reschedule_work : set wake lock timeout!\n");
    //wake_lock_timeout(&data->ps_wlock, msecs_to_jiffies(1500));
    pm_wakeup_event(&client->dev,1500);
    //cancel_delayed_work(&data->dwork);
    ret = queue_delayed_work(atmf04_workqueue, &data->dwork, delay);
    if (ret < 0) {
      PINFO("queue_work fail, ret = %d", ret);
    }
  } else {
    PINFO("sar controller enable pin is already high... power off status");
  }
}

static irqreturn_t atmf04_interrupt(int vec, void *info)
{
  struct i2c_client *client = (struct i2c_client *)info;
  struct atmf04_data *data = i2c_get_clientdata(client);
  int tmp = -1;
  tmp = atomic_read(&data->i2c_status);

  if (gpio_get_value(data->platform_data->chip_enable) == 0) { // if power on
    dev_err(&client->dev,"atmf04_interrupt\n");
    atmf04_reschedule_work(data, 0);
  }

  return IRQ_HANDLED;
}

static void atmf04_work_handler(struct work_struct *work)
{
  struct atmf04_data *data = container_of(work, struct atmf04_data, dwork.work);
  struct i2c_client *client = data->client;
  int irq_state, irq2_state;
  short cs_per[2];
  short cs_per_result1 = 0;
  short cs_per_result2 = 0;
  short tmp = 0;


/* add checking irq to detect NEAR or FAR */
#if defined(CONFIG_LGE_SAR_CONTROLLER_IGNORE_INT_ON_PROBE)
  if(probe_end_flag == true) {
#endif
    data->touch_out = atmf04_i2c_smbus_read_byte_data(client, I2C_ADDR_TCH_OUTPUT);

    irq_state = gpio_get_value(data->platform_data->irq_gpio);
    PINFO("touch_out[%d] irq_state[%d]", data->touch_out, irq_state);

    irq2_state = gpio_get_value(data->platform_data->irq2_gpio);
    PINFO("touch_out[%d] irq2_state[%d]", data->touch_out, irq2_state);

    cs_per[0] = atmf04_i2c_smbus_read_byte_data(client,I2C_ADDR_PER_H);
    cs_per[1] = atmf04_i2c_smbus_read_byte_data(client,I2C_ADDR_PER_L);
    tmp = MK_INT(cs_per[0], cs_per[1]);
    cs_per_result1 = tmp / 8;    // BIT_PERCENT_UNIT;

    mdelay(5);

    cs_per[0] = atmf04_i2c_smbus_read_byte_data(client,I2C_ADDR_PER_CH2_H);
    cs_per[1] = atmf04_i2c_smbus_read_byte_data(client,I2C_ADDR_PER_CH2_L);
    tmp = MK_INT(cs_per[0], cs_per[1]);
    cs_per_result2 = tmp / 8;    // BIT_PERCENT_UNIT;
    PINFO("capacitance percent ch1[%d] ch2[%d]", cs_per_result1, cs_per_result2);

    /* When I2C fail */
    if (data->touch_out <= 0)
    {
      PINFO("I2C Error[%d]", data->touch_out);
      input_report_abs(data->input_dev_sar, ABS_DISTANCE, 0);/* FAR-to-NEAR detection */
      input_sync(data->input_dev_sar);
      PINFO("[%d]NEAR ",data->platform_data->irq_gpio);

      input_report_abs(data->input_dev_sar2, ABS_DISTANCE, 0);/* FAR-to-NEAR detection */
      input_sync(data->input_dev_sar2);
      PINFO("[%d]NEAR ",data->platform_data->irq2_gpio);
      return;
    }

    if (gpio_get_value(data->platform_data->chip_enable) == 1) { // if power off
      PINFO("sar controller is already power off : touch_out(%d)", data->touch_out);
      return;
    }

    if(atomic_read(&data->lge_sar_rf_enable) == 1) // CH_1 enable
    {
      if(irq_state == 0 && ((data->touch_out & CH1_FAR) == CH1_FAR)){ // far 
        data->sar_detection = 0;

        input_report_abs(data->input_dev_sar, ABS_DISTANCE, 1);/* NEAR-to-FAR detection */
        input_sync(data->input_dev_sar);
        PINFO("[%d]CH_1 FAR ",data->platform_data->irq_gpio);
      }
      else
      {
        data->sar_detection = 1;
        input_report_abs(data->input_dev_sar, ABS_DISTANCE, 0);/* FAR-to-NEAR detection */
        input_sync(data->input_dev_sar);
        PINFO("[%d]CH_1 NEAR ",data->platform_data->irq_gpio);
      }
    }
    if(atomic_read(&data->lge_sar_rf2_enable) == 1) // CH_2 enable
    {
      if(irq2_state == 0 && ((data->touch_out & CH2_FAR) == CH2_FAR)){ // far 
        data->sar_detection = 0;
        input_report_abs(data->input_dev_sar2, ABS_DISTANCE, 1);/* NEAR-to-FAR detection */
        input_sync(data->input_dev_sar2);
        PINFO("[%d]CH_2 FAR ",data->platform_data->irq2_gpio);
      }
      else
      {
        data->sar_detection = 1;
        input_report_abs(data->input_dev_sar2, ABS_DISTANCE, 0);/* FAR-to-NEAR detection */
        input_sync(data->input_dev_sar2);
        PINFO("[%d]CH_2 NEAR ",data->platform_data->irq2_gpio);
      }
    }
#if defined(CONFIG_LGE_SAR_CONTROLLER_IGNORE_INT_ON_PROBE)
  }
  else{
    PINFO("probe_end_flag = False[%d]",probe_end_flag);
  }
#endif

}

static int reg_set_optimum_mode_check(struct regulator *reg, int load_uA)
{
  return (regulator_count_voltages(reg) > 0) ?
    regulator_set_optimum_mode(reg, load_uA) : 0;
}

static int controller_regulator_configure(struct atmf04_data *data, bool on)
{
  struct i2c_client *client = data->client;
  struct atmf04_platform_data *pdata = data->platform_data;
  int rc;

  if (on == false)
    goto hw_shutdown;

  pdata->vcc_ana = regulator_get(&client->dev, "Adsemicon,vdd_ana");
  if (IS_ERR(pdata->vcc_ana)) {
    rc = PTR_ERR(pdata->vcc_ana);
    dev_err(&client->dev,
        "Regulator get failed vcc_ana rc=%d\n", rc);
    return rc;
  }

  if (regulator_count_voltages(pdata->vcc_ana) > 0) {
    rc = regulator_set_voltage(pdata->vcc_ana, pdata->vdd_ana_supply_min,
        pdata->vdd_ana_supply_max);

    if (rc) {
      dev_err(&client->dev,
          "regulator set_vtg failed rc=%d\n", rc);
      goto error_set_vtg_vcc_ana;
    }
  }

  return 0;

error_set_vtg_vcc_ana:
  regulator_put(pdata->vcc_ana);
  return rc;

hw_shutdown:
  if (regulator_count_voltages(pdata->vcc_ana) > 0)
    regulator_set_voltage(pdata->vcc_ana, 0, pdata->vdd_ana_supply_max);

  regulator_put(pdata->vcc_ana);
  regulator_put(pdata->vcc_dig);

  if (pdata->i2c_pull_up) {
    regulator_put(pdata->vcc_i2c);
  }
  return 0;
}

static int controller_regulator_power_on(struct atmf04_data *data, bool on)
{
  struct i2c_client *client = data->client;
  struct atmf04_platform_data *pdata = data->platform_data;

  int rc;

  if (on == false)
    goto power_off;

  rc = reg_set_optimum_mode_check(pdata->vcc_ana, pdata->vdd_ana_load_ua);
  if (rc < 0) {
    dev_err(&client->dev,
        "Regulator vcc_ana set_opt failed rc=%d\n", rc);
    return rc;
  }

  rc = regulator_enable(pdata->vcc_ana);
  if (rc) {
    dev_err(&client->dev,
        "Regulator vcc_ana enable failed rc=%d\n", rc);
  }
  return 0;


power_off:
  reg_set_optimum_mode_check(pdata->vcc_ana, 0);
  regulator_disable(pdata->vcc_ana);
  if (pdata->i2c_pull_up) {
      reg_set_optimum_mode_check(pdata->vcc_i2c, 0);
      regulator_disable(pdata->vcc_i2c);
  }
  return 0;
}

static int controller_platform_hw_power_on(struct i2c_client *client, bool on)
{
  controller_regulator_power_on(i2c_get_clientdata(client), on);
  return 0;
}

static int controller_platform_hw_init(struct i2c_client *client)
{
  struct atmf04_data *data = i2c_get_clientdata(client);
  int error;

  error = controller_regulator_configure(data, true);

  //	PINFO("HW_REV : %d", lge_get_board_revno());

  error = gpio_request(data->platform_data->chip_enable, "atmf04_chip_enable");
  if(error) {
    PINFO("chip_enable request fail\n");
  }
  gpio_direction_output(data->platform_data->chip_enable, 0); // First, set chip_enable gpio direction to output.

  PINFO("gpio direction output ok\n");

#if defined (CONFIG_LGE_SAR_CONTROLLER_BOOTING_TIME_IMPROVEMENT)
  error = gpio_request(data->platform_data->chip_enable2, "atmf04_chip_enable2");
  if(error) {
    PINFO("chip_enable2 request fail\n");
  }
  gpio_direction_output(data->platform_data->chip_enable2, 0); // enable first, to reduce booting time.
  PINFO("set chip_enable2 power on succesfully\n");
  gpio_free(data->platform_data->chip_enable2);
  PINFO("set chip_enable2 free\n");
#endif

  if (gpio_is_valid(data->platform_data->irq_gpio)) {
    /* configure touchscreen irq gpio */
    error = gpio_request(data->platform_data->irq_gpio, "atmf04_irq_gpio");
    if (error) {
      dev_err(&client->dev, "unable to request gpio [%d]\n",
          data->platform_data->irq_gpio);
    }
    error = gpio_direction_input(data->platform_data->irq_gpio);
    if (error) {
      dev_err(&client->dev,
          "unable to set direction for gpio [%d]\n",
          data->platform_data->irq_gpio);
    }
    data->irq = client->irq = gpio_to_irq(data->platform_data->irq_gpio);
  } else {
    dev_err(&client->dev, "irq gpio not provided\n");
  }

#if 1
  if (gpio_is_valid(data->platform_data->irq2_gpio)) {
    /* configure touchscreen irq gpio */
    error = gpio_request(data->platform_data->irq2_gpio, "atmf04_irq2_gpio");
    if (error) {
      dev_err(&client->dev, "unable to request gpio [%d]\n",
          data->platform_data->irq2_gpio);
    }
    error = gpio_direction_input(data->platform_data->irq2_gpio);
    if (error) {
      dev_err(&client->dev,
          "unable to set direction for gpio [%d]\n",
          data->platform_data->irq2_gpio);
    }
    data->irq2 = gpio_to_irq(data->platform_data->irq2_gpio);
  } else {
    dev_err(&client->dev, "irq2 gpio not provided\n");
  }
#endif
  PINFO("controller_platform_hw_init entered\n");
  return 0;
}

static void controller_platform_hw_exit(struct i2c_client *client)
{
  struct atmf04_data *data = i2c_get_clientdata(client);;

  controller_regulator_configure(data, false);

  if (gpio_is_valid(data->platform_data->irq_gpio))
    gpio_free(data->platform_data->irq_gpio);

#if 1
  if (gpio_is_valid(data->platform_data->irq2_gpio))
    gpio_free(data->platform_data->irq2_gpio);
#endif
  PINFO("controller_platform_hw_exit entered\n");
}

static int controller_parse_dt(struct device *dev,
    struct atmf04_platform_data *pdata)
{
  struct device_node *np = dev->of_node;

  int ret, err = 0;
  struct controller_dt_to_pdata_map *itr;
  struct controller_dt_to_pdata_map map[] = {
    {"Adsemicon,irq-gpio",		&pdata->irq_gpio,		DT_REQUIRED,	DT_GPIO,	0},
    {"Adsemicon,irq2-gpio",		&pdata->irq2_gpio,		DT_REQUIRED,	DT_GPIO,	0},
    {"Adsemicon,vdd_ana_supply_min",	&pdata->vdd_ana_supply_min,	DT_SUGGESTED,	DT_U32,		0},
    {"Adsemicon,vdd_ana_supply_max",	&pdata->vdd_ana_supply_max,	DT_SUGGESTED,	DT_U32,		0},
    {"Adsemicon,vdd_ana_load_ua",	&pdata->vdd_ana_load_ua,	DT_SUGGESTED,	DT_U32,		0},
    {"Adsemicon,chip_enable",   &pdata->chip_enable,    DT_SUGGESTED,   DT_GPIO,     0},
    {"Adsemicon,InputPinsNum",         &pdata->input_pins_num,      DT_SUGGESTED,   DT_U32,  0},
    {"Adsemicon,fw_name",              &pdata->fw_name,             DT_SUGGESTED,   DT_STRING,  0},
#if defined (CONFIG_LGE_SAR_CONTROLLER_CURRENT_ISSUE) || defined (CONFIG_LGE_SAR_CONTROLLER_BOOTING_TIME_IMPROVEMENT)
    {"Adsemicon,chip_enable2",   &pdata->chip_enable2,    DT_SUGGESTED,   DT_GPIO,     0},
#endif
    /* add */
    {NULL,				NULL,				0,		0,		0},
  };

  for (itr = map; itr->dt_name ; ++itr) {
    switch (itr->type) {
      case DT_GPIO:
        ret = of_get_named_gpio(np, itr->dt_name, 0);
        if (ret >= 0) {
          *((int *) itr->ptr_data) = ret;
          ret = 0;
        }
        break;
      case DT_U32:
        ret = of_property_read_u32(np, itr->dt_name,
            (u32 *) itr->ptr_data);
        break;
      case DT_BOOL:
        *((bool *) itr->ptr_data) =
          of_property_read_bool(np, itr->dt_name);
        ret = 0;
        break;
      case DT_STRING:
        ret = of_property_read_string(np, itr->dt_name, itr->ptr_data);
        break;
      default:
        PINFO("%d is an unknown DT entry type\n",
            itr->type);
        ret = -EBADE;
    }

    PINFO("DT entry ret:%d name:%s val:%d\n",
        ret, itr->dt_name, *((int *)itr->ptr_data));

    if (ret) {
      *((int *)itr->ptr_data) = itr->default_val;

      if (itr->status < DT_OPTIONAL) {
        PINFO("Missing '%s' DT entry\n",
            itr->dt_name);

        /* cont on err to dump all missing entries */
        if (itr->status == DT_REQUIRED && !err)
          err = ret;
      }
    }
  }

  /* set functions of platform data */
  pdata->init = controller_platform_hw_init;
  pdata->exit = controller_platform_hw_exit;
  pdata->power_on = controller_platform_hw_power_on;
  /*pdata->ppcount = 12;	//no need to set, dt_parse */

  return err;

  return 0;
}

static int atmf04_probe(struct i2c_client *client,
    const struct i2c_device_id *id)
{
  struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
  struct atmf04_data *data;
#ifdef CONFIG_OF
  struct atmf04_platform_data *platform_data;
#endif
  int err = 0;
#if defined(CONFIG_MACH_SDM845_JUDYLN) && defined(CONFIG_LGE_ONE_BINARY_SKU)
  int sku_value = -1;
#endif

  if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE)) {
    return -EIO;
  }

  data = devm_kzalloc(&client->dev, sizeof(struct atmf04_data), GFP_KERNEL);
  if (!data) {
    return -ENOMEM;
  }
  PINFO("ATMF04 probe, kzalloc complete\n");

#ifdef CONFIG_OF
  if (client->dev.of_node) {
    platform_data = devm_kzalloc(&client->dev,
        sizeof(struct atmf04_platform_data), GFP_KERNEL);
    if (!platform_data) {
      dev_err(&client->dev, "Failed to allocate memory\n");
      return -ENOMEM;
    }
    data->platform_data = platform_data;
    client->dev.platform_data = platform_data;
    err = controller_parse_dt(&client->dev, platform_data);
    if (err)
      return err;

  } else {
    platform_data = client->dev.platform_data;
  }
#endif
#if defined(CONFIG_MACH_SDM845_JUDYLN) && defined(CONFIG_LGE_ONE_BINARY_SKU)
  ///Check SKU///
  sku_value = lge_get_sku_carrier();
  PINFO("ATMF04 probe, sku_value: %d\n", sku_value);
  if(sku_value == HW_SKU_GLOBAL)
  {
    PINFO("ATMF04 probe, change InitCodeVal_SKU\n");
    memcpy(InitCodeVal, InitCodeVal_SKU, sizeof(InitCodeVal));
  }
#endif
  data->client = client;
  atmf04_i2c_client = client;
  i2c_set_clientdata(client, data);
  data->sar_detection = 0;

#ifdef CONFIG_OF
  /* h/w initialization */
  if (platform_data->init)
    err = platform_data->init(client);

  if (platform_data->power_on)
    err = platform_data->power_on(client, true);
#endif
  PINFO("**SAR controller BLSP[%d]",platform_data->irq_gpio);

  client->adapter->retries = 15;

  if (client->adapter->retries == 0)
    goto exit;

  atomic_set(&data->i2c_status, ATMF04_STATUS_RESUME);

  atomic_set(&data->lge_sar_rf_enable, 0);
  atomic_set(&data->lge_sar_rf2_enable, 0);

  mutex_init(&data->update_lock);
  mutex_init(&data->enable_lock);

  device_init_wakeup(&client->dev, true);
  INIT_DELAYED_WORK(&data->dwork, atmf04_work_handler);

  if(request_irq(client->irq, atmf04_interrupt, IRQ_TYPE_EDGE_FALLING|IRQ_TYPE_EDGE_RISING,
        ATMF04_DRV_NAME, (void *)client )) {
    PINFO("Could not allocate ATMF04_INT %d !\n", data->irq);
    goto exit_irq_init_failed;
  }

  if(request_irq(data->irq2, atmf04_interrupt, IRQ_TYPE_EDGE_FALLING|IRQ_TYPE_EDGE_RISING,
        ATMF04_DRV_NAME2, (void *)client )) {
    PINFO("Could not allocate ATMF04_INT2 %d !\n", data->irq2);
    goto exit_irq_init_failed;
  }

  err = enable_irq_wake(data->irq);
  data->wake_irq_enabled = true;
  err = enable_irq_wake(data->irq2);
  data->wake_irq2_enabled = true;

  data->input_dev_sar = input_allocate_device();
  if (!data->input_dev_sar) {
    err = -ENOMEM;
    PINFO("Failed to allocate input device sar!\n");
    goto exit_free_irq;
  }
  data->input_dev_sar2 = input_allocate_device();
  if (!data->input_dev_sar2) {
    err = -ENOMEM;
    PINFO("Failed to allocate input device sar!\n");
    goto exit_free_irq;
  }

  set_bit(EV_ABS, data->input_dev_sar->evbit);
  set_bit(EV_ABS, data->input_dev_sar2->evbit);

  input_set_abs_params(data->input_dev_sar, ABS_DISTANCE, 0, 1, 0, 0);
  input_set_abs_params(data->input_dev_sar2, ABS_DISTANCE, 0, 1, 0, 0);

  data->input_dev_sar->name = ATMF04_DRV_NAME;
  data->input_dev_sar->dev.init_name = ATMF04_DRV_NAME;
  data->input_dev_sar->phys= SAR_INPUT;
  data->input_dev_sar->id.bustype = BUS_I2C;
  input_set_drvdata(data->input_dev_sar, data);

  err = input_register_device(data->input_dev_sar);
  if (err) {
    err = -ENOMEM;
    PINFO("Unable to register input device sar(%s)\n",
        data->input_dev_sar->name);
    goto exit_free_dev_sar;
  }
  data->input_dev_sar2->name = ATMF04_DRV_NAME2;
  data->input_dev_sar2->dev.init_name = ATMF04_DRV_NAME2;
  data->input_dev_sar2->phys= SAR2_INPUT;
  data->input_dev_sar2->id.bustype = BUS_I2C;
  input_set_drvdata(data->input_dev_sar2, data);

  err = input_register_device(data->input_dev_sar2);
  if (err) {
    err = -ENOMEM;
    PINFO("Unable to register input device sar(%s)\n",
        data->input_dev_sar2->name);
    goto exit_free_dev_sar; } if (data->platform_data->fw_name) {
      err = load_firmware(data, client, data->platform_data->fw_name);
      if (err) {
        PINFO("Failed to request firmware\n");
        //goto exit_free_irq;
        goto exit_irq_init_failed;
      }
    }
  err = sysfs_create_group(&data->input_dev_sar->dev.kobj, &atmf04_attr_group);
  if (err)
    PINFO("sysfs create fail!\n");
  err = sysfs_create_group(&data->input_dev_sar2->dev.kobj, &atmf04_attr_group);
  if (err)
    PINFO("sysfs create fail!\n");

  /* default controller off */
#if defined(CONFIG_LGE_SAR_CONTROLLER_IGNORE_INT_ON_PROBE)
  probe_end_flag = true;
#endif
  onoff_controller(data, DISABLE_controller_PINS, CH_1);
  PINFO("interrupt is hooked\n");

  return 0;

exit_irq_init_failed:
  mutex_destroy(&data->update_lock);
  mutex_destroy(&data->enable_lock);
exit_free_dev_sar:
exit_free_irq:
  free_irq(data->irq, client);
  free_irq(data->irq2, client);
exit:
  PINFO("Error");
  return err;
}

static int atmf04_remove(struct i2c_client *client)
{

  struct atmf04_data *data = i2c_get_clientdata(client);
  struct atmf04_platform_data *pdata = data->platform_data;


  if(data->wake_irq_enabled)
    disable_irq_wake(client->irq);
  if(data->wake_irq2_enabled)
    disable_irq_wake(data->irq2);

  free_irq(client->irq, client);
  free_irq(data->irq2, client);

  if (pdata->power_on)
    pdata->power_on(client, false);

  if (pdata->exit)
    pdata->exit(client);

  mutex_destroy(&data->update_lock);
  mutex_destroy(&data->enable_lock);
  PINFO("ATMF04 remove\n");
  return 0;
}

static const struct i2c_device_id atmf04_id[] = {
  { "atmf04", 0 },
  { }
};
MODULE_DEVICE_TABLE(i2c, atmf04_id);

#ifdef CONFIG_OF
static struct of_device_id atmf04_match_table[] = {
  { .compatible = "adsemicon,atmf04",},
  { },
};
#else
#define atmf04_match_table NULL
#endif

static struct i2c_driver atmf04_driver = {
  .driver = {
    .name   = ATMF04_DRV_NAME,
    .owner  = THIS_MODULE,
    .of_match_table = atmf04_match_table,
  },
  .probe  = atmf04_probe,
  .remove = atmf04_remove,
  .id_table = atmf04_id,
};

static void atmf04_async_init(void *data, async_cookie_t cookie)
{
  int ret;
  PINFO("ATMF04 async init start.\n");
  atmf04_workqueue = create_workqueue("sar_controller");
  ret = i2c_add_driver(&atmf04_driver);
  if (ret)
     PINFO("ATMF04 init failed");
  PINFO("ATMF04 async init end.\n");
}

static int __init atmf04_init(void)
{
  PINFO("ATMF04 init Proximity driver: init.\n");
  async_schedule(atmf04_async_init, NULL);
  return 0;
}

static void __exit atmf04_exit(void)
{
  PINFO("ATMF04 Proximity driver: release.\n");
  if (atmf04_workqueue)
    destroy_workqueue(atmf04_workqueue);

  atmf04_workqueue = NULL;
  i2c_del_driver(&atmf04_driver);
}

MODULE_DESCRIPTION("ATMF04 sar controller driver");
MODULE_LICENSE("GPL");

module_init(atmf04_init);
module_exit(atmf04_exit);

