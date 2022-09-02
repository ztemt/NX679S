#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/of_gpio.h>
#include <linux/regulator/consumer.h>

#include "nubia_fan.h"
#include "fan_fw_05.h"
#define FAN_PINCTRL_STATE_ACTIVE "pull_up_default"
#define FAN_PINCTRL_STATE_SUSPEND "pull_down_default"
#define FAN_VREG_NAME  "pm8350c_l11"
#define FAN_VREG_VOLTAGE 3312000
//#define FAN_VREG_VOLTAGE 3008000
static struct fan *nubia_fan;


#define STATUS_OK         	         (0)
#define STATUS_ERROR      	         (-1)
#define STATUS_I2C_OPEN_ERROR        (-2)
#define STATUS_I2C_CLOSE_ERROR       (-3)
#define STATUS_GPIO_OPEN_ERROR       (-4)
#define STATUS_GPIO_CLOSE_ERROR      (-5)
#define STATUS_READ_ERROR            (-6)
#define STATUS_WRITE_ERROR           (-7)
#define STATUS_SLADDR_ERROR          (-8)
#define STATUS_GPIO_HIGH_ERROR       (-9)
#define STATUS_GPIO_LOW_ERROR        (-10)
#define STATUS_INVALID_PROMPT_ERROR  (-11)
#define STATUS_INVALID_RESULT_ERROR  (-12)
#define STATUS_VERIFY_FAILED_ERROR   (-13)
#define STATUS_INVALID_ADDRESS_ERROR (-14)
#define VALUE_PROMPT            0x3E
#define DELAY_RESET_MS          (10 * 1000)  
#define DELAY_AFTER_RESET_US    (800)
#define DELAY_MASTERERASE_MS    (1000) 
#define DELAY_PROMPT_US			(200)

#define I2C_ADDRESS       (0x54 >> 1)
#define WRITECOMMAND      0xD0
#define READCOMMAND       0x20

#define FAN_MIN_RST_COUNT     2000
#define FAN_MAX_RST_COUNT     0x7fffffff		//for a max speed never reach
#define FAN_CHECK_DELAY   5000
#define PWM_ALLOW_RANGE   10
#define PWM_MAX_VAL       100
#define ALLOW_ERR_TIMES   5
#define ALLOW_CRT_TIMES   3
#define FAN_FW_VERSION    5

#define MAX28200_LOADER_ERROR_NONE		0x00
#define MAX28200_LOADER_VERIFY_FAILED   0x05
static unsigned int fan_speed = 0;
static unsigned int fan_current = 0;
static unsigned int fan_temp = 0;
static unsigned int fan_level = 0;
static unsigned int g_fan_enable = 0;
static bool fan_power_on = 0;
//static unsigned int fan_thermal_engine_level = 0;
static struct fan *nubia_fan;
static unsigned char firmware_magicvalue = 0x55;
static unsigned int fan_pwm_old_level = 0;
int gpio11_test;
int gpio_HY = 0;
struct delayed_work fan_delay_work;
static struct mutex	i2c_rw_lock;
static DEFINE_MUTEX(i2c_rw_lock);
static struct mutex	ck_lock;
static DEFINE_MUTEX(ck_lock);
static unsigned int ck_delay = 200;	//self check delay 200ms
static unsigned int adj_en = 1;		//for adjust and reset fan

enum{
	FAN_PWM = 0,
	FAN_RPM,
	FAN_RPM_LLIMIT,
	FAN_RPM_ULIMIT,
};
enum{
	FAN_RD = 1,		//fan ready.
	FAN_FW = 2,		//fan after firmware update
};
static const unsigned int fan_rpm[FAN_LEVEL_MAX+1][4] = {
	{0, 0, 0, 0},
	{20, 6000, 5500, 6500},		//level 1, pwm 20%, 6000 rpm.
	{40, 9000, 8500, 9500},
	{50, 11000, 10500, 11500},
	{75, 14000, 13500, 14500},
	{100, 17500, 17000, FAN_MAX_RST_COUNT}
};

const char *fan_name = "fan";
static int fan_firmware_check(void);
static void fan_set_enable(bool enable);
static void start_pwm(struct fan *fan,unsigned short pwm_value);
static ssize_t fan_speed_level_store(struct device* dev, struct device_attribute *attr, const char* buf, size_t len);

void fan_rpm_correct(int cur_level, int cur_rpm)
{
	int offset = 0;
	int direct = 0;
	static int correct_cnt = 0;

	if(cur_level<FAN_LEVEL_3 || adj_en==0)
		return;
	
	if(cur_rpm<=FAN_MIN_RST_COUNT || cur_rpm>=FAN_MAX_RST_COUNT)
		return;

	//printk(KERN_ERR "NB_FAN:%s:level=%d exp_rmp=%d cur_rpm=%d ulimit=%d llimit=%d", __func__, cur_level, fan_rpm[cur_level][FAN_RPM], cur_rpm, fan_rpm[cur_level][FAN_RPM_ULIMIT], fan_rpm[cur_level][FAN_RPM_LLIMIT]);
	if(cur_rpm<=fan_rpm[cur_level][FAN_RPM_ULIMIT] && cur_rpm>=fan_rpm[cur_level][FAN_RPM_LLIMIT] ){
		correct_cnt = 0;
		return;
	}
	correct_cnt++;
	if(correct_cnt <= ALLOW_CRT_TIMES)
		return;

	correct_cnt = 0;
	
	/*start to adjust rpm*/
	direct = cur_rpm>fan_rpm[cur_level][FAN_RPM_ULIMIT]?1:0;

	if(direct){
		offset =  fan_rpm[cur_level][FAN_PWM] - ((cur_rpm-fan_rpm[cur_level][FAN_RPM])*100/fan_rpm[cur_level][FAN_RPM])*fan_rpm[cur_level][FAN_PWM]/100;	//reduce the count
	}else{
		offset = fan_rpm[cur_level][FAN_PWM] + ((fan_rpm[cur_level][FAN_RPM]-cur_rpm)*100/fan_rpm[cur_level][FAN_RPM])*fan_rpm[cur_level][FAN_PWM]/100;		//add the count
	}

	if(offset < fan_rpm[FAN_LEVEL_1][FAN_PWM])
		offset = fan_rpm[FAN_LEVEL_1][FAN_PWM];
	if(offset > PWM_MAX_VAL)
		offset = PWM_MAX_VAL;

	start_pwm(nubia_fan, offset);
	printk(KERN_ERR "NB_FAN:%s:level=%d exp_rmp=%d cur_rpm=%d cur_pwm=%d adj_pwm=%d", __func__, cur_level, fan_rpm[cur_level][FAN_RPM], cur_rpm, fan_rpm[cur_level][FAN_PWM], offset);
}
static bool get_fan_power_on_state(void){
	//printk(KERN_ERR "%s: fan_power_on=%d\n",__func__,fan_power_on);
	return fan_power_on;
}	
static void set_fan_power_on_state(bool state){
	fan_power_on = state;
	//printk(KERN_ERR "%s: state=%d\n",__func__,state);
}
void lowlevel_delay(int microseconds) {
  //
  // perform platform specific delay here
    //udelay(microseconds);
    mdelay(microseconds);
}

