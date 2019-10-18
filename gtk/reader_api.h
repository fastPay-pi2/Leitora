/* file:          reader_api.h
 * 
 * description:   Gen2 RFID reader host software API.
 *                This API uses the BSD sockets.
 * 
 * date: 		  05/2019
 * author:		  Eduardo Gonçalves Sousa <eng.eduardo.gs@gmail.com>
 *
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netdb.h>
#include <time.h>

/*
reader error codes
*/

#define ERR_OK			0
#define ERR_SOCKET_ERROR	-100
#define ERR_SOCKET_NORESP	-101
#define ERR_SOCKET_SEND		-102
#define ERR_SOCKET_TRID		-103
#define ERR_SOCKET_WRONGDATA	-104

/*
reader internal definitions - DON'T TOUCH!
*/
#define UDP_PORT_COMMAND	8080			/* reader UDP command port */
#define UDP_COMM_TIMEOUT	3			/* socket timeout (in s) */
#define UDP_COMM_TIMEOUT_MS	0			/* socket timeout (in ms) */
#define HOSTMSG_SIZE		1500			/* host temporary buffers size */

#define TRID_OFS			0
#define CMD_RESP_OFS		4
#define GEN2_RESP_OFS		6
#define USER_DATA_OFS		8
#define HEADER_SIZE			8

/*
access bits for the control of the rfid module
*/
#define BIT_RESPONSE_LENGTH		6
#define BIT_RESPONSE_CODE		7
#define BIT_START_RESPONSE		8
#define BIT_NUM_TAGS			9
#define BIT_ALL_TAGS_LENGTH_MSB	10
#define BIT_ALL_TAGS_LENGTH_LSB	11
#define BIT_START_TAGS			12

// Version
#define BIT_LENGTH_VERSION_RF			6
#define BIT_LENGTH_VERSION_DIGITAL		7

/* maximum number of tags kept on the reader for each inventory round */
/* DON'T TOUCH!!! */
#define TAGLIST_SIZE		22
#define TAGLIST_SIZE_2		21
#define TAGLIST_SIZE_3		11
#define TAGLIST_SIZE_4		10

/*###############/
/* RFID MODULE  */
/*##############*/

#define PREAMBLE 0xbb
#define END_MARK 0x7e

// Message - Type
#define MSG_TYPE_COMMAND 0x00
#define MSG_TYPE_RESPONSE 0x01
#define MSG_TYPE_NOTIFICATION 0x02
#define MSG_TYPE_ERROR 0xFF
#define END_NOTIFICATION 0x1F

// Message - Code
#define SET_READER_POWER_CONTROL 0x01
#define GET_READER_INFORMATION 0x03
#define GET_REGION 0x06
#define SET_REGION 0x07
#define SET_SYSTEM_RESET 0x08
#define GET_TYPE_C_AI_SELECT_PARAMETERS 0x0b
#define SET_TYPE_C_AI_SELECT_PARAMETERS 0x0c
#define GET_TYPE_C_AI_QUERY_RELATED_PARAMETERS 0x0d
#define SET_TYPE_C_AI_QUERY_RELATED_PARAMETERS 0x0e
#define GET_CURRENT_RF_CHANNEL 0x11
#define SET_CURRENT_RF_CHANNEL 0x12
#define GET_FH_AND_LBT_PARAMETERS 0x13
#define SET_FH_AND_LBT_PARAMETERS 0x14
#define GET_TX_POWER_LEVEL 0x15
#define SET_TX_POWER_LEVEL 0x16
#define RF_CW_SIGNAL_CONTROL 0x17
#define READ_TYPE_C_UII 0x22
#define READ_TYPE_C_UII_RSSI 0x23
#define READ_TYPE_C_UII_TID 0x25
#define READ_TYPE_C_TAG_DATA 0x29
#define READ_TYPE_C_TAG_LONG_DATA 0x2a
#define GET_SESSION 0x2e
#define SET_SESSION 0x2f
#define GET_FREQUENCY_HOPPING_TABLE 0x30
#define SET_FREQUENCY_HOPPING_TABLE 0x31
#define GET_MODULATION 0x32
#define SET_MODULATION 0x33
#define GET_ANTI_COLLISION_MODE 0x34
#define SET_ANTI_COLLISION_MODE 0x35
#define START_AUTO_READ2 0x36
#define STOP_AUTO_READ2 0x37
#define START_AUTO_READ_RSSI 0x38
#define STOP_AUTO_READ_RSSI 0x39
#define WRITE_TYPE_C_TAG_DATA 0x46
#define BLOCKWRITE_TYPE_C_TAG_DATA 0x47
#define BLOCKERASE_TYPE_C_TAG_DATA 0x48
#define BLOCKPERMALOCK_TYPE_C_TAG 0x83
#define KILL_RECOM_TYPE_C_TAG 0x65
#define LOCK_TYPE_C_TAG 0x82
#define ANTENNA_CHECK 0xac
#define GET_TEMPERATURE 0xb7
#define GET_RSSI 0xc5
#define SCAN_RSSI 0xc6
#define GET_DTC_RESULT 0xca
#define UPDATE_REGISTRY 0xd2
#define GET_REGISTRY_ITEM 0xd4
#define SET_OPTIMUM_FREQUENCY_HOPPING_TABLE 0xe4
#define GET_FREQUENCY_HOPPING_MODE 0xe5
#define SET_FREQUENCY_HOPPING_MODE 0xe6
#define GET_TX_LEAKAGE_RSSI_LEVEL_FOR_SMART_HOPPING_MODE 0xe7
#define SET_TX_LEAKAGE_RSSI_LEVEL_FOR_SMART_HOPPING_MODE 0xe8
#define START_READ_WITH_FAST_LEAKAGE_CAL 0xec
#define REQUEST_FAST_LEAKAGE_CAL 0xed
//ARTEFATO SJ5511
#define START_AUTO_READ_ARTEFATO 0x40
#define STOP_AUTO_READ_ARTEFATO 0x41
#define START_AUTO_READ_RSSI_ARTEFATO 0x43
#define STOP_AUTO_READ_RSSI_ARTEFATO 0x44

