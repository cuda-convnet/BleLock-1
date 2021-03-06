/*********************************************************************
 * INCLUDES
 */

#include "bcomdef.h"
#include "OSAL.h"
#include "OSAL_PwrMgr.h"
#include "dsp_finger.h"
#include "OnBoard.h"
#include "hal_adc.h"
#include "hal_led.h"
#include "hal_key.h"
#include "hal_lcd.h"
#include "stdio.h"
#include "gatt.h"
#include "hal_uart.h"
#include "hci.h"
#include "MPR121.h"
#include "gapgattserver.h"
#include "gattservapp.h"
#include "devinfoservice.h"
#include "simpleGATTprofile.h"
#include "oad.h"
#include "oad_target.h"
#include "AT020.h"
#if defined( CC2540_MINIDK )
  #include "simplekeys.h"
#endif

#if defined ( PLUS_BROADCASTER )
  #include "peripheralBroadcaster.h"
#else
  #include "peripheral.h"
#endif

#include "gapbondmgr.h"

#include "simpleBLEPeripheral.h"

#if defined FEATURE_OAD
  #include "oad.h"
  #include "oad_target.h"
#endif
#include "hal_aes.h"
#include "hal_flash.h"
#include "npi.h"
#include "stdio.h"
#include "hal_i2c.h"
#include "osal_snv.h"
#include "stdlib.h" /*这句话是必不可少的*/
#include "PN532.h"
//#include "sbl_app.h"
/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

// How often to perform periodic event
#define SBP_PERIODIC_EVT_PERIOD                   3000

// How often to perform periodic event
//#define TIMEOUT_EVT1_PERIOD                       3000
// What is the advertising interval when device is discoverable (units of 625us, 160=100ms)
#define DEFAULT_ADVERTISING_INTERVAL              1600

// Limited discoverable mode advertises for 30.72s, and then stops
// General discoverable mode advertises indefinitely

#if defined ( CC2540_MINIDK )
#define DEFAULT_DISCOVERABLE_MODE             GAP_ADTYPE_FLAGS_LIMITED
#else
#define DEFAULT_DISCOVERABLE_MODE             GAP_ADTYPE_FLAGS_GENERAL
#endif  // defined ( CC2540_MINIDK )

// Minimum connection interval (units of 1.25ms, 80=100ms) if automatic parameter update request is enabled
#define DEFAULT_DESIRED_MIN_CONN_INTERVAL     80

// Maximum connection interval (units of 1.25ms, 800=1000ms) if automatic parameter update request is enabled
#define DEFAULT_DESIRED_MAX_CONN_INTERVAL     80

// Slave latency to use if automatic parameter update request is enabled
#define DEFAULT_DESIRED_SLAVE_LATENCY         0

// Supervision timeout value (units of 10ms, 1000=10s) if automatic parameter update request is enabled
#define DEFAULT_DESIRED_CONN_TIMEOUT          1000

// Whether to enable automatic parameter update request when a connection is formed
#define DEFAULT_ENABLE_UPDATE_REQUEST        FALSE

// Connection Pause Peripheral time value (in seconds)
#define DEFAULT_CONN_PAUSE_PERIPHERAL         6

// Company Identifier: Texas Instruments Inc. (13)
#define TI_COMPANY_ID                         0x000D

#define INVALID_CONNHANDLE                    0xFFFF

#define OPENCMD             0x01         //开门指令
#define Alert               0x00         //告警指令

#define FAIL                0x01         //不可以开门
#define SUCCESS             0x00         //可以开门

#define BLE_OPEN            0x01
#define Touch_OPEN          0x03
#define Finger_OPEN         0x02
#define Wifi_OPEN           0x04

/* 发送给wifi板的信号分类 */
#define Fingger_Signal      0x01
#define Ble_Signal          0x02
#define KeyCode_Signal      0x03

/* 锁种类 DHF or ASSA*/
#define DHF_LOCK
#define  CR

#define  NFC
#define  LOCAL
/* 蓝牙4分钟休眠定时器使能 */
#define BLE_STANDBY_ENABLE
//#define factory888888 
#ifndef DHF_LOCK
/* 语音 */
#define bank0
#endif

#ifdef bank0                // 旧版语音
#define  OPEN_SUCCESS           0x0b       // 锁开
#define  VERIFY_FAIL            0x0a       // 认证失败
#define  KEY_VIOCE              0x00       // 按键音d
#define  Finger_LOCK            0x10       // 指纹已锁定
#define  FgTd_FAIL              0x0f        // 指纹同步失败
#define  FgTd_Success           0x0e        // 指纹同步成功
#define  Finger_ADD_FAIL        0x0d        // 添加失败
#define  INIT_SUCCESS           0x01        // 初始化成功
#define  CODE_ERROR             0x02        // 电量不足，请更换电池
#define  START_LINK             0x03        // 开始配网
#define  START_TD               0x04       // 开始同步指纹
#define  CODE_LOCK              0x05        // 密码已锁定
#define  LINK_SUCCESS           0x06        // 配网成功
#define  LINK_FAIL              0x07       // 配网失败
#define  Finger_RQ              0x08        // 请按指纹
#define  Finger_RQA             0x09       // 请再按指纹
#define  Finger_ADD_SUCCESS     0x0c     // 添加成功


#else                       // 新版语音
/*
#define  OPEN_SUCCESS           0x0b        // 锁开
#define  VERIFY_FAIL            0x0a        // 认证失败
#define  KEY_VIOCE              0x00        // 按键音d
#define  Finger_LOCK            0x10        // 指纹已锁定
#define  FgTd_FAIL              0x0f        // 指纹同步失败
#define  FgTd_Success           0x0e       // 指纹同步成功
#define  Finger_ADD_FAIL        0x0d        // 添加失败
#define  INIT_SUCCESS           0x01        // 初始化成功
#define  CODE_ERROR             0x02        // 电量不足，请更换电池
#define  START_LINK             0x03        // 开始配网
#define  START_TD               0x04        // 开始同步指纹
#define  CODE_LOCK              0x05        // 密码已锁定
#define  LINK_SUCCESS           0x06        // 配网成功
#define  LINK_FAIL              0x07        // 配网失败
#define  Finger_RQ              0x08        // 请按指纹
#define  Finger_RQA             0x09        // 请再按指纹
#define  Finger_ADD_SUCCESS     0x0c        // 添加成功
*/
#define  OPEN_SUCCESS           0x00       // 锁开
#define  Finger_Delet_SUCCESS   0x01        // 指纹删除成功 
#define  START_Update           0x02        // 硬件开始升级-1
#define  Update_SUCCESS         0x03        // 硬件升级成功-2
#define  Update_FAIL            0x04        // 硬件升级失败-3
#define  Factory_SUCCESS        0x05        // 恢复工厂模式成功
#define  Device_LOCKED          0x06        // 设备已被锁 //已锁机 -4
#define  KEY_VIOCE              0x07        // 按键音d    
#define  INIT_SUCCESS           0x08        // 初始化成功
#define  CODE_ERROR             0x09        // 电量不足，请更换电池
#define  START_LINK             0x0a        // 开始配网
#define  START_TD               0x0b        // 开始同步指纹 -5
#define  CODE_LOCK              0x0c        // 密码已锁定
#define  LINK_SUCCESS           0x0d        // 配网成功
#define  LINK_FAIL              0x0e        // 配网失败
#define  Finger_RQ              0x0f        // 请按指纹
#define  Finger_RQA             0x10        // 请再按指纹
#define  VERIFY_FAIL            0x11        // 认证失败
#define  Finger_ADD_SUCCESS     0x12        // 添加成功
#define  Finger_ADD_FAIL        0x13        // 添加失败
#define  FgTd_Success           0x14       // 指纹同步成功-6
#define  FgTd_FAIL              0x15        // 指纹同步失败-7
#define  Finger_LOCK            0x16        // 指纹已锁定

//new
/*
#define  OPEN_SUCCESS           0x04       // 锁开
#define  Finger_Delet_SUCCESS   0x14        // 指纹删除成功 
#define  Factory_SUCCESS        0x15        // 恢复工厂模式成功
#define  KEY_VIOCE              0x00        // 按键音d    
#define  INIT_SUCCESS           0x01        // 初始化成功
#define  CODE_ERROR             0x02        // 电量不足，请更换电池
#define  START_LINK             0x03        // 开始配网
#define  CODE_LOCK              0x05        // 密码已锁定
#define  LINK_SUCCESS           0x06        // 配网成功
#define  LINK_FAIL              0x07        // 配网失败
#define  Finger_RQ              0x08        // 请按指纹
#define  Finger_RQA             0x09        // 请再按指纹
#define  VERIFY_FAIL            0x10        // 认证失败
#define  Finger_ADD_SUCCESS     0x11        // 添加成功
#define  Finger_ADD_FAIL        0x12        // 添加失败
#define  Finger_LOCK            0x13        // 指纹已锁定
*/
#endif

#define WAIT_MS()      {for(unsigned short i=0;i<32000;i++)asm("NOP"); }
#define OAD_FLASH_PAGE_MULT  ((uint16)(HAL_FLASH_PAGE_SIZE / HAL_FLASH_WORD_SIZE))
                   
typedef struct UarttoWifiData
{
	uint8_t type;                       // 本次操作的类别：蓝牙/指纹/密码/其他
	uint16_t userid;                    // 本次操作的用户ID
	uint8_t message;                    // 本次操作的指令
	//uint8_t ack5bit;
} UartData;

// Length of bd addr as a string
#define B_ADDR_STR_LEN                        15

#if defined ( PLUS_BROADCASTER )
  #define ADV_IN_CONN_WAIT                    500   // delay 500 ms 
#endif

#define HAL_AES_DELAY()   \
  do {                    \
       volatile uint8 delay = 15;  \
       while(delay--);    \
     } while(0)

#define AES_SET_ENCR_DECR_KEY_IV(cmd)    \
  do {                                   \
       ENCCS = (ENCCS & ~0x07) | (cmd);  \
     } while(0)

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */
// Task ID for internal task/event processing
static uint8 simpleBLEPeripheral_TaskID;   

static gaprole_States_t gapProfileState = GAPROLE_INIT;

// GAP - SCAN RSP data (max size = 31 bytes)
static uint8 scanRspData[] =
{
  // complete name
  0x11,   // length of this data
  GAP_ADTYPE_LOCAL_NAME_COMPLETE,   //%e6%99%ba%e8%83%bd%e4%ba%91%e9%94%81 -“智能云锁”URL编码值
  0xe6,
  0x99,
  0xba,
  0xe8,
  0x83,
  0xbd,
  0xe4,
  0xba,
  0x91,
  0xe9,
  0x94,
  0x81,
  0x48,     // 'H'
  0x49,     // 'I'
  //0x70,   // 'p'
  0x53,     // 'S'
  0x53,     // 'S'

  // connection interval range
  0x05,     // length of this data
  GAP_ADTYPE_SLAVE_CONN_INTERVAL_RANGE,
  LO_UINT16( DEFAULT_DESIRED_MIN_CONN_INTERVAL ),   // 100ms
  HI_UINT16( DEFAULT_DESIRED_MIN_CONN_INTERVAL ),
  LO_UINT16( DEFAULT_DESIRED_MAX_CONN_INTERVAL ),   // 1s
  HI_UINT16( DEFAULT_DESIRED_MAX_CONN_INTERVAL ),

  // Tx power level
  0x02,     // length of this data
  GAP_ADTYPE_POWER_LEVEL,
  0         // 0dBm
};
static uint8 scanRspDatamac[] =
{
  // complete name
  0x0D,   // length of this data
  GAP_ADTYPE_LOCAL_NAME_COMPLETE,   //%e6%99%ba%e8%83%bd%e4%ba%91%e9%94%81 -“智能云锁”URL编码值
  0xe6,//c
  0x99,//8
  0xba,//f
  0xe8,//d
  0x83,//1
  0xbd,//9
  0xe4,//0
  0xba,//c
  0x91,//1
  0xe9,//7
  0x94,//5
  0x81,//8
  // connection interval range
  0x05,     // length of this data
  GAP_ADTYPE_SLAVE_CONN_INTERVAL_RANGE,
  LO_UINT16( DEFAULT_DESIRED_MIN_CONN_INTERVAL ),   // 100ms
  HI_UINT16( DEFAULT_DESIRED_MIN_CONN_INTERVAL ),
  LO_UINT16( DEFAULT_DESIRED_MAX_CONN_INTERVAL ),   // 1s
  HI_UINT16( DEFAULT_DESIRED_MAX_CONN_INTERVAL ),

  // Tx power level
  0x02,     // length of this data 
  GAP_ADTYPE_POWER_LEVEL,
  0         // 0dBm
};
// GAP - Advertisement data (max size = 31 bytes, though this is
// best kept short to conserve power while advertisting)
#define ADVERT_MAC_ADDR         13
static uint8 advertData[] =
{
  0x02,   // length of this data
  GAP_ADTYPE_FLAGS,
  DEFAULT_DISCOVERABLE_MODE | GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED,

  // service UUID, to notify central devices what services are included
  // in this peripheral
  0x05,   // length of this data
          // some of the UUID's, but not all
  GAP_ADTYPE_16BIT_MORE,  
          //0x02
  //LO_UINT16( WECHAT_SERV_UUID ),
  //HI_UINT16( WECHAT_SERV_UUID ),
  0,
  0,
  LO_UINT16( SIMPLEPROFILE_SERV_UUID ),
  HI_UINT16( SIMPLEPROFILE_SERV_UUID ),
  0x09, // length of this data (9 Bytes )
  GAP_ADTYPE_MANUFACTURER_SPECIFIC, //0xFF
  /*TI Pre-Amble*/
  0x0D,
  0x00,
  /*Device MAC (6 Bytes)*/
  0,0,0,0,0,0
};

// GAP GATT Attributes
static uint8 attDeviceName[GAP_DEVICE_NAME_LEN] = "Simple BLE Peripheral";
extern gapBondRec_t bonds[10];
extern void HalUARTClose ( uint8 port );

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static void simpleBLEPeripheral_ProcessOSALMsg( osal_event_hdr_t *pMsg );
//static   bStatus_t uartoadImgBlockWrite( void);
static void peripheralStateNotificationCB( gaprole_States_t newState );
//static void performPeriodicTask( void );
static void simpleProfileChangeCB( uint8 paramID );
//static void NpiSerialCallback( uint8 port, uint8 events );
//static uint8 PasscodeV(uint32  passcode);
static void wechatIOTGetMacAddrUpdatetoAdvertData( void ); 
// Application states
enum
{
  BLE_STATE_IDLE,
  BLE_STATE_CONNECTED,
};

// Application state
static uint8 simpleBLEState = BLE_STATE_IDLE;
static uint8 gPairStatus=0; /*用来管理当前的状态，如果密码不正确，立即取消连接，0表示未配对，1表示已配对*/
void ProcessPasscodeCB(uint8 *deviceAddr,uint16 connectionHandle,uint8 uiInputs,uint8 uiOutputs );
static void ProcessPairStateCB( uint16 connHandle, uint8 state, uint8 status );
static void simpleBLEPeripheral_HandleKeys( uint8 shift, uint8 keys );
#if defined( CC2540_MINIDK )
static void simpleBLEPeripheral_HandleKeys( uint8 shift, uint8 keys );
#endif

#if (defined HAL_LCD) && (HAL_LCD == TRUE)
static char *bdAddr2Str ( uint8 *pAddr );
#endif // (defined HAL_LCD) && (HAL_LCD == TRUE)

typedef unsigned char uchar;
typedef unsigned int  uint;

#define UART0_RX    1
#define UART0_TX    2
#define SIZE       51

bool  kchange = false; 
uint8 ownAddress[B_ADDR_LEN] = {0,0,0,0,0,0};
uint8 getfinger=0;
uint8 nfcs=0;                      // NFC步骤
uint8 localerror=0;
uint8 fpid[2]={0};
uint8 index[2]={11,11};                 //第一位0：用户，1：指纹
uint8 voiceb = 0;                   // 置一 则不允许再报声音
uint8 NC = 0;                       // 置一 则不允许修改接收数组值
uint8 MPR121 = 1;                   // 默认是采用的触控芯片是MPR121
uint8 k1 = 0;
uint8 LocalPro = 0;                 //本地锁操作状态标志
uint8 i = 0;                        // 接收开门包/配网包
uint8 Uart_Colsed = 0;
uint8 kpress = 0;
//uint8 a=0;
uint8 fpprocess = 0;                // 指纹正在处理标志位为一表示正在处理不接收其他命令
uint8 fpress = 0;
char RxBuf;
char UartState;
uchar count;
//char RxData[SIZE];                // 存储发送字符串
uint8 deccryptedData[16];           // 解密后数据存储数组
uint8 KeyCode[31];                  // 按键值存储数组
uint8 UID[4];                       
uint8  k=0;                          // 按键计数
uint8 ble=0;                        // 蓝牙开门
uint8 wbtd=0;                        // 串口数据同步
uint8 Error=0;              
uint8_t FingerReceMes[10] = {'\0'}; // 接收指纹命令数组
uint8 wifiwp=0;                     // wifi唤醒
uint8 pressc=0;                     // 第几次按压 前两次都是唤醒蓝牙 后几次是按键
uint8 touch=0;                      // 第几次按指纹
uint8 MessageSum = 0;                 // 接收指纹数据字数计数
uint8_t fgbuffer[10] = {'\0'};      // 发送指纹命令
uint8_t fgbuffer1[32]={'\0'};      // 发送指纹特征值数组
uint8_t wifisenddata[40]={0};       // 接收wifi数据数组
uint8 FeaturnSum1 = 0;                // 特征值计数
uint8 MessageSum1 = 0;                // wifi发送字节数计数
uint8 rec=0;                          // wifi接收数据包计数
uint8 check=0;                        // 判断标志 
uint8 luru=0;                         // 判断是录入还是验证，录入则中断时不允许发送验证命令
//int fp_delete=0;                    // 判断一对一指纹删除过程 过程中不能校验=1 收到录指纹命令后=0
bool workmode=0;                    // 默认在工厂模式
bool wifisinglemode=0;                    // 默认在非单机模式
uint8 GetReData=0;
extern void Finger_Start(void);
extern void hp02_Finger_Config(void);
extern uint8 checkDL(void);
extern uint8 halPwrMgtMode;
static uint8_t flag_fp = 0; //现用于记录指纹个数
static uint16_t fp_rigister_userId_gl = 0; // 保存录指纹过程中录入的一个ID 用于在发生错误时删除改指纹

uint8 finalData[2][16] = {0}; // char7收到数据 存所有
uint8 char7Data[20] = {0}; // char7 收到加密数据20
uint8 char7DataSub[16] = {0}; //char7 收到数据16
bool mqttvoice=0; //是不是要报配网成功
uint8 fpvoice = 0;
unsigned char sanswer[2]={0};
uint8 need_update = 0;
uint8 nonate = 0; //no need update
uint8 fpadds = 0; //fp add success
uint8 stateOpen = 0;
uint8 sendSleep = 0; //send sleep to wifi
//传macID
uint8 TempMacAddress[B_ADDR_LEN];
void Send_Notify_EVT_With(uint8 char6[16]);