int lowlevel_i2cWrite(uint8_t *val, int length){
	int ret;
	struct i2c_client *i2c = nubia_fan->i2c;
	struct i2c_msg msg[1];	
		msg[0].addr = i2c->addr;
		msg[0].flags = 0;
		msg[0].len = length;
		msg[0].buf = val;
	
	ret = i2c_transfer(i2c->adapter, msg, 1);
	if(ret){
		return STATUS_WRITE_ERROR;
	}
	return STATUS_OK;
}

int lowlevel_i2cRead(uint8_t *val, int length){
		int ret;
		struct i2c_client *i2c = nubia_fan->i2c;
		struct i2c_msg msg[1];
		msg[0].addr = i2c->addr;
		msg[0].flags = I2C_M_RD;
		msg[0].len = length;
		msg[0].buf = val;
	ret = i2c_transfer(i2c->adapter, msg, 1);
	if(ret){
		return STATUS_READ_ERROR;
	}
	return STATUS_OK;
}

static unsigned char fan_i2c_firmware_read(struct fan *fan, unsigned char *data, unsigned int length){
	struct i2c_client *i2c = fan->i2c;
	unsigned int ret;
	char read_data[1] = {0};
	struct i2c_msg msg[1];
		msg[0].addr = i2c->addr;
		msg[0].flags = I2C_M_RD;
		msg[0].len = sizeof(read_data);
		msg[0].buf = read_data;
	ret=i2c_transfer(i2c->adapter, msg, 1);
	//printk(KERN_ERR "George fan_i2c_firmware_read addr:%02x,ret=%d,read_data[0]=%x,\n", i2c ->addr,ret,read_data[0]);
    return read_data[0];
}