/*
RFID module error codes
*/
#define EPC_G2V2_ERROR_01	0x01
#define EPC_G2V2_ERROR_02	0x02
#define EPC_G2V2_ERROR_03	0x03
#define EPC_G2V2_ERROR_04	0x04
#define EPC_G2V2_ERROR_05	0x05
#define EPC_G2V2_ERROR_06	0x06
#define EPC_G2V2_ERROR_07	0x07
#define EPC_G2V2_ERROR_08	0x08
#define EPC_G2V2_ERROR_09	0x0B
#define EPC_G2V2_ERROR_10	0x0F
#define VENDOR_SPECIFIC_ERROR_01	0x11
#define VENDOR_SPECIFIC_ERROR_02	0x12
#define VENDOR_SPECIFIC_ERROR_03	0x13
#define PROTOCOL_ERROR_01	0x80
#define PROTOCOL_ERROR_02	0x81
#define PROTOCOL_ERROR_03	0x82
#define MODEM_ERROR_01	0x90
#define MODEM_ERROR_02	0x91
#define REGISTRY_ERROR_01	0xA0
#define REGISTRY_ERROR_02	0xA1
#define REGISTRY_ERROR_03	0xA2
#define REGISTRY_ERROR_04	0xA3
#define PERIPHERAL_ERROR_01	0xB0
#define PERIPHERAL_ERROR_02	0xB1
#define PERIPHERAL_ERROR_03	0xB2
#define PERIPHERAL_ERROR_04	0xB3
#define CUSTOM_ERROR_01	0xE0
#define CUSTOM_ERROR_02	0xE1
#define CUSTOM_ERROR_03	0xE2
#define CUSTOM_ERROR_04	0xE3
#define CUSTOM_ERROR_05	0xE4
#define CUSTOM_ERROR_06	0xE5
#define CUSTOM_ERROR_07	0xE6
#define CUSTOM_ERROR_08	0xE7
#define CUSTOM_ERROR_09	0xE8
#define CUSTOM_ERROR_10	0xE9
#define CUSTOM_ERROR_11	0xEA
#define CUSTOM_ERROR_12	0xEB
#define CUSTOM_ERROR_13	0xF0
#define CUSTOM_ERROR_14	0xF1
#define CUSTOM_ERROR_15	0xFC
#define CUSTOM_ERROR_16	0xFF