// 加密秘钥 16个字节=128bit
uint8 key1[16] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
uint8_t mac[6] = {0xD4,0xF5,0x13,0x78,0xE7,0x9D};
 /* 用户信息
**/
typedef struct _USER_t{
  char userid[11];                                                               // 11  手机号
  char kcode[6];                                                                 //6 开门密码
  uint8_t fp[5];                                                                // 5 对应指纹
} user_t;    
typedef struct _YLSLFlash{
   bool       factory; //1 为初始化完成
   bool       singleV;//1 为单机模式
 // sysControl_t  syscontorl_info;                                                // 27  
  //lock_t        lock_info;                                                      // 100    
   char          custom;
   bool          voice;
   user_t        user[10]; 
} slFlash_t;  
slFlash_t userd;
slFlash_t userdTemp;
user_t nowuser; //char5 的临时用户

 /* 状态信息
**/
typedef struct _SmartLockState_t{
  uint8 FPERROR_state;                                                               // 4 
  uint8 CODEERROR_state;                                                                // 4
} altState_t;                                                                    // 8
 altState_t  alert;
/* 20170511 minnow
**/
void WiFi_WakeUp()
{
  wakewifi = 0;
}

void WiFi_PowerDownIsAllowed()
{
  wakewifi = 1;
}

/* BLE 4分钟 休眠定时器
*  正常情况下不会用到
*  只有在WIFI掉电的情况下 蓝牙没有正常进入休眠状态才有可能进入
*  唤醒后开始计时
*  进入休眠事件后关闭
**/
#ifdef BLE_STANDBY_ENABLE
#define BLE_STANDBY_TIMER_MIN   4
static void BleStandbyTimerStart()
{
  osal_start_timerEx( simpleBLEPeripheral_TaskID, GoSleep_EVT, 120000 ); 
}

static void BleStandbyTimerStop()
{
  osal_stop_timerEx(simpleBLEPeripheral_TaskID, GoSleep_EVT);
}
#endif


/*********************************************************************
 * PROFILE CALLBACKS
 */

// GAP Role Callbacks
static gapRolesCBs_t simpleBLEPeripheral_PeripheralCBs =
{
  peripheralStateNotificationCB,  // Profile State Change Callbacks
  NULL                            // When a valid RSSI is read from controller (not used by application)
};


// GAP Bond Manager Callbacks
static gapBondCBs_t simpleBLEPeripheral_BondMgrCBs =
{
  ProcessPasscodeCB,                     // 密码回调
  ProcessPairStateCB                     // 绑定状态回调
};

// Simple GATT Profile Callbacks
static simpleProfileCBs_t simpleBLEPeripheral_SimpleProfileCBs =
{
  simpleProfileChangeCB    // Charactersitic value change callback
};

void AesLoadBlock( uint8 *ptr )
{
  uint8 i;

  /* Kick it off */
  AES_START();

  /* Copy block to encryption input register */
  for (i = 0; i < STATE_BLENGTH; i++)
  {
    ENCDI = *ptr++;
  }
}
/****************************************************************************
* 功    能: 生成随机字符串 
****************************************************************************/
void genRandomString(uint8 length)  
{  
    uint8 flag, i;    
    
    uint8_t* string = (uint8_t*)calloc(length, 1*sizeof(uint8_t));
  
    for (i = 0; i < length; i++)  
    {  
        uint16 send = rand()%60000;
        srand(send); 
        flag = rand() % 3;  
        switch (flag)  
        {  
            case 0:  
                string[i] = 'A' + rand() % 26;  
                break;  
            case 1:  
                string[i] = 'a' + rand() % 26;  
                break;  
            case 2:  
                string[i] = '0' + rand() % 10;  
                break;  
            default:  
                string[i] = 'x';  
                break;  
        }  
    }  
    
    memcpy(key1,string,16);
    
    if(string != NULL)
      free(string);
}  
/****************************************************************************
* 名    称: InitUart()
* 功    能: 串口初始化函数
* 入口参数: 无
* 出口参数: 无
****************************************************************************/
void InitUart(void)
{ 
  //P1.6 as UART1_TXD,P1.7 as UART1_RXD
  EA = 1;                   // ENABLE INTERRUPT
  P1SEL |= 0xC0;
  PERCFG |= 0x02;           // Uart1 mapped to Alternative 2
  P2SEL |= 0x20;            // Uart1 has priority to Timer3
  P2DIR &= ~0x80;           // P0优先作为UART0
  P2DIR |= 0x40;            // P0优先作为UART0
   
  U1CSR |= 0x80;            // 设置为UART方式
  U1GCR |= 11;				       
  U1BAUD |= 216;            // 波特率设为115200
  UTX1IF = 0;               // UART0 TX中断标志初始置位0
  U1CSR |= 0x40;            // 允许接收 
  IEN0 |= 0x88;             // 开总中断允许接收中断  
        
  PERCFG &= ~0x01;          // 外设控制寄存器 USART 0的IO位置:0为P0口位置1 
  P0SEL = 0x0c;             // P0_2,P0_3用作串口（外设功能）
  P2DIR &= ~0xC0;           // P0优先作为UART0
    
  U0CSR |= 0x80;            // 设置为UART方式
  U0GCR |= 11;				       
  U0BAUD |= 216;            // 波特率设为115200
  UTX0IF = 0;               // UART0 TX中断标志初始置位0
  U0CSR |= 0x40;            // 允许接收 
  IEN0 |= 0x84;             // 开总中断允许接收中断  
}

/****************************************************************************
* 名    称: UartSendString()
* 功    能: 串口1发送函数
* 入口参数: Data:发送缓冲区   len:发送长度
* 出口参数: 无
****************************************************************************/
void UartSendString( char *Data, uint8 len)
{
  uint8 i;
  uint16 js=0;
  for(i=0; i<len; i++)
  {
    U1DBUF = *Data++;
    while((UTX1IF == 0))
    {
      js++;
      if(js>=10000)
        break;
    }
    UTX1IF = 0;
  }
}

/****************************************************************************
* 名    称: UartSendString()
* 功    能: 串口0发送函数
* 入口参数: Data:发送缓冲区   len:发送长度
* 出口参数: 无
****************************************************************************/
void UartSendString1( unsigned char *Data, uint8 len)
{
  uint8 i;
  uint16 js=0;
  for(i=0; i<len; i++)
  {
    U0DBUF = *Data++;
    while(UTX0IF == 0)
    {
      js++;
       if(js>=10000)
        break;
    }
    UTX0IF = 0;
  }
 // U0DBUF=0x00;
}

/**
  * @brief 蓝牙命令写入wifi模块
  * @param type 命令代码
  * @param p1   参数高八位
  * @param p2	低八位
  * @param ack	命令返回｛0：正确；1：失败｝
  * @retval None
  */
void BLE_Cmd(uint8_t Type, uint8_t p1, uint8_t p2, uint8_t ack)
{
  uint8 i = 0;
  char cmd=0;
  uint8_t sendData[9] = {'\0'};
  sendData[0] = 0xF5;
  sendData[1] = Type;
  sendData[2] = p1;
  sendData[3] = p2;
  sendData[4] = ack;
  for(i=1;i<5;i++)
    sendData[5] ^= sendData[i];
  sendData[6] = 0xF6;

  for(i=0;i<7;i++)
  {
    cmd=sendData[i];   
    UartSendString(&cmd,1);
  }
}
/**
延迟函数用来播放重复语音
*/
void delay5ms() 
{
   for(uint8 l=0;l<5;l++)
   {
     WAIT_MS();
   }
}
/**
  * @brief 开门命令
  * @param 命令代码
  * @param 参数高八位
  * @param 低八位
  * @retval None
  */
void OpenCmd(uint8 type, uint8 status)
{
  uint8 signal = 0;
  uint8 signals = 0;
  signal = type;
  signals = status;
  
  switch(signal)
  {
   case 0x01:
     {
       if(signals==0)
       {
        BLE_Cmd(0x22,0,0,0);  //蓝牙开门成功
       }
       else
       {
         BLE_Cmd(0x23,0,0,0); //蓝牙开门失败
       }
     }
     break;
     
   case 0x02:
     {
       if(signals==0)
       {
         BLE_Cmd(0x24,0,0,0); //指纹开门成功
       }
       else
       {
         BLE_Cmd(0x25,0,0,1); //指纹开门失败
       }
     }
     break;
     
   case 0x03:
     {
       if(signals==0)
       {
         BLE_Cmd(0x26,0,0,0); //密码开门成功
       }else
       {
         BLE_Cmd(0x27,0,0,0); //密码开门失败
       }
     }
     break;
  }
}

/**
  * @brief 蓝牙数据写入wifi模块
  * @param 命令代码
  * @param 参数高八位
  * @param 低八位
  * @retval None
  */
void BLE_Data(uint8_t Type, uint8_t L1, uint8_t L2, uint8 *buffer)
{
  uint16 i = 0;
  char cmd = 0;
  uint16 num = L1<<8|L2;
  uint16_t sendData[504] = {'\0'};
  sendData[0] = 0xF5;
  sendData[1] = Type;
  sendData[2] = L1;
  sendData[3] = L2;
  
  if(num == 0)
  {
    sendData[4] = *buffer;
    for(i=1;i<4;i++)
      sendData[5] ^= sendData[i];
    sendData[6] = 0xF6;
    for(i=0;i<7;i++)
    {
      cmd=sendData[i];
      UartSendString(&cmd,1);
    }
  }
  else 
  {
    for(uint16 i=0;i<num;i++)
    {
      sendData[4+i] =*buffer++;
    }
    for(i=1;i<num+4;i++)
      sendData[num+4] ^= sendData[i];
    sendData[num+5] = 0xF6;
    
    for(i=0;i<num+6;i++)
    {
      cmd=sendData[i];
      UartSendString(&cmd,1);
    }
  }
}

/*
 * @fn      SimpleBLEPeripheral_Init
 *
 * @brief   Initialization function for the Simple BLE Peripheral App Task.
 *          This is called during initialization and should contain
 *          any application specific initialization (ie. hardware
 *          initialization/setup, table initialization, power up
 *          notificaiton ... ).
 *
 * @param   task_id - the ID assigned by OSAL.  This ID should be
 *                    used to send messages and set timers.
 *
 * @return  none
 */
void SimpleBLEPeripheral_Init( uint8 task_id )
{
  simpleBLEPeripheral_TaskID = task_id;
  
  // 串口初始化 波特率默认是115200, 形参是回调函数
  // NPI_InitTransport(NpiSerialCallback);
  // Setup the GAP
  VOID GAP_SetParamValue( TGAP_CONN_PAUSE_PERIPHERAL, DEFAULT_CONN_PAUSE_PERIPHERAL );
  
  // Setup the GAP Peripheral Role Profile
  {
    #if defined( CC2540_MINIDK )
      // For the CC2540DK-MINI keyfob, device doesn't start advertising until button is pressed
      uint8 initial_advertising_enable = FALSE;
    #else
      // For other hardware platforms, device starts advertising upon initialization
      uint8 initial_advertising_enable = FALSE;//TRUE;
    #endif

    // By setting this to zero, the device will go into the waiting state after
    // being discoverable for 30.72 second, and will not being advertising again
    // until the enabler is set back to TRUE
    uint16 gapRole_AdvertOffTime = 0;

    uint8 enable_update_request = DEFAULT_ENABLE_UPDATE_REQUEST;
    uint16 desired_min_interval = DEFAULT_DESIRED_MIN_CONN_INTERVAL;
    uint16 desired_max_interval = DEFAULT_DESIRED_MAX_CONN_INTERVAL;
    uint16 desired_slave_latency = DEFAULT_DESIRED_SLAVE_LATENCY;
    uint16 desired_conn_timeout = DEFAULT_DESIRED_CONN_TIMEOUT;

    // Set the GAP Role Parameters
    GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &initial_advertising_enable );
    GAPRole_SetParameter( GAPROLE_ADVERT_OFF_TIME, sizeof( uint16 ), &gapRole_AdvertOffTime );

    GAPRole_SetParameter( GAPROLE_SCAN_RSP_DATA, sizeof ( scanRspData ), scanRspData );
    GAPRole_SetParameter( GAPROLE_ADVERT_DATA, sizeof( advertData ), advertData );

    GAPRole_SetParameter( GAPROLE_PARAM_UPDATE_ENABLE, sizeof( uint8 ), &enable_update_request );
    GAPRole_SetParameter( GAPROLE_MIN_CONN_INTERVAL, sizeof( uint16 ), &desired_min_interval );
    GAPRole_SetParameter( GAPROLE_MAX_CONN_INTERVAL, sizeof( uint16 ), &desired_max_interval );
    GAPRole_SetParameter( GAPROLE_SLAVE_LATENCY, sizeof( uint16 ), &desired_slave_latency );
    GAPRole_SetParameter( GAPROLE_TIMEOUT_MULTIPLIER, sizeof( uint16 ), &desired_conn_timeout );
  }

  // Set the GAP Characteristics
  GGS_SetParameter( GGS_DEVICE_NAME_ATT, GAP_DEVICE_NAME_LEN, attDeviceName );

  // Set advertising interval
  {
    uint16 advInt = DEFAULT_ADVERTISING_INTERVAL;

    GAP_SetParamValue( TGAP_LIM_DISC_ADV_INT_MIN, advInt );
    GAP_SetParamValue( TGAP_LIM_DISC_ADV_INT_MAX, advInt );
    GAP_SetParamValue( TGAP_GEN_DISC_ADV_INT_MIN, advInt );
    GAP_SetParamValue( TGAP_GEN_DISC_ADV_INT_MAX, advInt );
	
  }

  //下面是与配对相关的设置  
  // Setup the GAP Bond Manager
  { 
    uint32 passkey = 0; // passkey "000000"
    
    uint8 pairMode = GAPBOND_PAIRING_MODE_WAIT_FOR_REQ;//GAPBOND_PAIRING_MODE_INITIATE;
    if ( osal_isbufset( bonds[0].publicAddr, 0xFF, B_ADDR_LEN ) == FALSE )
      pairMode = GAPBOND_PAIRING_MODE_WAIT_FOR_REQ;
    
    uint8 mitm = TRUE;
    uint8 ioCap = GAPBOND_IO_CAP_DISPLAY_ONLY;  //显示密码， 以便主机输入配对的密码
    uint8 bonding = TRUE;
    GAPBondMgr_SetParameter( GAPBOND_DEFAULT_PASSCODE, sizeof ( uint32 ), &passkey );
    GAPBondMgr_SetParameter( GAPBOND_PAIRING_MODE, sizeof ( uint8 ), &pairMode );
    GAPBondMgr_SetParameter( GAPBOND_MITM_PROTECTION, sizeof ( uint8 ), &mitm );
    GAPBondMgr_SetParameter( GAPBOND_IO_CAPABILITIES, sizeof ( uint8 ), &ioCap );
    GAPBondMgr_SetParameter( GAPBOND_BONDING_ENABLED, sizeof ( uint8 ), &bonding );
  }

  // Initialize GATT attributes
  GGS_AddService( GATT_ALL_SERVICES );            // GAP
  GATTServApp_AddService( GATT_ALL_SERVICES );    // GATT attributes
  DevInfo_AddService();                           // Device Information Service
  SimpleProfile_AddService( GATT_ALL_SERVICES );  // Simple GATT Profile
  // Setup the SimpleProfile Characteristic Values
  {
    uint8 charValue1 = 1;
    uint8 charValue2 = 2;
    uint8 charValue3 = 3;
    uint8 charValue4 = 4;
    SimpleProfile_SetParameter( SIMPLEPROFILE_CHAR1, sizeof ( uint8 ), &charValue1 );
    SimpleProfile_SetParameter( SIMPLEPROFILE_CHAR2, sizeof ( uint8 ), &charValue2 );
    SimpleProfile_SetParameter( SIMPLEPROFILE_CHAR3, sizeof ( uint8 ), &charValue3 );
    SimpleProfile_SetParameter( SIMPLEPROFILE_CHAR4, sizeof ( uint8 ), &charValue4 );
    //SimpleProfile_SetParameter( SIMPLEPROFILE_CHAR5, SIMPLEPROFILE_CHAR5_LEN, charValue5 );
    //uint8 TempMacAddress[B_ADDR_LEN];
    TempMacAddress[5]=XREG(0x780E);                 // 直接指向指针内容  
    TempMacAddress[4]=XREG(0x780F);  
    TempMacAddress[3]=XREG(0x7810);  
    TempMacAddress[2]=XREG(0x7811);                 // define 函数直接读出数据  
    TempMacAddress[1]=XREG(0x7812);  
    TempMacAddress[0]=XREG(0x7813);                                                                                                                                                                                                                                                                                                                                                                                                                                                                                
    SimpleProfile_SetParameter( SIMPLEPROFILE_CHAR5, 6, TempMacAddress );
  }
  
  hp02_Finger_Config();                                   
  Finger_Start();
  RegisterForKeys( simpleBLEPeripheral_TaskID );
  HalLedSet( (HAL_LED_1 | HAL_LED_2), HAL_LED_MODE_OFF );
  VOID SimpleProfile_RegisterAppCBs( &simpleBLEPeripheral_SimpleProfileCBs );
  //HCI_EXT_ClkDivOnHaltCmd( HCI_EXT_ENABLE_CLK_DIVIDE_ON_HALT );  关闭防止干扰串口
  HCI_EXT_HaltDuringRfCmd(HCI_EXT_HALT_DURING_RF_DISABLE);
   
  //初始化默认外设电源设置以及引脚状态
  /* DIR
  * 0：input 
  * 1: output
  **/
  P1DIR|=0x1e;      // 0001 1110
  P2DIR|=0x01;      // 0000 0001
  
  keylight=0;       // 关闭背光灯关闭键盘
  klpower=1;        // 背光灯电源打开
  
  /* INP : input mode
  * 0: Pullup
  * 1: Pulldown
  **/
  P0INP|=0x08;      // 0000 1000        // ?
  
  fingerpw=0;       //指纹供电关闭
  fingerth=0;       //确保指纹触摸监测一直在工作

  WiFi_PowerDownIsAllowed();
  
  AT080init();
 
  P1DIR&=~0x02;     //防止背光灯使能引脚无故被拉高
  
  // 开始 SBP_START_DEVICE_EVT 事件
  osal_set_event( simpleBLEPeripheral_TaskID, SBP_START_DEVICE_EVT );
}

void AesStartBlock( uint8 *out, uint8 *in )
{
  uint8 i;

  /* Kick it off */
  AES_START();

  /* Copy data to encryption input register */
  for (i = 0; i < STATE_BLENGTH; i++)
  {
    ENCDI = *in++;
  }

  /* Delay is required for non-DMA AES */
  HAL_AES_DELAY();

  /* Copy encryption output register to out */
  for (i = 0; i < STATE_BLENGTH; i++)
  {
    *out++ = ENCDO;
  }
}

/*********************************************************************
 * @fn      SimpleBLEPeripheral_ProcessEvent
 *
 * @brief   Simple BLE Peripheral Application Task event processor.  This function
 *          is called to process all events for the task.  Events
 *          include timers, messages and any other user defined events.
 *
 * @param   task_id  - The OSAL assigned task ID.
 * @param   events - events to process.  This is a bit map and can
 *                   contain more than one event.
 *
 * @return  events not processed
 */