int MAX28200_program(uint8_t *image, int size) {
  int i;
  uint8_t val;
  int status;
  int index;
  int address;
  int payloadLength;
  // send the Magic Value
  val = firmware_magicvalue;
  fan_i2c_write(nubia_fan,&val,1);

  udelay(75);
  
  // S 55 [3E*] P
  // read the prompt
  status=fan_i2c_firmware_read(nubia_fan,&firmware_magicvalue,1);
  if (status != VALUE_PROMPT) {
	 printk(KERN_ERR "%s<-->%d STATUS_INVALID_PROMPT_ERROR status:%x\n",__FUNCTION__,__LINE__,status);
	 return -1;
  }
  // NOP
  // S 54 00 P
  val = 0x00;
  fan_i2c_write(nubia_fan,&val,1);
  // S 55 [3E*] P
  // read the prompt
  //DEBUG_PRINT(("read the prompt...\n"));
    //status=i2c_smbus_read_byte_data(nubia_fan ->i2c,MAGIC_VALUE);
  status=fan_i2c_firmware_read(nubia_fan,&firmware_magicvalue,1);
  if (status != VALUE_PROMPT) {
	printk(KERN_ERR "%s<-->%d STATUS_INVALID_PROMPT_ERROR status:%x\n",__FUNCTION__,__LINE__,status);
    return -1;
  }

  // Master Erase
  // S 54 02 P
  //DEBUG_PRINT(("master erase...\n"));
  //printk(KERN_ERR "%s<-->%d\n",__FUNCTION__,__LINE__);
  val = 0x02;
  fan_i2c_write(nubia_fan,&val,1);
  
  // delay after issuing master erase
  lowlevel_delay(DELAY_MASTERERASE_MS);
  
  // S 55 [3E*] P
  // read the prompt
  status=fan_i2c_firmware_read(nubia_fan,&firmware_magicvalue,1);
  if (status != VALUE_PROMPT) {
	printk(KERN_ERR "%s<-->%d STATUS_INVALID_PROMPT_ERROR status:%x\n",__FUNCTION__,__LINE__,status);
	return -1;
  }

  // Get Status 
  // S 54 04 P
  val = 0x04;
  fan_i2c_write(nubia_fan,&val,1);

  // S 55 [04*] P
  status=fan_i2c_firmware_read(nubia_fan,&firmware_magicvalue,1);
  if (status != 0x04) {
	  printk(KERN_ERR "%s<-->%d STATUS_INVALID_PROMPT_ERROR status:%x\n",__FUNCTION__,__LINE__,status);
	  return -1;
	}

  // S 55 [00*] P
  status=fan_i2c_firmware_read(nubia_fan,&firmware_magicvalue,1);
  if (status != 0x00) {
	  printk(KERN_ERR "%s<-->%d STATUS_INVALID_PROMPT_ERROR status:%x\n",__FUNCTION__,__LINE__,status);
	  return -1;
	}

  // S 55 [3E*] P
  // read the prompt
  status=fan_i2c_firmware_read(nubia_fan,&firmware_magicvalue,1);
  if (status != VALUE_PROMPT) {
	  printk(KERN_ERR "%s<-->%d STATUS_INVALID_PROMPT_ERROR status:%x\n",__FUNCTION__,__LINE__,status);
	  return -1;
	}

  // Bogus Command
  // S 54 55 P
  //DEBUG_PRINT(("bogus command...\n"));
  val = 0x55;
  fan_i2c_write(nubia_fan,&val,1);
  //if (status != STATUS_OK) return status;

  // S 54 00 P
  val = 0x00;
  fan_i2c_write(nubia_fan,&val,1);
  //if (status != STATUS_OK) return status;

  // S 54 00 P
  val = 0x00;
  fan_i2c_write(nubia_fan,&val,1);
  //if (status != STATUS_OK) return status;

  // S 54 00 P
  val = 0x00;
  fan_i2c_write(nubia_fan,&val,1);
  //if (status != STATUS_OK) return status;

  // S 55 [3E*] P
  // read the prompt
 // DEBUG_PRINT(("read prompt...\n"));
  status=fan_i2c_firmware_read(nubia_fan,&firmware_magicvalue,1);
  if (status != VALUE_PROMPT) {
	  printk(KERN_ERR "%s<-->%d STATUS_INVALID_PROMPT_ERROR status:%x\n",__FUNCTION__,__LINE__,status);
	  return -1;
	}

  // Get Status 
  // S 54 04 P
  //DEBUG_PRINT(("get status...\n"));
  //printk(KERN_ERR "%s<-->%d Get Status  S 54 04 P\n",__FUNCTION__,__LINE__);
  val = 0x04;
  fan_i2c_write(nubia_fan,&val,1);
  //if (status != STATUS_OK) return status;

  // S 55 [04*] P
  status=fan_i2c_firmware_read(nubia_fan,&firmware_magicvalue,1);
  printk(KERN_ERR "%s<-->%d S55 [04*] P val:%x\n",__FUNCTION__,__LINE__,status);
  if (status != 0x04) {
	  printk(KERN_ERR "%s<-->%d STATUS_INVALID_PROMPT_ERROR status:%x\n",__FUNCTION__,__LINE__,status);
	  return -1;
	}

  //S 55 [01*] P
  status=fan_i2c_firmware_read(nubia_fan,&firmware_magicvalue,1);
  if (status != 0x01) {
	  printk(KERN_ERR "%s<-->%d STATUS_INVALID_PROMPT_ERROR status:%x\n",__FUNCTION__,__LINE__,status);
	  return -1;
	}

  // S 55 [3E*] P
  // read the prompt
  //DEBUG_PRINT(("read the prompt...\n"));
  printk(KERN_ERR "%s<-->%d\n",__FUNCTION__,__LINE__);
  status=fan_i2c_firmware_read(nubia_fan,&firmware_magicvalue,1);
  if (status != VALUE_PROMPT) {
	printk(KERN_ERR "%s<-->%d STATUS_INVALID_PROMPT_ERROR status:%x\n",__FUNCTION__,__LINE__,status);
	return -1;
  }
  //if (status != STATUS_OK) return status;
  //if (val != VALUE_PROMPT) return STATUS_INVALID_PROMPT_ERROR;

  // Set Multiplier - This value needs to be set such that bytes written multiplied by 4 gives the actual desired length in the length byte for load command.
  // S 54 0B P
  //DEBUG_PRINT(("set multiplier...\n"));
  //printk(KERN_ERR "%s<-->%d\n",__FUNCTION__,__LINE__);
  val = 0x0B;
  fan_i2c_write(nubia_fan,&val,1);
  //if (status != STATUS_OK) return status;
  // S 54 00 P
  val = 0x00;
   fan_i2c_write(nubia_fan,&val,1);
  //if (status != STATUS_OK) return status;
  // S 54 04 P
  val = 0x04;
  fan_i2c_write(nubia_fan,&val,1);
  //if (status != STATUS_OK) return status;
  // S 54 00 P
  val = 0x00;
  fan_i2c_write(nubia_fan,&val,1);
  //if (status != STATUS_OK) return status;

  // S 55 [3E*] P
  // read the prompt
  status=fan_i2c_firmware_read(nubia_fan,&firmware_magicvalue,1);
  if (status != VALUE_PROMPT) {
	printk(KERN_ERR "%s<-->%d STATUS_INVALID_PROMPT_ERROR status:%x\n",__FUNCTION__,__LINE__,status);
	return -1;
  }
  //if (status != STATUS_OK) return status;
  //if (val != VALUE_PROMPT) return STATUS_INVALID_PROMPT_ERROR;

  //
  // load the image
  //
  index = 0;
  address = 0;
  payloadLength = 16;
  //DEBUG_PRINT(("load the image...\n"));
  while (index < size) { 
    // Write Command
    val = WRITECOMMAND;
    //status = lowlevel_i2cWrite(&val, 1);
	//if (status != STATUS_OK) return status;
    fan_i2c_write(nubia_fan,&val,1);
	
    // Byte Count
    val = (uint8_t)payloadLength;
    //DEBUG_PRINT(("byte count %02x ",val));
    //status = lowlevel_i2cWrite(&val, 1);
    //if (status != STATUS_OK) return status;
	fan_i2c_write(nubia_fan,&val,1);
	

    // Low Address
    val = (uint8_t)(address & 0xff);
    //DEBUG_PRINT(("lo %02x ",val));
    //status = lowlevel_i2cWrite(&val, 1);
    //if (status != STATUS_OK) return status;
	fan_i2c_write(nubia_fan,&val,1);

    // High Address
    val = (uint8_t)((address >> 8) & 0xff);
    //DEBUG_PRINT(("hi %02x ",val));
    //status = lowlevel_i2cWrite(&val, 1);
    //if (status != STATUS_OK) return status;
	fan_i2c_write(nubia_fan,&val,1);

	
    for (i = 0; i < payloadLength; i++) {
      val = image[index++];
      //DEBUG_PRINT(("%02x ",val));
      //status = lowlevel_i2cWrite(&val, 1);
      //if (status != STATUS_OK) return status;
	  fan_i2c_write(nubia_fan,&val,1);
    }

    // advance the address
    address += payloadLength;

    // S 55 [3E*] P
    // read the prompt
    //status = lowlevel_i2cRead(&val, 1);
    //if (status != STATUS_OK) return status;
    //if (val != VALUE_PROMPT) return STATUS_INVALID_PROMPT_ERROR;
	status=fan_i2c_firmware_read(nubia_fan,&firmware_magicvalue,1);
    if (status != VALUE_PROMPT) {
	printk(KERN_ERR "%s<-->%d STATUS_INVALID_PROMPT_ERROR status:%x\n",__FUNCTION__,__LINE__,status);
	return -1;
    }

	
    // Get Status 
    // S 54 04 P
    //DEBUG_PRINT(("get status...\n"));
    val = 0x04;
    //status = lowlevel_i2cWrite(&val, 1);
    //if (status != STATUS_OK) return status;
    fan_i2c_write(nubia_fan,&val,1);
	

    // S 55 [04*] P
    //status = lowlevel_i2cRead(&val, 1);
    //if (status != STATUS_OK) return status;
    //if (val != 0x04) return STATUS_VERIFY_FAILED_ERROR;
	status=fan_i2c_firmware_read(nubia_fan,&firmware_magicvalue,1);
	if (status != 0x04) {
		printk(KERN_ERR "%s<-->%d STATUS_INVALID_PROMPT_ERROR status:%x\n",__FUNCTION__,__LINE__,status);
		return -1;
	  }


    // S 55 [00*] P
    //status = lowlevel_i2cRead(&val, 1);
    //if (status != STATUS_OK) return status;
    //if (val != MAX28200_LOADER_ERROR_NONE) return STATUS_VERIFY_FAILED_ERROR;
	status=fan_i2c_firmware_read(nubia_fan,&firmware_magicvalue,1);
	if (status != MAX28200_LOADER_ERROR_NONE) {
		printk(KERN_ERR "%s<-->%d STATUS_INVALID_PROMPT_ERROR status:%x\n",__FUNCTION__,__LINE__,status);
		return -1;
	  }

    // S 55 [3E*] P
    // read the prompt
    //status = lowlevel_i2cRead(&val, 1);
    //if (status != STATUS_OK) return status;
    //if (val != VALUE_PROMPT) return STATUS_INVALID_PROMPT_ERROR; 
    status=fan_i2c_firmware_read(nubia_fan,&firmware_magicvalue,1);
	if (status != VALUE_PROMPT) {
		printk(KERN_ERR "%s<-->%d STATUS_INVALID_PROMPT_ERROR status:%x\n",__FUNCTION__,__LINE__,status);
		return -1;
	  }
  }
  return STATUS_OK;
}