// Gen2 TX power
#define RF_POWER_DEFAULT	30		// 30 dBm (default)
#define RF_POWER_01DBM		1		
#define RF_POWER_02DBM		2		
#define RF_POWER_03DBM		3		
#define RF_POWER_04DBM		4		
#define RF_POWER_05DBM		4		
#define RF_POWER_06DBM		6		
#define RF_POWER_07DBM		7		
#define RF_POWER_08DBM		8		
#define RF_POWER_09DBM		9		
#define RF_POWER_10DBM		10		
#define RF_POWER_11DBM		11		
#define RF_POWER_12DBM		12		
#define RF_POWER_13DBM		13		
#define RF_POWER_14DBM		14		
#define RF_POWER_15DBM		15		
#define RF_POWER_16DBM		16		
#define RF_POWER_17DBM		17		
#define RF_POWER_18DBM		18		
#define RF_POWER_19DBM		19		
#define RF_POWER_20DBM		20		
#define RF_POWER_21DBM		21		
#define RF_POWER_22DBM		22		
#define RF_POWER_23DBM		23		
#define RF_POWER_24DBM		24		
#define RF_POWER_25DBM		25		
#define RF_POWER_26DBM		26		
#define RF_POWER_27DBM		27		
#define RF_POWER_28DBM		28		
#define RF_POWER_29DBM		29		
#define RF_POWER_30DBM		30

// Gen2 backscatter link frequency BLF -> USED ONLY TO CONFIG READER DEFAULT SETTINGS
#define GEN2_BLF_DEFAULT	4		// 320 kHz (default)
#define GEN2_BLF_40			0		// 40 kHz
#define GEN2_BLF_80			1		// 80 kHz
#define GEN2_BLF_160		2		// 160 kHz
#define GEN2_BLF_250		3		// 250 kHz
#define GEN2_BLF_320		4		// 320 kHz
#define GEN2_BLF_640		5		// 640 kHz

// received BLF
#define BLF_40		0x0028		// 40 kHz
#define BLF_80		0x0050		// 80 kHz
#define BLF_160		0x00A0		// 160 kHz
#define BLF_250		0x00FA		// 250 kHz
#define BLF_320		0x0140		// 320 kHz
#define BLF_640		0x0280		// 640 kHz

// Gen2 RX coding
#define GEN2_CODING_DEFAULT	0		// FM0 (default)
#define GEN2_CODING_FM0		0		// FM0
#define GEN2_CODING_M2		1		// Miller 2 (Dense Reader Mode)
#define GEN2_CODING_M4		2		// Miller 4 (Dense Reader Mode)
#define GEN2_CODING_M8		3		// Miller 8 (Dense Reader Mode)

// Gen2 Q mode
#define GEN2_Q_MODE_DEFAULT		0		// Fixed (default)
#define GEN2_Q_MODE_FIXED		0		// Fixed
#define GEN2_Q_MODE_DYNAMIC		1		// Dynamic

// Gen2 initial Q
#define GEN2_IQ_DEFAULT		4		// Q4 (default)
#define GEN2_IQ_Q4		4		// Q4
#define GEN2_IQ_Q5		5		// Q5
#define GEN2_IQ_Q6		6		// Q6
#define GEN2_IQ_Q7		7		// Q7
#define GEN2_IQ_Q8		8		// Q8

// Gen2 Q
#define GEN2_Q_Q2		2		// Q4
#define GEN2_Q_Q3		3		// Q5
#define GEN2_Q_Q4		4		// Q6
#define GEN2_Q_Q5		5		// Q7
#define GEN2_Q_Q6		6		// Q8
#define GEN2_Q_Q7		7		// Q4
#define GEN2_Q_Q8		8		// Q5
#define GEN2_Q_Q9		9		// Q6
#define GEN2_Q_Q10		10		// Q7
#define GEN2_Q_Q11		11		// Q8
#define GEN2_Q_Q12		12		// Q8

// Gen2 DR
#define GEN2_DR_DEFAULT		0		// Q4 (default)
#define GEN2_DR_8		0		// 8
#define GEN2_DR_21		1		// 64/3

// Session
#define GEN2_SESSION_DEVMOD	0xf0
#define GEN2_SESSION_S0 	0x00
#define GEN2_SESSION_S1		0x01
#define GEN2_SESSION_S2		0x02
#define GEN2_SESSION_S3		0x03

/*
reader type definitions
*/
struct readerinfo{
	// network configuration
	uint8_t ip_addr[20];
	uint8_t mask_addr[20];
	uint8_t gate_addr[20];

	// RF module configuration
	uint8_t tx_power; //0 to 30 [dBm]
	uint8_t blf; // 0 - 40kHz, 1 - 80kHz, 2 - 160kHz, 3 - 250kHz, 4 - 320kHz, 5 - 640kHz
	uint8_t encoding; // 0 - FM0, 1 - M2, 2 - M4, 3 - M8
	uint8_t dr; // 0 - 8, 1 - 64/3
	uint8_t anti_collision_q[4]; // [0] 0 - FIXED, 1 - Dynamic; [1] Q start; [2] Q min; [3] Q max
};