uint16 SimpleBLEPeripheral_ProcessEvent( uint8 task_id, uint16 events )
{
  VOID task_id; // OSAL required parameter that isn't used in this function
  
  /* SYS_EVENT_MSG (0x8000)
  * messaging between tasks.
  **/
  
  if ( events & SYS_EVENT_MSG )
  {
    uint8 *pMsg;

    if ( (pMsg = osal_msg_receive( simpleBLEPeripheral_TaskID )) != NULL )
    {
      simpleBLEPeripheral_ProcessOSALMsg( (osal_event_hdr_t *)pMsg );
      // Release the OSAL message
      VOID osal_msg_deallocate( pMsg );
    }
    // return unprocessed events
    return (events ^ SYS_EVENT_MSG);
  }

  /* SBP_START_DEVICE_EVT (0x0001)
  * indicating the initial device start is complete, the application processing begin. 
  *
  **/
  if ( events & SBP_START_DEVICE_EVT )
  {
    // Start the Device. set up the GAP function. Generic Access Profile.
    VOID GAPRole_StartDevice( &simpleBLEPeripheral_PeripheralCBs );    
    // Start Bond Manager
    VOID GAPBondMgr_Register( &simpleBLEPeripheral_BondMgrCBs );

    /*
    uint8 change = 0;
    int ret;
    ret = osal_snv_read( 0x80,1, &change );
    if(ret==0x0a)   //未被改变过 首次读但是里面右没写过会返回0x0a
    {
      osal_pwrmgr_device( PWRMGR_ALWAYS_ON );   //关闭低功耗使能32M时钟
      uint8 ps[6]={5,5,5,5,5,5};
      osal_snv_write( 0x80,6, ps );
      osal_snv_read( 0x80,(uint8)sizeof( slFlash_t ), &userd );
    }
    */
    osal_pwrmgr_device( PWRMGR_ALWAYS_ON );     //关闭低功耗使能32M时钟
    HalI2CInit(0x5a, i2cClock_267KHZ);
    mpr121QuickConfig();
    
    uint8 read1[94]={0};
    ReadTouch121(read1);    //读取按键寄存器
    if(read1[93]!=0x04)     //不是MPR121芯片
    {
      HalI2CInit(0x56, i2cClock_267KHZ);  //是APT8L16
      APT8L16QuickConfig();
      MPR121=0;
    }
    InitUart();
    
    
 /*   PN532WakeUp();
    delay5ms();
    SearchIC();
    delay5ms();
    uint8_t read[20];
    PN532_R( read ,20);
    BLE_Data(0x70,0,20,read); //发送给wifi新密码*/
     //PlayVoiceP1(18);
    /*uint8 TempMacAddress[B_ADDR_LEN];
    TempMacAddress[5]=XREG(0x780E);       // 直接指向指针内容  
    TempMacAddress[4]=XREG(0x780F);  
    TempMacAddress[3]=XREG(0x7810);  
    TempMacAddress[2]=XREG(0x7811);       // define 函数直接读出数据  
    TempMacAddress[1]=XREG(0x7812);  
    TempMacAddress[0]=XREG(0x7813);
    //获取蓝牙地址发送给wifi
    BLE_Cmd(0x01,0,6,0);
    BLE_Data(0x01,0,0x06, TempMacAddress);*/
    
    /*uint8 key2[16] ={0xce,0xa5,0xd5,0x4a,0xcb,0x37,0x29,0x4e,0xaf,0x66,0xbb,0x18,0xc1,0x56,0x03,0xca};//"adudashabi666666"; //{1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6};
    uint8 key[16] = "fourw.zeepson.cn"; 
    uint8 iv[16]  = "sanw.zeepson.com";
    //LL_Encrypt( key, key2, key2 );  
    HalAesInit();
    //AES_SETMODE(CBC);
    //AesLoadKey( key );
    //AesLoadIV(iv);
    //AesDmaSetup( key2, STATE_BLENGTH, key2, STATE_BLENGTH );
    //AES_SET_ENCR_DECR_KEY_IV( AES_ENCRYPT );  
    //AES_START();
    AES_SETMODE(CBC);
    AesLoadKey( key );
    AesLoadIV(iv);
    AesDmaSetup( key2, STATE_BLENGTH, key2, STATE_BLENGTH );
    AES_SET_ENCR_DECR_KEY_IV( AES_DECRYPT );  
    AES_START();
    AES_SETMODE(CBC);
    AesLoadKey( key );
    AesLoadIV(iv);
    AesDmaSetup( key2, STATE_BLENGTH, key2, STATE_BLENGTH );
    AES_SET_ENCR_DECR_KEY_IV( AES_ENCRYPT );  
    AES_START();*/

   //osal_pwrmgr_device( PWRMGR_BATTERY ); //使能低功耗
    
    // 上电后 进入低功耗逻辑 100ms后打开低功耗时间待定
    osal_start_timerEx( simpleBLEPeripheral_TaskID, GoSleep_EVT, 100 ); 
    /* Uart_Colsed=1;    // 不允许自动进入低功耗
     fingerpw=1;       //指纹供电关闭
     delay5ms();
     delay5ms();
     PN532WakeUp();*/
    return ( events ^ SBP_START_DEVICE_EVT );
  }

  if ( events & SBP_PERIODIC_EVT )
  { 
    static uint8_t times = 0;
    // 周期性调度 用于打开蓝牙广播 同时向WIFI查询是否可以控制掉电管脚
    if( gapProfileState != GAPROLE_CONNECTED )
    {
      uint8 current_adv_enabled_status;
      uint8 new_adv_enabled_status;

      // Find the current GAP advertisement status
      GAPRole_GetParameter( GAPROLE_ADVERT_ENABLED, &current_adv_enabled_status );

      if( current_adv_enabled_status == FALSE )
      {
        new_adv_enabled_status = TRUE;
        // change the GAP advertisement status to opposite of current status
        GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &new_adv_enabled_status );
      }
    }
    times++;
    if(times >= 15){
      wbtd++;
      if( wbtd >= 40)
      {
       
        if(0!=luru)
        {
          wbtd=0;
        }
        if((wbtd>=2)&&0==GetReData)//30s未收到回复认为wifi休眠ble自行进入休眠
        {
          // WiFi_PowerDownIsAllowed();
          // osal_pwrmgr_device( PWRMGR_BATTERY );                          //使能低功耗
         // 停止周期查询定时器
          // osal_stop_timerEx(simpleBLEPeripheral_TaskID, SBP_PERIODIC_EVT);
          //这里不理的情况下会去休眠
          //osal_start_timerEx(simpleBLEPeripheral_TaskID, GoSleep_EVT, 400);
        }else
        {
          GetReData=0;
          //BLE_Cmd(0x08,0,0,0);
          times = 0;
        }
      }
       times = 0;
    }
    
    return (events ^ SBP_PERIODIC_EVT);
  }
  
  
  
  if ( events & Datacleanfx_EVT )
  {
   // BLE_Cmd(0x29,0,0,51);
    osal_snv_write( 0xfa,(uint8)sizeof(slFlash_t), (uint8 *)&userd );
    return (events ^ Datacleanfx_EVT);
  }
  

  
  if ( events & wawf_EVT )
  {
    osal_snv_write( 0xfa,(uint8)sizeof(slFlash_t), (uint8 *)&userdTemp );
    memset(&userdTemp,0,sizeof(userdTemp)); 
    memset(&userd,0,sizeof(userd));
    osal_snv_read(0xfa,(uint8)sizeof(slFlash_t), (uint8 *)&userd);
    return (events ^ wawf_EVT);
  }
   if ( events & ClearLock_EVT )
  {
    if(flag_fp == 13){
        
      FingerUser_DeleteAll(); 
      BLE_Cmd(0x44,0,0,0);//恢复出厂
    } else if(flag_fp == 15){
      
      FingerUser_DeleteAll(); 
    } 
    alert.CODEERROR_state=0;
    alert.FPERROR_state=0;
    flag_fp = 0;
    return (events ^ ClearLock_EVT);
  }
  
  /*
  指纹超时没返回就重启
  */
  if ( events & SBP_PERIODIC_EVT1 )
  { 
    //HAL_SYSTEM_RESET();
    ChangeKey();  //给手机传输新密钥   
    return (events ^ SBP_PERIODIC_EVT1);
  }
  
 
  //清空
  if(events & Delay_Check_EVT){
    
    if(voiceb == 1){
     //关掉中断
      voiceb = 0;
      P1IEN &= ~0x21; //控制密码中断，这里才会起作用
    }
   // HAL_SYSTEM_RESET(); //录完指纹重启
    
    uint8 char6[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    SimpleProfile_GetParameter( SIMPLEPROFILE_CHAR6, &char6);
    if(char6[5] == 0x30 && char6[0] == 0x33){
      
      if(sendSleep == 0){
        
        BLE_Cmd(0x0C,0,0,1);
      } else {
        BLE_Cmd(0x0C,0,0,0);
      }
    }
    memset(char6,0,sizeof(char6));//清空特征值数组
    SimpleProfile_SetParameter( SIMPLEPROFILE_CHAR6, 16, char6);
   
    //测试语音
    //osal_start_reload_timer( simpleBLEPeripheral_TaskID, Delay_Check_EVT, 5000);
    //static uint8 avoicep = 0x00;
    //PlayVoiceP1(avoicep);
    //avoicep += 0x01;
    return (events ^ Delay_Check_EVT);
  }
  if ( events & GoSleep_EVT   )
  {
    
    BLE_Cmd(0x07,0,0,1);//休眠
    delay5ms();
    osal_stop_timerEx(simpleBLEPeripheral_TaskID, SBP_PERIODIC_EVT);
                             
    osal_stop_timerEx(simpleBLEPeripheral_TaskID, FPDatafx_EVT);
    osal_stop_timerEx(simpleBLEPeripheral_TaskID, FPTIMEOUT_EVT);
    osal_stop_timerEx(simpleBLEPeripheral_TaskID, Delay_Check_EVT);
    
#ifdef BLE_STANDBY_ENABLE
    BleStandbyTimerStop();
#endif
     if(alert.FPERROR_state<5)
    {
       alert.FPERROR_state =0;
    }
    if(alert.CODEERROR_state<5)
    {
      alert.CODEERROR_state =0;
    }
    sendSleep = 0;//sleep 1
    fpadds = 0;//这次可以重新录指纹
    wbtd=0;
    luru=0;
    stateOpen=0;
    LocalPro=0;
    getfinger=0;
    fpress=0;   
    k=0;            //按键清零
    voiceb=0;
    nfcs=0;
    pressc=0;
    keylight=0;     //关闭背光灯关闭键盘？   
    P1DIR&=~0x02;   //防止背光灯使能引脚无故被拉高
    touch=0;        // 记录指纹头被按压次数，在进入休眠前，清空
    klpower=0;      //背光灯电源关闭
    IEN0&=~0x0c;
    U0DBUF=0x00;
    U0UCR|=0x80;
    U1UCR|=0x80;
    P0SEL &= ~0x0c; //P0_2,P0_3用作串口（外设功能）
    P0DIR&=~0x0c;
    P0INP|=0x08;    //将串口TX置位高阻态防止其馈电导致指纹芯片无法正常唤醒
    kchange = false;
    fingerpw=0;     //指纹供电关闭
    fingerth=1;      // 确保指纹触摸监测一直在工作 0
    P2DIR|=0x01;    //wakewifi设为输出
    FeaturnSum1 = 0;
    fpprocess=0;
    WiFi_PowerDownIsAllowed();
    localerror=0;
    
    memset(fgbuffer1,0,sizeof(fgbuffer1));//清空特征值数组
    memset(fgbuffer,0,sizeof(fgbuffer)); //清空指纹返回数据数组
    memset(wifisenddata,0,sizeof(wifisenddata));//清空wifi发送的数据数组    
    memset(KeyCode,0,sizeof(KeyCode));
    if((MPR121==1)&&kpress==1)
    {
      mpr121QuickConfig();//重新配置iic
      kpress=0;
    }else if((MPR121==0)&&kpress==1)
    {
      APT8L16QuickConfig();
      kpress=0;
    }
   
   /* P0INP|=0xFF; //    
    P1INP|=0xEA; //将串口TX置位高阻态防止其馈电导致指纹芯片无法正常唤醒
    P2INP|=0X0E;*/
    P0SEL = 0x00;

    P0DIR = 0x0C;
 
    P0 = 0x00;
 
    P0INP = 0x08;

 
    P1SEL = 0x00;
  
    P1DIR = 0xDE;
  
    P1 = 0x05;
 
    P1INP = 0x00;


    P2SEL = 0xF8;
 
   P2DIR = 0x01;
  
    P2 = 0x01;
 
    P2INP = 0x00;
    
    Uart_Colsed=0;//允许进入低功耗
     // advertising on and off
    if( gapProfileState == GAPROLE_CONNECTED )
    {
      GAPRole_TerminateConnection();  // 终止连接
      uint8 current_adv_enabled_status;
      uint8 new_adv_enabled_status;

      //Find the current GAP advertisement status
      GAPRole_GetParameter( GAPROLE_ADVERT_ENABLED, &current_adv_enabled_status );

      if( current_adv_enabled_status == TRUE )
      {
        new_adv_enabled_status = FALSE;
        //change the GAP advertisement status to opposite of current status
        GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &new_adv_enabled_status );
      }
      new_adv_enabled_status = FALSE;
      //change the GAP advertisement status to opposite of current status
      GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &new_adv_enabled_status );
    }
    else
    {
      uint8 current_adv_enabled_status;
      uint8 new_adv_enabled_status;
      //Find the current GAP advertisement status
      GAPRole_GetParameter( GAPROLE_ADVERT_ENABLED, &current_adv_enabled_status );
      if( current_adv_enabled_status == TRUE )
      {
        new_adv_enabled_status = FALSE;
        //change the GAP advertisement status to opposite of current status
        GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &new_adv_enabled_status );
      }  
      new_adv_enabled_status = FALSE;
      //change the GAP advertisement status to opposite of current status
      GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &new_adv_enabled_status );   
    }
    
    // HAL_SYSTEM_RESET();
    // 休眠前 清理按键中断标志位
    P1IFG&=~0x21;
    P2IFG&=~0x01;
    // 修眠前 打开中断
    P1IEN |= 0x01;
    P1IEN |=0x20;   // 将指纹触摸中断打开
    P2IEN |= 0x01;
    P1DIR&=~0x20;   //防止背光灯使能引脚无故被拉高
    osal_pwrmgr_device( PWRMGR_BATTERY ); //使能低功耗
    return (events ^ GoSleep_EVT  );
  }
  
    if(events & FP_Voice_EVT){
   
       switch(fpvoice)
    {
    case 1:
      {
        PlayVoiceP1(Finger_RQ); 
        fpvoice = 2;
      }
      break;
    case 2:
      {
         //PlayVoiceP1(Finger_RQA);//KEY_VIOCE Finger_RQA
         fpvoice = 3;
      }
      break;
    case 3:
      {
        PlayVoiceP1(Finger_RQA);//KEY_VIOCE Finger_RQA
        fpvoice = 4;
      }
      break;
    case 4:
      {
        PlayVoiceP1(Finger_RQA);
        fpvoice = 0;
      }
      break;
    }
    return (events ^ FP_Voice_EVT);
    }
    
    if(events & FPDatafx_EVT)
  {
    switch(getfinger)
    {
    case 1:
      {
         User_Registration_2C2R(0x0000,0x01);
         //PlayVoiceP1(Finger_RQA);
         //PlayVoiceP1(KEY_VIOCE);
      }
      break;
    case 2:
      {
         User_Registration_3C3R(0x0000,0x01);
         //PlayVoiceP1(Finger_RQA);
         //PlayVoiceP1(KEY_VIOCE);
      }
      break;
    case 3:
      {
            fgbuffer1[1] = fgbuffer[2];
            fgbuffer1[2] = fgbuffer[3];
            fgbuffer1[31] = 1;
            BLE_Cmd(0x32,0x01,0xf2,0);
            BLE_Data(0x32,0x01,0xf2,fgbuffer1);
            FeaturnSum1=0;
            fgbuffer[9]=0;
            memset(fgbuffer1,0,sizeof(fgbuffer1));
            wifisenddata[0]=0;
      }
      break;
      case 4:
      {
         User_Registration_1C3R(0x0000,0x01);
         PlayVoiceP1(Finger_RQA);
      }
      break;
    }  
   
    return (events ^ FPDatafx_EVT  );
  }

  if(events & Wakeup_EVT1)
  {
    if(workmode==0 && wifisinglemode==0)//工厂模式
    {
      P1IEN&=~0x21;
      delay5ms();
      BLE_Cmd(0x20,0,0,0); //工厂模式直接开门
    }else
    {
       if((alert.FPERROR_state>=5)&&((LocalPro%10)!=2))
     {
        PlayVoiceP1(Finger_LOCK);//指纹锁定
        P1IEN&=~0x21;  //关闭指纹中断
        delay5ms();
        BLE_Cmd(0x0d,0,0,0);//通知wifi休眠
        
     }else
     {
       memset(fgbuffer,0,sizeof(fgbuffer));    //修改与2017/2/27 待测试 用于防止串口数据有其他干扰导致接收异常不处理
       User_FingerOneToAll();                  //每次第一次唤醒都对比一次指纹
     }
    }
    return (events ^ Wakeup_EVT1  );
  }
  if(events & FPHSTIMEOUT_EVT)
  {
    P1IEN|=0x20;//打开指纹中断
    fpprocess=0;
    User_Registration_1C3R(0x0000,0x01); //发送注册指令注意：权限若为00则一直注册失败
           
    return (events ^ FPHSTIMEOUT_EVT  );
  }
   if(events & FPTIMEOUT_EVT)
  {
    if(LocalPro==0)
    {
      if(luru==2) 
      {  
         luru=0;
         BLE_Cmd(0x36,0,0,1);                        
         PlayVoiceP1(Finger_ADD_FAIL);
         if(fp_rigister_userId_gl != 0)
         User_DeleteSpecific(fp_rigister_userId_gl);
    
         //添加失败后去休眠
         Send_Notify_EVT_With("300001");
      }else if(luru==0){
        
        BLE_Cmd(0x07,0,0,1);//允许休眠
      }
    }else
    {
       PlayVoiceP1(VERIFY_FAIL);                         
       if(fp_rigister_userId_gl != 0)
       User_DeleteSpecific(fp_rigister_userId_gl); 
       memset(fpid,0,sizeof(fpid));
       if(LocalPro/100==1)
         {
           BLE_Cmd(0x72,0,0,1);
                          
          }else if(LocalPro/100==2)
           {
              BLE_Cmd(0x74,0,0,1);        
            }  
         kchange = false;
         delay5ms();
         delay5ms();
         localerror++;
         if(localerror<3)
          {
          PlayVoiceP1(Finger_RQ);     //按键音
          }
    }
    return (events ^ FPTIMEOUT_EVT  );
  }