int MAX28200_fw_updata(void) {
  int status;
  status = MAX28200_program(fw_002, sizeof(fw_002)/sizeof(fw_002[0]));
  if (status != STATUS_OK) {
    printk(KERN_ERR "Error: Programming returned with error: %d\n", status);
  }
  //printk(KERN_ERR "\ndone.\n");
  return status;
}


static void fan_i2c_write(struct fan *fan, unsigned char *data, unsigned int length){
	int ret;
	struct i2c_client *i2c = fan->i2c;
	struct i2c_msg msg[1];	
	//printk(KERN_ERR "George %s<-->%d,addr:%02x\n",__FUNCTION__,__LINE__,nubia_fan ->i2c ->addr);
	msg[0].addr = i2c->addr;
	msg[0].flags = 0;
	msg[0].len = length;
	msg[0].buf = data;

	mutex_lock(&i2c_rw_lock);
	ret = i2c_transfer(i2c->adapter, msg, 1);
	mutex_unlock(&i2c_rw_lock);
	//printk(KERN_ERR "FAN fan_i2c_write ret=%d,msg[0].addr=%x, msg[0].buf:%x,len=%u\n", ret,msg[0].addr,*data,msg[0].len);
}

static  int fan_i2c_read(struct fan *fan, unsigned char *data, unsigned int length){
	struct i2c_client *i2c = fan->i2c;
	unsigned int ret1,ret2;
	int ret;
	char read_data[2] = {0,0};
	struct i2c_msg msg[2];

	msg[0].addr = i2c->addr;
	msg[0].flags = 0;
	msg[0].len = length;
	msg[0].buf = data;

	msg[1].addr = i2c->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].len = sizeof(read_data);
	msg[1].buf = read_data;

	mutex_lock(&i2c_rw_lock);
	ret=i2c_transfer(i2c->adapter, msg, 2);
	mutex_unlock(&i2c_rw_lock);

	ret1 = read_data[0];
	ret2 = read_data[1];
	ret = ret1 + ret2*256;
	//printk(KERN_ERR "George fan_i2c_read addr:%02x,ret=%d,ret1=%d,ret2=%d,read_data[0]=%x,read_data[1]=%x\n", i2c ->addr,ret,ret1,ret2,read_data[0],read_data[1]);
	
	return ret;
}

static  int get_speed_count(struct fan *fan){
	unsigned char start_cmd = 0x01;
	unsigned char get_cmd = 0x02;
	unsigned char stop_cmd = 0x03;
    int count = 0;
	static int fan_HY = 0;
	unsigned int HY_adj[5] = {0, 0, 8, 4, 3};

	if(fan_HY<0 && gpio_HY>0){
		fan_HY = gpio_get_value(gpio_HY);
		if(fan_HY == 1)
			printk(KERN_ERR "NB_FAN:%s:Huaying fan!status from gpio=%d.", __func__, gpio_HY);
		else
			printk(KERN_ERR "NB_FAN:%s:TaiDa fan!status from gpio=%d,fan_HY=%d.", __func__, gpio_HY, fan_HY);
	}

	fan_i2c_write(fan,&start_cmd,sizeof(start_cmd));
	msleep(1000);
	fan_i2c_write(fan,&stop_cmd,sizeof(stop_cmd));
	count = fan_i2c_read(fan,&get_cmd,sizeof(get_cmd));
	if(fan_HY == 1)
		count =count * (30 + (fan_level>0?HY_adj[fan_level-1]:0)); //the HuaYing fan minute speed
	else
		count =count * 20; //the TaiDa fan minute speed

	return count;
}

static unsigned int get_fan_current(struct fan *fan){
	static unsigned char data = 0x06;
	fan_current = fan_i2c_read(fan,&data,sizeof(data));
	return(fan_current);
}

static unsigned int get_fan_temp(struct fan *fan){
	static unsigned char data = 0x07;
	fan_temp = fan_i2c_read(fan,&data,sizeof(data));
	return(fan_temp);
}