typedef struct readerinfo readerconfig_t;

struct readersession{
	int32_t sockfd, numbytes, addr_len;
	struct sockaddr_in their_addr;
	struct hostent *he;
	struct timeval tv;
};

typedef struct readersession reader_t;

typedef struct{
	uint32_t eth_frames;
	uint32_t arp_replies;
	uint32_t ip_packets;
	uint32_t ip_err_vhl;
	uint32_t ip_err_frag;
	uint32_t ip_err_destaddr;
	uint32_t ip_err_chksum;
	uint32_t ip_err_proto;
	uint32_t icmp_packets;
	uint32_t icmp_pings;
	uint32_t icmp_errors;
	uint32_t udp_datagrams;
	uint32_t udp_commands;
	uint32_t udp_err_chksum;
	uint32_t udp_err_port;
} ip_stack_data;

typedef union{
	ip_stack_data data;
	uint8_t raw_data[sizeof(ip_stack_data)];
} ip_stack_t;

typedef struct{
	uint32_t inv_tags;
	uint32_t read_tags;
	uint32_t write_tags;
	uint32_t conf_tags;
	uint32_t lock_tags;
	uint32_t kill_tags;
	uint32_t error_tags;
} gen2_data;

typedef union{
	gen2_data data;
	uint8_t raw_data[sizeof(gen2_data)];
} gen2_t;

uint32_t num_tags;

struct rcp{
	uint8_t preamble;
	uint8_t msg_type;
	uint8_t code;
	uint8_t payload_length[2];
	uint8_t payload[256];
	uint8_t end_mark;
	uint16_t crc_16;
};

typedef struct rcp _rcp;

struct tag1
{
    uint8_t pc[2];
    uint8_t epc[62];
};


typedef struct tag1 _tag1;

typedef union{
	_tag1 tags1[TAGLIST_SIZE];
	uint8_t raw_data[sizeof(_tag1) * TAGLIST_SIZE] ;
} tag1_t;

struct tag2
{
    uint8_t pc[2];
    uint8_t epc[62];
    uint8_t rssi[4];
};

typedef struct tag2 _tag2;

typedef union{
	_tag2 tags2[TAGLIST_SIZE];
	uint8_t raw_data[sizeof(_tag2) * TAGLIST_SIZE] ;
} tag2_t;

struct tag3
{
    uint8_t tid_len;
    uint8_t pc[2];
    uint8_t epc[62];
    uint8_t tid[62];
};

typedef struct tag3 _tag3;

typedef union{
	_tag3 tags3[TAGLIST_SIZE];
	uint8_t raw_data[sizeof(_tag3) * TAGLIST_SIZE] ;
} tag3_t;

// "Read type C tag long data"
struct tag4
{
    uint8_t start_addr[2];
    uint8_t word_count;
    uint8_t tag_mem[128];
};


typedef struct tag4 _tag4;

typedef union{
	_tag4 tags4[TAGLIST_SIZE];
	uint8_t raw_data[sizeof(_tag4) * TAGLIST_SIZE] ;
} tag4_t;

// for the command "AutoRead Artefato SJ5511 + Gen2v1/2"
struct tag5
{
    uint8_t pc[2];
    uint8_t epc[62];
    uint8_t r96[12];
    uint8_t data256[32];
};


typedef struct tag5 _tag5;

typedef union{
	_tag5 tags5[TAGLIST_SIZE];
	uint8_t raw_data[sizeof(_tag5) * TAGLIST_SIZE] ;
} tag5_t;

// for the command "AutoRead Artefato SJ5511 + Gen2v1/2 with RSSI"
struct tag6
{
    uint8_t pc[2];
    uint8_t epc[62];
    uint8_t rssi[4];
    uint8_t r96[12];
    uint8_t data256[32];
};

typedef struct tag6 _tag6;

typedef union{
	_tag6 tags6[TAGLIST_SIZE];
	uint8_t raw_data[sizeof(_tag6) * TAGLIST_SIZE] ;
} tag6_t;

/* reader configuration */
int32_t Gen2ReaderConfig(uint8_t *addr, readerconfig_t *new_config);
//int32_t Gen2ReaderAntennaConfig(uint8_t *addr, uint16_t antenna, antennaconfig_t *new_config);

/* reader session management */
int32_t Gen2ReaderCreate(uint8_t *addr, reader_t *session);
int32_t Gen2ReaderDestroy(reader_t *session);

/* remote reset */
int32_t Gen2ReaderReset(reader_t *session);