if ( events & ClearFPLock_EVT )
  {
      alert.CODEERROR_state=0;
      alert.FPERROR_state=0;
      osal_start_timerEx(simpleBLEPeripheral_TaskID, GoSleep_EVT, 400);
    return (events ^ ClearFPLock_EVT);
  }
#if defined ( PLUS_BROADCASTER )
  if ( events & SBP_ADV_IN_CONNECTION_EVT )
  {
    uint8 turnOnAdv = TRUE;
    // Turn on advertising while in a connection
    GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &turnOnAdv );

    return (events ^ SBP_ADV_IN_CONNECTION_EVT);
  }
#endif // PLUS_BROADCASTER

  // Discard unknown events
  return 0;
}
/**************************
* function :ChangePasscode(void)
* brief :ChangePasscode
*
**************************/
void ChangePasscode(uint8 road)
{
    uint8 newpasscode[6];
      if(road==0x01)  //本次修改来自蓝牙
      {
        for(uint8 i=0;i<6;i++)
        {
           newpasscode[i]=deccryptedData[i+8];
        }
      }else if(road==0x02)  //本次修改来自wifi串口
      {
           for(uint8 i=0;i<6;i++)
        {
           newpasscode[i]=deccryptedData[i+8];
        }
      }
        osal_snv_write( 0x80,  6, newpasscode );//将新改变的密码写进snv
        BLE_Cmd(0x70,0,0,0);              
        BLE_Data(0x70,0,0x06,newpasscode); //发送给wifi新密码

}

/****************************************************************************
* 名    称: UART1_ISR(void) 串口中断处理函数 
* 描    述: 当串口1产生接收中断，将收到的数据保存在RxBuf中
****************************************************************************/
#pragma vector = URX1_VECTOR 
__interrupt void UART1_ISR(void) //和wifi通信
{ 
    char   RxBuf;
    URX1IF = 0;       // 清中断标志 
    RxBuf = U1DBUF;   
    uint8_t ucTemp;
    ucTemp = RxBuf;
    
    if(((ucTemp==0xf5)||FingerReceMes[0] ==0xf5)&&wifisenddata[0]==0 )//接收开始标志不是数据包执行下面语句
    {
       FingerReceMes[MessageSum] = ucTemp;
       MessageSum++;
    }
    if((wifisenddata[0]!=0)&&(wifisenddata[0]!=0x33)&&rec<=39)  //是数据包接收执行下面但是不是特征值
    {
      wifisenddata[rec+1]=ucTemp;
      rec++;
    }
    if(wifisenddata[0]==0x33)
    {
      if((NC==0)&&((ucTemp==0xf5)||fgbuffer1[0] ==0xf5))
      {
       fgbuffer1[rec] = ucTemp;
       rec++;
      }
       if(rec >= 255)
        {
          //此处没用
         flag_fp++;
         if(flag_fp >= 2)
           flag_fp = 0; 	   
         
           fgbuffer1[31] = 1;
           for(uint8 i=0;i<63;i++)
           {
              fgbuffer1[i] =  fgbuffer1[i+4];
           }
           NC=1;
           User_GetNoUseID();
           rec=0;
            
       }
	//else
	//rec++;
    
    }
    if((ucTemp==0xF6)&&(wifisenddata[0]!=0x33)) //接收结束标志且不是下传的特征值的第一个包
    {
      switch(wifisenddata[0])//判断数据包的命令来自那个
      {
        case 0x30:    //录入指纹
        {          
          rec=0;      //清零
          if(luru == 0)
          {
            luru=1;     //关闭中断验证指纹命令
            PlayVoiceP1(Finger_RQ);                //播放请按指纹
            //P1IEN&=~0x20;//播放语音时关掉指纹中断
            osal_start_timerEx(simpleBLEPeripheral_TaskID, FPHSTIMEOUT_EVT, 500);
            //fp_delete = 0;
            
          }
          
        }
        break;
        
        case 0x32:  //wifi获取指定特征值
        {
          uint16_t userID=wifisenddata[5]<<8|wifisenddata[6];
          User_GetID_FeatureValue(userID);
          rec=0;
        }
        break;
        
        case 0x33:  //wifi下发特征值
        {
           //uartoadImgBlockWrite( );
        }
        break;
        
        case 0x34:  //wifi删除指定用户指纹
        {
          uint16_t userID = 0;
          //fp_delete = 1;
          userID = (wifisenddata[5]<<8)|wifisenddata[6];
          if(userID!=0)
            User_DeleteSpecific(userID);
          rec=0;
        }
        break;
        
        case 0x36:   //一对一验证
        {
          if(luru ==1){
           luru=2;     //关闭中断验证指纹命令           
           PlayVoiceP1(Finger_RQA); //播放请再按指纹
           osal_start_timerEx(simpleBLEPeripheral_TaskID, FPTIMEOUT_EVT, 10000);
          }
          rec=0;

        }
        break;
         case 0x40:  //密码操作
        {
          uint8 ver=0;
          for(uint8 i=2;i<rec-1;i++)
          {
            ver ^= wifisenddata[i];

          }
          if(ver == wifisenddata[rec-1] && wifisenddata[rec] == 0xF6 && wifisenddata[1] == 0xF5 && wifisenddata[2] == 0x40 && luru==0)     
          {

            uint8 flagTemp = 0;
            for(uint8 i=16;i<22;i++){
            
              if(wifisenddata[i] == 0)
                flagTemp++;
            } 
            if(flagTemp == 6){

              static char ti=0;
              uint8 str=0;
              for(uint8 a=0;a<10;a++)
              {
               if(wifisenddata[a]==0x1B)
                {
                  str=a+1;
                  break;
                 }
              }
          //if(ti < userd.custom)
          {
            for(uint8 i=0;i<11;i++)
            {
              userdTemp.user[ti].userid[i]=wifisenddata[str+i];
            }
            
            for(uint8 f=0;f<5;f++)
            {
             // for(int f1=0;f1<2;f1++)
             // {
                userdTemp.user[ti].fp[f]=wifisenddata[str+17+f*2]<<8|wifisenddata[str+18+f*2];
             // }
            }
            ti++;
          }
          //接受完毕
          if(ti == userdTemp.custom)
          {
            ti=0;
            uint8 tag = 0;
            for(uint8 i=0;i<userdTemp.custom-1;i++){
              for(uint8 j=i+1;j<userdTemp.custom;j++){
              if(memcmp(userdTemp.user[i].userid, userdTemp.user[j].userid, 11) == 0){
                
                tag = 1;
               // BLE_Cmd(0x29,0,0,2);
                break;
              }
              }
            }
            
            if(1 != tag){
              
            if(userdTemp.custom > 0 && userdTemp.custom <= 10){
              
              userdTemp.factory=1;
              userdTemp.singleV=0;
              //userdTemp.custom=userd.custom;
              userdTemp.voice = userd.voice;
            } else {
              //这基本没啥用
              userdTemp.factory=0;
              userdTemp.singleV=0;
            }

            for(uint8 g=0; g<userdTemp.custom; g++){
             
              uint8 j=0;
              for(; j<10; j++){

                if(memcmp(userdTemp.user[g].userid, userd.user[j].userid, 11) == 0){
                 
                  for(uint8 l=0; l<6; l++){
                    userdTemp.user[g].kcode[l] = 0xFF; //000000
                    userdTemp.user[g].kcode[l] = userd.user[j].kcode[l];
                  }
                  for(uint8 n=0; n<5; n++){
                   
                    if(userdTemp.user[g].fp[n] != 0){
                      userdTemp.user[g].fp[n] = userd.user[j].fp[n];
                    } else {

                    if(userd.user[j].fp[n]!=0 && nonate != 1)
                        User_DeleteSpecific(userd.user[j].fp[n]);
                    }
                  }
                  break;
               }
              }
              if(j == 10){
                //同步的空密码
                 for(uint8 l=0; l<6; l++){
                    userdTemp.user[g].kcode[l] = 0xFF;
                  }
                // BLE_Cmd(0x29,0,0,0);
              }
            }
            //write
            if(luru==0 && nonate != 1)
            {
              osal_start_timerEx(simpleBLEPeripheral_TaskID, wawf_EVT, 0);
             // BLE_Cmd(0x29,0,0,1);
            }
            }
          }
            }//flagTemp
          }//ver==wifisenddata
          wifisenddata[0]=0;
          rec = 0;
        }
        break;
        case 0x70:  //密码操作
        {
          rec=0;
        }
        break;
      }
      
    if(wifisenddata[0]==0)  //不是数据包
    {
      //if(wbtd>1)
       //wbtd--;
      switch( FingerReceMes[1]) //判断命令
      {
        case 0x00: //锁信息
        {
        }
        break;
       
        case 0x01: //蓝牙mac信息
        {
          InitUart(); 
          /*
          uint8 TempMacAddress[B_ADDR_LEN];
          TempMacAddress[5]=XREG(0x780E); // 直接指向指针内容  
          TempMacAddress[4]=XREG(0x780F);  
          TempMacAddress[3]=XREG(0x7810);  
          TempMacAddress[2]=XREG(0x7811);                // define 函数直接读出数据  
          TempMacAddress[1]=XREG(0x7812);  
          TempMacAddress[0]=XREG(0x7813);  
          //获取蓝牙地址发送给wifi
          BLE_Cmd(0x01,0,6,0);
          BLE_Data(0x01,0,0x06,TempMacAddress);
          */
       }
       break;
       
       case 0x02: //wifi mac
       {
         //待添加
          FingerReceMes[0]=0;
       }
       break;
       
       case 0x03: // WIFI重启  不允许wifi掉电
       {
        // LocalPro=0;//这里清零可能导致本地添加的时候验证流程变成开门流程思考下去掉后又什么后果//20170814注释掉
         WiFi_WakeUp();
         P1IEN |= 0x21;
       }
       break;
       
       case 0x04: //wifi休眠
       {
         P1IEN&=~0x21;//即将进入休眠不允许任何中断干扰   20170612

         BLE_Cmd(0x04,0,0,0); //回wifi
         
         osal_start_timerEx(simpleBLEPeripheral_TaskID, GoSleep_EVT, 200);
       }
       break;
       
       case 0x05: //wifi唤醒
       {
        
         WiFi_PowerDownIsAllowed();

         GetReData=1;
       }
       break;
       
       case 0x06: //复位蓝牙
       {
         //  HAL_SYSTEM_RESET(); 
       }
       break;       
       
       case 0x07: //休眠蓝牙 //极限低电量关闭所有中断
       {
          //no use //swan//
         BLE_Cmd(0x07,0,0,0); 
       }
       break; 
       
       case 0x08: //蓝牙唤醒
       {
         FingerReceMes[0]=0;
       }
       break;
       
       case 0x09: //wifi完成事件通知
       {
         FingerReceMes[0]=0;
         if(FingerReceMes[4]==0){ //wifi成功
            
             //notify 出去
             Send_Notify_EVT_With("wifisuccess");
         } else if(FingerReceMes[4]==202){ //密码错误
           
           //notify 出去
           Send_Notify_EVT_With("wifipasswordfail");
         } else {
           Send_Notify_EVT_With("wififail");
         }
       }
       break;

       case 0x0B: //MQTT结果
       {
         if(FingerReceMes[4]!=0) //连接失败
         {
             //notify 出去
           if(mqttvoice==1){
             PlayVoiceP1(LINK_FAIL);
             mqttvoice=0;
             
             Send_Notify_EVT_With("mqttfail");
           }
         }
         else   //连接成功
         {
             //notify 出去
           
           if(mqttvoice==1){
            
             PlayVoiceP1(LINK_SUCCESS);
             mqttvoice=0;
             
             //notify 出去
             Send_Notify_EVT_With("mqttsuccess");
           }
           sendSleep = 1;
         }
       }
       break;
       
       case 0x10: //开始配网
       {
         
       }
       break;
       
       case 0x11: //连接成功
       {
         PlayVoiceP1(LINK_SUCCESS);
         FingerReceMes[0]=0; 
         uint8 char6[16]="success";
         SimpleProfile_SetParameter( SIMPLEPROFILE_CHAR6, 16, char6);
         osal_start_timerEx( simpleBLEPeripheral_TaskID, SBP_PERIODIC_EVT1, 100 );
       }
       break;
       
       case 0x12: //连接失败
       {
         PlayVoiceP1(LINK_FAIL);
         WiFi_PowerDownIsAllowed();
         uint8 char6[16]="fail";
         SimpleProfile_SetParameter( SIMPLEPROFILE_CHAR6, 16, char6);
         osal_start_timerEx( simpleBLEPeripheral_TaskID, SBP_PERIODIC_EVT1, 100 );
         osal_pwrmgr_device( PWRMGR_BATTERY );  //使能低功耗
         osal_start_timerEx(simpleBLEPeripheral_TaskID, GoSleep_EVT, 400);         
       }
       break;
       
       case 0x21: //wifi开锁结果 
       { 
         stateOpen = 0;
         if(FingerReceMes[4]!=0) 
         {
            alert.FPERROR_state++;
             if(alert.FPERROR_state>=5)
            {               
               osal_start_timerEx(simpleBLEPeripheral_TaskID, ClearFPLock_EVT,180000);
             }
            {
              PlayVoiceP1(VERIFY_FAIL);
              FingerReceMes[0]=0;
            }
         }
         else
         {
          // if( voiceb==0)
           {
              P1IEN &= ~0x21;
             if(userd.voice == 1 || workmode == 0){
               
               PlayVoiceP1(OPEN_SUCCESS);
             } 
             FingerReceMes[0]=0;
             // 屏蔽按键和指纹中断

           
             voiceb=1;
             
             Send_Notify_EVT_With("opnsuccess");
           }
         }
       }
       break;
       
       case 0x27:               // 密码错误
       {
         alert.CODEERROR_state++;
          if(alert.CODEERROR_state>=5)
          {             
            osal_start_timerEx(simpleBLEPeripheral_TaskID, ClearFPLock_EVT,180000);
          }
#ifdef bank0
         PlayVoiceP1(VERIFY_FAIL);
#else
         PlayVoiceP1(VERIFY_FAIL);
#endif
         FingerReceMes[0]=0;
         k=0;                   //按键清零
       //  pressc=0;
        // keylight=0;
         memset(KeyCode,0,sizeof(KeyCode));
       }
       break;    
       
       case 0x30: //录入指纹
       {
         wifisenddata[0]=0x30;
         osal_pwrmgr_device( PWRMGR_ALWAYS_ON );//串口工作需要32M时钟
         //osal_stop_timerEx(simpleBLEPeriphe  ral_TaskID, SBP_PERIODIC_EVT); // 录入指纹时停止 周期性事件

         P1IEN|=0x20;  //打开指纹中断
         /*
         P1DIR|=0x08;
         fingerpw=1;   //指纹供电打开
         for(int l=0;l<20;l++)
         {
           WAIT_MS();
         }*/
       }
       break;    
        
       case 0x31: //验证指纹
       {
         Finger_Cmd(CMD_VERIFY_ONE_TO_ALL,0x00,0x00,0x00,0x00);
         PlayVoiceP1(Finger_RQA);
       }
       break;
       
       case 0x32: //wifi获取指纹特征值
       {
         wifisenddata[0]=0x32;
       }
       break;
       
       case 0x33: //wifi下传来指纹特征值
       {
        wifisenddata[0]=0x33;
         P1DIR|=0x08;
         fingerpw=1;   //指纹供电打开
       }
       break;       
       
       case 0x34: //删除特定指纹
       {
         wifisenddata[0]=0x34;
         P1DIR|=0x08;
         fingerpw=1;   //指纹供电打开  
       }
       break;    
       
       case 0x35: //删除全部指纹
       {
         FingerUser_DeleteAll(); 
         P1DIR|=0x08;
         fingerpw=1;   //指纹供电打开
         
       }
       break;    
       
       case 0x36: //一对一验证
       {
         wifisenddata[0]=0x36;  
       }
       break;    
       case 0x40: //获取当前用户个数
        {
         wifisenddata[0]=0x40;
        }
        break;
      case 0x41: //获取当前用户个数
        {
         userdTemp.custom =FingerReceMes[4];
         memset(&userd,0,sizeof(userd));
         osal_snv_read(0xfa,(uint8)sizeof(slFlash_t), (uint8 *)&userd);
       //  wifisenddata[0]=0x40;
        }
        break;
        case 0x42: //同步用户信息结果
        {
         wifisenddata[0]=0x00;
         if(FingerReceMes[4]!=0){
           //失败后，不清空，下次继续同步
         } else {
           //成功
           if(nonate != 1){
             need_update = 0;
           }
           nonate = 0;
         }
        }
        break;
         case 0x44: //恢复工厂
        {
           memset(&userd,0,sizeof(userd));
           flag_fp = 15;
           osal_start_timerEx(simpleBLEPeripheral_TaskID, ClearLock_EVT,0);
 
           //BLE_Cmd(0x29,0,0,52);
           osal_snv_write( 0xfa,(uint8)sizeof(slFlash_t), (uint8 *)&userd );
           
           //PlayVoiceP1(FgTd_FAIL);//检测是不是丢数据是这里
        }
        break;
       case 0x50: //获取当前密吗
       {
         uint8 newValue1[6]={0};
         osal_snv_read( 0x80,6, newValue1 );
         BLE_Cmd(0x70,0,6,0);
         BLE_Data(0x70,0,0x06,newValue1);
       }
       break;
       
       case 0x51: //修改密码
       {
         for(uint8 i=0;i<6;i++)
         {
           FingerReceMes[i+2]=deccryptedData[i+8];           
         }
        // ChangePasscode(0x02);
       }
       break;
          case 0x52: //进入SBL wait
       {
         appForceBoot();
       }
       break;
       case 0x60: //初始化成功 //现在没用
       {
//#ifndef bank0         
         PlayVoiceP1(INIT_SUCCESS);
         workmode=1; 
//#endif         
       }
       break;    
       
       case 0x61: //电量不足警告
       {
//#ifndef bank0 
         PlayVoiceP1(CODE_ERROR);
//#endif 
       }
       break;    
       
       case 0x62: //开始同步
       {
//#ifndef bank0 
        // PlayVoiceP1(START_TD);
//#endif 
       }
       break;    
       
       case 0x63: //密码锁定
       {
//#ifndef bank0 
         PlayVoiceP1(CODE_LOCK);
//#endif 
       }
       break;    
       
       case 0x64: //同步成功
       {
//#ifndef bank0 
       //  PlayVoiceP1(FgTd_Success);
//#endif 
       }
       break; 
       
       case 0x65: //同步失败
       {
//#ifndef bank0 
        // PlayVoiceP1(FgTd_FAIL);
//#endif     
       }
       break; 

       case 0x66: //指纹已锁
       {
//#ifndef bank0 
         PlayVoiceP1(Finger_LOCK);
//#endif 
       }
       break;    
       
       case 0x70: //新密钥
       {
         for(uint8 i=0;i<16;i++)
         {
           FingerReceMes[i+2]=key1[i];
           
         } 
         uint8 key[16] = "fourw.zeepson.cn"; 
         uint8 iv[16]  = "sanw.zeepson.com";  
         HalAesInit();
         AES_SETMODE(CBC);
         AesLoadKey( key );
         AesLoadIV(iv);
         AesDmaSetup( key1, STATE_BLENGTH, key1, STATE_BLENGTH );
         AES_SET_ENCR_DECR_KEY_IV( AES_DECRYPT );  
         AES_START();
       }
       break;     
#ifdef LOCAL
      case 0x71:
        {
            LocalPro=FingerReceMes[3];//获取状态码
            if(LocalPro%10==3)
            {
              k=0;
              PlayVoiceP1(KEY_VIOCE);     //按键音
              delay5ms();
              PlayVoiceP1(KEY_VIOCE);     //按键音
              delay5ms();
              PlayVoiceP1(KEY_VIOCE);     //按键音
              delay5ms();
              PlayVoiceP1(KEY_VIOCE);     //按键音
            }else if(LocalPro%10==2)      //验证管理员
            {
              if((LocalPro/10%10)==3)
              {
                k=0;
                PlayVoiceP1(KEY_VIOCE);     //按键音
                delay5ms();
                PlayVoiceP1(KEY_VIOCE);     //按键音
                delay5ms();
                PlayVoiceP1(KEY_VIOCE);     //按键音
                delay5ms();
                PlayVoiceP1(KEY_VIOCE);     //按键音
              }else
              {
                k=0;
                PlayVoiceP1(KEY_VIOCE);     //按键音
                delay5ms();
                PlayVoiceP1(KEY_VIOCE);     //按键音
              }
            }
            else
            {
               k=0;
               PlayVoiceP1(Finger_RQ);     //按键音
            }
             
            if(userd.singleV != 1){
              wifisinglemode = 1;
            }
        }
        break;
         case 0x74://用户不存在
        {
           PlayVoiceP1(VERIFY_FAIL);     //按键音
           k=0;
           delay5ms();
           PlayVoiceP1(VERIFY_FAIL);     //按键音
           localerror++;
           if(localerror<3)
            {
              KeyCode[1]=0;
              KeyCode[2]=0;
              KeyCode[3]=0;
              k=1;
              delay5ms();
              PlayVoiceP1(KEY_VIOCE);     //按键音
              delay5ms();
              PlayVoiceP1(KEY_VIOCE);     //按键音   
            } else if(localerror>=3)
            {
               BLE_Cmd(0x0d,0,0,0);   //通知wifi休眠
            }
          
          //PlayVoiceP1(KEY_VIOCE);     //按键音
        }
        break;
      case 0x73://删除成功返回
        {
          if(FingerReceMes[4]!=1)
          {
          k=0;
          LocalPro=0;
          PlayVoiceP1(KEY_VIOCE);     //按键音
          delay5ms();
          PlayVoiceP1(KEY_VIOCE);     //按键音
          delay5ms();
          PlayVoiceP1(KEY_VIOCE);     //按键音
          }else
          {
          if(!(LocalPro%10==0))
          PlayVoiceP1(VERIFY_FAIL);
          delay5ms();
          delay5ms();
          localerror++;
         if(localerror<3)
           {
             PlayVoiceP1(Finger_RQ);     //按键音
            }
          }
        }
        break;
#endif
     }
      
     MessageSum=0;
     memset(FingerReceMes,0,sizeof(FingerReceMes));
    }
    }
}
/*


*/
/*
char TouchKey_GetKeyChar(uint16_t value)
{
	char result = 0;
	switch(value)
	{
		case 0:			result = '-';break; //0:没有按下值
		case 1:			result = '*';break;
		case 2:			result = '7';break;
		case 4:			result = '4';break;
		case 8:			result = '1';break;
		case 16:	        result = '2';break;
		case 32:		result = '5';break;
		case 64:		result = '8';break;
		case 128:		result = '0';break;
		case 256:		result = '3';break;
		case 512:		result = '6';break;
		case 1024:		result = '9';break;
		case 2048:		result = '#';break;
		default: result = '!';				 //读取数据错误
	}
	return result;
}*/
/*********************************************************************
 * @fn      simpleBLEPeripheral_ProcessOSALMsg
 *
 * @brief   Process an incoming task message.
 *
 * @param   pMsg - message to process
 *
 * @return  none
 */