static void start_pwm(struct fan *fan,unsigned short pwm_value){
	//static unsigned char data[2] ={0x04,0x00};//30KHZ
	static unsigned char data[2] ={0x08,0x00};// 25khz reg


	data[1] = pwm_value;
	//printk(KERN_ERR "George start_pwm data[0]=%x, data[1]=%x,sizeof=%d\n", data[0],data[1],sizeof(data));
	fan_i2c_write(fan,data,sizeof(data));
}
static int fan_rpm_reset(unsigned int err_cnt)
{
	//int fan_levle_back = 0;
	//unsigned int fan_speed_back = 0;

	if(err_cnt==0 || (err_cnt%ALLOW_ERR_TIMES) || fan_level==0 || adj_en==0)
		return 0;

	//fan_levle_back = fan_level;
	//fan_speed_back = fan_speed;
	//fan_set_enable(false);	//power down
	//msleep(20);
	fan_pwm_old_level = 0;
	set_fan_power_on_state(false);
	fan_set_pwm_by_level(fan_level);	//reback old level
	//fan_speed = fan_speed_back;		//reback old speed
	ck_delay = (200*err_cnt)>FAN_CHECK_DELAY?FAN_CHECK_DELAY:(200*err_cnt);
	
	if(err_cnt > ALLOW_ERR_TIMES*2){
		fan_speed = 0;
		printk(KERN_ERR "NB_FAN:%s:reset seem not work! set speed to 0.\n",__func__);
	}

	printk(KERN_ERR "NB_FAN:%s:begin reset the fan!!!,fan_level=%d fan_speed=%d.\n",__func__,fan_level, fan_speed);
	
	return 1;
}
static void fan_monitor(void){
	int ret = 0;
	int fan_count=0;
	unsigned int adc_current=0;
	unsigned int adc_temp=0;
	static unsigned int err_cnt = 0;

	mutex_lock(&ck_lock);

	if((get_fan_power_on_state() == true) ){

		fan_count = get_speed_count(nubia_fan);

		adc_current = get_fan_current(nubia_fan);

		adc_temp = get_fan_temp(nubia_fan);
#ifndef CONFIG_FAN_REMOVE_RPM_CORRECT
		fan_rpm_correct(fan_level, fan_count);
#endif
		if(fan_count <= FAN_MIN_RST_COUNT){	//speed error
			err_cnt++;
			ck_delay = 200;
			if(fan_count < 0)
				err_cnt = ALLOW_ERR_TIMES;
		}else{
			err_cnt = 0;
			fan_speed = fan_count;
			ck_delay = (ck_delay*2)>FAN_CHECK_DELAY?FAN_CHECK_DELAY:(ck_delay*2);
		}

        /*if fan is running and read fan_count is 0,reset the fan*/
		ret = fan_rpm_reset(err_cnt);
#ifndef CONFIG_FAN_REMOVE_FOR_GKI
        if(!ret)
            schedule_delayed_work(&fan_delay_work, round_jiffies_relative(msecs_to_jiffies(ck_delay)));
#endif
	}
	else{
		fan_speed = 0;
		fan_current = 0;
		fan_temp = 0;
	}

	mutex_unlock(&ck_lock);

	printk(KERN_ERR "NB_FAN:%s:fan_count=%d(%d<=val<=%d),adc_current=%d,adc_temp=%d err_cnt=%d\n",
		__func__,fan_count,fan_rpm[fan_level][FAN_RPM_LLIMIT],fan_rpm[fan_level][FAN_RPM_ULIMIT],adc_current,adc_temp, err_cnt);
}

static void fan_delay_workqueue(struct work_struct *work){
	int ret = 0;

	cancel_delayed_work(&fan_delay_work);

	ret = fan_firmware_check();
	if(ret != FAN_RD)
        return;

	fan_monitor();
}


static int fan_enable_reg(struct fan *fan,
		bool enable)
{
	int ret;

	//printk(KERN_ERR "%s: enable=%d\n",__func__,enable);
	
	if (!enable) {
		ret = 0;
		goto disable_pwr_reg;
	}

	if ((fan->pwr_reg)&& (regulator_is_enabled(fan->pwr_reg) ==0)) {
		ret = regulator_enable(fan->pwr_reg);
		if (ret < 0) {
			dev_err(fan->dev->parent,"%s: Failed to enable power regulator\n",__func__);
		}
		printk(KERN_ERR "NB_FAN: open fan power!\n");
	}


     gpio_free(gpio11_test);
     ret = gpio_request(gpio11_test,"GPIO11");
     if (ret) {
	   pr_err("%s: fan reset gpio request failed\n",__func__);
          return ret;
     }
     gpio_direction_output(gpio11_test, 0);
     msleep(100);
     gpio_direction_output(gpio11_test,1);
     msleep(100);
   return ret;

disable_pwr_reg:
      gpio_direction_output(gpio11_test,0);
      gpio_free(gpio11_test);

	if (fan->pwr_reg){
		regulator_disable(fan->pwr_reg);
		printk(KERN_ERR "NB_FAN: close fan power!\n");
	}

	return ret;
}