static void simpleBLEPeripheral_ProcessOSALMsg( osal_event_hdr_t *pMsg )
{
  switch ( pMsg->event )
  {
 case KEY_CHANGE:
   {
    // if( workmode==1)
     {
   
      
       simpleBLEPeripheral_HandleKeys( ((keyChange_t *)pMsg)->state, ((keyChange_t *)pMsg)->keys );
     }/*else
     {
         P1DIR|=0x02;//设为输出
         keylight=1;    //使能背光灯
      
         P1DIR|=0x08;
         fingerth = 1; // 打开指纹触摸供电
         fingerpw=1;   // 指纹供电打开
         P1IEN|=0x20;  // 指纹触摸中断打
         //klpower=0;     //打开背光灯供电电源 
         P1DIR|=0x10;
         klpower=1;     //打开背光灯供电电
         InitUart(); 
         IEN0|=0x8c;
         Uart_Colsed=1;//不允许自动进入低功耗
      
        P2DIR|=0x01;    //设为输出
        
        // TEST
        wakewifi=0;     //唤醒wifi
        
        BLE_Cmd(0x20,0,0,0); //工厂模式直接开门
        AT080init();
     }*/
      
   }  
     break;
  default:
    // do nothing
    break;
  }
}


/*********************************************************************
 * @fn      simpleBLEPeripheral_HandleKeys
 *
 * @brief   Handles all key events for this device.
 *
 * @param   shift - true if in shift/alt.
 * @param   keys - bit field for key events. Valid entries:
 *                 HAL_KEY_SW_2
 *                 HAL_KEY_SW_1
 *
 * @return  none
 */