static int fan_hw_reset(struct fan *fan,unsigned int delay)
{
    int ret;
	unsigned int reset_delay_time = 0;

    pr_info("%s enter %d\n", __func__,delay);
	reset_delay_time = delay;

    gpio_free(gpio11_test);
    ret = gpio_request(gpio11_test,"GPIO11");
    if (ret) {
	   pr_err("%s: fan reset gpio request failed\n",__func__);
          return ret;
    }
    gpio_direction_output(gpio11_test,1);
    mdelay(1);
    gpio_direction_output(gpio11_test,0);
    mdelay(1);
    gpio_direction_output(gpio11_test,1);

    switch(reset_delay_time){
			case DELAY_900:
				udelay(900);
				//pr_info("%s enter 900\n", __func__);
				break;
			case DELAY_1000:
				udelay(500);
				udelay(500);
				//pr_info("%s enter 1000\n", __func__);
				break;
			case DELAY_1100:
				udelay(500);
				udelay(600);
				//pr_info("%s enter 1100\n", __func__);
				break;
			case DELAY_1200:
				udelay(600);
				udelay(600);
				//pr_info("%s enter 1200\n", __func__);
				break;
			case DELAY_800:
				udelay(800);
				//pr_info("%s enter 800\n", __func__);
				break;
			case DELAY_600:	
				udelay(600);
				//pr_info("%s enter 600\n", __func__);
				break;
			case DELAY_400:	
				udelay(400);
				//pr_info("%s enter 400\n", __func__);
				break;	
			case DELAY_1300:
				udelay(600);
				udelay(700);
				//pr_info("%s enter 1300\n", __func__);
				break;	
			case DELAY_1400:
				udelay(700);
				udelay(700);
				//pr_info("%s enter 1400\n", __func__);
				break;	
			case DELAY_1500:
				udelay(800);
				udelay(700);
				//pr_info("%s enter 1500\n", __func__);
				break;		
			case DELAY_200:
				udelay(200);
				//pr_info("%s enter 200\n", __func__);
				break;	
			case DELAY_1800:
				udelay(900);
				udelay(900);
				//pr_info("%s enter 1800\n", __func__);
				break;					
			default:
				udelay(900);
				//pr_info("%s enter default\n", __func__);
				break;
		}
   	
       return 0;
}

static void fan_set_enable(bool enable) 
{
    //printk(KERN_ERR "%s: enable=%d\n",__func__,enable);

	if(!enable)
	{
   	    start_pwm(nubia_fan,0);
		fan_speed = 0;
		fan_level = 0;
	}
	set_fan_power_on_state(enable);
	fan_enable_reg(nubia_fan, enable);
}

static void fan_set_pwm_by_level(unsigned int level)
{

	//printk(KERN_ERR "%s: level=%d,fan_pwm_old_level=%d,fan_speed=%d\n",__func__,level,fan_pwm_old_level,fan_speed);

	fan_level = level;
	if(level == FAN_LEVEL_0){
       fan_set_enable(false);
	   fan_pwm_old_level = level;
	}else {
		 if(get_fan_power_on_state() == false){
            fan_set_enable(true);
            fan_pwm_old_level = 0;
        }

	     if(fan_pwm_old_level != level){
			switch(level){
				case FAN_LEVEL_1:
					start_pwm(nubia_fan,fan_rpm[FAN_LEVEL_1][FAN_PWM]);
					break;
				case FAN_LEVEL_2:
					start_pwm(nubia_fan,fan_rpm[FAN_LEVEL_2][FAN_PWM]);
					break;
				case FAN_LEVEL_3:
					//start_pwm(nubia_fan,60);
					start_pwm(nubia_fan,fan_rpm[FAN_LEVEL_3][FAN_PWM]);
					break;
				case FAN_LEVEL_4:
					start_pwm(nubia_fan,fan_rpm[FAN_LEVEL_4][FAN_PWM]);
					break;
				case FAN_LEVEL_5:
					start_pwm(nubia_fan,fan_rpm[FAN_LEVEL_5][FAN_PWM]);
					break;
				default:
					break;
			}
	    }
		if(fan_pwm_old_level!=0){
			cancel_delayed_work(&fan_delay_work);
		}
		ck_delay = 200;
		fan_pwm_old_level = level;
#ifndef CONFIG_FAN_REMOVE_FOR_GKI
		schedule_delayed_work(&fan_delay_work, round_jiffies_relative(msecs_to_jiffies(ck_delay)));		//start check fan status
#endif
	}
}

static ssize_t fan_enable_show(struct device* dev,struct device_attribute *attr, char* buf){

	printk(KERN_ERR "%s: g_fan_enable=%d\n",__func__,g_fan_enable);	
	return sprintf(buf, "%d\n", g_fan_enable);
}

static ssize_t fan_enable_store(struct device* dev, struct device_attribute *attr, const char* buf, size_t len){
	    
 	sscanf(buf, "%d", &g_fan_enable);
    printk(KERN_ERR "%s: g_fan_enable=%d\n",__func__,g_fan_enable);

	if(!g_fan_enable){
       fan_speed_level_store(NULL, NULL, "0", 1);
    }

    return len;
}

static ssize_t fan_speed_level_show(struct device* dev,struct device_attribute *attr, char* buf){
		
	return sprintf(buf, "%d\n", fan_level);
}
	
static ssize_t fan_speed_level_store(struct device* dev, struct device_attribute *attr, const char* buf, size_t len){

    unsigned int level;
	static unsigned int old_level = 0;
 	sscanf(buf, "%d", &level);

	if(level < FAN_LEVEL_0 || level > FAN_LEVEL_MAX){
		printk(KERN_ERR "%s: level=%d,it is a bad level,not in(%d-%d).\n",__func__,level,FAN_LEVEL_0, FAN_LEVEL_MAX);
		return EINVAL;
	}
	if(dev==NULL && attr==NULL){
		printk(KERN_ERR "fan switch closed, force set fan stop run!!!level=%d,old_level=%d\n",level,old_level);
	}else{
		printk(KERN_ERR "%s: level=%d,old_level=%d\n",__func__,level,old_level);
	}
	if(level == old_level){
	    printk(KERN_ERR "%s: fan have in level=%d.\n",__func__, level);
		return len;
	}

	old_level = level;
	mutex_lock(&ck_lock);
    fan_set_pwm_by_level(level);
	mutex_unlock(&ck_lock);

    return len;
}

static ssize_t fan_speed_count_show(struct device* dev,struct device_attribute *attr, char* buf){

	int len = 0;

	mutex_lock(&ck_lock);
	len = sprintf(buf, "%d\n", fan_speed);
	/*if(get_fan_power_on_state() != false){
		get_speed_count(nubia_fan);
	}*/
	mutex_unlock(&ck_lock);

	printk(KERN_ERR "%s: fan_speed_level=%d fan_speed=%d\n",__func__,fan_level, fan_speed);
	return len;
}

/*static ssize_t fan_current_show(struct kobject *kobj,
		struct kobj_attribute *attr, char *buf){
		
	return sprintf(buf, "%d\n", fan_current);
}*/