static void simpleBLEPeripheral_HandleKeys( uint8 shift, uint8 keys )
{
  //uint8  SK_Keys = 0;
  
  VOID shift;  // Intentionally unreferenced parameter
  
  if ( keys & TOUCH_OUT )//假设此中断为指纹中断唤醒
  {       
    
    //osal_start_timerEx(simpleBLEPeripheral_TaskID, FPHSTIMEOUT_EVT, 100);
    osal_pwrmgr_device( PWRMGR_ALWAYS_ON );//串口工作需要32M时钟

    if((touch==0)&&(pressc==0))
    {
     // fingerth=0;      // 确保指纹触摸监测一直在工作 0
      // 指纹头为首次被按压 && 
      P1DIR|=0x08;
      P1DIR|=0x02;      //设为输出
      //flag_fp = 30;
      //osal_start_timerEx(simpleBLEPeripheral_TaskID, ClearLock_EVT,0);
      
      fingerpw=1;       //指纹供电打开
      InitUart(); 
      keylight=1;       //使能背光灯
      P1DIR|=0x10;
      klpower=1;        //打开背光灯供电电源 
      IEN0|=0x0c;
      Uart_Colsed=1;    //不允许自动进入低功耗
      P2DIR|=0x01;      //设为输出
      WiFi_WakeUp(); 
      
      memset(&userd,0,sizeof(userd));
      osal_snv_read(0xfa,(uint8)sizeof(slFlash_t), (uint8 *)&userd);
      //ret = osal_snv_read( 0x80,(uint8)sizeof( slFlash_t ), &userd );
     // if(ret == NV_OPER_FAILED){
      //}
      
      //同步数据  
      if(need_update > 0){
        nonate = 0;
        //BLE_Cmd(0x42,0,0,0); 
      }
      //这里优化的话，就不用workmode 以及 wifisinglemode
      if(userd.factory==1){
        
        workmode=1; //初始化正常
        wifisinglemode = 0;  //非单机模式
      } else{ 
       
        if(userd.singleV==1){
       
         workmode=0; //非初始化正常
         wifisinglemode = 1;  //单机模式
        } else {
        
         //这就是工厂模式
         workmode = 0;
         wifisinglemode = 0; 
       }
      }

      osal_start_reload_timer( simpleBLEPeripheral_TaskID, SBP_PERIODIC_EVT, 200);
      
#ifdef BLE_STANDBY_ENABLE
      BleStandbyTimerStart();
#endif
      // advertising on and off
      if( gapProfileState != GAPROLE_CONNECTED )
      {
        uint8 current_adv_enabled_status;
        uint8 new_adv_enabled_status;

        //Find the current GAP advertisement status
        GAPRole_GetParameter( GAPROLE_ADVERT_ENABLED, &current_adv_enabled_status );

        if( current_adv_enabled_status == FALSE )
        {
          new_adv_enabled_status = TRUE;
        }
        //change the GAP advertisement status to opposite of current status
        GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &new_adv_enabled_status );
      }
    }
    if(LocalPro==0)
    {
      if(luru==1)
      {
      
      }
      else if(luru==2)
      {
      // uint16_t userID=wifisenddata[5]<<8|wifisenddata[6];
        uint16_t userID = fgbuffer[2]<<8|fgbuffer[3];
        if(userID!=0)
        User_FingerOneToOne(userID);
      }
       else
      {
        {
          // P1IEN&=~0x20;     // 关闭指纹中断
           P1DIR|=0x20;      // 
           fpprocess=0;
         //if(fp_delete == 0)
           osal_start_timerEx( simpleBLEPeripheral_TaskID, Wakeup_EVT1, 100 );//等指纹芯片稳定
        } 
      }
    }
#ifdef LOCAL   
    else
    {
      uint16_t userID = fpid[0]<<8|fpid[1];
      if(userID!=0)
      {
        User_FingerOneToOne( userID);
       
      }else
      { 
      switch(LocalPro%100)
      {       
      case 0:  //开始注册管理员指纹
        {
          fpprocess=0;
          if(kchange==false)
          {
            User_Registration_1C3R(0x0000,0x01); //发送注册指令注意：权限若为00则一直注册失败
            kchange=true;
          }
        }
        break;
        case 10:  //指纹已满
        {
          PlayVoiceP1(VERIFY_FAIL);//密码已满报错
          delay5ms();
          PlayVoiceP1(VERIFY_FAIL);//密码已满报错
        }
        break;
         case 1:  //开始注册管理员指纹
        {
          fpprocess=0;
          if(kchange==false)
          {
            User_Registration_1C3R(0x0000,0x01); //发送注册指令注意：权限若为00则一直注册失败
            kchange=true;
          }
        }
        break;
        case 11:  //指纹已满
        {
          //PlayVoiceP1(Finger_ADD_FAIL);//密码已满报错
          PlayVoiceP1(VERIFY_FAIL);//密码已满报错
          delay5ms();
          PlayVoiceP1(VERIFY_FAIL);//密码已满报错
          
        }
        break;
      case 22:  //需要验证管理员
        {
          fpprocess=0;
         //if(fp_delete == 0)
           osal_start_timerEx( simpleBLEPeripheral_TaskID, Wakeup_EVT1, 100 );//等指纹芯片稳定
        }
        break;
        case 32:  //需要验证管理员
        {
          fpprocess=0;
         //if(fp_delete == 0)
           osal_start_timerEx( simpleBLEPeripheral_TaskID, Wakeup_EVT1, 100 );//等指纹芯片稳定
        }
        break;
      }
      }
    }
#endif
    touch++;
  }
  
 
  if ( keys & TOUCH_IRQ )
  {
    
    osal_pwrmgr_device( PWRMGR_ALWAYS_ON ); //IIC读写需要32M时钟播放语音需要32M时钟
    
    P1IEN&=~0x21;                           // 按键中断关闭
    pressc++;                               // 按键计次 
    
    if(pressc==1)
    {      
      if(MPR121==0)
      {
        HalI2CInit(0x56, i2cClock_267KHZ);  // 是APT8L16 低功耗每次唤醒都要初始化IIC
        APT8L16QuickConfig();
      }
      else
      {
        HalI2CInit(0x5a, i2cClock_267KHZ);  // 是MPR121初始化地址为0x5A  低功耗每次唤醒都要初始化IIC
        mpr121QuickConfig();
      }
    }
    
    if((keylight==0)||(klpower==0))
    {
      // 打开一个周期事件 用于周期性查询蓝牙广播状态
      osal_start_reload_timer( simpleBLEPeripheral_TaskID, SBP_PERIODIC_EVT, 200); //两分钟休眠
      
      P1DIR|=0x12;      // 为输出
      keylight=1;       // 使能背光灯
      P1DIR|=0x08;
      fingerth = 1;     // 打开指纹触摸供电
      fingerpw=1;       // 指纹供电打开
      P1IEN|=0x20;      // 指纹触摸中断打开
  
      P1DIR|=0x10;
      klpower=1;        // 打开背光灯供电电源 
      InitUart(); 
      IEN0|=0x8c;
      Uart_Colsed=1;    // 不允许自动进入低功耗
      //BLE_Cmd(0x42,0,0,0);
       /*唤醒wifi*/
      P2DIR|=0x01;      //设为输出
      WiFi_WakeUp();
      delay5ms();
      delay5ms(); 
      //同步数据  
      //BLE_Cmd(0x42,0,0,0); 
      if(need_update > 0){
        nonate = 0;
        //BLE_Cmd(0x42,0,0,0); 
      }
    //  PN532WakeUp();
#ifdef BLE_STANDBY_ENABLE
      BleStandbyTimerStart();
#endif   
      memset(&userd,0,sizeof(userd));
      osal_snv_read(0xfa,(uint8)sizeof(slFlash_t), (uint8 *)&userd);
      
      if(userd.factory==1){
        
        workmode=1; //初始化正常
        wifisinglemode = 0;  //非单机模式
      } else{ 
       
        if(userd.singleV==1){
       
         workmode=0; //非初始化正常
         wifisinglemode = 1;  //单机模式
        } else {
        
         //这就是工厂模式
         workmode = 0;
         wifisinglemode = 0; 
       }
      }
      
    }
    if((pressc>=2) )//&& (MPR121==1))||(pressc>=3)
    {
      // 第二次按压键盘 开始处理键盘信息
     if(MPR121==0)                 // 非 MPR121
      {
        
        uint8 read[3];
        ReadTouchKey1(read);        //读取按键寄存器
        
        if((read[0]!=0) && (read[1]==0))
        {
          PlayVoiceP1(KEY_VIOCE);     // 按键音
          switch(read[0])
          {
            case 0x04:
#ifdef DHF_LOCK
              KeyCode[k]=3;
              if((LocalPro%10==3)&&(k==0))//删除动作
              {
                 BLE_Cmd(0x73,0,4,0);                                             //将用户输入的组合按键传给wifi
                 BLE_Data(0x73,0,0x04,KeyCode);   
                // memset(KeyCode,0,sizeof(KeyCode));
              }
#else
              KeyCode[k]='*';
#endif
              break;
              
            case 0x08:
#ifdef DHF_LOCK
             KeyCode[k]=6;
#else
             KeyCode[k]=7;
#endif
             break;
              
            case 0x10:
#ifdef DHF_LOCK
             KeyCode[k]=9;
#else
             KeyCode[k]=4;
             if((LocalPro%10==3)&&(k==0))//删除动作
              {
                 BLE_Cmd(0x73,0,4,0);                                             //将用户输入的组合按键传给wifi
                 BLE_Data(0x73,0,0x04,KeyCode);   
                // memset(KeyCode,0,sizeof(KeyCode));
                 
              }
#endif
             break;
              
            case 0x20:
#ifdef DHF_LOCK     
             KeyCode[k]= '#';
             //test  
              //BLE_Cmd(0x29,0,k,0);
             // BLE_Data(0x29,0,k,KeyCode);
             if(workmode==0 && wifisinglemode==0)
             {
               BLE_Cmd(0x20,0,0,0); //工厂模式直接开门
             }else
             {
             if(k != 6)
             {
                if(LocalPro%10==3)//删除动作
                {
                  if(k!=4)
                  {
                    KeyCode[1]=1;
                    KeyCode[2]=1;
                    KeyCode[3]=1;
                  } 
                  BLE_Cmd(0x73,0,4,0);                                             //将用户输入的组合按键传给wifi
                  BLE_Data(0x73,0,0x04,KeyCode);   
               //   memset(KeyCode,0,sizeof(KeyCode));
                 k=0;
                }else if((LocalPro%10==0)&&(LocalPro!=0))
                {
                  PlayVoiceP1(VERIFY_FAIL);//密码已满报错
                  memset(KeyCode,0,sizeof(KeyCode));
                  k=0;                      //计数清零
                  switch(LocalPro/100)
                  {
                    case 1:
                      BLE_Cmd(0x72,0,0,1); 
                    break;
                    case 2:
                    BLE_Cmd(0x74,0,0,1); 
                    break;
                 }
                 localerror++;
                 if(localerror<3)
                 {
                   PlayVoiceP1(Finger_RQ);     //按键音
                 } 
                 }else if((LocalPro%10)==2)//验证密码
                 {
                    PlayVoiceP1(VERIFY_FAIL);//密码已满报错
                    memset(KeyCode,0,sizeof(KeyCode));
                    k=0;
                   BLE_Cmd(0x71,0,0,1);
                 }
                else
                {
                   alert.CODEERROR_state++;
                   if(alert.CODEERROR_state>=5)
                  {
                    BLE_Cmd(0x2B,0,0,2);   //通知密码多次失败
                    PlayVoiceP1(CODE_LOCK);     //密码已锁定
                    BLE_Cmd(0x0d,0,0,0);   //通知wifi休眠
                  }else
                  {
                    if(wifisinglemode==1){
                      
                      KeyCode[5]=15;
                      BLE_Cmd(0x26,0,6,0);                                             //将用户输入的组合按键传给wifi
                      BLE_Data(0x26,0,0x06,KeyCode);
                    }
                    PlayVoiceP1(VERIFY_FAIL);
                  }
                   memset(KeyCode,0,sizeof(KeyCode));
                   k=0;
                   pressc = 1;
                 }
             }
             }
#else
             KeyCode[k]= 1;
             if((LocalPro%10==3)&&(k==0))//删除动作
             {
                PlayVoiceP1(KEY_VIOCE);     //按键音
                delay5ms();
                PlayVoiceP1(KEY_VIOCE);     //按键音
             }
#endif
             break;
              
            case 0x40:
#ifdef DHF_LOCK 
             KeyCode[k]=5; 
#else
             KeyCode[k]=2;
             if((LocalPro%10==3)&&(k==0))//删除动作
             {
                PlayVoiceP1(KEY_VIOCE);     //按键音
                delay5ms();
                PlayVoiceP1(KEY_VIOCE);     //按键音
             }
#endif
             break;
              
            case 0x80:
#ifdef DHF_LOCK               
             KeyCode[k]=2;
             if((LocalPro%10==3)&&(k==0))//删除动作
             {
                PlayVoiceP1(KEY_VIOCE);     //按键音
                delay5ms();
                PlayVoiceP1(KEY_VIOCE);     //按键音
             }
#else
             KeyCode[k]=5;
#endif
             break;
          default:
            KeyCode[k]=read[0];
            //KeyCode[7]=read[1];
            break;
          }
          if(workmode == 1){
            
            if(pressc!=2 && k==0){
            
            pressc = 1;
            } else {
            k++; 
            }
          } else {
           
            k++;
          }
          
          keylight=1;
        }
        else if ((read[1]!=0)&&(read[0]==0))
        {
          PlayVoiceP1(KEY_VIOCE);     //按键音
          switch(read[1])
          {
            case 0x01:              
              KeyCode[k]=8;
              break;     
              
            case 0x02:
              KeyCode[k]=0;
              break;          
              
            case 0x04:
#ifdef DHF_LOCK 
              KeyCode[k]='*';
#else
              KeyCode[k]=3;
              if((LocalPro%10==3)&&(k==0))//删除动作
              {
                 BLE_Cmd(0x73,0,4,0);                                             //将用户输入的组合按键传给wifi
                 BLE_Data(0x73,0,0x04,KeyCode);   
                // memset(KeyCode,0,sizeof(KeyCode));
              }
#endif
              break;          
              
            case 0x08:
#ifdef DHF_LOCK               
              KeyCode[k]=7;
#else
              KeyCode[k]=6;
#endif
              break;
              
            case 0x10:
#ifdef DHF_LOCK               
              KeyCode[k]=4;
              if((LocalPro%10==3)&&(k==0))//删除动作
              {
                 BLE_Cmd(0x73,0,4,0);                                             //将用户输入的组合按键传给wifi
                 BLE_Data(0x73,0,0x04,KeyCode);   
              //   memset(KeyCode,0,sizeof(KeyCode));
              }
#else
              KeyCode[k]=9;
#endif
              break;          
              
            case 0x20:
#ifdef DHF_LOCK 
              KeyCode[k]=1;
            if((LocalPro%10==3)&&(k==0))//删除动作
             {
                PlayVoiceP1(KEY_VIOCE);     //按键音
                delay5ms();
                PlayVoiceP1(KEY_VIOCE);     //按键音
             }
#else
              KeyCode[k] = '#';
              if(workmode==0 && wifisinglemode==0)
              {
                BLE_Cmd(0x20,0,0,0); //工厂模式直接开门
              }else
              {
              if(k != 6)
              {
                if(LocalPro%10==3)//删除动作
                {
                   if(k!=4) 
                  {
                    KeyCode[1]=1;
                    KeyCode[2]=1;
                    KeyCode[3]=1;
                  } 
                  BLE_Cmd(0x73,0,4,0);                                             //将用户输入的组合按键传给wifi
                  BLE_Data(0x73,0,0x04,KeyCode);   
                //  memset(KeyCode,0,sizeof(KeyCode));
                 k=0;
                }else if((LocalPro%10==0)&&(LocalPro!=0))
                {
                  PlayVoiceP1(VERIFY_FAIL);//密码已满报错
                  memset(KeyCode,0,sizeof(KeyCode));
                  k=-1;                      //计数清零
                  switch(LocalPro/100)
                  {
                    case 1:
                      BLE_Cmd(0x72,0,0,1); 
                    break;
                    case 2:
                    BLE_Cmd(0x74,0,0,1); 
                    break;
                 }
                 localerror++;
                 if(localerror<3)
                 {
                   PlayVoiceP1(Finger_RQ);     //按键音
                 } 
                 }
                else if((LocalPro%10)==2)//验证密码
                 {
                    PlayVoiceP1(VERIFY_FAIL);//密码已满报错
                    memset(KeyCode,0,sizeof(KeyCode));
           
                   BLE_Cmd(0x71,0,0,1);
                 }
                else
                {
                   alert.CODEERROR_state++;
                   if(alert.CODEERROR_state>=5)
                  {
                    PlayVoiceP1(CODE_LOCK);     //密码已锁定
                    BLE_Cmd(0x0d,0,0,0);   //通知wifi休眠
                  }else
                  {
                    KeyCode[5]=15;
                    BLE_Cmd(0x26,0,6,0);                                             //将用户输入的组合按键传给wifi
                    BLE_Data(0x26,0,0x06,KeyCode);
                    PlayVoiceP1(VERIFY_FAIL);
                  }
                   k=0;
                   memset(KeyCode,0,sizeof(KeyCode));
                }
              }
              }
#endif              
              break;
              
              default:
            
                KeyCode[k]=read[0];
                //KeyCode[10]=read[1];
                break; 
          }
          
          if(workmode == 1){
            
            if(pressc!=2 && k==0){
            
            pressc = 1;
          } else {
          
            k++; 
          }
          } else {
            
            k++;
          }
          
          keylight=1;
        }

      }/*else  //是MPR121
      {
          uint8 read[3]={0,0,0};
        ReadTouchKey(read);   //读取按键寄存器
         if((read[0]!=0)&&(read[1]==0))
      {
        //osal_start_timerEx( simpleBLEPeripheral_TaskID, TIMEOUT_EVT1, 1000 );
        PlayVoiceP1(KEY_VIOCE);  //按键音
        
        switch(read[0])
       {
          
        case 0x01:
          KeyCode[k]='*';
          break;
       case 0x02:
          KeyCode[k]=7;
          break;
          
        case 0x04:
          KeyCode[k]=4;
          break;
          
        case 0x08:
           KeyCode[k]=1;
          break;
          
        case 0x10:
           KeyCode[k]=2;
          break;
          
        case 0x20:
           KeyCode[k]=5;
          break;
          
        case 0x40:
          KeyCode[k]=8;
          break;
          
        case 0x80:
          KeyCode[k]=0;
          break;
           
      
        
        }
      k++;
      }else if ((read[1]!=0)&&(read[0]==0))
      {
        // osal_start_timerEx( simpleBLEPeripheral_TaskID, TIMEOUT_EVT1, 1000 );
         PlayVoiceP1(KEY_VIOCE);
      
       // read[1]=read[1]&0x0f;
        switch(read[1])
        {
        case 0x01:
          KeyCode[k]=3;
          break;
        
        case 0x02:
          KeyCode[k]=6;
          break;
          
        case 0x04:
          KeyCode[k]=9;
          break;
          
        case 0x08:
          KeyCode[k]=0x23;
          if(k < 6 && k >1)
              {             
                  KeyCode[5]=15;
                  BLE_Cmd(0x26,0,6,0);                                             //将用户输入的组合按键传给wifi
                  BLE_Data(0x26,0,0x06,KeyCode);   
                  memset(KeyCode,0,sizeof(KeyCode));
               
              }  
          break;
        
        }
      k++;
      }
       
      memset(read,0,sizeof(read));    
      
      
      }*/
      /*
      添加虚位密码最多14个数字
      */
      if(LocalPro==0)
      {
      if(k==7)           //输入够6个就可以进去验证了
      {
        if(KeyCode[k-1]==0x23)      //按#结束
        {        
          kpress=1;           
          pressc=1;
            if((alert.CODEERROR_state>=5)&&((LocalPro%10)!=2))
          {
            BLE_Cmd(0x2B,0,0,2);   //通知多次操作失败
            PlayVoiceP1(CODE_LOCK);     //密码已锁定
            BLE_Cmd(0x0d,0,0,0);   //通知wifi休眠
           }else
           {
              bool vfy=false;
                for(uint8 i=0;i<userd.custom;i++)         
                {       
                  if(memcmp(userd.user[i].kcode, KeyCode, 6) == 0){ //验证成功
                             
                    BLE_Cmd(0x2A,0,11,2);            
                    BLE_Data(0x2A,0,11,userd.user[i].userid);
                 //   PlayVoiceP1(OPEN_SUCCESS);
                    vfy=true;  
                    //error 清零
                    alert.FPERROR_state = 0;
                    alert.CODEERROR_state = 0;
                    
                    break;       
                  }
                }
                         
                if(vfy!=true)//返回验证失败        
                {    
                  if(wifisinglemode==1){
                    
                    BLE_Cmd(0x26,0,6,0);
                    BLE_Data(0x26,0,6,KeyCode);
                  } else {
                    
                     alert.CODEERROR_state++;
                     if(alert.CODEERROR_state>=5)
                     {
                       BLE_Cmd(0x2B,0,0,2);
                       PlayVoiceP1(CODE_LOCK);
                       osal_start_timerEx(simpleBLEPeripheral_TaskID, ClearFPLock_EVT,180000);
                     } else {

                       if(wifisinglemode!=0 || workmode!=0){
                         PlayVoiceP1(VERIFY_FAIL);
                       }
                     }
                    
                  }
                }
                
           }
           memset(KeyCode,0,sizeof(KeyCode));  
           k=0;                      //计数清零
        }        
      }
      }

      else if((LocalPro%10)==0)//非101 111 201 211 则可以加密码
      {
        if(k==7)
        {
          if(KeyCode[k-1]==0x23)      //按#结束
          {
            if(KeyCode[13]!=0x23)
            {
              for(uint8 i=0;i<7;i++)
              {
                KeyCode[7+i]=KeyCode[i];
              }
             // for(int vc=0;vc<3;vc++)
              {
                PlayVoiceP1(Finger_RQA);     //按键音
              }
            }else
            {
              bool ver=true;
              for(uint i=0;i<7;i++)
              {
                if(KeyCode[7+i]!=KeyCode[i])
                {
                   PlayVoiceP1(VERIFY_FAIL);//两次密码不一致报错
                   ver=false;
                   delay5ms();
                   delay5ms();
                   localerror++;
                   memset(KeyCode,0,sizeof(KeyCode));
                   k=0;                      //计数清零
                  if(localerror<3)
                    {
                       PlayVoiceP1(Finger_RQ);     //按键音
                    }
                   break;
                }
              }
              if(ver==true)
              {
                switch(LocalPro/100)
                {
                  case 1://返回管理员密码
                    {
                      BLE_Cmd(0x72,0,6,0);    //将用户输入的组合按键传给wifi
                      BLE_Data(0x72,0,6,KeyCode);
                      
                      if(userd.singleV != 1){
                        userd.singleV = 1; 
                        osal_start_timerEx(simpleBLEPeripheral_TaskID, Datacleanfx_EVT, 0);
                      }
                      
                      memset(KeyCode,0,sizeof(KeyCode));
                      k=0;                      //计数清零
                    }
                    break;
                  
                  case 2: //返回普通用户密码
                    {
                      BLE_Cmd(0x74,0,6,0);    //将用户输入的组合按键传给wifi
                      BLE_Data(0x74,0,6,KeyCode);
                      memset(KeyCode,0,sizeof(KeyCode));
                      k=0;                      //计数清零
                    }
                    break;                
                }
              }else
              {
                 switch(LocalPro/100)
                 {
                 case 1:
                       BLE_Cmd(0x72,0,0,1); 
                   break;
                  case 2:
                     BLE_Cmd(0x74,0,0,1); 
                   break;
                 }
              
              }
            }
          }else  //输入的个数不对
        {
           PlayVoiceP1(VERIFY_FAIL);//密码已满报错
           memset(KeyCode,0,sizeof(KeyCode));
           k=0;                      //计数清零
           switch(LocalPro/100)
           {
             case 1:
                 BLE_Cmd(0x72,0,0,1); 
                 break;
             case 2:
                  BLE_Cmd(0x74,0,0,1); 
                  break;
                 }
             localerror++;
           if(localerror<3)
             {
               PlayVoiceP1(Finger_RQ);     //按键音
              }
           
        }
        }
      }
      else if((LocalPro%10)==2)//验证密码
      {
        if(k==7)           //输入够6个就可以进去验证了
        {
          if(KeyCode[k-1]==0x23)      //按#结束
          {
            kpress=1;           
             BLE_Cmd(0x71,0,6,0);    //将用户输入的组合按键传给wifi
            BLE_Data(0x71,0,k-1,KeyCode);   //返回要验证的管理员密码
            memset(KeyCode,0,sizeof(KeyCode));
            k=0;                      //计数清零
            LocalPro=0;
          } else
          {
            PlayVoiceP1(VERIFY_FAIL);//密码已满报错
            memset(KeyCode,0,sizeof(KeyCode));
            BLE_Cmd(0x71,0,0,1);
          }
        }else
        {
          if(KeyCode[k-1]==0x23)      //按#结束
          {
            PlayVoiceP1(VERIFY_FAIL);//密码已满报错
           memset(KeyCode,0,sizeof(KeyCode));
           
            BLE_Cmd(0x71,0,0,1);
          }
        }
      }
      else if((LocalPro%10)==1)//密码已满
      {
       // PlayVoiceP1(Finger_ADD_FAIL);//密码已满报错
        PlayVoiceP1(VERIFY_FAIL);//密码已满报错
        delay5ms();
         PlayVoiceP1(VERIFY_FAIL);//密码已满报错
      }else  if((LocalPro%10)==3)//删除动作
      {
        if(k==1)
        {
#ifndef NFC
          if((KeyCode[0]>4)||(KeyCode[0]==0))//超过了删除的4
          {
            PlayVoiceP1(VERIFY_FAIL);//密码已满报错
            
            k=0;
           localerror++;
           if(localerror>=3)
           {
              BLE_Cmd(0x0d,0,0,0);//通知wifi休眠
           }
                 
            
          }
#else
          if((KeyCode[0]>4))//超过了删除的4
          {
            PlayVoiceP1(VERIFY_FAIL);//密码已满报错
            
            k=0;
           localerror++;
           if(localerror>=3)
           {
              BLE_Cmd(0x0d,0,0,0);//通知wifi休眠
           }
                 
            
          } 
#endif
        }
      }

      if(workmode==1){
     
        if ((KeyCode[k-1]==0x23)||k>=7)//接到#就清零
        {
          memset(KeyCode,0,sizeof(KeyCode));
          k=0;
         }
      }  else {
        
         if ((KeyCode[k-1]==0x23)||k>=31)//接到#就清零
         {
           k=0;
         }
      }   
    }
    P1IEN|=0x21;  // 指纹触摸中断打开
  }
}
/*********************************************************************
 * @fn      peripheralStateNotificationCB
 *
 * @brief   Notification from the profile of a state change.
 *
 * @param   newState - new state
 *
 * @return  none
 */
static void peripheralStateNotificationCB( gaprole_States_t newState )
{
  switch ( newState )
  {
    case GAPROLE_STARTED:
      {
        uint8 ownAddress[B_ADDR_LEN];
        uint8 systemId[DEVINFO_SYSTEM_ID_LEN];

        GAPRole_GetParameter(GAPROLE_BD_ADDR, ownAddress);
        
        // use 6 bytes of device address for 8 bytes of system ID value
        systemId[0] = ownAddress[0];
        systemId[1] = ownAddress[1];
        systemId[2] = ownAddress[2];

        // set middle bytes to zero
        systemId[4] = 0x00;
        systemId[3] = 0x00;

        // shift three bytes up
        systemId[7] = ownAddress[5];
        systemId[6] = ownAddress[4];
        systemId[5] = ownAddress[3];

        DevInfo_SetParameter(DEVINFO_SYSTEM_ID, DEVINFO_SYSTEM_ID_LEN, systemId);
        wechatIOTGetMacAddrUpdatetoAdvertData() ; //ъ MAC
        #if (defined HAL_LCD) && (HAL_LCD == TRUE)
          // Display device address
          HalLcdWriteString( bdAddr2Str( ownAddress ),  HAL_LCD_LINE_2 );
          HalLcdWriteString( "Initialized",  HAL_LCD_LINE_3 );
        #endif // (defined HAL_LCD) && (HAL_LCD == TRUE)
      }
      break;

    case GAPROLE_ADVERTISING:
      {
        #if (defined HAL_LCD) && (HAL_LCD == TRUE)
          HalLcdWriteString( "Advertising",  HAL_LCD_LINE_3 );
        #endif // (defined HAL_LCD) && (HAL_LCD == TRUE)
        simpleBLEState = BLE_STATE_IDLE;
      }
      break;

    case GAPROLE_CONNECTED:
      { 
        genRandomString(16);
        //strncpy(key1,newkey,16);
        //strcpy(key1,newkey);
        uint8 char6[16]={2,34,56,66,12,15,45,88, key1[2], key1[3], key1[5], key1[7], key1[10], key1[13],22,56};
        uint8 key2[16] = "1234567812345678";//{1,2,3,4,5,6,7,8,1,2,3,4,5,6,7,8};
       //  LL_Encrypt( key2, char6, ss );
        LL_Encrypt( key2, key1, char6 );
        //free(newkey);
        i=0;
        SimpleProfile_SetParameter( SIMPLEPROFILE_CHAR5, 16, &char6);
        //osal_start_timerEx( simpleBLEPeripheral_TaskID, SBP_PERIODIC_EVT1, 200 );
        
        #if (defined HAL_LCD) && (HAL_LCD == TRUE)
          HalLcdWriteString( "Connected",  HAL_LCD_LINE_3 );
          uint8 ownAddress[B_ADDR_LEN];
          GAPRole_GetParameter( GAPROLE_CONN_BD_ADDR , ownAddress);
          HalLcdWriteString( bdAddr2Str( ownAddress ),  HAL_LCD_LINE_4 );
        #endif // (defined HAL_LCD) && (HAL_LCD == TRUE)
        simpleBLEState = BLE_STATE_CONNECTED;
      }
      break;

    case GAPROLE_WAITING:
      {
        #if (defined HAL_LCD) && (HAL_LCD == TRUE)
          HalLcdWriteString( "Disconnected",  HAL_LCD_LINE_3 );
        #endif // (defined HAL_LCD) && (HAL_LCD == TRUE)
      }
      break;

    case GAPROLE_WAITING_AFTER_TIMEOUT:
      {
        #if (defined HAL_LCD) && (HAL_LCD == TRUE)
          HalLcdWriteString( "Timed Out",  HAL_LCD_LINE_3 );
        #endif // (defined HAL_LCD) && (HAL_LCD == TRUE)
      }
      break;

    case GAPROLE_ERROR:
      {

        #if (defined HAL_LCD) && (HAL_LCD == TRUE)
          HalLcdWriteString( "Error",  HAL_LCD_LINE_3 );
        #endif // (defined HAL_LCD) && (HAL_LCD == TRUE)
      }
      break;

    default:
      {
        #if (defined HAL_LCD) && (HAL_LCD == TRUE)
          HalLcdWriteString( "",  HAL_LCD_LINE_3 );
        #endif // (defined HAL_LCD) && (HAL_LCD == TRUE)
      }
      break;
  }

  gapProfileState = newState;

#if !defined( CC2540_MINIDK )
  VOID gapProfileState;     // added to prevent compiler warning with
                            // "CC2540 Slave" configurations
#endif
}

/*********************************************************************
 * @fn      simpleProfileChangeCB
 *
 * @brief   Callback from SimpleBLEProfile indicating a value change
 *
 * @param   paramID - parameter ID of the value that was changed.
 *
 * @return  none
 */