static ssize_t fan_temp_show(struct device* dev,struct device_attribute *attr, char* buf){

	get_fan_temp(nubia_fan);

	return sprintf(buf, "%d\n", fan_temp);
}
/*static ssize_t fan_thermal_engine_levell_show(struct kobject *kobj,
		struct kobj_attribute *attr, char *buf){

	return sprintf(buf, "%d\n", fan_thermal_engine_level);
}

static ssize_t fan_thermal_engine_level_store(struct kobject *kobj,
	    struct kobj_attribute *attr, const char *buf, size_t count){
	    sscanf(buf, "%d", &fan_thermal_engine_level);
          //printk(KERN_ERR "%s: fan_thermal_engine_level=%d\n",__func__,fan_thermal_engine_level);
          return count;
}*/
static ssize_t fan_pwm_store(struct device* dev, struct device_attribute *attr, const char* buf, size_t len){
	unsigned int fan_pwm_val = 0;

	sscanf(buf, "%d", &fan_pwm_val);
	printk(KERN_ERR "%s: set fan_pwm to %d%%.\n",__func__,fan_pwm_val);
	
	if(fan_pwm_val==200){	//for disable adj & reset
		adj_en = 0;
		return len;
	}else if(fan_pwm_val==300){	//for enable adj & reset
		adj_en = 1;
		return len;
	}
	start_pwm(nubia_fan, fan_pwm_val);

    return len;
}
/*static struct kobj_attribute fan_enable_attr=
    __ATTR(fan_enable, 0664, fan_enable_show, fan_enable_store);
static struct kobj_attribute fan_level_attr=
    __ATTR(fan_speed_level, 0664, fan_speed_level_show, fan_speed_level_store);
static struct kobj_attribute fan_speed_attr=
    __ATTR(fan_speed_count, 0664, fan_speed_count_show, NULL);
static struct kobj_attribute fan_current_attr=
    __ATTR(fan_current, 0664, fan_current_show, NULL);
static struct kobj_attribute fan_temp_attr=
    __ATTR(fan_temp, 0664, fan_temp_show, NULL);
static struct kobj_attribute fan_thermal_engine_level_attr=
    __ATTR(fan_thermal_engine_level, 0664, fan_thermal_engine_levell_show, fan_thermal_engine_level_store);
static struct kobj_attribute fan_pwm_attr=
    __ATTR(fan_pwm, 0664, NULL, fan_pwm_store);



static struct attribute *fan_attrs[] = {
	&fan_enable_attr.attr,
    &fan_level_attr.attr,
	&fan_speed_attr.attr,
	&fan_current_attr.attr,
	&fan_temp_attr.attr,
	&fan_thermal_engine_level_attr.attr,
	&fan_pwm_attr.attr,
    NULL,
};
*/
static DEVICE_ATTR(fan_enable, S_IWUSR | S_IRUGO, fan_enable_show, fan_enable_store);
static DEVICE_ATTR(fan_speed_level, S_IWUSR | S_IRUGO, fan_speed_level_show, fan_speed_level_store);
static DEVICE_ATTR(fan_speed_count, S_IWUSR | S_IRUGO, fan_speed_count_show, NULL);
static DEVICE_ATTR(fan_temp, S_IWUSR | S_IRUGO, fan_temp_show, NULL);
static DEVICE_ATTR(fan_pwm, S_IWUSR | S_IRUGO, NULL, fan_pwm_store);

static struct attribute *fan_attrs[] = {
    &dev_attr_fan_enable.attr,
    &dev_attr_fan_speed_level.attr,
    &dev_attr_fan_speed_count.attr,
    &dev_attr_fan_temp.attr,
	&dev_attr_fan_pwm.attr,
    NULL
};
static struct attribute_group fan_attr_group = {
    .attrs = fan_attrs,
};
struct kobject *fan_kobj;

static struct pinctrl_state *gpio_pins_active;
static struct pinctrl *pinctrl;
static int fan_pinctrl_active(struct device *dev)
{
    int err;
    pinctrl = devm_pinctrl_get(dev);
    if (IS_ERR_OR_NULL(pinctrl)) {
        dev_err(dev, "Failed to get pin ctrl\n");
        return PTR_ERR(pinctrl);
    }else{
		printk(KERN_ERR "nb_fan:%s:get pinctrl success", __func__);
	}
	
    gpio_pins_active = pinctrl_lookup_state(pinctrl, "default");
    if (IS_ERR_OR_NULL(gpio_pins_active)) {
        dev_err(dev, "Failed to lookup stk_pinctrl default state\n");
        return PTR_ERR(gpio_pins_active);
    }else{
		printk(KERN_ERR "nb_fan:%s:lookup pinctrl success", __func__);
	}
	
    err = pinctrl_select_state(pinctrl, gpio_pins_active);
    return err;
}
static int fan_firmware_check(void)
{
	int i = 0;
	int ret = 0;
	int try_count = 0;
	static char fw_rd = 0;
	unsigned char firmware_version_reg = 0x09;
	unsigned int delay_table[] = {DELAY_900,DELAY_1000,DELAY_1100,DELAY_1200,DELAY_800,
		                DELAY_600,DELAY_400,DELAY_1300,DELAY_1400,DELAY_1500,
		                DELAY_1800,DELAY_200};

	if(fw_rd)
		return FAN_RD;
	/*confirm mcu is valid*/
	ret= fan_i2c_read(nubia_fan,&firmware_version_reg,sizeof(firmware_version_reg));

	printk(KERN_ERR "NB_FAN:%s: fan mcu firmware version is %d.\n",__func__,ret);

	/*firmware update just for first boot*/
    if(ret != FAN_FW_VERSION){
		
		try_count = sizeof(delay_table)/sizeof(delay_table[0]);
		
		for(i = 0; i < try_count; i++){
			fan_hw_reset(nubia_fan,delay_table[i]);
			
			if(MAX28200_fw_updata()< 0){
			    printk(KERN_ERR "fan_fw_updata failed %d\n",delay_table[i]);
			    continue;
			}else {
				printk(KERN_ERR "fan_fw_updata done\n");
				break;
			}
		}
	    msleep(10);		//delay for firmware stable
	    fan_hw_reset(nubia_fan,1200);
	    mdelay(100); //delay 0.1 seconds
    }
	
	fan_set_enable(false);
	fw_rd = 1;
	
	return FAN_FW;
}
static int fan_probe(struct i2c_client *i2c, const struct i2c_device_id *id)
{
	struct fan *fan;
	struct device_node *np = i2c->dev.of_node;
	int ret;

	printk(KERN_ERR "fan_probe enter\n");

	fan = devm_kzalloc(&i2c->dev, sizeof(struct fan), GFP_KERNEL);
        if (fan == NULL){
            return -ENOMEM;
        }

    fan->dev = &i2c->dev;
    fan->i2c = i2c;

	fan->pwr_reg = regulator_get(fan->dev->parent,
					FAN_VREG_NAME);
	if (IS_ERR(fan->pwr_reg)) {
			dev_err(fan->dev->parent,
					"%s: Failed to get power regulator\n",
					__func__);
			ret = PTR_ERR(fan->pwr_reg);
			//goto regulator_put;
	}else{
		ret = regulator_set_voltage(fan->pwr_reg, FAN_VREG_VOLTAGE, FAN_VREG_VOLTAGE);
		if (ret) {
			dev_err(fan->dev->parent,
						"Regulator vdd_fan set vtg failed rc=%d\n", ret);
			//goto regulator_put;
		}
		if (fan->pwr_reg) {
			ret = regulator_enable(fan->pwr_reg);
			if (ret < 0) {
			dev_err(fan->dev->parent,
					"%s: Failed to enable power regulator\n",
					__func__);
			}
			dev_err(fan->dev->parent,
					"%s: success to enable power regulator=%d.\n",
					__func__, FAN_VREG_VOLTAGE);
	   }
	}

    nubia_fan = fan;
	ret = fan_pinctrl_active(fan->dev);
	if(ret){
		printk(KERN_ERR "nb_fan:%s: fan_pinctrl_active failed.ret=%d\n",__func__, ret);
		return ret;
	}else{
		printk(KERN_ERR "nb_fan:%s:fan_pinctrl_active success", __func__);
	}

    gpio11_test =of_get_named_gpio(np,"fan,reset-gpio",0);
	if(gpio11_test < 0){
		printk(KERN_ERR "nb_fan:%s: reset-gpio=%d.\n",__func__, gpio11_test);
		return gpio11_test;
	}else{
		fan_hw_reset(fan,1200);	//delay 1200 us after reset.
		//mdelay(100); //delay 0.1 seconds
	}
	printk(KERN_ERR "nb_fan:%s:get gpio ret=%d.", __func__, gpio11_test);
	
	gpio_HY =of_get_named_gpio(np, "fan,HY-gpio", 0);
	if(gpio_HY < 0){
		printk(KERN_ERR "nb_fan:%s: gpio_HY get from DT failed.\n",__func__, gpio_HY);
	}
	
	fan->cdev.name = fan_name;
	fan->cdev.brightness_set = NULL;
    ret = led_classdev_register(fan->dev, &fan->cdev);
    if (ret) {
        printk(KERN_ERR "%s: failed.\n", __func__);
    }else{
		printk(KERN_ERR "%s: success.\n", __func__);
	}

	/*fan_kobj = kobject_create_and_add("fan", kernel_kobj);
	if (!fan_kobj)
	{
		printk(KERN_ERR "%s: fan kobj create error\n", __func__);
		return -ENOMEM;
	}*/
	ret = sysfs_create_group(&fan->cdev.dev->kobj, &fan_attr_group);
	if(ret)
	{
		printk(KERN_ERR "%s: failed to create fan group attributes\n", __func__);
	}else{
		printk(KERN_ERR "%s: success to create fan group attributes\n", __func__);
	}

    //Begin [0016004715,fix the factory test result to panic,20181121]
    INIT_DELAYED_WORK(&fan_delay_work,  fan_delay_workqueue);
    //End [0016004715,fix the factory test result to panic,20181121]
    //fan_set_enable(false);	//move this to aftert firmware check
	//fan_enable_reg(nubia_fan, false);
#ifndef CONFIG_FAN_REMOVE_FOR_GKI
    schedule_delayed_work(&fan_delay_work, round_jiffies_relative(msecs_to_jiffies(200)));	//check firmware after 200ms
#endif

	return 0;

//regulator_put:
        
    if (fan->pwr_reg) {
		regulator_put(fan->pwr_reg);
		fan->pwr_reg = NULL;
       }
	    devm_kfree(&i2c->dev, fan);
    fan = NULL;
	return ret;
}

static int fan_remove(struct i2c_client *i2c)
{
	struct fan *fan = i2c_get_clientdata(i2c);

	pr_info("%s remove\n", __func__);
   
	sysfs_remove_group(fan_kobj,&fan_attr_group);
	fan_kobj = NULL;

#ifndef CONFIG_FAN_REMOVE_FOR_GKI
	if (gpio_is_valid(fan->reset_gpio))
		devm_gpio_free(&i2c->dev, fan->reset_gpio);
#endif

	fan_enable_reg(fan, false);
	
	if (fan->pwr_reg) {
		regulator_put(fan->pwr_reg);
		fan->pwr_reg = NULL;
	}

    devm_kfree(&i2c->dev, fan);
    fan = NULL;
	return 0;
}

static const struct i2c_device_id fan_i2c_id[] = {
	{ "nubia_fan_i2c", 0 },
    {}
};


static const struct of_device_id of_match[] = {
        { .compatible = "nubia_fan_i2c" },
        { }
};

static struct i2c_driver fan_i2c_driver = {
	.driver = {
		.name = "nubia_fan",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(of_match),
	},
	.probe = fan_probe,
	.remove = fan_remove,
	.id_table = fan_i2c_id,
};

static int __init fan_init(void)
{
    int ret = 0;

	printk("fan_init\n");

    ret = i2c_add_driver(&fan_i2c_driver);
	
    if(ret){
        pr_err("fail to add fan device into i2c\n");
        return ret;
    }
	
	printk("fan_init exit\n");
    return 0;

}

static void __exit fan_exit(void)
{
    i2c_del_driver(&fan_i2c_driver);
}

module_init(fan_init);
module_exit(fan_exit);

MODULE_AUTHOR("Fan, Inc.");
MODULE_DESCRIPTION("Fan Driver");
MODULE_LICENSE("GPL v2");