static void simpleProfileChangeCB( uint8 paramID )
{
  uint8 newValue;
  
  switch( paramID )
  {
    case SIMPLEPROFILE_CHAR1:
      SimpleProfile_GetParameter( SIMPLEPROFILE_CHAR1, &newValue );
     
      break;

    case SIMPLEPROFILE_CHAR3:
      SimpleProfile_GetParameter( SIMPLEPROFILE_CHAR3, &newValue );
      break;
      
     case SIMPLEPROFILE_CHAR5: 
      { 
         uint8 char5Data[16] = {0};
         SimpleProfile_GetParameter( SIMPLEPROFILE_CHAR5, &char5Data );     
         LL_EXT_Decrypt( key1, char5Data, char5Data); 
         if( char5Data[0]=='E')//录指纹
         {
           index[1]=char5Data[4]-48;
           index[0]=11;
           for(uint8 i=0;i<11;i++)
           { 
             nowuser.userid[i]=char5Data[5+i];
           }
           for(uint8 a=0;a<userd.custom;a++)
           {
             if(memcmp(nowuser.userid, userd.user[a].userid, 11) == 0){
                
               index[0]=a;
               break;
             }
           }
           if(index[0]==11){
              if(userd.custom > 0 && userd.custom < 10){
               index[0]=userd.custom;
               userd.custom += 1;
               for(uint8 i=0;i<11;i++){

                 userd.user[userd.custom-1].userid[i]=nowuser.userid[i];
               }

               for(uint8 i=0;i<6;i++){
             
                 userd.user[userd.custom-1].kcode[i]=0xFF;
               }
              }
           }
           if(luru == 0 && fpadds == 0){
           
             luru=1;   //关闭中断验证指纹命令
           }
           if(luru == 1){
             
             fpvoice = 1;
             osal_stop_timerEx( simpleBLEPeripheral_TaskID, FPTIMEOUT_EVT );
             osal_start_timerEx(simpleBLEPeripheral_TaskID, FP_Voice_EVT, 50);
             osal_start_timerEx(simpleBLEPeripheral_TaskID, FPHSTIMEOUT_EVT, 500);

             BLE_Cmd(0x07,0,0,0); 
           }
         }else if(char5Data[0]=='D')//删除指纹
         {
            //nowuser.fp[0]=newValue5[4]-48;
           index[1]=char5Data[4]-48;
           for(uint8 i=0;i<11;i++)
           {
             nowuser.userid[i]=char5Data[5+i];
           }
            for(uint8 a=0;a<userd.custom;a++)
           {
             if(memcmp(nowuser.userid, userd.user[a].userid, 11) == 0){
               index[0]=a;
               break;
             }
           }
           
           if(userd.user[index[0]].fp[index[1]]!=0){
             
              User_DeleteSpecific(userd.user[index[0]].fp[index[1]]);
              userd.user[index[0]].fp[index[1]]=0;
            }
           
           osal_snv_write( 0xfa,(uint8)sizeof(slFlash_t), (uint8 *)&userd ); 
            //notify 出去
           Send_Notify_EVT_With("defsuccess");
           
           PlayVoiceP1(Finger_Delet_SUCCESS);
         }else if(char5Data[0]=='M'){//修改密码 1
           
           for(uint8 i=0;i<11;i++){
             nowuser.userid[i]=char5Data[4+i];
           }
         }else if(char5Data[0]=='W')//修改密码 2
         {
           //复位
            index[0]=11;
             for(uint8 a=0;a<userd.custom;a++)
           {
             if(memcmp(nowuser.userid, userd.user[a].userid, 11) == 0){
                
               index[0]=a;
               break;
             }
           }
           if(index[0]!=11){
             
             for(uint8 i=0;i<6;i++){
             
               userd.user[index[0]].kcode[i]=char5Data[4+i]-48;
             }
             //notify 出去
             Send_Notify_EVT_With("pwdsuccess");
             osal_start_timerEx(simpleBLEPeripheral_TaskID, Datacleanfx_EVT, 0);
           } else {
             
             if(userd.custom > 0 && userd.custom < 10 && userd.custom != 0xFF){
               userd.custom += 1;
               for(uint8 i=0;i<11;i++){

                 userd.user[userd.custom-1].userid[i]=nowuser.userid[i];
               }

               for(uint8 i=0;i<6;i++){
             
                 userd.user[userd.custom-1].kcode[i]=char5Data[4+i]-48;
               }
               
               //notify 出去
               Send_Notify_EVT_With("addsuccess");
               osal_start_timerEx(simpleBLEPeripheral_TaskID, Datacleanfx_EVT, 0);
             } else {
               
               //notify 出去
                Send_Notify_EVT_With("addfail");
             }
           }
            
         }else if(char5Data[0]=='R')//恢复出厂
         {
           memset(&userd,0,sizeof(userd));
           osal_start_timerEx(simpleBLEPeripheral_TaskID, Datacleanfx_EVT, 0);
           //notify 出去
           Send_Notify_EVT_With("reosuccess");

           PlayVoiceP1(Factory_SUCCESS);
           flag_fp = 13;
           osal_start_timerEx(simpleBLEPeripheral_TaskID, ClearLock_EVT,0);
           
         } else if(char5Data[0]=='L') //删除用户
         {
           for(uint8 i=0;i<11;i++)
           {
             nowuser.userid[i]=char5Data[4+i];
           }
            for(uint8 a=0;a<userd.custom;a++)
           {
             if(memcmp(nowuser.userid, userd.user[a].userid, 11) == 0){
                
             slFlash_t deleUsers;
             deleUsers.factory=userd.factory;
             deleUsers.singleV=userd.singleV;
             deleUsers.custom=userd.custom - 1;
             deleUsers.voice=userd.voice;
             for(uint8 j=0; j<userd.custom; j++){

                if(j<a){
                  
                  memcpy(deleUsers.user[j].userid,userd.user[j].userid,11);
                  memcpy(deleUsers.user[j].kcode,userd.user[j].kcode,6);
                  memcpy(deleUsers.user[j].fp,userd.user[j].fp,5);
                } else if(j==a){
                  for(uint8 n=0; n<5; n++){
                  
                    if(userd.user[j].fp[n]!=0)
                    User_DeleteSpecific(userd.user[j].fp[n]);
                  }
                    
                } else if(j>a){
                  
                  memcpy(deleUsers.user[j-1].userid,userd.user[j].userid,11);
                  memcpy(deleUsers.user[j-1].kcode,userd.user[j].kcode,6);
                  memcpy(deleUsers.user[j-1].fp,userd.user[j].fp,5);
                }
             }
             osal_snv_write( 0xfa,(uint8)sizeof(slFlash_t), (uint8 *)&deleUsers ); 
             memset(&userd,0,sizeof(userd));
             osal_snv_read(0xfa,(uint8)sizeof(slFlash_t), (uint8 *)&userd);
             break;
             }
           }
           //notify 出去
           Send_Notify_EVT_With("deusuccess");
         } else if(char5Data[0]=='V'){
           
           userd.voice = char5Data[4]-48;
           
           osal_snv_write( 0xfa,(uint8)sizeof(slFlash_t), (uint8 *)&userd ); 
            //notify 出去
           Send_Notify_EVT_With("voisuccess");
         }
      }
      break;
      
      case SIMPLEPROFILE_CHAR6: 
      { 
        if(stateOpen == 0){
         stateOpen = 1;
         uint8 BLEdata[16]={0}; //蓝牙数据包
         SimpleProfile_GetParameter( SIMPLEPROFILE_CHAR6, &BLEdata);
         LL_EXT_Decrypt( key1, BLEdata, BLEdata);
         if(BLEdata[0]=='O'){
             
             uint8 opendoor[10] = {0};
             memcpy(opendoor,BLEdata+4,10);
             BLE_Cmd(0x2A,0,10,0x01);  //蓝牙开门成功
             BLE_Data(0x2A,0,10,opendoor);
             alert.FPERROR_state = 0;
             alert.CODEERROR_state = 0;
         } 
        }
       }
      break;
   case SIMPLEPROFILE_CHAR7: 
      {
        memset(&char7Data,0,sizeof(char7Data));
        memset(&char7DataSub,0,sizeof(char7DataSub));
        
        SimpleProfile_GetParameter( SIMPLEPROFILE_CHAR7, &char7Data);      
        for(uint8 i=0;i<16;i++)
		{     
          char7DataSub[i]=char7Data[i+4];
        }
        LL_EXT_Decrypt( key1, char7DataSub, char7DataSub);
        
        uint8 ndatalen=char7Data[1]-64;//A==65
        
        /*
        if(char7Data[2]-48 != 0 && char7Data[3]-48 == 0){
            
             finalData = (uint8**)calloc(char7Data[2]-47, 1*sizeof(uint8_t));//16 * (char7Data[2]-47)
             for(int i = 0; i < char7Data[2]-47; i++){
              
               finalData[i] = (uint8 *)calloc(16, 1*sizeof(uint8_t));
             }
        } 
        */
        
        
        if(char7Data[0]=='N'){

          if(char7Data[2]-48 == 0 && char7Data[3]-48 == 0){
             
            uint8 finalDataOne[16] = {0};
             
             for(uint8 i = 0; i < ndatalen; i++){
             
               finalDataOne[i] = char7DataSub[i];
             }
             
             BLE_Cmd(0x53,0,ndatalen,0);
             BLE_Data(0x53,0,ndatalen,finalDataOne);
           }
           if(char7Data[2]-48 != 0){
            
             for(uint8 i = 0; i < ndatalen; i++){

               finalData[char7Data[3]-48][i] = char7DataSub[i];
             }
             if(char7Data[2] == char7Data[3]){

               uint8 totallen = (char7Data[2]-48) * 16 + ndatalen;
               uint8 write_temp[32] = {0};
               for(uint8 i = 0; i < totallen; i++){

                 write_temp[i] = finalData[i/16][i%16];
               }
               
               BLE_Cmd(0x53,0,ndatalen + 16 * (char7Data[2]-48),0);
               BLE_Data(0x53,0,ndatalen + 16 * (char7Data[2]-48),write_temp);//
             }
           }
        } else if(char7Data[0]=='P'){

           if(char7Data[2]-48 == 0 && char7Data[3]-48 == 0){
             
             uint8 finalDataOne[16] = {0};
             
             for(uint8 i = 0; i < ndatalen; i++){
             
               finalDataOne[i] = char7DataSub[i];
             }
             
             BLE_Cmd(0x54,0,ndatalen,0);
             BLE_Data(0x54,0,ndatalen,finalDataOne);
           }
           if(char7Data[2]-48 != 0){
            
             for(uint8 i = 0; i < ndatalen; i++){

               finalData[char7Data[3]-48][i] = char7DataSub[i];
             }
             if(char7Data[2] == char7Data[3]){

               uint8 totallen = (char7Data[2]-48) * 16 + ndatalen;
               uint8 write_temp[32] = {0};
               for(uint8 i = 0; i < totallen; i++){

                 write_temp[i] = finalData[i/16][i%16];
               }
               
               BLE_Cmd(0x54,0,ndatalen + 16 * (char7Data[2]-48),0);
               BLE_Data(0x54,0,ndatalen + 16 * (char7Data[2]-48),write_temp);
             }
           }
        } else if(char7Data[0]=='I')//输入用户id
         {
            for(uint8 i=0;i<11;i++)
           {
             userd.user[0].userid[i]=char7DataSub[i];
           }
         }else if(char7Data[0]=='C')//输入control password
         {
           for(uint8 i=0;i<6;i++)
           {
            userd.user[0].kcode[i]=char7DataSub[i]-48;
           }
           userd.factory=1; 
           userd.singleV=0; 
           userd.custom=1;
           userd.voice = 1;
           workmode=1;
           wifisinglemode=0;
          
           osal_snv_write(0xfa, (uint8)sizeof(slFlash_t), (uint8 *)&userd);
           //初始化成功
            BLE_Cmd(0x44,0,0,1);
            
            PlayVoiceP1(INIT_SUCCESS);
           //notify 出去
           Send_Notify_EVT_With("initsuccess");
           
           //下次同步数据
           //need_update++;
         } else if(char7Data[0]=='Y'){  // ali key
          
           if(char7Data[2]-48 == 0 && char7Data[3]-48 == 0){
             
             uint8 finalDataOne[16] = {0};
             
             for(uint8 i = 0; i < ndatalen; i++){
             
               finalDataOne[i] = char7DataSub[i];
             }
             
             BLE_Cmd(0x0E,0,ndatalen,0);
             BLE_Data(0x0E,0,ndatalen,finalDataOne);
           }
           if(char7Data[2]-48 != 0){
            
             for(uint8 i = 0; i < ndatalen; i++){

               finalData[char7Data[3]-48][i] = char7DataSub[i];
             }
             if(char7Data[2] == char7Data[3]){

               uint8 totallen = (char7Data[2]-48) * 16 + ndatalen;
               uint8 write_temp[32] = {0};
               for(uint8 i = 0; i < totallen; i++){

                 write_temp[i] = finalData[i/16][i%16];
               }
               
               BLE_Cmd(0x0E,0,ndatalen + 16 * (char7Data[2]-48),0);
               BLE_Data(0x0E,0,ndatalen + 16 * (char7Data[2]-48),write_temp);
             }
           }
         } else if(char7Data[0]=='S'){ //ali secret
          
           if(char7Data[2]-48 == 0 && char7Data[3]-48 == 0){
             
             uint8 finalDataOne[16] = {0};
             
             for(uint8 i = 0; i < ndatalen; i++){
             
               finalDataOne[i] = char7DataSub[i];
             }
             
             BLE_Cmd(0x0F,0,ndatalen,0);
             BLE_Data(0x0F,0,ndatalen,finalDataOne);

             //传macID
             //获取蓝牙地址发送给wifi
             TempMacAddress[5]=XREG(0x780E);                 // 直接指向指针内容  
             TempMacAddress[4]=XREG(0x780F);  
             TempMacAddress[3]=XREG(0x7810);  
             TempMacAddress[2]=XREG(0x7811);                 // define 函数直接读出数据  
             TempMacAddress[1]=XREG(0x7812);  
             TempMacAddress[0]=XREG(0x7813); 
             
             BLE_Cmd(0x01,0,6,0);
             BLE_Data(0x01,0,0x06, TempMacAddress);
             
             //osal_stop_timerEx(simpleBLEPeripheral_TaskID, GoSleep_EVT);
             mqttvoice=1;
             
             PlayVoiceP1(START_LINK);
           }
           if(char7Data[2]-48 != 0){
            
             for(uint8 i = 0; i < ndatalen; i++){

               finalData[char7Data[3]-48][i] = char7DataSub[i];
             }
             if(char7Data[2] == char7Data[3]){

               uint8 totallen = (char7Data[2]-48) * 16 + ndatalen;
               uint8 write_temp[32] = {0};
               for(uint8 i = 0; i < totallen; i++){

                 write_temp[i] = finalData[i/16][i%16];
               }
               
               BLE_Cmd(0x0F,0,ndatalen + 16 * (char7Data[2]-48),0);
               BLE_Data(0x0F,0,ndatalen + 16 * (char7Data[2]-48),write_temp);
              
               //获取蓝牙地址发送给wifi
               TempMacAddress[5]=XREG(0x780E);                 // 直接指向指针内容  
               TempMacAddress[4]=XREG(0x780F);  
               TempMacAddress[3]=XREG(0x7810);  
               TempMacAddress[2]=XREG(0x7811);                 // define 函数直接读出数据  
               TempMacAddress[1]=XREG(0x7812);  
               TempMacAddress[0]=XREG(0x7813); 
               
               BLE_Cmd(0x01,0,6,0);
               BLE_Data(0x01,0,0x06, TempMacAddress);
               
               //osal_stop_timerEx(simpleBLEPeripheral_TaskID, GoSleep_EVT);
               mqttvoice=1;
               PlayVoiceP1(START_LINK);
             }
           }
         }
      }
      break;
    default:
      // should not reach here!
      break;
  }
}

#if (defined HAL_LCD) && (HAL_LCD == TRUE)
/*********************************************************************
 * @fn      bdAddr2Str
 *
 * @brief   Convert Bluetooth address to string. Only needed when
 *          LCD display is used.
 *
 * @return  none
 */
char *bdAddr2Str( uint8 *pAddr )
{
  uint8       i;
  char        hex[] = "0123456789ABCDEF";
  static char str[B_ADDR_STR_LEN];
  char        *pStr = str;

  *pStr++ = '0';
  *pStr++ = 'x';

  // Start from end of addr
  pAddr += B_ADDR_LEN;

  for ( i = B_ADDR_LEN; i > 0; i-- )
  {
    *pStr++ = hex[*--pAddr >> 4];
    *pStr++ = hex[*pAddr & 0x0F];
  }

  *pStr = 0;

  return str;
}
#endif 
//绑定过程中的密码管理回调函数
static void ProcessPasscodeCB(uint8 *deviceAddr,uint16 connectionHandle,uint8 uiInputs,uint8 uiOutputs )
{
  uint32  passcode;
  uint8   str[7];

  //在这里可以设置存储，保存之前设定的密码，这样就可以动态修改配对密码了。
  // Create random passcode
#if 0
  LL_Rand( ((uint8 *) &passcode), sizeof( uint32 ));
  passcode %= 1000000;
#else
  passcode = 123456;        // 连接密码， 固定的 123456
#endif
  //在lcd上显示当前的密码，这样手机端，根据此密码连接。
  // Display passcode to user
  if ( uiOutputs != 0 )
  {
    HalLcdWriteString( "Passcode:",  HAL_LCD_LINE_1 );
    HalLcdWriteString( (char *) _ltoa(passcode, str, 10),  HAL_LCD_LINE_2 );
  }
  
  // Send passcode response  发送密码请求给主机
  GAPBondMgr_PasscodeRsp( connectionHandle, SUCCESS, passcode );
}

//绑定过程中的状态管理，在这里可以设置标志位，当密码不正确时不允许连接。
static void ProcessPairStateCB( uint16 connHandle, uint8 state, uint8 status )
{
  if ( state == GAPBOND_PAIRING_STATE_STARTED )/*主机发起连接，会进入开始绑定状态*/
  {
    HalLcdWriteString( "Pairing started", HAL_LCD_LINE_1 );
	gPairStatus = 0;
  }
  else if ( state == GAPBOND_PAIRING_STATE_COMPLETE )/*当主机提交密码后，会进入完成*/
  {
    if ( status == SUCCESS )
    {
      HalLcdWriteString( "Pairing success", HAL_LCD_LINE_1 );/*密码正确*/
	  gPairStatus = 1;
   
      GAPRole_TerminateConnection();  // 终止连接
       HAL_SYSTEM_RESET();
    }
    else
    {
      HalLcdWriteStringValue( "Pairing fail", status, 10, HAL_LCD_LINE_1 );/*密码不正确，或者先前已经绑定*/
	  if(status ==8){/*已绑定*/
		gPairStatus = 1;
	  }else{
		gPairStatus = 0;
	  }
    }
	//判断配对结果，如果不正确立刻停止连接。
	if(simpleBLEState == BLE_STATE_CONNECTED && gPairStatus !=1){
	  GAPRole_TerminateConnection();  // 终止连接
      // 终止连接后， 需要复位从机
      HAL_SYSTEM_RESET();
    }
  }
  else if ( state == GAPBOND_PAIRING_STATE_BONDED )
  {
    if ( status == SUCCESS )
    {
      HalLcdWriteString( "Bonding success", HAL_LCD_LINE_1 );
      
    }
  }

}

/****************************************************************************
* 名    称: UART1_ISR(void) 串口中断处理函数 
* 描    述: 当串口1产生接收中断，将收到的数据保存在RxBuf中
****************************************************************************/
#pragma vector = URX0_VECTOR 
__interrupt void UART0_ISR(void) //和指纹通信
{
  
   char R0Buf;
   URX0IF = 0;       // 清中断标志 
   R0Buf = U0DBUF;   
   uint8_t uc0Temp;
   uc0Temp = R0Buf;
   //uint8_t CheckCode=0; //êy?YD￡?é??
   //UartSendString(&uc0Temp,1);
   if(uc0Temp==0xf5)
     check++;
 
   if((fgbuffer[1] == 0x31 || fgbuffer[1] == 0x23) && fgbuffer[9] == 1)//指纹特征值数据包处理
   {
     fgbuffer1[FeaturnSum1] = uc0Temp;
     if(FeaturnSum1 >= 31)
	 {
	    fgbuffer1[31] = 1;
            BLE_Cmd(0x32,0x01,0xf2,0);
            BLE_Data(0x32,0x01,0xf2,fgbuffer1);
            FeaturnSum1=0;
            fgbuffer[9]=0;
            memset(fgbuffer1,0,sizeof(fgbuffer1));
            wifisenddata[0]=0;
          }  
       else
	 FeaturnSum1++;
   }
	          
    if((uc0Temp==0xf5)||fgbuffer[0] ==0xf5)
    {
       fgbuffer[MessageSum1] = uc0Temp;
       if(MessageSum1 >= 7)
       {
	 fgbuffer[9] = 1;
       }
	   else
       MessageSum1++;
       
    }
    if(fgbuffer[1]==0xf5)
    {
       check=1;
       memset(fgbuffer,0,sizeof(fgbuffer));
       fgbuffer[0]=0xf5;
       MessageSum1=1;
    } 
    if(check>=2)
    {
       P1IEN|=0x20;
       fpprocess=0;
       wifisenddata[0]=0x00;
       osal_stop_timerEx( simpleBLEPeripheral_TaskID, SBP_PERIODIC_EVT1 ); //将复位机制关闭
       
             switch(fgbuffer[1] )
                {
                  case 0x01:
#ifdef CR
                  if(LocalPro==0)
                  {
                    if(fgbuffer[4]==0)//第一次成功
                     {
                        {
                          getfinger=1;
                          Send_Notify_EVT_With("100000");
                          BLE_Cmd(0x07,0,0,0);
                          osal_start_timerEx(simpleBLEPeripheral_TaskID, FP_Voice_EVT, 50);
                          osal_start_timerEx(simpleBLEPeripheral_TaskID, FPDatafx_EVT,500); 
                        }
                     }else
                        {
                          luru=0;
                         // BLE_Cmd(0x30,0,0,1);
                          
                          PlayVoiceP1(Finger_ADD_FAIL); //新的录指纹
                          Send_Notify_EVT_With("100001");
                          
                          //PlayVoiceP1(0x0d);//旧的录指纹 失败
                          //osal_start_timerEx(simpleBLEPeripheral_TaskID, FPHSTIMEOUT_EVT, 5000);
                        }
                   }else
                   {
                      if(fgbuffer[4]==0)//第一次成功
                     {
                        {
                          getfinger=1;
                          fpvoice = 2;
                          osal_start_timerEx(simpleBLEPeripheral_TaskID, FP_Voice_EVT, 50);
                          osal_start_timerEx(simpleBLEPeripheral_TaskID, FPDatafx_EVT,500); 
                        }
                     }
                      else
                        {
                          kchange = false;
                          BLE_Cmd(0x72,0,0,1);
                          PlayVoiceP1(VERIFY_FAIL); 
                          delay5ms();
                          delay5ms();
                          localerror++;
                          if(localerror<3)
                          {
                            PlayVoiceP1(Finger_RQ);     //按键音
                          }
//                      
                        }
                   
                   }
#endif
                    break;
                  
                  case 0x02:
#ifdef CR           
                    if(LocalPro==0)
                    {
                      
                      if(fgbuffer[4]==0)//第二次成功
                     {
                          getfinger=2;
                          Send_Notify_EVT_With("100000");
                          BLE_Cmd(0x07,0,0,0);
                          osal_start_timerEx(simpleBLEPeripheral_TaskID, FP_Voice_EVT, 50);
                          osal_start_timerEx(simpleBLEPeripheral_TaskID, FPDatafx_EVT,1050);
                     }else
                        {
                          luru=0;
                         // BLE_Cmd(0x30,0,0,1);
                          
                          PlayVoiceP1(Finger_ADD_FAIL); 
                          Send_Notify_EVT_With("100001");
                        }
                    }
                    else
                    {
                        if(fgbuffer[4]==0)//第一次成功
                     {
                        {
                          getfinger=2;
                          osal_start_timerEx(simpleBLEPeripheral_TaskID, FP_Voice_EVT, 50);
                          osal_start_timerEx(simpleBLEPeripheral_TaskID, FPDatafx_EVT,1050); 
                        }
                     }else
                        {
                          kchange = false;
                          BLE_Cmd(0x72,0,0,1);
                          PlayVoiceP1(VERIFY_FAIL); 
                          delay5ms();
                          delay5ms();
                          localerror++;
                          if(localerror<3)
                          {
                            PlayVoiceP1(Finger_RQ);     //按键音
                          }
                        }
                    }
#endif                
                   break;
                  case 0x03://录入指纹
                      {
                        if(LocalPro==0)
                        {
                          if(fgbuffer[4]==0)//录入成功
                          {
                            getfinger=3;
                            
                            memset(sanswer,0,sizeof(sanswer));
                            sanswer[0]=fgbuffer[2];
                            sanswer[1]=fgbuffer[3];
                          
                             fp_rigister_userId_gl = (sanswer[0]<<8)&0xFF|sanswer[1];      

                             Send_Notify_EVT_With("200000");
                             BLE_Cmd(0x07,0,0,0);
                          //  osal_start_timerEx(simpleBLEPeripheral_TaskID, FPDatafx_EVT,500);
                          
                            fpid[0]=fgbuffer[2];
                            fpid[1]=fgbuffer[3];
                            luru=2;     //关闭中断验证指纹命令
                            //成功后要验证，要不要再提示按指纹
                            osal_start_timerEx(simpleBLEPeripheral_TaskID, FP_Voice_EVT, 50);
                            osal_start_timerEx(simpleBLEPeripheral_TaskID, FPTIMEOUT_EVT, 10000);
                          }
                          else
                          {
                            luru=0;
                            //BLE_Cmd(0x30,0,0,1);
                            PlayVoiceP1(Finger_ADD_FAIL); 
                            Send_Notify_EVT_With("200001");
                          }
                        }
#ifdef LOCAL                        
                        else
                        {
                          
                          if(fgbuffer[4]==0)//录入成功
                          {
                            fpid[0]=fgbuffer[2];
                            fpid[1]=fgbuffer[3];
                            osal_start_timerEx(simpleBLEPeripheral_TaskID, FP_Voice_EVT, 50);
                            osal_start_timerEx(simpleBLEPeripheral_TaskID, FPTIMEOUT_EVT, 10000);
                          }
                          else
                          {
                            kchange = false;
                            BLE_Cmd(0x72,0,0,1);
                            PlayVoiceP1(VERIFY_FAIL); 
                          delay5ms();
                          delay5ms();
                          localerror++;
                          if(localerror<3)
                          {
                            PlayVoiceP1(Finger_RQ);     //按键音
                          }
                        
                        }
#endif
                      }
                      }
                     break;
                      case 0x09:
                    break;
                    case 0x0B://验证指纹 录指纹用0b
                    {
                      if((0!=luru)&&(LocalPro==0))
                      {
                       osal_stop_timerEx( simpleBLEPeripheral_TaskID, FPTIMEOUT_EVT ); //将复位机制关闭 
                      luru=0;
                      if(fgbuffer[4]!=5)
                      {
                       if(fgbuffer[4]==0)//验证成功
                        {
                          PlayVoiceP1(Finger_ADD_SUCCESS);
                          Send_Notify_EVT_With("300000");
                          userd.user[index[0]].fp[index[1]]=fp_rigister_userId_gl;
                          fpadds = 1;
                          osal_start_timerEx(simpleBLEPeripheral_TaskID, Datacleanfx_EVT, 0);
                        }
                        else
                        {
                          PlayVoiceP1(Finger_ADD_FAIL);
                          Send_Notify_EVT_With("300001");
                          
                          if(fp_rigister_userId_gl != 0)
                          User_DeleteSpecific(fp_rigister_userId_gl);
                        }  
                      }
                      
                      else
                        {
                          PlayVoiceP1(VERIFY_FAIL);
                          Send_Notify_EVT_With("300001");
                          if(fp_rigister_userId_gl != 0)
                          User_DeleteSpecific(fp_rigister_userId_gl);
                        }
                    }else
                    {
                      if(fgbuffer[4]==0)//验证成功 
                        {
                         osal_stop_timerEx( simpleBLEPeripheral_TaskID, FPTIMEOUT_EVT ); //将复位机制关闭 
                         if(LocalPro/100==1)
                         {
                           BLE_Cmd(0x72,0,2,0);
                           BLE_Data(0x72,0,0x02,fpid);
                           if(userd.singleV != 1){
                        
                             userd.singleV = 1; 
                             osal_start_timerEx(simpleBLEPeripheral_TaskID, Datacleanfx_EVT, 0);
                           }
                            memset(fpid,0,sizeof(fpid));
                         }else if(LocalPro/100==2)
                         {
                           BLE_Cmd(0x74,0,2,0);
                           BLE_Data(0x74,0,0x02,fpid); //普通用户
                            memset(fpid,0,sizeof(fpid));
                         }
                         LocalPro=0;//管理员添加完成     
                         kchange = false;
                        }
                        else
                        {                     
                         
                          PlayVoiceP1(VERIFY_FAIL);
                          if(fp_rigister_userId_gl != 0)
                          User_DeleteSpecific(fp_rigister_userId_gl); 
                          memset(fpid,0,sizeof(fpid));
                          if(LocalPro/100==1)
                         {
                           BLE_Cmd(0x72,0,0,1);
                          
                         }else if(LocalPro/100==2)
                         {
                           BLE_Cmd(0x74,0,0,1);
                       
                         }  
                          kchange = false;
                          delay5ms();
                          delay5ms();
                          localerror++;
                          if(localerror<3)
                          {
                            PlayVoiceP1(Finger_RQ);     //按键音
                          }
                        }                                        
                    	}
                    }
                    break;
                    
                    case 0x0C://验证指纹 验证指纹用oc
                    {                  
#ifdef LOCAL
                      if((LocalPro%10)==2)
                      {
                         if(fgbuffer[2]!=0||fgbuffer[3]!=0)//验证成功
                        {
                          memset(sanswer,0,sizeof(sanswer));
                          sanswer[0]=fgbuffer[2];
                          sanswer[1]=fgbuffer[3];
                         
                           BLE_Cmd(0x71,0,2,0);
                           BLE_Data(0x71,0,0x02,sanswer);  
                        }
                       else
                        {
                           alert.FPERROR_state++; 
                           BLE_Cmd(0x71,0,0,1);

                            if(alert.FPERROR_state>=5){
                              
                              PlayVoiceP1(Finger_LOCK);
                              osal_start_timerEx(simpleBLEPeripheral_TaskID, ClearFPLock_EVT,180000);
                            } else {
                              
                              PlayVoiceP1(VERIFY_FAIL); 
                            }
                        }
                      }else
#endif
                      {
                        if(luru == 0){
                          
                       uint16_t fp_id_get = 0;
                       if((fgbuffer[2]!=0||fgbuffer[3]!=0)&&luru==0)//验证成功
                        {
                          memset(sanswer,0,sizeof(sanswer));
                          sanswer[0]=fgbuffer[2];
                          sanswer[1]=fgbuffer[3];
                          fp_id_get =(sanswer[0]<<8)&0xFF|sanswer[1]; 
                          
                          //BLE_Cmd(0x29,0,0,fp_id_get);
                          delay5ms();//20170814加上调试用以跳过wifi串口接收建立之前的所需时间
                           
                           bool very=false;
                             for(uint8 i=0;i<userd.custom;i++)
                          {
                            for(uint8 u=0;u<5;u++)
                            {
                              if(fp_id_get==userd.user[i].fp[u])//验证成功// userd.user[index[i].fp[index[1]]=
                                    {
                                       very=true;
                                       alert.FPERROR_state = 0;
                                       alert.CODEERROR_state = 0;
                                       if(u == 4){ //胁迫报警
                                         BLE_Cmd(0x2B,0,11,3);
                                         BLE_Data(0x2B,0,11,userd.user[i].userid);
                                         delay5ms();
                                       } 
                                         BLE_Cmd(0x2A,0,11,3);
                                         BLE_Data(0x2A,0,11,userd.user[i].userid); 
                                       break;
                                    }    
                            }
                          }
                          if(very!=true)
                          {
                           if(wifisinglemode==1){
                             
                            memset(sanswer,0,sizeof(sanswer));
                            sanswer[0]=fgbuffer[2];
                            sanswer[1]=fgbuffer[3];
                         
                            BLE_Cmd(0x31,0,2,0);
                            BLE_Data(0x31,0,0x02,sanswer);
                           }else {
                            
                            alert.FPERROR_state++;
                            if(alert.FPERROR_state>=5){
                              
                              BLE_Cmd(0x2B,0,0,1);
                              PlayVoiceP1(Finger_LOCK);
                              osal_start_timerEx(simpleBLEPeripheral_TaskID, ClearFPLock_EVT,180000);
                            } else {
                              
                              PlayVoiceP1(VERIFY_FAIL); 
                            }
                           }
                          }
                        }
                       else if(fgbuffer[4]!=8)
                        {
                            alert.FPERROR_state++;
                            if(alert.FPERROR_state>=5)
                           {
                               BLE_Cmd(0x2B,0,0,1);
                               PlayVoiceP1(Finger_LOCK);
                               osal_start_timerEx(simpleBLEPeripheral_TaskID, ClearFPLock_EVT,180000);
                           } else {
                             
                             PlayVoiceP1(VERIFY_FAIL);
                           }
                          
                        }
                        }//luru != 0
                        else {
                        
                          if(luru == 1){
                            
                            osal_start_timerEx(simpleBLEPeripheral_TaskID, FPHSTIMEOUT_EVT, 100);
                          }
                        }
                      }
                    }
                    break;
                    
                    case 0x0D://获取未使用ID
                    {
                      if(fgbuffer[2]!=0||fgbuffer[3]!=0)//有可用ID
                        {                            
                          fpprocess=0;
                          NC = 0;
                          memset(fgbuffer1,0,sizeof(fgbuffer1));
                          wifisenddata[0]=0;
                        }
                    }
                    break;
                    case 0x05://删除所有指纹
                    {
                     case 0x41://下传成功
                    {
                       if(fgbuffer[4]==0)//下传成功
                       {
                        BLE_Cmd(0x33,0,0,0);  
#ifdef bank0
                        PlayVoiceP1(KEY_VIOCE);
#endif
                        wifisenddata[0]=0;
                        NC=0;
                       }
                       else  //下传失败
                       {
                         BLE_Cmd(0x33,0,0,0xff);
#ifdef bank0
                         PlayVoiceP1(VERIFY_FAIL);
#endif
                         wifisenddata[0]=0;
                         NC=0;
                       }
                    }
                    break;
                  }
                 
                 P1DIR&=~0x20;    //设为输入
                 
	               //把收到的数据发送到串口-实现回环 
                  // UartSendString(buffer, numBytes);
                 MessageSum1=0;
                 check=0;
                 if(fgbuffer[1] !=0x31)
                 {
                     fgbuffer[9] =0;
                 }
   
                  fgbuffer[0] =0;
                  //fpprocess=0;
                  FingerReceMes[0]=0;
                  wifisenddata[0]=0;
    }
}
/****************************************************************************
* 名    称: wechatIOTGetMacAddrUpdatetoAdvertData()
* 功    能: 广播名
* 入口参数: 无
* 出口参数: 无
****************************************************************************/

static void wechatIOTGetMacAddrUpdatetoAdvertData( void )
{
	
	GAPRole_GetParameter(GAPROLE_BD_ADDR, ownAddress);

	
	//(Mov)Abellee 20140716 move advertData for Wechat need MAC ID
	{
		advertData[ADVERT_MAC_ADDR+0] = ownAddress[5];
		advertData[ADVERT_MAC_ADDR+1] = ownAddress[4];
		advertData[ADVERT_MAC_ADDR+2] = ownAddress[3];
		advertData[ADVERT_MAC_ADDR+3] = ownAddress[2];
		advertData[ADVERT_MAC_ADDR+4] = ownAddress[1];
		advertData[ADVERT_MAC_ADDR+5] = ownAddress[0];
		ownAddress[5] = advertData[ADVERT_MAC_ADDR+5];
		ownAddress[4] = advertData[ADVERT_MAC_ADDR+4];
		ownAddress[3] = advertData[ADVERT_MAC_ADDR+3];
		ownAddress[2] = advertData[ADVERT_MAC_ADDR+2];
		ownAddress[1] = advertData[ADVERT_MAC_ADDR+1];
		ownAddress[0] = advertData[ADVERT_MAC_ADDR+0];

		 uint8 scanRspDatamac1[12];
                sprintf(scanRspDatamac1, "%02x%02x%02x%02x%02x%02x", ownAddress[0],ownAddress[1],ownAddress[2],ownAddress[3],ownAddress[4],ownAddress[5]);
		for(uint8 i=0;i<12;i++)
                {
                  scanRspDatamac[2+i]=scanRspDatamac1[i];
                }
                GAP_UpdateAdvertisingData(simpleBLEPeripheral_TaskID,   
                              TRUE,  
                              sizeof(advertData),  
                              advertData );
                 GAP_UpdateAdvertisingData(simpleBLEPeripheral_TaskID,   
                              FALSE,  
                              sizeof(scanRspDatamac),  
                              scanRspDatamac );
                bool new_adv_enabled_status = TRUE;
        // change the GAP advertisement status to opposite of current status
                GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &new_adv_enabled_status );
		VOID osal_memcpy(mac, &ownAddress[0], 6 );
           
	}
}
/****************************************************************************
* 名    称: Send_Notify_EVT_With()
* 功    能: 广播状态
* 入口参数: char6
* 出口参数: 无
****************************************************************************/
void Send_Notify_EVT_With(uint8 char6[16]){
 
  if(char6[5] == 0x30 && char6[0] == 0x33){
    //添加完指纹后，                  
     P1IEN &= ~0x21;
     voiceb=1;
     
    //等wifi0x04
     BLE_Cmd(0x07,0,0,1);
  } else if(char6[5] == 0x31){
    //voiceb=1;
    
    osal_start_timerEx(simpleBLEPeripheral_TaskID, FPTIMEOUT_EVT, 10000);
  }
  SimpleProfile_SetParameter( SIMPLEPROFILE_CHAR6, 16, char6);       
  osal_start_timerEx( simpleBLEPeripheral_TaskID, SBP_PERIODIC_EVT1, 50);
  osal_start_timerEx( simpleBLEPeripheral_TaskID, Delay_Check_EVT, 200);
}
