#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <machine/_types.h>

#include "rdma_opcode.h"


enum {
	MLX5_CMD_OP_QUERY_HCA_CAP                   = 0x100,
	MLX5_CMD_OP_QUERY_ADAPTER                   = 0x101,
	MLX5_CMD_OP_INIT_HCA                        = 0x102,
	MLX5_CMD_OP_TEARDOWN_HCA                    = 0x103,
	MLX5_CMD_OP_ENABLE_HCA                      = 0x104,
	MLX5_CMD_OP_DISABLE_HCA                     = 0x105,
	MLX5_CMD_OP_QUERY_PAGES                     = 0x107,
	MLX5_CMD_OP_MANAGE_PAGES                    = 0x108,
	MLX5_CMD_OP_SET_HCA_CAP                     = 0x109,
	MLX5_CMD_OP_QUERY_ISSI                      = 0x10a,
	MLX5_CMD_OP_SET_ISSI                        = 0x10b,
	MLX5_CMD_OP_SET_DRIVER_VERSION              = 0x10d,
	MLX5_CMD_OP_QUERY_SF_PARTITION              = 0x111,
	MLX5_CMD_OP_ALLOC_SF                        = 0x113,
	MLX5_CMD_OP_DEALLOC_SF                      = 0x114,
	MLX5_CMD_OP_CREATE_MKEY                     = 0x200,
	MLX5_CMD_OP_QUERY_MKEY                      = 0x201,
	MLX5_CMD_OP_DESTROY_MKEY                    = 0x202,
	MLX5_CMD_OP_QUERY_SPECIAL_CONTEXTS          = 0x203,
	MLX5_CMD_OP_PAGE_FAULT_RESUME               = 0x204,
	MLX5_CMD_OP_ALLOC_MEMIC                     = 0x205,
	MLX5_CMD_OP_DEALLOC_MEMIC                   = 0x206,
	MLX5_CMD_OP_CREATE_EQ                       = 0x301,
	MLX5_CMD_OP_DESTROY_EQ                      = 0x302,
	MLX5_CMD_OP_QUERY_EQ                        = 0x303,
	MLX5_CMD_OP_GEN_EQE                         = 0x304,
	MLX5_CMD_OP_CREATE_CQ                       = 0x400,
	MLX5_CMD_OP_DESTROY_CQ                      = 0x401,
	MLX5_CMD_OP_QUERY_CQ                        = 0x402,
	MLX5_CMD_OP_MODIFY_CQ                       = 0x403,
	MLX5_CMD_OP_CREATE_QP                       = 0x500,
	MLX5_CMD_OP_DESTROY_QP                      = 0x501,
	MLX5_CMD_OP_RST2INIT_QP                     = 0x502,
	MLX5_CMD_OP_INIT2RTR_QP                     = 0x503,
	MLX5_CMD_OP_RTR2RTS_QP                      = 0x504,
	MLX5_CMD_OP_RTS2RTS_QP                      = 0x505,
	MLX5_CMD_OP_SQERR2RTS_QP                    = 0x506,
	MLX5_CMD_OP_2ERR_QP                         = 0x507,
	MLX5_CMD_OP_2RST_QP                         = 0x50a,
	MLX5_CMD_OP_QUERY_QP                        = 0x50b,
	MLX5_CMD_OP_SQD_RTS_QP                      = 0x50c,
	MLX5_CMD_OP_INIT2INIT_QP                    = 0x50e,
	MLX5_CMD_OP_CREATE_PSV                      = 0x600,
	MLX5_CMD_OP_DESTROY_PSV                     = 0x601,
	MLX5_CMD_OP_CREATE_SRQ                      = 0x700,
	MLX5_CMD_OP_DESTROY_SRQ                     = 0x701,
	MLX5_CMD_OP_QUERY_SRQ                       = 0x702,
	MLX5_CMD_OP_ARM_RQ                          = 0x703,
	MLX5_CMD_OP_CREATE_XRC_SRQ                  = 0x705,
	MLX5_CMD_OP_DESTROY_XRC_SRQ                 = 0x706,
	MLX5_CMD_OP_QUERY_XRC_SRQ                   = 0x707,
	MLX5_CMD_OP_ARM_XRC_SRQ                     = 0x708,
	MLX5_CMD_OP_CREATE_DCT                      = 0x710,
	MLX5_CMD_OP_DESTROY_DCT                     = 0x711,
	MLX5_CMD_OP_DRAIN_DCT                       = 0x712,
	MLX5_CMD_OP_QUERY_DCT                       = 0x713,
	MLX5_CMD_OP_ARM_DCT_FOR_KEY_VIOLATION       = 0x714,
	MLX5_CMD_OP_CREATE_XRQ                      = 0x717,
	MLX5_CMD_OP_DESTROY_XRQ                     = 0x718,
	MLX5_CMD_OP_QUERY_XRQ                       = 0x719,
	MLX5_CMD_OP_ARM_XRQ                         = 0x71a,
	MLX5_CMD_OP_QUERY_XRQ_DC_PARAMS_ENTRY       = 0x725,
	MLX5_CMD_OP_SET_XRQ_DC_PARAMS_ENTRY         = 0x726,
	MLX5_CMD_OP_QUERY_XRQ_ERROR_PARAMS          = 0x727,
	MLX5_CMD_OP_RELEASE_XRQ_ERROR               = 0x729,
	MLX5_CMD_OP_MODIFY_XRQ                      = 0x72a,
	MLX5_CMD_OP_QUERY_ESW_FUNCTIONS             = 0x740,
	MLX5_CMD_OP_QUERY_VPORT_STATE               = 0x750,
	MLX5_CMD_OP_MODIFY_VPORT_STATE              = 0x751,
	MLX5_CMD_OP_QUERY_ESW_VPORT_CONTEXT         = 0x752,
	MLX5_CMD_OP_MODIFY_ESW_VPORT_CONTEXT        = 0x753,
	MLX5_CMD_OP_QUERY_NIC_VPORT_CONTEXT         = 0x754,
	MLX5_CMD_OP_MODIFY_NIC_VPORT_CONTEXT        = 0x755,
	MLX5_CMD_OP_QUERY_ROCE_ADDRESS              = 0x760,
	MLX5_CMD_OP_SET_ROCE_ADDRESS                = 0x761,
	MLX5_CMD_OP_QUERY_HCA_VPORT_CONTEXT         = 0x762,
	MLX5_CMD_OP_MODIFY_HCA_VPORT_CONTEXT        = 0x763,
	MLX5_CMD_OP_QUERY_HCA_VPORT_GID             = 0x764,
	MLX5_CMD_OP_QUERY_HCA_VPORT_PKEY            = 0x765,
	MLX5_CMD_OP_QUERY_VNIC_ENV                  = 0x76f,
	MLX5_CMD_OP_QUERY_VPORT_COUNTER             = 0x770,
	MLX5_CMD_OP_ALLOC_Q_COUNTER                 = 0x771,
	MLX5_CMD_OP_DEALLOC_Q_COUNTER               = 0x772,
	MLX5_CMD_OP_QUERY_Q_COUNTER                 = 0x773,
	MLX5_CMD_OP_SET_MONITOR_COUNTER             = 0x774,
	MLX5_CMD_OP_ARM_MONITOR_COUNTER             = 0x775,
	MLX5_CMD_OP_SET_PP_RATE_LIMIT               = 0x780,
	MLX5_CMD_OP_QUERY_RATE_LIMIT                = 0x781,
	MLX5_CMD_OP_CREATE_SCHEDULING_ELEMENT       = 0x782,
	MLX5_CMD_OP_DESTROY_SCHEDULING_ELEMENT      = 0x783,
	MLX5_CMD_OP_QUERY_SCHEDULING_ELEMENT        = 0x784,
	MLX5_CMD_OP_MODIFY_SCHEDULING_ELEMENT       = 0x785,
	MLX5_CMD_OP_CREATE_QOS_PARA_VPORT           = 0x786,
	MLX5_CMD_OP_DESTROY_QOS_PARA_VPORT          = 0x787,
	MLX5_CMD_OP_ALLOC_PD                        = 0x800,
	MLX5_CMD_OP_DEALLOC_PD                      = 0x801,
	MLX5_CMD_OP_ALLOC_UAR                       = 0x802,
	MLX5_CMD_OP_DEALLOC_UAR                     = 0x803,
	MLX5_CMD_OP_CONFIG_INT_MODERATION           = 0x804,
	MLX5_CMD_OP_ACCESS_REG                      = 0x805,
	MLX5_CMD_OP_ATTACH_TO_MCG                   = 0x806,
	MLX5_CMD_OP_DETACH_FROM_MCG                 = 0x807,
	MLX5_CMD_OP_GET_DROPPED_PACKET_LOG          = 0x80a,
	MLX5_CMD_OP_MAD_IFC                         = 0x50d,
	MLX5_CMD_OP_QUERY_MAD_DEMUX                 = 0x80b,
	MLX5_CMD_OP_SET_MAD_DEMUX                   = 0x80c,
	MLX5_CMD_OP_NOP                             = 0x80d,
	MLX5_CMD_OP_ALLOC_XRCD                      = 0x80e,
	MLX5_CMD_OP_DEALLOC_XRCD                    = 0x80f,
	MLX5_CMD_OP_ALLOC_TRANSPORT_DOMAIN          = 0x816,
	MLX5_CMD_OP_DEALLOC_TRANSPORT_DOMAIN        = 0x817,
	MLX5_CMD_OP_QUERY_CONG_STATUS               = 0x822,
	MLX5_CMD_OP_MODIFY_CONG_STATUS              = 0x823,
	MLX5_CMD_OP_QUERY_CONG_PARAMS               = 0x824,
	MLX5_CMD_OP_MODIFY_CONG_PARAMS              = 0x825,
	MLX5_CMD_OP_QUERY_CONG_STATISTICS           = 0x826,
	MLX5_CMD_OP_ADD_VXLAN_UDP_DPORT             = 0x827,
	MLX5_CMD_OP_DELETE_VXLAN_UDP_DPORT          = 0x828,
	MLX5_CMD_OP_SET_L2_TABLE_ENTRY              = 0x829,
	MLX5_CMD_OP_QUERY_L2_TABLE_ENTRY            = 0x82a,
	MLX5_CMD_OP_DELETE_L2_TABLE_ENTRY           = 0x82b,
	MLX5_CMD_OP_SET_WOL_ROL                     = 0x830,
	MLX5_CMD_OP_QUERY_WOL_ROL                   = 0x831,
	MLX5_CMD_OP_CREATE_LAG                      = 0x840,
	MLX5_CMD_OP_MODIFY_LAG                      = 0x841,
	MLX5_CMD_OP_QUERY_LAG                       = 0x842,
	MLX5_CMD_OP_DESTROY_LAG                     = 0x843,
	MLX5_CMD_OP_CREATE_VPORT_LAG                = 0x844,
	MLX5_CMD_OP_DESTROY_VPORT_LAG               = 0x845,
	MLX5_CMD_OP_CREATE_TIR                      = 0x900,
	MLX5_CMD_OP_MODIFY_TIR                      = 0x901,
	MLX5_CMD_OP_DESTROY_TIR                     = 0x902,
	MLX5_CMD_OP_QUERY_TIR                       = 0x903,
	MLX5_CMD_OP_CREATE_SQ                       = 0x904,
	MLX5_CMD_OP_MODIFY_SQ                       = 0x905,
	MLX5_CMD_OP_DESTROY_SQ                      = 0x906,
	MLX5_CMD_OP_QUERY_SQ                        = 0x907,
	MLX5_CMD_OP_CREATE_RQ                       = 0x908,
	MLX5_CMD_OP_MODIFY_RQ                       = 0x909,
	MLX5_CMD_OP_SET_DELAY_DROP_PARAMS           = 0x910,
	MLX5_CMD_OP_DESTROY_RQ                      = 0x90a,
	MLX5_CMD_OP_QUERY_RQ                        = 0x90b,
	MLX5_CMD_OP_CREATE_RMP                      = 0x90c,
	MLX5_CMD_OP_MODIFY_RMP                      = 0x90d,
	MLX5_CMD_OP_DESTROY_RMP                     = 0x90e,
	MLX5_CMD_OP_QUERY_RMP                       = 0x90f,
	MLX5_CMD_OP_CREATE_TIS                      = 0x912,
	MLX5_CMD_OP_MODIFY_TIS                      = 0x913,
	MLX5_CMD_OP_DESTROY_TIS                     = 0x914,
	MLX5_CMD_OP_QUERY_TIS                       = 0x915,
	MLX5_CMD_OP_CREATE_RQT                      = 0x916,
	MLX5_CMD_OP_MODIFY_RQT                      = 0x917,
	MLX5_CMD_OP_DESTROY_RQT                     = 0x918,
	MLX5_CMD_OP_QUERY_RQT                       = 0x919,
	MLX5_CMD_OP_SET_FLOW_TABLE_ROOT		        = 0x92f,
	MLX5_CMD_OP_CREATE_FLOW_TABLE               = 0x930,
	MLX5_CMD_OP_DESTROY_FLOW_TABLE              = 0x931,
	MLX5_CMD_OP_QUERY_FLOW_TABLE                = 0x932,
	MLX5_CMD_OP_CREATE_FLOW_GROUP               = 0x933,
	MLX5_CMD_OP_DESTROY_FLOW_GROUP              = 0x934,
	MLX5_CMD_OP_QUERY_FLOW_GROUP                = 0x935,
	MLX5_CMD_OP_SET_FLOW_TABLE_ENTRY            = 0x936,
	MLX5_CMD_OP_QUERY_FLOW_TABLE_ENTRY          = 0x937,
	MLX5_CMD_OP_DELETE_FLOW_TABLE_ENTRY         = 0x938,
	MLX5_CMD_OP_ALLOC_FLOW_COUNTER              = 0x939,
	MLX5_CMD_OP_DEALLOC_FLOW_COUNTER            = 0x93a,
	MLX5_CMD_OP_QUERY_FLOW_COUNTER              = 0x93b,
	MLX5_CMD_OP_MODIFY_FLOW_TABLE               = 0x93c,
	MLX5_CMD_OP_ALLOC_PACKET_REFORMAT_CONTEXT   = 0x93d,
	MLX5_CMD_OP_DEALLOC_PACKET_REFORMAT_CONTEXT = 0x93e,
	MLX5_CMD_OP_QUERY_PACKET_REFORMAT_CONTEXT   = 0x93f,
	MLX5_CMD_OP_ALLOC_MODIFY_HEADER_CONTEXT     = 0x940,
	MLX5_CMD_OP_DEALLOC_MODIFY_HEADER_CONTEXT   = 0x941,
	MLX5_CMD_OP_QUERY_MODIFY_HEADER_CONTEXT     = 0x942,
	MLX5_CMD_OP_FPGA_CREATE_QP                  = 0x960,
	MLX5_CMD_OP_FPGA_MODIFY_QP                  = 0x961,
	MLX5_CMD_OP_FPGA_QUERY_QP                   = 0x962,
	MLX5_CMD_OP_FPGA_DESTROY_QP                 = 0x963,
	MLX5_CMD_OP_FPGA_QUERY_QP_COUNTERS          = 0x964,
	MLX5_CMD_OP_CREATE_GENERAL_OBJECT           = 0xa00,
	MLX5_CMD_OP_MODIFY_GENERAL_OBJECT           = 0xa01,
	MLX5_CMD_OP_QUERY_GENERAL_OBJECT            = 0xa02,
	MLX5_CMD_OP_DESTROY_GENERAL_OBJECT          = 0xa03,
	MLX5_CMD_OP_CREATE_UCTX                     = 0xa04,
	MLX5_CMD_OP_DESTROY_UCTX                    = 0xa06,
	MLX5_CMD_OP_CREATE_UMEM                     = 0xa08,
	MLX5_CMD_OP_DESTROY_UMEM                    = 0xa0a,
	MLX5_CMD_OP_SYNC_STEERING                   = 0xb00,
	MLX5_CMD_OP_MAX
};

// Constants
#define RD_CMD_RAM_ADDR          ((volatile unsigned long *)0xC0020000)
#define RD_DTA_RAM_ADDR          ((volatile unsigned long *)0xC0020800)
#define RD_CMD_RAM_PTR           ((unsigned int *)RD_CMD_RAM_ADDR)
#define RD_DTA_RAM_PTR           ((unsigned int *)RD_DTA_RAM_ADDR)
#define READ_DESC_PCIE_ADDR_LO   ((volatile unsigned long *)0xC0010000)
#define READ_DESC_PCIE_ADDR_HI   ((volatile unsigned long *)0xC0010004)
#define READ_DESC_RAM_ADDR       ((volatile unsigned long *)0xC0010008)
#define READ_DESC_LEN            ((volatile unsigned long *)0xC001000C)
#define READ_DESC_TAG            ((volatile unsigned long *)0xC0010010)
#define READ_DESC_VALID          ((volatile unsigned long *)0xC0010014)
#define READ_DESC_STAT_TAG       ((volatile unsigned long *)0xC0010018)
#define READ_DESC_STAT_VALID     ((volatile unsigned long *)0xC001001C)

#define WR_RSP_RAM_ADDR          ((volatile unsigned long *)0xC0030000)
#define WR_DTA_RAM_ADDR          ((volatile unsigned long *)0xC0030800)
#define WR_RSP_RAM_PTR           ((unsigned int *)WR_RSP_RAM_ADDR)
#define WR_DTA_RAM_PTR           ((unsigned int *)WR_DTA_RAM_ADDR)
#define WRITE_DESC_PCIE_ADDR_LO  ((volatile unsigned long *)0xC0010020)
#define WRITE_DESC_PCIE_ADDR_HI  ((volatile unsigned long *)0xC0010024)
#define WRITE_DESC_RAM_ADDR      ((volatile unsigned long *)0xC0010028)
#define WRITE_DESC_LEN           ((volatile unsigned long *)0xC001002C)
#define WRITE_DESC_TAG           ((volatile unsigned long *)0xC0010030)
#define WRITE_DESC_VALID         ((volatile unsigned long *)0xC0010034)
#define WRITE_DESC_STAT_TAG      ((volatile unsigned long *)0xC0010038)
#define WRITE_DESC_STAT_VALID    ((volatile unsigned long *)0xC001003C)

#define MSIX_ADDRESS_LO          ((volatile unsigned long *)0xC0010040)
#define MSIX_ADDRESS_HI          ((volatile unsigned long *)0xC0010044)
#define MSIX_DATA                ((volatile unsigned long *)0xC0010048)
#define MSIX_INT                 ((volatile unsigned long *)0xC001004C)

#define MSIX_TBL_ADDR_LO         ((volatile unsigned long *)0xC0002000)
#define MSIX_TBL_ADDR_HI         ((volatile unsigned long *)0xC0002004)
#define MSIX_TBL_DATA            ((volatile unsigned long *)0xC0002008)
#define MSIX_TBL_VEC_MASK        ((volatile unsigned long *)0xC000200C)


#define RX_RAM_ADDR              ((unsigned long *)0xC0040000)
#define RX_DESC_ADDR             ((volatile unsigned long *)0xC0060000)
#define RX_DESC_LEN              ((volatile unsigned long *)0xC0060004)
#define RX_DESC_TAG              ((volatile unsigned long *)0xC0060008)
#define RX_DESC_VALID            ((volatile unsigned long *)0xC006000C)
#define RX_DESC_STAT_LEN          ((volatile unsigned long *)0xC0060010)
#define RX_DESC_STAT_TAG         ((volatile unsigned long *)0xC0060014)
#define RX_DESC_STAT_VALID       ((volatile unsigned long *)0xC0060018)

#define TX_RAM_ADDR              ((unsigned long *)0xC0050000)
#define TX_DESC_ADDR             ((volatile unsigned long *)0xC0060020)
#define TX_DESC_LEN              ((volatile unsigned long *)0xC0060024)
#define TX_DESC_TAG              ((volatile unsigned long *)0xC0060028)
#define TX_DESC_VALID            ((volatile unsigned long *)0xC006002C)
#define TX_DESC_STAT_TAG         ((volatile unsigned long *)0xC0060030)
#define TX_DESC_STAT_VALID       ((volatile unsigned long *)0xC0060034)
#define RXTX_FIFO_RESET          ((volatile unsigned long *)0xC0060038)


#define SWAP32(x) \
	((((x) & 0x000000FF) << 24) | \
     (((x) & 0x0000FF00) << 8) | \
     (((x) & 0x00FF0000) >> 8) | \
 	 (((x) & 0xFF000000) >> 24))

struct mlx5_init_seg {
	 unsigned int			fw_rev;
	 unsigned int			cmdif_rev_fw_sub;
	 unsigned int			rsvd0[2];
	 unsigned int			cmdq_addr_h;
	 unsigned int			cmdq_addr_l_sz;
	 unsigned int			cmd_dbell;
	 unsigned int			rsvd1[120];
	 unsigned int			initializing;
	 unsigned int 	        health_buffer[16];
	 unsigned int			rsvd2[880];
	 unsigned int			internal_timer_h;
	 unsigned int			internal_timer_l;
	 unsigned int			rsvd3[2];
	 unsigned int			health_counter;
	 unsigned int			rsvd4[1019];
	 unsigned long long		ieee1588_clk;
	 unsigned int			ieee1588_clk_type;
	 unsigned int			clr_intx;
}__packed;

struct msix_tlb_struct {
	unsigned int msix_tbl_addr_lo;
	unsigned int msix_tbl_addr_hi;
	unsigned int msix_tbl_data;
	unsigned int msix_tbl_enable;
}__packed;

struct msix_rsp_struct {
	unsigned int msix_addr_lo;
	unsigned int msix_addr_hi;
	unsigned int msix_data;
	unsigned int msix_int;
}__packed;


struct mlx5_init_seg *i_seg = (void *)0xC0000000;  // bar0
//unsigned int *cmd_ptr = (unsigned int *)0xC0020000;
//unsigned int *rsp_ptr = (unsigned int *)0xC0030000;
//unsigned int *cdta_ptr = (unsigned int *)0xC0020800;
//unsigned int *dta_ptr = (unsigned int *)0xC0030800;
struct msix_tlb_struct *msix_tlb_ptr = (void *)0xC0002000;
struct msix_rsp_struct *msix_rsp_ptr = (void *)0xC0010040;

void write32(void *addr, unsigned int value){
	volatile int *ptr = (volatile int *)addr;
	ptr[0] = value;
}

unsigned int read32(void *addr){
	volatile int *ptr = (volatile int *)addr;
	return ptr[0];
}

void memset32(void *dst,unsigned int val, unsigned int len)
{
	unsigned int *ptr = dst;
	unsigned int idx;

	for (idx = 0; idx < (len/sizeof(unsigned int)); idx++)
	{
		ptr[idx] = 0;
	}
}

void memcpy_be32(unsigned int *dst,unsigned int *src, unsigned int len)
{
	unsigned int idx;

	for (idx = 0; idx < (len/sizeof(unsigned int)); idx++)
	{
		dst[idx] = SWAP32(src[idx]);
	}
}

/*void memcpy(unsigned int *dst,unsigned int *src, unsigned int len)
{
        unsigned int idx;

        for (idx = 0; idx < (len/sizeof(unsigned int)); idx++)
        {
                dst[idx] = src[idx];
        }
}*/

void memcpy_ram(unsigned int len, unsigned int cmd)
{
	unsigned int idx;
	unsigned int data;

	unsigned int *src_ptr = cmd ? RD_CMD_RAM_PTR : RD_DTA_RAM_PTR;
	unsigned int *dst_ptr = cmd ? WR_RSP_RAM_PTR : WR_DTA_RAM_PTR;

	for (idx = 0; idx < (len/sizeof(unsigned int)); idx++)
	{
		data =  read32(&src_ptr[idx]);
		write32(&dst_ptr[idx], data);
	}
}

unsigned int get_set_bit(unsigned int val)
{
	unsigned int pos;
	if(!val)
	{
		return 32;
	}

	pos = 0;
	for (pos = 0; pos < 32; pos++)
	{
		if(val & (1 << pos))
		{
			break;
		}
	}

	return (pos);
}

void delay(unsigned int val)
{
    while(val){
    	val--;
    }
	return;
}

unsigned int running_read_tag = 0;
unsigned int running_write_tag = 0;
int h2c_activate (unsigned int caddr_l, unsigned int caddr_h, unsigned int len, unsigned int cmd) {
    unsigned int stat_valid;
    unsigned int stat_tag;
    unsigned int Addr = cmd ? 0xc0020000 : 0xc0020800;

	*READ_DESC_PCIE_ADDR_LO = caddr_l;
	*READ_DESC_PCIE_ADDR_HI = caddr_h;
	*READ_DESC_RAM_ADDR = Addr;
	*READ_DESC_LEN = len;
	*READ_DESC_TAG = running_read_tag % 0x100;
	*READ_DESC_VALID = 1;

	do {
		stat_valid = *READ_DESC_STAT_VALID;
	} while (stat_valid != 1);

	do {
		stat_tag = *READ_DESC_STAT_TAG;
	} while (stat_tag != (running_read_tag % 0x100));
	running_read_tag++;

	*READ_DESC_STAT_VALID = 0;
	return stat_valid;
}

unsigned int IntMode = 0;
int c2h_activate (unsigned int caddr_l, unsigned int caddr_h, unsigned int len, unsigned int cmd) {
    unsigned int stat_valid;
    unsigned int stat_tag;
    unsigned int Addr = cmd ? 0xc0030000 : 0xc0030800;

    delay(10);

    *WRITE_DESC_PCIE_ADDR_LO = caddr_l;
	*WRITE_DESC_PCIE_ADDR_HI = caddr_h;
	*WRITE_DESC_RAM_ADDR = Addr;
	*WRITE_DESC_LEN = len;
	*WRITE_DESC_TAG = running_write_tag % 0x100;
	*WRITE_DESC_VALID = 1;

	do {
		stat_valid = *WRITE_DESC_STAT_VALID;
	} while (stat_valid != 1);

	do {
		stat_tag = *WRITE_DESC_STAT_TAG;
	} while (stat_tag != (running_write_tag % 0x100));
	running_write_tag++;

	*WRITE_DESC_STAT_VALID = 0;
	return stat_valid;
}

unsigned int rx_tag = 0;
unsigned int rx_flag = 0;
int eth_rx_active (unsigned int addr, unsigned int sz)
{
	unsigned int rx_valid;
	//unsigned int tag;
	unsigned int o_sz;
	//unsigned int cntr;

	//tag = *RX_DESC_STAT_TAG;
	if(!rx_flag){
		rx_tag++;
		*RX_DESC_ADDR = addr;
		*RX_DESC_LEN = sz;
		*RX_DESC_TAG = rx_tag;
		*RX_DESC_VALID = 0x1;
		rx_flag = 1;
	}

	rx_valid = *RX_DESC_STAT_VALID;
	if(!rx_valid){
		return 0;
	}
	*RX_DESC_STAT_VALID = 0;

	rx_flag = 0;
	o_sz = *RX_DESC_STAT_LEN;
	return o_sz;
}

unsigned int tx_tag = 0;
int eth_tx_active (unsigned int addr, unsigned int sz)
{
        unsigned int valid;
        unsigned int tag;
        unsigned int cntr;

        tag = *TX_DESC_STAT_TAG;
        if (tag == (tx_tag & 0x1F)){
            tx_tag++;
            *TX_DESC_ADDR = addr;
            *TX_DESC_LEN = sz;
            *TX_DESC_TAG = tx_tag;
            *TX_DESC_VALID = 0x1;
        }

        cntr = 0;
        do {
            valid = *TX_DESC_STAT_VALID;
            cntr++;
            if(cntr > 100){
				return 1;
			}
        }while(valid != 1);
        *TX_DESC_STAT_VALID = 0;

        cntr = 0;
        do {
            tag = *TX_DESC_STAT_TAG;
            cntr++;
            if(cntr > 100){
				return 1;
			}
        }while(tag != (tx_tag & 0x1F));

        return 0;
}

void health(void)
{
	unsigned int health_ctr;

	health_ctr = SWAP32(read32(&i_seg->health_counter)) & 0x00FFFFFF;
	health_ctr = (health_ctr + 1) & 0x00FFFFFF;
	write32(&i_seg->health_counter, SWAP32(health_ctr));

	return;
}

unsigned int hca_cap_gen[] = {0x80000001, 0x00000000, 0x00000000, 0x00000000,
		0x020f804e, 0x00170000, 0x00160018, 0x16980397,//0x00160018
		0x04c0d0d0, 0x00040004, 0x00440004, 0xf0e10000,
		0xfc7f0000, 0xfff0f501, 0x7ee0f000, 0x0001b001,
		0xc41b2f3f, 0xfbfbdbaf, 0x0005070c, 0x00000000, //0xf0090000 0xD0090000
		0x00930400, 0x00000200, 0x00800100, 0x000000f0,
		0x8ce40415, 0x10180018, 0xfe17ff40, 0xd7171017,
		0x97100b00, 0x89040606, 0x900f090f, 0xeb8c0e0a,
		0x00000000, 0x00000d00, 0x00000000, 0x1018007c,
		0x10000000, 0x00000000, 0x0000004e, 0x0001312d,
		0x00000020, 0x00000001, 0x0000048f, 0x01010000,
		0x00000000, 0x0000000f, 0x00020040, 0x00060008,
		0x00010006, 0x00200006, 0x00000026, 0x00000000,
		0x00000000, 0x00000003, 0x03000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000 };


unsigned int hca_cap_set[0x400];

unsigned int hca_cap_atomic[] = { 0x00000000, 0x00000000, 0x02000000, 0x00000000,
                                  0x0000000f, 0x0000003c, 0x0000003c, 0x00000000,
                                  0x00000000, 0x00000000, 0x00000000, 0x00000000,
                                  0x00000000, 0x00000000, 0x00000000, 0x00000000,
                                  0x00000000, 0x00000000, 0x00000000, 0x00000000,
                                  0x00000000, 0x00000000, 0x00000000, 0x00000000,
                                  0x00000000, 0x00000000, 0x00000000, 0x00000000,
                                  0x00000000, 0x00000000, 0x00000000, 0x00000000,
                                  0x00000000, 0x00000000, 0x00000000, 0x00000000,
                                  0x00000000, 0x00000000, 0x00000000, 0x00000000,
                                  0x00000000, 0x00000000, 0x00000000, 0x00000000,
                                  0x00000000, 0x00000000, 0x00000000, 0x00000000,
                                  0x00000000, 0x00000000, 0x00000000, 0x00000000,
                                  0x00000000, 0x00000000, 0x00000000, 0x00000000,
                                  0x00000000, 0x00000000, 0x00000000, 0x00000000,
                                  0x00000000, 0x00000000, 0x00000000, 0x00000000 };

unsigned int hca_cap_odp[] = { 0x00000000, 0x00000000, 0x00000000, 0x00000000,
                               0xf0000000, 0x00000000, 0x80000000, 0x00000000,
                               0x00000000, 0x00000000, 0x00000000, 0x00000000,
                               0x00000000, 0x00000000, 0x00000000, 0x00000000,
                               0x00000000, 0x00000000, 0x00000000, 0x00000000,
                               0x00000000, 0x00000000, 0x00000000, 0x00000000,
                               0x00000000, 0x00000000, 0x00000000, 0x00000000,
                               0x00000000, 0x00000000, 0x00000000, 0x00000000,
                               0x00000000, 0x00000000, 0x00000000, 0x00000000,
                               0x00000000, 0x00000000, 0x00000000, 0x00000000,
                               0x00000000, 0x00000000, 0x00000000, 0x00000000,
                               0x00000000, 0x00000000, 0x00000000, 0x00000000,
                               0x00000000, 0x00000000, 0x00000000, 0x00000000,
                               0x00000000, 0x00000000, 0x00000000, 0x00000000,
                               0x00000000, 0x00000000, 0x00000000, 0x00000000,
                               0x00000000, 0x00000000, 0x00000000, 0x00000000 };


unsigned int hca_cap_roce[] = { 0x70000000, 0x00000000, 0x00000000, 0x00000000,
                                0x00070007, 0x000012b7, 0xffffc000, 0x00000100,
                                0x00000000, 0x00000000, 0x00000000, 0x00000000,
                                0x00000000, 0x00000000, 0x00000000, 0x00000000,
                                0x00000000, 0x00000000, 0x00000000, 0x00000000,
                                0x00000000, 0x00000000, 0x00000000, 0x00000000,
                                0x00000000, 0x00000000, 0x00000000, 0x00000000,
                                0x00000000, 0x00000000, 0x00000000, 0x00000000,
                                0x00000000, 0x00000000, 0x00000000, 0x00000000,
                                0x00000000, 0x00000000, 0x00000000, 0x00000000,
                                0x00000000, 0x00000000, 0x00000000, 0x00000000,
                                0x00000000, 0x00000000, 0x00000000, 0x00000000,
                                0x00000000, 0x00000000, 0x00000000, 0x00000000,
                                0x00000000, 0x00000000, 0x00000000, 0x00000000,
                                0x00000000, 0x00000000, 0x00000000, 0x00000000,
                                0x00000000, 0x00000000, 0x00000000, 0x00000000 };

unsigned int hca_cap_eth_offload[] = { 0xeaf26be3, 0xe0000003, 0x00000544, 0x00000000,
                                       0x00000000, 0x00000000, 0x00000000, 0x00000000,
                                       0x00000000, 0x00000000, 0x00000000, 0x00000000,
                                       0x00000008, 0x00000010, 0x00000020, 0x00000400,
                                       0x00000000, 0x00000000, 0x00000000, 0x00000000,
                                       0x00000000, 0x00000000, 0x00000000, 0x00000000,
                                       0x00000000, 0x00000000, 0x00000000, 0x00000000,
                                       0x00000000, 0x00000000, 0x00000000, 0x00000000,
                                       0x00000000, 0x00000000, 0x00000000, 0x00000000,
                                       0x00000000, 0x00000000, 0x00000000, 0x00000000,
                                       0x00000000, 0x00000000, 0x00000000, 0x00000000,
                                       0x00000000, 0x00000000, 0x00000000, 0x00000000,
                                       0x00000000, 0x00000000, 0x00000000, 0x00000000,
                                       0x00000000, 0x00000000, 0x00000000, 0x00000000,
                                       0x00000000, 0x00000000, 0x00000000, 0x00000000,
                                       0x00000000, 0x00000000, 0x00000000, 0x00000000 };

int query_hca_cap(void)
{
	unsigned int *cmd_ptr;
	unsigned int *dta_ptr;
	unsigned int addr_h;
	unsigned int addr_l;
    unsigned int OpType;
    unsigned int status;

    cmd_ptr = RD_CMD_RAM_PTR;
    dta_ptr = WR_DTA_RAM_PTR;

    addr_h = SWAP32((read32(&cmd_ptr[12])));
	addr_l = SWAP32((read32(&cmd_ptr[13])));
	h2c_activate(addr_l, addr_h, 0x240, 0);
	memcpy_ram(0x240, 0);

	OpType = SWAP32(read32(&cmd_ptr[5])) & 0xFE;
	OpType = (OpType >> 1);
	switch(OpType)
	{
		case 0:
			memcpy_be32(dta_ptr, &hca_cap_gen[0], sizeof(hca_cap_gen));
			break;
		case 1:
			memcpy_be32(dta_ptr, &hca_cap_eth_offload[0], sizeof(hca_cap_eth_offload));
			break;
		case 2:
			memcpy_be32(dta_ptr, &hca_cap_odp[0], sizeof(hca_cap_odp));
			break;
		case 3:
			memcpy_be32(dta_ptr, &hca_cap_atomic[0], sizeof(hca_cap_atomic));
			break;
		case 4:
			memcpy_be32(dta_ptr, &hca_cap_roce[0], sizeof(hca_cap_roce));
			break;
		default:
			break;
	}

	status = c2h_activate(addr_l, addr_h, 0x240, 0);

	return status;
}

int set_hca_cap (void)
{
	unsigned int *cmd_ptr;
	unsigned int *cdta_ptr;
	unsigned int addr_h;
	unsigned int addr_l;
	signed int sz;
	unsigned int *HcaPtr;

    cmd_ptr = RD_CMD_RAM_PTR;
    cdta_ptr = RD_DTA_RAM_PTR;

	sz = SWAP32((read32(&cmd_ptr[1]))) - 0x10;
	addr_h = SWAP32((read32(&cmd_ptr[2])));
	addr_l = SWAP32((read32(&cmd_ptr[3])));
	HcaPtr = (unsigned int *)&hca_cap_set;

	while (1) {
		h2c_activate(addr_l, addr_h, 0x240, 0);
		memcpy_ram(0x240, 0);

		memcpy_be32(HcaPtr,cdta_ptr, (sz > 0x240) ? 0x240 : sz);
		sz = sz - 0x240;
		if (sz <= 0){
			break;
		}
		HcaPtr += (0x200/sizeof(unsigned int));
		addr_h = SWAP32((read32(&cdta_ptr[0x8c])));
		addr_l = SWAP32((read32(&cdta_ptr[0x8d])));
	}

	return 0;
}


unsigned int special_ctx[] = { 0x00000600, 0x00000100, 0x20000400, 0x00000000,
		                       0x0000000f, 0x0000003c, 0x0000003c, 0x00000000};
int query_special_ctx(void)
{
	unsigned int *cmd_ptr;
	unsigned int *dta_ptr;
	unsigned int addr_h;
	unsigned int addr_l;
    unsigned int status;

    cmd_ptr = RD_CMD_RAM_PTR;
    dta_ptr = WR_DTA_RAM_PTR;

    addr_h = SWAP32((read32(&cmd_ptr[12])));
	addr_l = SWAP32((read32(&cmd_ptr[13])));
	h2c_activate(addr_l, addr_h, 0x240, 0);
	memcpy_ram(0x240, 0);

	memcpy_be32(dta_ptr, &special_ctx[0], sizeof(special_ctx));

    status = c2h_activate(addr_l, addr_h, 0x240, 0);

	return status;
}

unsigned int nic_vport_ctx[] = { 0x01000000, 0x00000000, 0x00000000, 0x00000000,
		                         0x00000000, 0x00000000, 0x00000000, 0x00000000,
		                         0x00000000, 0x000005f2, 0x043f7203, 0x00c2516c,
		                         0x043f7203, 0x00c2516c, 0x043f7203, 0x00c2516c,
		                         0x00000000, 0x00000000, 0x00000000, 0x00000000,
		                         0x00000000, 0x00000000, 0x00000000, 0x00000000,
		                         0x00000000, 0x00000000, 0x00000000, 0x00000000,
		                         0x00000000, 0x00000000, 0x00000000, 0x00000000,
		                         0x00000000, 0x00000000, 0x00000000, 0x00000000,
		                         0x00000000, 0x00000000, 0x00000000, 0x00000000,
		                         0x00000000, 0x00000000, 0x00000000, 0x00000000,
		                         0x00000000, 0x00000000, 0x00000000, 0x00000000,
		                         0x00000000, 0x00000000, 0x00000000, 0x00000000,
		                         0x00000000, 0x00000000, 0x00000000, 0x00000000,
		                         0x00000000, 0x00000000, 0x00000000, 0x00000000,
		                         0x00000000, 0x0000043f, 0x72c2516c, 0x00000000 };

int query_nic_vport_ctx(void)
{
	unsigned int *cmd_ptr;
	unsigned int *dta_ptr;
	unsigned int addr_h;
	unsigned int addr_l;
    unsigned int status;

    cmd_ptr = RD_CMD_RAM_PTR;
    dta_ptr = WR_DTA_RAM_PTR;

    addr_h = SWAP32((read32(&cmd_ptr[12])));
	addr_l = SWAP32((read32(&cmd_ptr[13])));
	h2c_activate(addr_l, addr_h, 0x240, 0);
	memcpy_ram(0x240, 0);

    memcpy_be32(dta_ptr, &nic_vport_ctx[0], sizeof(nic_vport_ctx));

    status= c2h_activate(addr_l, addr_h, 0x240, 0);

	return status;
}

unsigned int adapter[] = { 0x00000000, 0x00000000, 0x00000000, 0x00000000,
                           0x00000000, 0x00000000, 0x000002c9, 0x000015b3,
                           0x00000000, 0x00000000, 0x00000000, 0x00000000,
                           0x00000000, 0x00000000, 0x00000000, 0x00000000,
                           0x00000000, 0x00000000, 0x00000000, 0x00000000,
                           0x00000000, 0x00000000, 0x00000000, 0x00000000,
                           0x00000000, 0x00000000, 0x00000000, 0x00000000,
                           0x00000000, 0x00000000, 0x00000000, 0x00000000,
                           0x00000000, 0x00000000, 0x00000000, 0x00000000,
                           0x00000000, 0x00000000, 0x00000000, 0x00000000,
                           0x00000000, 0x00000000, 0x00000000, 0x00000000,
                           0x00000000, 0x00000000, 0x00000000, 0x00000000,
                           0x00000000, 0x00000000, 0x00000000, 0x00000000,
                           0x00000000, 0x00000000, 0x00000000, 0x00000000,
                           0x00000000, 0x00000000, 0x00000000, 0x00000000,
                           0x4d534630, 0x30303030, 0x30303031, 0x32000000 };

int query_adapter(void)
{
	unsigned int *cmd_ptr;
	unsigned int *dta_ptr;
	unsigned int addr_h;
	unsigned int addr_l;
    unsigned int status;

    cmd_ptr = RD_CMD_RAM_PTR;
    dta_ptr = WR_DTA_RAM_PTR;

    addr_h = SWAP32((read32(&cmd_ptr[12])));
	addr_l = SWAP32((read32(&cmd_ptr[13])));
	h2c_activate(addr_l, addr_h, 0x240, 0);
	memcpy_ram(0x240, 0);

    memcpy_be32(dta_ptr, &adapter[0], sizeof(adapter));

    status= c2h_activate(addr_l, addr_h, 0x240, 0);

	return status;
}

unsigned int unknown_cmd[] = { 0x80000000, 0x00000000, 0x00000000, 0x00000000,
		                   0x00000000, 0x00000000, 0x00000000, 0x00000000,
                           0x00000000, 0x00000000, 0x00000000, 0x00000000,
                           0x00000000, 0x00000000, 0x00000000, 0x00000000,
                           0x00000000, 0x00000000, 0x00000000, 0x00000000,
                           0x00000000, 0x00000000, 0x00000000, 0x00000000,
                           0x00000000, 0x00000000, 0x00000000, 0x00000000,
                           0x00000000, 0x00000000, 0x00000000, 0x00000000,
                           0x00000000, 0x00000000, 0x00000000, 0x00000000,
                           0x00000000, 0x00000000, 0x00000000, 0x00000000,
                           0x00000000, 0x00000000, 0x00000000, 0x00000000,
                           0x00000000, 0x00000000, 0x00000000, 0x00000000,
                           0x00000000, 0x00000000, 0x00000000, 0x00000000,
                           0x00000000, 0x00000000, 0x00000000, 0x00000000,
                           0x00000000, 0x00000000, 0x00000000, 0x00000000,
						   0x00000000, 0x00000000, 0x00000000, 0x00000000 };

int unknown_cmd_10e(void)
{
	unsigned int *cmd_ptr;
	unsigned int *dta_ptr;
	unsigned int addr_h;
	unsigned int addr_l;
    unsigned int status;

    cmd_ptr = RD_CMD_RAM_PTR;
    dta_ptr = WR_DTA_RAM_PTR;

    addr_h = SWAP32((read32(&cmd_ptr[12])));
	addr_l = SWAP32((read32(&cmd_ptr[13])));
	h2c_activate(addr_l, addr_h, 0x240, 0);
	memcpy_ram(0x240, 0);

    memcpy_be32(dta_ptr, &unknown_cmd[0], sizeof(unknown_cmd));

    status= c2h_activate(addr_l, addr_h, 0x240, 0);

	return status;
}


struct eq_struct {
	unsigned int signature;
	unsigned int eqn;
	unsigned int msix_mode;
	unsigned int eq_ci[10];
	unsigned int eq_ctx[10][0x500];
};

struct cq_struct {
	unsigned int cqn;
	unsigned int cq_ci[10];
	unsigned int cq_ctx[10][0x500];
};

struct qp_struct {
	unsigned int qpn;
	unsigned int wqe_cnt[10];
	unsigned int widx[10];
	unsigned int sq_ci[10];
	unsigned int rq_ci[10];
	unsigned int qp_ctx[10][0x500];
};

struct mk_struct {
        unsigned int mkn;
        unsigned int mk_ctx[128][320];
};

struct bth_pkt_struct {
	struct r_bth bth;
	struct r_reth reth;
	unsigned char dta[1024];
};

struct nt_data_struct {
	unsigned int qpn;
	unsigned int cqn;
	unsigned int bth_opcode;
	unsigned int wqe[16];
	struct bth_pkt_struct pkt;
};

struct mk_struct mk;
struct eq_struct eq;
struct cq_struct cq;
struct qp_struct qp;


void init (void)
{
    memset32(&mk, 0, sizeof(mk));
    mk.mkn = 0x200;
	memset32(&eq, 0, sizeof(eq));
	eq.signature = 0xFFAADDEE;
	eq.eqn = 0x04;
	eq.msix_mode = 0;
	memset32(&cq, 0, sizeof(cq));
	cq.cqn = 0x400;
	memset32(&qp, 0, sizeof(qp));
	qp.qpn = 0xE0;

	*RX_DESC_ADDR = 0;
	*RX_DESC_LEN = 0;
	*RX_DESC_TAG = 0;
	*RX_DESC_VALID = 0;
	*RX_DESC_STAT_TAG = 0;
	*RX_DESC_STAT_LEN = 0;
	if(*RX_DESC_STAT_VALID){
		*RX_DESC_STAT_VALID = 0;
	}

    *TX_DESC_ADDR = 0;
    *TX_DESC_LEN = 0;
    *TX_DESC_TAG = 0;
    *TX_DESC_VALID = 0;
	*TX_DESC_STAT_TAG = 0;
	if(*TX_DESC_STAT_VALID){
		*TX_DESC_STAT_VALID = 0;
	}

	*RXTX_FIFO_RESET = 1;
	*RXTX_FIFO_RESET = 0;

	return;
}

int create_cq(void)
{
	unsigned int *cmd_ptr;
	unsigned int *cdta_ptr;
	signed int sz;
	unsigned int addr_h;
	unsigned int addr_l;
	unsigned int *CqCtxPtr;

    cmd_ptr = RD_CMD_RAM_PTR;
    cdta_ptr = RD_DTA_RAM_PTR;

	sz = SWAP32((read32(&cmd_ptr[1]))) - 0x10;
	addr_h = SWAP32((read32(&cmd_ptr[2])));
	addr_l = SWAP32((read32(&cmd_ptr[3])));
	CqCtxPtr = (unsigned int *)&cq.cq_ctx[cq.cqn & 0x3FF];

	while (1) {
		h2c_activate(addr_l, addr_h, 0x240, 0);
		memcpy_ram(0x240, 0);

		memcpy_be32(CqCtxPtr,cdta_ptr,0x240);
		sz = sz - 0x240;
		if (sz <= 0){
			break;
		}
		CqCtxPtr += (0x200/sizeof(unsigned int));
		addr_h = SWAP32((read32(&cdta_ptr[0x8c])));
		addr_l = SWAP32((read32(&cdta_ptr[0x8d])));
	}

	cq.cqn++;

	return 0;
}

int create_mkey(void)
{
	unsigned int *cmd_ptr;
	unsigned int *cdta_ptr;
	signed int sz;
	unsigned int addr_h;
	unsigned int addr_l;
	unsigned int *MkCtxPtr;

	cmd_ptr = RD_CMD_RAM_PTR;
	cdta_ptr = RD_DTA_RAM_PTR;

	sz = SWAP32((read32(&cmd_ptr[1]))) - 0x10;
	addr_h = SWAP32((read32(&cmd_ptr[2])));
	addr_l = SWAP32((read32(&cmd_ptr[3])));
	MkCtxPtr = (unsigned int *)&mk.mk_ctx[mk.mkn & 0x7F];

	if (mk.mkn < 0x280){
		while (1) {
			h2c_activate(addr_l, addr_h, 0x240, 0);
			memcpy_ram(0x240, 0);

			memcpy_be32(MkCtxPtr,cdta_ptr,0x240);
			sz = sz - 0x200;
			if (sz <= 0){
					break;
			}
			MkCtxPtr += (0x200/sizeof(unsigned int));
			addr_h = SWAP32((read32(&cdta_ptr[0x8c])));
			addr_l = SWAP32((read32(&cdta_ptr[0x8d])));
		}
	}

	mk.mkn++;

	return 0;
}

int create_eq(void)
{
	unsigned int *cmd_ptr;
	unsigned int *cdta_ptr;
	unsigned int addr_h;
	unsigned int addr_l;
	signed int sz;
	unsigned int *EqCtxPtr;

    cmd_ptr = RD_CMD_RAM_PTR;
    cdta_ptr = RD_DTA_RAM_PTR;

	sz = SWAP32((read32(&cmd_ptr[1]))) - 0x10;
	addr_h = SWAP32((read32(&cmd_ptr[2])));
	addr_l = SWAP32((read32(&cmd_ptr[3])));
	EqCtxPtr = (unsigned int *)&eq.eq_ctx[eq.eqn & 0x3];

	if (eq.eqn < 8) {
		while (1) {
			h2c_activate(addr_l, addr_h, 0x240, 0);
			memcpy_ram(0x240, 0);

			memcpy_be32(EqCtxPtr,cdta_ptr,0x240);
			sz = sz - 0x240;
			if (sz <= 0){
				break;
			}
			EqCtxPtr += (0x200/sizeof(unsigned int));
			addr_h = SWAP32((read32(&cdta_ptr[0x8c])));
			addr_l = SWAP32((read32(&cdta_ptr[0x8d])));
		}
	}

	if (eq.eqn > 4) eq.msix_mode = 1;
	eq.eqn++;

	return 0;
}

int event_hndl (unsigned int eqn, unsigned int ev_type, unsigned int ofst)
{
	unsigned int *cdta_ptr;
	unsigned int *dta_ptr;
	unsigned int addr_h;
	unsigned int addr_l;
	unsigned int *EqCtxPtr;
	unsigned int eqe_idx;
	unsigned int *eqe_ptr;
	unsigned int *eqe_w_ptr;
	unsigned int _eq;
	unsigned int owner;
	unsigned int event_type;
	unsigned int Tmp;
	unsigned int vec = 0;
	unsigned int num_ent;

    cdta_ptr = RD_DTA_RAM_PTR;
    dta_ptr = WR_DTA_RAM_PTR;

	if (eq.msix_mode) {
		//eqe_idx = SWAP32((read32((unsigned int *)0xc0004040)));
		//eq = eqe_idx >> 24;
		//eqe_idx	= eqe_idx & 0x00FFFFFF;

		_eq = eqn & 0x3;
		//eqn++;
		//for (eq = eqn; eq < eq_cnt; eq++){
	        EqCtxPtr = (unsigned int *)&eq.eq_ctx[_eq];
		    eqe_idx = EqCtxPtr[0xA];
			Tmp = EqCtxPtr[0x3] >> 24;
		    EqCtxPtr[0xA] = (eqe_idx + 1) & ((1 << Tmp) - 1);

		    num_ent = EqCtxPtr[6] >> 24;
		    num_ent = (1 << num_ent) * 0x1000;
		    num_ent = num_ent / 0x40;

		    Tmp = eqe_idx / num_ent;
		    addr_h = EqCtxPtr[0x40 + (2 * Tmp)];
			addr_l = EqCtxPtr[0x41 + (2 * Tmp)];

			Tmp = eqe_idx & (num_ent - 1);
			addr_l = addr_l + (Tmp * 0x40);
			addr_l = addr_l & 0xFFFFF800;

			h2c_activate(addr_l, addr_h, 0x800, 0);
			memcpy_ram(0x800, 0);

			eqe_idx = eqe_idx & 0x1F;
			eqe_ptr = &cdta_ptr[0x10 * eqe_idx];
			eqe_w_ptr = &dta_ptr[0x10 * eqe_idx];
			event_type = SWAP32((read32(&eqe_ptr[0])));
			event_type = event_type & 0xFF00FFFF;
			event_type = event_type | (ev_type << 16);//0x000A0000; // 0xA
			event_type = SWAP32(event_type);
			write32(&eqe_w_ptr[0], event_type);

			if (ev_type != 0){//0x0A
				event_type = SWAP32((read32(&eqe_ptr[8])));
				event_type = 1 << ofst;// << 0x00000001;
				event_type = SWAP32(event_type);
				write32(&eqe_w_ptr[8], event_type);
			} else
			{
				event_type = SWAP32((read32(&eqe_ptr[0xE])));
				event_type = ofst;// << 0x00000001;
				event_type = SWAP32(event_type);
				write32(&eqe_w_ptr[0xE], event_type);
			}

			owner = SWAP32((read32(&eqe_ptr[0xF])));
			owner = owner  & 0x000000FF;
			owner ^= 1;
			owner = (SWAP32((read32(&eqe_ptr[0xF]))) & 0xFFFFFF00) | owner;
			owner = SWAP32(owner);
			write32(&eqe_w_ptr[0xF], owner);

			c2h_activate(addr_l, addr_h, 0x800, 0);
//		}

		// Activate MSI-X
		vec = EqCtxPtr[0x5];
		while ((MSIX_TBL_VEC_MASK[vec*4]) != 0);
		while (MSIX_TBL_ADDR_LO[vec*4] == 0);
		*MSIX_ADDRESS_LO = (MSIX_TBL_ADDR_LO[vec*4]);
		*MSIX_ADDRESS_HI = (MSIX_TBL_ADDR_HI[vec*4]);
		*MSIX_DATA = (MSIX_TBL_DATA[vec*4]);
		*MSIX_INT = 1;
		//write32((void *)0xC0000040, SWAP32(0x1));
		while (*MSIX_INT);

		delay(0x80);
	}

	return vec;
}

int create_qp_ctx (void)
{
	unsigned int *cmd_ptr;
	unsigned int *cdta_ptr;
	unsigned int addr_h;
	unsigned int addr_l;
	signed int sz;
	unsigned int *QpCtxPtr;

    cmd_ptr = RD_CMD_RAM_PTR;
    cdta_ptr = RD_DTA_RAM_PTR;

	sz = SWAP32((read32(&cmd_ptr[1]))) - 0x10;
	addr_h = SWAP32((read32(&cmd_ptr[2])));
	addr_l = SWAP32((read32(&cmd_ptr[3])));
	QpCtxPtr = (unsigned int *)&qp.qp_ctx[(qp.qpn & 0x1F)];

	while (1) {
		h2c_activate(addr_l, addr_h, 0x240, 0);
		memcpy_ram(0x240, 0);

		memcpy_be32(QpCtxPtr,cdta_ptr,0x240);
		sz = sz - 0x240;
		if (sz <= 0){
			break;
		}
		QpCtxPtr += (0x200/sizeof(unsigned int));
		addr_h = SWAP32((read32(&cdta_ptr[0x8c])));
		addr_l = SWAP32((read32(&cdta_ptr[0x8d])));
	}

	qp.qpn++;
	return 0;
}

int create_mod_qp (void)
{
	unsigned int *cmd_ptr;
	unsigned int *cdta_ptr;
	unsigned int addr_h;
	unsigned int addr_l;
	//signed int sz;
	unsigned int qpn;
	unsigned int *QpCtxPtr;

    cmd_ptr = RD_CMD_RAM_PTR;
    cdta_ptr = RD_DTA_RAM_PTR;

	//sz = SWAP32((read32(&cmd_ptr[1]))) - 0x10;
	addr_h = SWAP32((read32(&cmd_ptr[2])));
	addr_l = SWAP32((read32(&cmd_ptr[3])));
	qpn = SWAP32((read32(&cmd_ptr[6])));
	QpCtxPtr = (unsigned int *)&qp.qp_ctx[(qpn & 0x1F)];

	h2c_activate(addr_l, addr_h, 0x240, 0);
	memcpy_ram(0x240, 0);

	QpCtxPtr[7] = SWAP32((read32(&cdta_ptr[7])));

	return 0;
}



void default_hndl (void)
{
	unsigned int ctr = 0;

	while (1)
	{
		if (ctr == 1000)
		{
			ctr = 0;
			health();
		}
		ctr++;
	}

	return;
}

#define MKEY(key) ((key >> 8) & 0x7F)
unsigned int create_cqe (unsigned int qpn, unsigned int cqn, unsigned int wqe_cnt);

unsigned int _rdma_sd (unsigned int qpn, unsigned int wqe_cnt,
                      unsigned int widx)
{
	unsigned int *cdta_ptr;
	unsigned int *QpPtr = (unsigned int *)&qp.qp_ctx[(qpn & 0x1F)];
	unsigned int sq_ci = qp.sq_ci[(qpn & 0x1F)];
	unsigned int Idx;
	unsigned int addr_h;
	unsigned int addr_l;
	signed int sz;
	unsigned int wqe[16];
	unsigned int r_wqe[16];
	unsigned int v_addr_l;
	unsigned int key;
	unsigned int r_qpn;
	unsigned int cqn;
	unsigned int eqn;

    cdta_ptr = RD_DTA_RAM_PTR;

	Idx = ((widx + 1) & 0x3F) / 0x40;
	addr_h = QpPtr[64 + (Idx * 2)];
	addr_l = QpPtr[65 + (Idx * 2)];
	Idx = (widx + 1) & 0x3F;
	addr_l = addr_l + (Idx * 0x40);
	sz = ((wqe_cnt * 0x10) + 0x40) & (~(0x40 - 1));
	h2c_activate(addr_l, addr_h, sz, 0);
	memcpy_be32(&wqe[0], cdta_ptr,sizeof(wqe));
	qp.sq_ci[(qpn & 0x1F)] = (sq_ci + wqe_cnt);

	sz = wqe[4];
	key = wqe[5];
	v_addr_l = wqe[7];

	addr_h = mk.mk_ctx[MKEY(key)][16];
	addr_l = mk.mk_ctx[MKEY(key)][17] + (v_addr_l & 0xFFF);
	addr_l = addr_l & 0xFFFFFFFC;
	h2c_activate(addr_l, addr_h, sz, 0);
	memcpy_ram(sz, 0);

	r_qpn = qpn - 1;
	QpPtr = (unsigned int *)&qp.qp_ctx[(r_qpn & 0x1F)];
	sq_ci = qp.sq_ci[(r_qpn & 0x1F)];

	Idx = ((widx + 1) & 0x3F) / 0x40;
	addr_h = QpPtr[64 + (Idx * 2)];
	addr_l = QpPtr[65 + (Idx * 2)];
	Idx = widx & 0x3F;
	addr_l = addr_l + (Idx * 0x40);
	sz = ((wqe_cnt * 0x10) + 0x40) & (~(0x40 - 1));
	h2c_activate(addr_l, addr_h, sz, 0);
	memcpy_be32(&r_wqe[0], cdta_ptr,sizeof(r_wqe));
	qp.sq_ci[(r_qpn & 0x1F)] = (sq_ci + wqe_cnt);

	sz = r_wqe[0];
	key = r_wqe[1];
	v_addr_l = r_wqe[3];

	addr_h = mk.mk_ctx[MKEY(key)][16];
	addr_l = mk.mk_ctx[MKEY(key)][17] + (v_addr_l & 0xFFF);
	addr_l = addr_l & 0xFFFFFFFC;
	c2h_activate(addr_l, addr_h, sz, 0);

	QpPtr = (unsigned int *)&qp.qp_ctx[(qpn & 0x1F)];
	cqn = QpPtr[33];
	eqn = create_cqe (qpn, cqn, wqe_cnt);

	event_hndl(eqn, 0x00, cqn);
	
	r_qpn = qpn - 1;
	QpPtr = (unsigned int *)&qp.qp_ctx[(r_qpn & 0x1F)];
	cqn = QpPtr[33];
	eqn = create_cqe (r_qpn, cqn, 1);

	event_hndl(eqn, 0x00, cqn);

	return cqn;
}

unsigned int _rdma_rd (unsigned int qpn, unsigned int wqe_cnt,
                      unsigned int widx)
{
	unsigned int *cdta_ptr;
	unsigned int *QpPtr = (unsigned int *)&qp.qp_ctx[(qpn & 0x1F)];
	unsigned int sq_ci = qp.sq_ci[(qpn & 0x1F)];
	unsigned int Idx;
	unsigned int addr_h;
	unsigned int addr_l;
	signed int sz;
	unsigned int wqe[16];
	unsigned int v_addr_l;
	unsigned int key;
	unsigned int cqn;
	unsigned int eqn;

    cdta_ptr = RD_DTA_RAM_PTR;

	Idx = ((widx + 1) & 0x3F) / 0x40;
	addr_h = QpPtr[64 + (Idx * 2)];
	addr_l = QpPtr[65 + (Idx * 2)];
	Idx = (widx + 1) & 0x3F;
	addr_l = addr_l + (Idx * 0x40);
	sz = ((wqe_cnt * 0x10) + 0x40) & (~(0x40 - 1));
	h2c_activate(addr_l, addr_h, sz, 0);
	memcpy_be32(&wqe[0], cdta_ptr,sizeof(wqe));
	qp.sq_ci[(qpn & 0x1F)] = (sq_ci + wqe_cnt);

	//v_addr_h = wqe[4];
	v_addr_l = wqe[5];
	key = wqe[6];

	addr_h = mk.mk_ctx[MKEY(key)][16];
	addr_l = mk.mk_ctx[MKEY(key)][17] + (v_addr_l & 0xFFF);
	addr_l = addr_l & 0xFFFFFFFC;
	sz = wqe[8];
	h2c_activate(addr_l, addr_h, sz, 0);
	memcpy_ram(sz, 0);

	//v_addr_h = wqe[10];
	v_addr_l = wqe[11];
	key = wqe[9];
	addr_h = mk.mk_ctx[MKEY(key)][16];
	addr_l = mk.mk_ctx[MKEY(key)][17] + (v_addr_l & 0xFFF);
	addr_l = addr_l & 0xFFFFFFFC;
	c2h_activate(addr_l, addr_h, sz, 0);
	
	cqn = QpPtr[33];
	eqn = create_cqe (qpn, cqn, wqe_cnt);

	event_hndl(eqn, 0x00, cqn);
	
	return cqn;
}
 
unsigned int _rdma_wr_imm (unsigned int qpn, unsigned int wqe_cnt,
                          unsigned int widx)
{
	unsigned int *cdta_ptr;
	unsigned int *QpPtr = (unsigned int *)&qp.qp_ctx[(qpn & 0x1F)];
	unsigned int sq_ci = qp.sq_ci[(qpn & 0x1F)];
	unsigned int Idx;
	unsigned int addr_h;
	unsigned int addr_l;
	signed int sz;
	unsigned int wqe[16];
	unsigned int v_addr_l;
	unsigned int key;
	unsigned int cqn;
	unsigned int eqn;
	unsigned int r_wqe[16];
	unsigned int r_qpn;

    cdta_ptr = RD_DTA_RAM_PTR;

	Idx = ((widx + 1) & 0x3F) / 0x40;
	addr_h = QpPtr[64 + (Idx * 2)];
	addr_l = QpPtr[65 + (Idx * 2)];
	Idx = (widx + 1) & 0x3F;
	addr_l = addr_l + (Idx * 0x40);
	sz = ((wqe_cnt * 0x10) + 0x40) & (~(0x40 - 1));
	h2c_activate(addr_l, addr_h, sz, 0);
	memcpy_be32(&wqe[0], cdta_ptr,sizeof(wqe));
	qp.sq_ci[(qpn & 0x1F)] = (sq_ci + wqe_cnt);
	
	v_addr_l = wqe[11];
	key = wqe[9];

	addr_h = mk.mk_ctx[MKEY(key)][16];
	addr_l = mk.mk_ctx[MKEY(key)][17] + (v_addr_l & 0xFFF);
	addr_l = addr_l & 0xFFFFFFFC;
	sz = wqe[8];
	h2c_activate(addr_l, addr_h, sz, 0);
	memcpy_ram(sz, 0);

	v_addr_l = wqe[5];
	key = wqe[6];
	addr_h = mk.mk_ctx[MKEY(key)][16];
	addr_l = mk.mk_ctx[MKEY(key)][17] + (v_addr_l & 0xFFF);
	addr_l = addr_l & 0xFFFFFFFC;
	c2h_activate(addr_l, addr_h, sz, 0);
	
	cqn = QpPtr[33];
	eqn = create_cqe (qpn, cqn, wqe_cnt);

	event_hndl(eqn, 0x00, cqn);
	
	r_qpn = qpn - 1;
	QpPtr = (unsigned int *)&qp.qp_ctx[(r_qpn & 0x1F)];
	sq_ci = qp.sq_ci[(r_qpn & 0x1F)];

	Idx = ((widx + 1) & 0x3F) / 0x40;
	addr_h = QpPtr[64 + (Idx * 2)];
	addr_l = QpPtr[65 + (Idx * 2)];
	Idx = widx & 0x3F;
	addr_l = addr_l + (Idx * 0x40);
	sz = ((wqe_cnt * 0x10) + 0x40) & (~(0x40 - 1));
	h2c_activate(addr_l, addr_h, sz, 0);
	memcpy_be32(&r_wqe[0], cdta_ptr,sizeof(r_wqe));
	qp.sq_ci[(r_qpn & 0x1F)] = (sq_ci + wqe_cnt);

	cqn = QpPtr[33];
	eqn = create_cqe (r_qpn, cqn, 1);

	event_hndl(eqn, 0x00, cqn);

	return cqn;
}

static int bth_opcode(unsigned int bth_opcode, unsigned int wr_opcode, int fits)
{
	switch (wr_opcode) {
	case MLX5_OPCODE_RDMA_WRITE:
		if (bth_opcode == IB_OPCODE_RC_RDMA_WRITE_FIRST ||
			bth_opcode == IB_OPCODE_RC_RDMA_WRITE_MIDDLE){
			return fits ? IB_OPCODE_RC_RDMA_WRITE_LAST :
						  IB_OPCODE_RC_RDMA_WRITE_MIDDLE;
		}
		else {
			return fits ? IB_OPCODE_RC_RDMA_WRITE_ONLY :
				          IB_OPCODE_RC_RDMA_WRITE_FIRST;
		}

	case MLX5_OPCODE_RDMA_WRITE_IMM:
		if (bth_opcode == IB_OPCODE_RC_RDMA_WRITE_FIRST ||
			bth_opcode == IB_OPCODE_RC_RDMA_WRITE_MIDDLE) {
			return fits ? IB_OPCODE_RC_RDMA_WRITE_LAST_WITH_IMMEDIATE :
				          IB_OPCODE_RC_RDMA_WRITE_MIDDLE;
		}
		else {
			return fits ? IB_OPCODE_RC_RDMA_WRITE_ONLY_WITH_IMMEDIATE :
				          IB_OPCODE_RC_RDMA_WRITE_FIRST;
		}

	case MLX5_OPCODE_SEND:
		if (bth_opcode == IB_OPCODE_RC_SEND_FIRST ||
			bth_opcode == IB_OPCODE_RC_SEND_MIDDLE) {
			return fits ? IB_OPCODE_RC_SEND_LAST :
				          IB_OPCODE_RC_SEND_MIDDLE;
		}
		else {
			return fits ? IB_OPCODE_RC_SEND_ONLY :
				          IB_OPCODE_RC_SEND_FIRST;
		}
	case MLX5_OPCODE_SEND_IMM:
		if (bth_opcode == IB_OPCODE_RC_SEND_FIRST ||
			bth_opcode == IB_OPCODE_RC_SEND_MIDDLE) {
			return fits ? IB_OPCODE_RC_SEND_LAST_WITH_IMMEDIATE :
				          IB_OPCODE_RC_SEND_MIDDLE;
		}
		else {
			return fits ? IB_OPCODE_RC_SEND_ONLY_WITH_IMMEDIATE :
				          IB_OPCODE_RC_SEND_FIRST;
		}

	case MLX5_OPCODE_RDMA_READ:
		return IB_OPCODE_RC_RDMA_READ_REQUEST;

	case MLX5_OPCODE_ATOMIC_CS:
		return IB_OPCODE_RC_COMPARE_SWAP;

	case MLX5_OPCODE_ATOMIC_FA:
		return IB_OPCODE_RC_FETCH_ADD;

	case MLX5_OPCODE_SEND_INVAL:
		if (bth_opcode == IB_OPCODE_RC_SEND_FIRST ||
			bth_opcode == IB_OPCODE_RC_SEND_MIDDLE) {
			return fits ? IB_OPCODE_RC_SEND_LAST_WITH_INVALIDATE :
				          IB_OPCODE_RC_SEND_MIDDLE;
		}
		else {
			return fits ? IB_OPCODE_RC_SEND_ONLY_WITH_INVALIDATE :
				          IB_OPCODE_RC_SEND_FIRST;
		}
	case MLX5_OPCODE_UMR:
		return wr_opcode;
	}

	return -1;
}

unsigned int dbg_offset = 0;
struct nt_data_struct dbg_n_dta;
unsigned int run_psn = 0;
unsigned int init_bth (struct nt_data_struct *n_ptr, unsigned int sz)
{
	unsigned int MigReq = 0;
	unsigned int bt_opcode;
	enum rxe_hdr_mask mask;
	unsigned int se;
	unsigned int qpn;
	unsigned int psn;
	unsigned int ack_req;
	unsigned int _sz;
	unsigned int mtu_fit;
	unsigned int wr_opcde;

	//memset(&n_ptr->pkt.bth,0,sizeof(n_ptr->pkt.bth));
	memcpy(&dbg_n_dta, n_ptr, sizeof(*n_ptr));
	mtu_fit = 1;
	wr_opcde = (n_ptr->wqe[0] & 0xFF);
	if ((wr_opcde != MLX5_OPCODE_RDMA_READ) && (sz > 1024)) {
		mtu_fit = 0;
	}

    bt_opcode = bth_opcode(n_ptr->bth_opcode, wr_opcde, mtu_fit); //??
	n_ptr->bth_opcode = bt_opcode;
	mask = pkt_opcode[bt_opcode].mask;

	se = (n_ptr->wqe[2] & IB_SEND_SOLICITED) &&
		  (mask & RXE_END_MASK) &&
		  ((mask & (RXE_SEND_MASK)) ||
		  (mask & (RXE_WRITE_MASK | RXE_IMMDT_MASK)) ==
		  (RXE_WRITE_MASK | RXE_IMMDT_MASK));
	if (se){
		n_ptr->pkt.bth.flags |= BTH_SE_MASK;
	}

	if (MigReq){
		n_ptr->pkt.bth.flags |= BTH_MIG_MASK;
	}

	if (mask & RXE_START_MASK)
	{
		run_psn = 0;
	}

	qpn = n_ptr->qpn; //??
	psn = (++run_psn) & BTH_PSN_MASK;
	ack_req = (mask & RXE_END_MASK);
	if (ack_req){
		psn |= BTH_ACK_MASK;
	}

	n_ptr->pkt.bth.opcode = bt_opcode;
	n_ptr->pkt.bth.pkey = 0xFFFF; // DEFAULT_PKEY_FULL
	n_ptr->pkt.bth.qpn = qpn;
	n_ptr->pkt.bth.apsn = psn;

	if(n_ptr->pkt.bth.opcode == IB_OPCODE_RC_RDMA_WRITE_FIRST)
	{
		dbg_offset = 4;
	}

	_sz = 1024;
	if (mtu_fit){
		_sz = sz;
	}

	if (mask & RXE_RETH_MASK){
		memset(&n_ptr->pkt.reth,0,sizeof(n_ptr->pkt.reth));
		n_ptr->pkt.reth.va_h = n_ptr->wqe[4];
		n_ptr->pkt.reth.va_l = n_ptr->wqe[5];
		n_ptr->pkt.reth.rkey = n_ptr->wqe[6];
		n_ptr->pkt.reth.len = _sz;
	}

	return _sz;
}

#define RECV 0
#define SEND 1
unsigned int fetch_wqe (unsigned int qpn, unsigned int wqe_cnt,
                        unsigned int widx, struct nt_data_struct *data_ptr, int send)
{
	unsigned int *cdta_ptr;
	unsigned int *QpPtr;
	unsigned int sq_ci;
	unsigned int Idx;
	unsigned int addr_h;
	unsigned int addr_l;
	signed int sz;
	unsigned int cqn;

    cdta_ptr = RD_DTA_RAM_PTR;

	QpPtr = (unsigned int *)&qp.qp_ctx[(qpn & 0x1F)];
	Idx = ((widx + 1) & 0x3F) / 0x40;
	addr_h = QpPtr[64 + (Idx * 2)];
	addr_l = QpPtr[65 + (Idx * 2)];
	Idx = (widx + send) & 0x3F;
	addr_l = addr_l + (Idx * 0x40);
	sz = ((wqe_cnt * 0x10) + 0x40) & (~(0x40 - 1));
	h2c_activate(addr_l, addr_h, sz, 0);
	memcpy_be32(&data_ptr->wqe[0], cdta_ptr,sizeof(data_ptr->wqe));

	sq_ci = qp.sq_ci[(qpn & 0x1F)];
	qp.sq_ci[(qpn & 0x1F)] = (sq_ci + wqe_cnt);

	qp.widx[(qpn & 0x1F)] = widx;
	qp.wqe_cnt[(qpn & 0x1F)] = wqe_cnt;

	cqn = QpPtr[33];
	data_ptr->qpn = qpn;
	data_ptr->cqn = cqn;
	return cqn;
}

unsigned int fetch_dta (unsigned int v_addr_l, unsigned int key, signed int sz,
		                void *d_ptr)
{
	void *cdta_ptr;
	unsigned int addr_h;
	unsigned int addr_l;
	unsigned int pg_offset;
	unsigned int pg_idx;
	unsigned int v_addr_end;
	unsigned int dma_sz;
	unsigned int v_addr;
	unsigned int _sz;

    cdta_ptr = RD_DTA_RAM_PTR;
    v_addr = v_addr_l;
    _sz = sz;

    while(_sz)
    {
		pg_offset = v_addr & 0xFFF;
		pg_idx = (v_addr >> 12);
		v_addr_end = v_addr + sz - 1;
		dma_sz = _sz;
		if(pg_idx != (v_addr_end >> 12))
		{
			dma_sz = ((pg_idx + 1) << 12) - v_addr;
		}
		addr_h = mk.mk_ctx[MKEY(key)][16 + (pg_idx * 2)];
		addr_l = mk.mk_ctx[MKEY(key)][17 + (pg_idx * 2)] + pg_offset;
		addr_l = addr_l & 0xFFFFFFFC;
		h2c_activate(addr_l, addr_h, dma_sz, 0);
		memcpy_be32((unsigned int *)d_ptr, cdta_ptr,dma_sz);
		v_addr = v_addr + dma_sz;
		d_ptr = (unsigned char *)d_ptr + dma_sz;
		cdta_ptr = (unsigned char *)cdta_ptr + dma_sz;
		_sz = _sz - dma_sz;
    }

    /*addr_h = mk.mk_ctx[MKEY(key)][16 + (pg_idx * 2)];
	addr_l = mk.mk_ctx[MKEY(key)][17 + (pg_idx * 2)] + v_addr_l;
	addr_l = addr_l & 0xFFFFFFFC;
	h2c_activate(addr_l, addr_h, sz, 0);
	memcpy_be32((unsigned int *)d_ptr, cdta_ptr,sz);
	*/
	return 0;
}

unsigned int flush_dta (unsigned int v_addr_l, unsigned int key, signed int sz,
		                void *d_ptr)
{
	void *dta_ptr;
	unsigned int addr_h;
	unsigned int addr_l;
	unsigned int pg_offset;
	unsigned int pg_idx;
	unsigned int v_addr_end;
	unsigned int dma_sz;
	unsigned int v_addr;
	unsigned int _sz;

    dta_ptr = WR_DTA_RAM_PTR;
    v_addr = v_addr_l;
    _sz = sz;

    while(_sz)
    {
		pg_offset = v_addr & 0xFFF;
		pg_idx = (v_addr >> 12);
		v_addr_end = v_addr + sz - 1;
		dma_sz = _sz;
		if(pg_idx != (v_addr_end >> 12))
		{
			dma_sz = ((pg_idx + 1) << 12) - v_addr;
		}
	    addr_h = mk.mk_ctx[MKEY(key)][16 + (pg_idx * 2)];
		addr_l = mk.mk_ctx[MKEY(key)][17 + (pg_idx * 2)] + pg_offset;
		addr_l = addr_l & 0xFFFFFFFC;
		memcpy_be32(dta_ptr,(unsigned int *)d_ptr,dma_sz);
		c2h_activate(addr_l, addr_h, dma_sz, 0);
		v_addr = v_addr + dma_sz;
		d_ptr = (unsigned char *)d_ptr + dma_sz;
		dta_ptr = (unsigned char *)dta_ptr + dma_sz;
		_sz = _sz - dma_sz;
    }
/*
    addr_h = mk.mk_ctx[MKEY(key)][16];
	addr_l = mk.mk_ctx[MKEY(key)][17] + v_addr_l;
	addr_l = addr_l & 0xFFFFFFFC;
	memcpy_be32(dta_ptr,(unsigned int *)d_ptr,sz);
	c2h_activate(addr_l, addr_h, sz, 0);
*/
	return 0;
}

unsigned int xmit_data (struct nt_data_struct *data_ptr)
{
	memcpy_be32((unsigned int *)TX_RAM_ADDR, (unsigned int *)&data_ptr->pkt, sizeof(data_ptr->pkt));
	while(eth_tx_active(0, sizeof(data_ptr->pkt)));
	return 0;
}

unsigned int recv_data (struct nt_data_struct *data_ptr)
{
	unsigned int sz;
	sz = eth_rx_active(0, 0x800);
	if (!sz){
		return sz;
	}

    memcpy_be32((unsigned int *)&data_ptr->pkt,(unsigned int *)RX_RAM_ADDR, sz);

    return sz;
}

unsigned int rdma_sd (unsigned int qpn, unsigned int wqe_cnt,
                      unsigned int widx)
{
	unsigned int cqn;
	struct nt_data_struct tx_dta;
	unsigned int v_addr_l;
	unsigned int key;
	signed int sz;
	signed int _sz;
	unsigned int offset;

	memset(&tx_dta.pkt.bth, 0, sizeof(tx_dta.pkt.bth));
	cqn = fetch_wqe(qpn, wqe_cnt, widx, &tx_dta,SEND);
	tx_dta.qpn = qpn - 1; // Temp
	sz = tx_dta.wqe[4];
	while (sz > 0){
		_sz = init_bth(&tx_dta, sz);
		key = tx_dta.wqe[5];
		v_addr_l = tx_dta.wqe[7] & 0xFFF;
		v_addr_l = v_addr_l + offset;
		fetch_dta(v_addr_l, key, _sz, &tx_dta.pkt.dta);
		if(tx_dta.pkt.bth.opcode == IB_OPCODE_RC_RDMA_WRITE_FIRST)
		{
			xmit_data(&tx_dta);
		}
		xmit_data(&tx_dta);
		offset = offset + _sz;
		sz = sz - _sz;
	}

	return cqn;
/*
	struct nt_data_struct tx_dta;
	unsigned int v_addr_l;
	unsigned int key;
	signed int sz;

	fetch_wqe(qpn, wqe_cnt, widx, &tx_dta);
	sz = tx_dta.wqe[8];
	key = tx_dta.wqe[9];
	v_addr_l = tx_dta.wqe[11];

	fetch_dta(v_addr_l, key, sz, &tx_dta.pkt.dta);
	xmit_data(&tx_dta);

	return 0;
*/
}


unsigned int rdma_rd (unsigned int qpn, unsigned int wqe_cnt,
                       unsigned int widx)
{
	unsigned int cqn;
	struct nt_data_struct tx_dta;
	signed int sz;

	memset(&tx_dta.pkt.bth, 0, sizeof(tx_dta.pkt.bth));
	cqn = fetch_wqe(qpn, wqe_cnt, widx, &tx_dta,SEND);
	tx_dta.qpn = qpn - 1; // Temp
	sz = tx_dta.wqe[8];
	init_bth(&tx_dta, sz);
	xmit_data(&tx_dta);

	return cqn;
}

unsigned int rdma_wr_imm (unsigned int qpn, unsigned int wqe_cnt,
                          unsigned int widx)
{
	unsigned int cqn;
	struct nt_data_struct tx_dta;
	unsigned int v_addr_l;
	unsigned int key;
	signed int sz;
	signed int _sz;
	unsigned int offset = 0;

	memset(&tx_dta.pkt.bth, 0, sizeof(tx_dta.pkt.bth));
	cqn = fetch_wqe(qpn, wqe_cnt, widx, &tx_dta,SEND);
	tx_dta.qpn = qpn - 1; // Temp
	tx_dta.pkt.bth.opcode = -1;
	sz = tx_dta.wqe[8];
	while (sz > 0){
		_sz = init_bth(&tx_dta, sz);
		key = tx_dta.wqe[9];
		v_addr_l = tx_dta.wqe[11] & 0xFFF;
		v_addr_l = v_addr_l + offset;
		fetch_dta(v_addr_l, key, _sz, &tx_dta.pkt.dta);
		if(tx_dta.pkt.bth.opcode == IB_OPCODE_RC_RDMA_WRITE_FIRST)
		{
			dbg_offset = 3;
			xmit_data(&tx_dta);
		}
		xmit_data(&tx_dta);
		offset = offset + _sz;
		sz = sz - _sz;
	}

	/*
	unsigned int cqn;
	//unsigned int eqn;
	struct nt_data_struct tx_dta;
	unsigned int v_addr_l;
	unsigned int key;
	signed int sz;

	memset(&tx_dta.pkt.bth, 0, sizeof(tx_dta.pkt.bth));
	cqn = fetch_wqe(qpn, wqe_cnt, widx, &tx_dta);
	sz = tx_dta.wqe[8];
	init_bth(&tx_dta, sz);
	key = tx_dta.wqe[9];
	v_addr_l = tx_dta.wqe[11];
	fetch_dta(v_addr_l, key, sz, &tx_dta.pkt.dta);
	xmit_data(&tx_dta);
*/
	return cqn;
}

unsigned int rdma_wr (unsigned int qpn, unsigned int wqe_cnt,
                      unsigned int widx)
{
	unsigned int cqn;
	struct nt_data_struct tx_dta;
	unsigned int v_addr_l;
	unsigned int key;
	signed int sz;
	signed int _sz;

	memset(&tx_dta.pkt.bth, 0, sizeof(tx_dta.pkt.bth));
	cqn = fetch_wqe(qpn, wqe_cnt, widx, &tx_dta,SEND);
	tx_dta.qpn = qpn - 1; // Temp
	sz = tx_dta.wqe[8];
	while (sz > 0){
		_sz = init_bth(&tx_dta, sz);
		key = tx_dta.wqe[9];
		v_addr_l = tx_dta.wqe[11] & 0xFFF;
		fetch_dta(v_addr_l, key, _sz, &tx_dta.pkt.dta);
		xmit_data(&tx_dta);
		sz = sz - _sz;
	}

	return cqn;
}

unsigned int reg_mr (unsigned int qpn, unsigned int wqe_cnt,
                     unsigned int widx)
{
	unsigned int *cdta_ptr;
	unsigned int *QpPtr = (unsigned int *)&qp.qp_ctx[(qpn & 0x1F)];
	unsigned int sq_ci = qp.sq_ci[(qpn & 0x1F)];
	unsigned int Idx;
	unsigned int addr_h;
	unsigned int addr_l;
	signed int sz;
	unsigned int key;
	unsigned int *MkCtxPtr;
	unsigned int cqn;
	unsigned int eqn;

    cdta_ptr = RD_DTA_RAM_PTR;

	Idx = widx / 0x40;
	addr_h = QpPtr[64 + (Idx * 2)];
	addr_l = QpPtr[65 + (Idx * 2)];
	Idx = widx & 0x3F;
	addr_l = addr_l + (Idx * 0x40);
	sz = (wqe_cnt * 0x40);
	h2c_activate(addr_l, addr_h, sz, 0);

	qp.sq_ci[(qpn & 0x1F)] = (sq_ci + wqe_cnt);

	key = SWAP32(read32(&cdta_ptr[0x3]));
	key = (key >> 8) & 0x7F;
	MkCtxPtr = (unsigned int *)&mk.mk_ctx[key];

	memcpy_be32(MkCtxPtr, &cdta_ptr[16], sizeof(mk.mk_ctx[0]));
	addr_h = SWAP32(read32(&cdta_ptr[34]));
	addr_l = SWAP32(read32(&cdta_ptr[35]));
	h2c_activate(addr_l, addr_h, 0x100, 0);

	memcpy_be32(&MkCtxPtr[16], &cdta_ptr[0], 0x100);

	cqn = QpPtr[33];
	eqn = create_cqe (qpn, cqn, wqe_cnt);

	event_hndl(eqn, 0x00, cqn);
	
	return cqn;
}

unsigned int wqe_opcode (unsigned int qpn, unsigned int wqe_cnt,
		                 unsigned int widx, unsigned int opcode)
{
	unsigned int cqn;

	switch(opcode){
		case 0x25:
			cqn = reg_mr(qpn, wqe_cnt, widx);
			break;
		case 0x08:
			cqn = rdma_wr(qpn, wqe_cnt, widx);
			break;
		case 0x09:
			cqn = rdma_wr_imm(qpn, wqe_cnt, widx);
			break;
		case 0x10:
			cqn = rdma_rd(qpn, wqe_cnt, widx);
			break;
		case 0x0A:
			cqn = rdma_sd(qpn, wqe_cnt, widx);
			break;
		default:
			cqn = 0xFFFFFFFF;			
	}

	return cqn;
}

unsigned int create_cqe (unsigned int qpn, unsigned int cqn, unsigned int wqe_cnt)
{
	unsigned int *cdta_ptr;
	unsigned int *dta_ptr;
	unsigned int *CqCtxPtr;
	unsigned int wqe_cq_ci;
	unsigned int cq_pg = 0;
	unsigned int addr_h;
	unsigned int addr_l;
	unsigned int *cqe_ptr;
	unsigned int *cqe_w_ptr;
	unsigned int owner;
	unsigned int eqn = 0xFFFFFFFF;

    cdta_ptr = RD_DTA_RAM_PTR;
    dta_ptr = WR_DTA_RAM_PTR;

	CqCtxPtr = &cq.cq_ctx[(cqn & 0x3FF)][0];

	wqe_cq_ci = cq.cq_ci[(cqn & 0x3FF)];
	addr_h = CqCtxPtr[0x40 + (2 * (cq_pg >> 1))];
	addr_l = CqCtxPtr[0x41 + (2 * (cq_pg >> 1))];
	addr_l = addr_l + (wqe_cq_ci << 6);

	h2c_activate(addr_l, addr_h, 0x40, 0);
	memcpy_ram(0x100, 0);

	cqe_ptr = &cdta_ptr[0];//[0x10 * idx];
	cqe_w_ptr = &dta_ptr[0];//[0x10 * idx];

	owner = SWAP32((read32(&cqe_ptr[0xE])));
	owner = owner | qpn;
	owner = SWAP32(owner);
	write32(&cqe_w_ptr[0xE], owner);

	owner = SWAP32((read32(&cqe_ptr[0xF])));
	owner = owner  & 0x00000001;
	//owner ^= 1;
	owner = owner | (wqe_cnt << 16);
	owner = (SWAP32((read32(&cqe_ptr[0xF]))) & 0xFFFFFF00) | owner;
	owner = SWAP32(owner);

	write32(&cqe_w_ptr[0xF], owner);

	wqe_cq_ci++;
	cq.cq_ci[(cqn & 0x3FF)] = wqe_cq_ci;

	c2h_activate(addr_l, addr_h, 0x40, 0);

	eqn = CqCtxPtr[5] & 0xFF;

	return eqn;
}

unsigned int uar_bf_dbs (unsigned int *wqe_cnt, unsigned int *widx, unsigned int *opcode)
{
	unsigned int db_bf_addr;
	unsigned int qpn_ds;
	unsigned int widx_opcd;
	unsigned int qpn = 0xFFFFFFFF;

    db_bf_addr = 0xC0004800;
    qpn_ds = read32((unsigned int *)(db_bf_addr + 0x8));
    if (qpn_ds ){
        write32((unsigned int *)(db_bf_addr + 0x8), 0);

        widx_opcd = read32((unsigned int *)(db_bf_addr + 0x4));
        *opcode = (widx_opcd >> 8) & 0xFF;
        *widx = widx_opcd & 0xFF;
        
        //*wqe_cnt = (qpn_ds >> 8) & 0xFF;
        //qpn = (qpn_ds & 0xFF);
		
        *wqe_cnt = read32((unsigned int *)0xC0004804) & 0xFF;
		qpn = (qpn_ds & 0xFF);
        return qpn;
    }

    db_bf_addr = 0xC0005800;
    while (1){
        qpn_ds = SWAP32(read32((unsigned int *)(db_bf_addr + 0x4)));
        if (qpn_ds ){
            write32((unsigned int *)(db_bf_addr + 0x4), 0);

            widx_opcd = SWAP32(read32((unsigned int *)db_bf_addr));
            *opcode = widx_opcd & 0xFF;
            *widx = (widx_opcd >> 8) & 0xFFFF;
            
            *wqe_cnt = qpn_ds & 0xFF;
            qpn = (qpn_ds >> 8) & 0xFFFFFF;
            return qpn;
        }

        db_bf_addr += 0x1000;
        if (db_bf_addr >= 0xC0010000){
        	break;
        }
    }

	return qpn;
}

void wqe_process (void)
{
	unsigned int wqe_cnt;
	unsigned int widx;
	unsigned int opcode;
	unsigned int qpn;
	unsigned int cqn;
	
	
	qpn = uar_bf_dbs(&wqe_cnt, &widx, &opcode);
	if (qpn == 0xFFFFFFFF){
		return;
	}

	cqn = wqe_opcode(qpn, wqe_cnt, widx, opcode);
	if (cqn == 0xFFFFFFFF){
		return;
	}

	return;
}

unsigned int dbg_wqe[16];
unsigned int recv_rdma_sd (struct nt_data_struct *rx_ptr)
{
	struct r_bth *bth_ptr;
	struct r_aeth *aeth_ptr;
	unsigned int v_addr_l;
	unsigned int key;
	signed int sz;
	enum rxe_hdr_mask mask;
	unsigned int apsn;
	unsigned int qpn;
	unsigned int cqn;
	unsigned int eqn;
	unsigned int wqe_cnt;
	unsigned int widx;
	unsigned int offset;

	bth_ptr = &rx_ptr->pkt.bth;
	qpn = bth_ptr->qpn;
	cqn = qp.qp_ctx[(qpn & 0x1F)][33];
	wqe_cnt = 1;
	widx = 0;//qp.sq_ci[(qpn & 0x1F)];
	cqn = fetch_wqe(qpn, wqe_cnt, widx,rx_ptr,RECV);

	sz = (rx_ptr->wqe[0] < 1024) ? rx_ptr->wqe[0] : 1024;
	key = rx_ptr->wqe[1];
	v_addr_l = rx_ptr->wqe[3] & 0xFFF;
	offset = (bth_ptr->apsn - 1) & BTH_PSN_MASK;
	v_addr_l = v_addr_l + (offset * 0x400);

	memcpy(&dbg_wqe, &rx_ptr->wqe[0], sizeof(dbg_wqe));
	flush_dta(v_addr_l, key, sz, &rx_ptr->pkt.dta);

	mask = pkt_opcode[bth_ptr->opcode].mask;
	if (mask & RXE_COMP_MASK){
		cqn = qp.qp_ctx[(qpn & 0x1F)][33];
		eqn = create_cqe (qpn, cqn, 1);
		event_hndl(eqn, 0x00, cqn);
	}

	if (mask & RXE_END_MASK){
		apsn = bth_ptr->apsn;
		apsn = apsn & ~BTH_ACK_MASK;

		bth_ptr->opcode = IB_OPCODE_RC_ACKNOWLEDGE;
		bth_ptr->flags = 0;
		bth_ptr->qpn = qpn + 1; // Temp
		bth_ptr->pkey = 0xFFFF;
		bth_ptr->apsn = apsn;

		aeth_ptr = (struct r_aeth *)(((unsigned char *)bth_ptr) + RXE_BTH_BYTES);
		aeth_ptr->smsn = 0;
		xmit_data(rx_ptr);
	}

/*
	unsigned int opcode;
	unsigned int v_addr_l;
	unsigned int key;
	signed int sz;
	unsigned int qpn;
	unsigned int wqe_cnt;
	unsigned int widx;
	unsigned int cqn;
	unsigned int eqn;

    opcode = rx_ptr->wqe[0];

    if (!(opcode & 0x80)){
		qpn = rx_ptr->qpn - 1;
		wqe_cnt = rx_ptr->wqe[1] & 0xFF;
	    widx = (opcode >> 8) & 0xFFFF;
		cqn = fetch_wqe(qpn, wqe_cnt, widx  ,rx_ptr);
	    v_addr_l = rx_ptr->wqe[5];
		key = rx_ptr->wqe[6];
	    sz = rx_ptr->wqe[8];
		flush_dta(v_addr_l, key, sz, &rx_ptr->pkt.dta);

		eqn = create_cqe (qpn, cqn, wqe_cnt);
		event_hndl(eqn, 0x00, cqn);

		rx_ptr->wqe[0] = opcode | 0x80;
		xmit_data(rx_ptr);
    } else {
		qpn = rx_ptr->qpn;
		cqn = rx_ptr->cqn;
		wqe_cnt = rx_ptr->wqe[1] & 0xFF;
		eqn = create_cqe (qpn, cqn, wqe_cnt);
		event_hndl(eqn, 0x00, cqn);
    }
*/
	return 0;
}
unsigned int dgb_rd = 0;
unsigned int rd_psn = 0;
unsigned int recv_rdma_rd (struct nt_data_struct *rx_ptr)
{
	struct r_bth *bth_ptr;
	struct r_reth *reth_ptr;
	struct r_aeth *aeth_ptr;
	unsigned int v_addr_l;
	unsigned int key;
	signed int sz;
	unsigned int apsn;
	unsigned int qpn;
	unsigned int wqe_cnt;
	unsigned int cqn;
	unsigned int eqn;
	unsigned int _sz;
	unsigned int opcode;
	enum rxe_hdr_mask mask;

	bth_ptr = &rx_ptr->pkt.bth;
	reth_ptr = &rx_ptr->pkt.reth;

	opcode = bth_ptr->opcode;
	if (opcode == IB_OPCODE_RDMA_READ_REQUEST){
		v_addr_l = reth_ptr->va_l & 0xFFF;
		key = reth_ptr->rkey;
		sz = reth_ptr->len;
		opcode = (sz > 1024) ? IB_OPCODE_RDMA_READ_RESPONSE_FIRST :
				               IB_OPCODE_RDMA_READ_RESPONSE_ONLY;
		while (sz > 0){
			_sz = sz;
			if (sz > 1024){
				_sz = 1024;
			}

			fetch_dta(v_addr_l, key, _sz, &rx_ptr->pkt.dta);

			mask = pkt_opcode[opcode].mask;
			apsn = bth_ptr->apsn;
			apsn = apsn & ~BTH_ACK_MASK;

			bth_ptr->opcode = opcode;
			bth_ptr->flags = 0;
			bth_ptr->qpn = 0xE2;
			bth_ptr->pkey = 0xFFFF;
			bth_ptr->apsn = apsn;

			if(mask & RXE_AETH_MASK) {
				aeth_ptr = (struct r_aeth *)(((unsigned char *)bth_ptr) + RXE_BTH_BYTES);
				aeth_ptr->smsn = 0;
			}

			xmit_data(rx_ptr);

			sz = sz - _sz;
			opcode = (sz > 1024) ? IB_OPCODE_RDMA_READ_RESPONSE_MIDDLE :
							       IB_OPCODE_RDMA_READ_RESPONSE_LAST;

		}
	} else {
		if ( (opcode == IB_OPCODE_RDMA_READ_RESPONSE_FIRST) &&
			 (opcode == IB_OPCODE_RDMA_READ_RESPONSE_ONLY)){
			rd_psn = 0;
		}

		sz = dbg_n_dta.wqe[8];
		key = dbg_n_dta.wqe[9];
		v_addr_l = dbg_n_dta.wqe[11] & 0xFFF;

		flush_dta(v_addr_l, key, 1024, &rx_ptr->pkt.dta);
		rd_psn++;

		if ( (opcode == IB_OPCODE_RDMA_READ_RESPONSE_LAST) ||
			 (opcode == IB_OPCODE_RDMA_READ_RESPONSE_ONLY)){
			qpn = dbg_n_dta.qpn;
			cqn = dbg_n_dta.cqn;
			wqe_cnt = dbg_n_dta.wqe[1] & 0xFF;
			eqn = create_cqe (qpn, cqn, wqe_cnt);
			event_hndl(eqn, 0x00, cqn);
		}
	}


/*
	unsigned int opcode;
	unsigned int v_addr_l;
	unsigned int key;
	signed int sz;
	unsigned int qpn;
	unsigned int wqe_cnt;
	unsigned int cqn;
	unsigned int eqn;

	opcode = rx_ptr->wqe[0] & 0xFF;
	if (!(opcode & 0x80)){
	    v_addr_l = rx_ptr->wqe[5];
		key = rx_ptr->wqe[6];
	    sz = rx_ptr->wqe[8];
		fetch_dta(v_addr_l, key, sz, &rx_ptr->pkt.dta);
		rx_ptr->wqe[0] = opcode | 0x80;
		xmit_data(rx_ptr);
		dgb_rd++;
	} else{
		sz = rx_ptr->wqe[8];
		key = rx_ptr->wqe[9];
		v_addr_l = rx_ptr->wqe[11];
		flush_dta(v_addr_l, key, sz, &rx_ptr->pkt.dta);
		qpn = rx_ptr->qpn;
		cqn = rx_ptr->cqn;
		wqe_cnt = rx_ptr->wqe[1] & 0xFF;
		eqn = create_cqe (qpn, cqn, wqe_cnt);
		event_hndl(eqn, 0x00, cqn);
		dgb_rd++;
	}
*/
	return 0;
}

unsigned int recv_rdma_wr_imm (struct nt_data_struct *rx_ptr)
{
	struct r_bth *bth_ptr;
	struct r_reth *reth_ptr;
	struct r_aeth *aeth_ptr;
	unsigned int v_addr_l;
	unsigned int key;
	signed int sz;
	unsigned int apsn;
	unsigned int qpn;
	//unsigned int wqe_cnt;
	unsigned int cqn;
	unsigned int eqn;

	bth_ptr = &rx_ptr->pkt.bth;
	reth_ptr = &rx_ptr->pkt.reth;

	v_addr_l = reth_ptr->va_l & 0xFFF;
	key = reth_ptr->rkey;
	sz = reth_ptr->len;

	flush_dta(v_addr_l, key, sz, &rx_ptr->pkt.dta);

	qpn = bth_ptr->qpn;
	cqn = qp.qp_ctx[(qpn & 0x1F)][33];
	eqn = create_cqe (qpn, cqn, 1);
	event_hndl(eqn, 0x00, cqn);

	apsn = bth_ptr->apsn;
	apsn = apsn & ~BTH_ACK_MASK;

	bth_ptr->opcode = IB_OPCODE_RC_ACKNOWLEDGE;
	bth_ptr->flags = 0;
	bth_ptr->qpn = 0xe2;
	bth_ptr->pkey = 0xFFFF;
	bth_ptr->apsn = apsn;

	aeth_ptr = (struct r_aeth *)(((unsigned char *)bth_ptr) + RXE_BTH_BYTES);
	aeth_ptr->smsn = 0;
	xmit_data(rx_ptr);

/*
	unsigned int opcode;
	unsigned int v_addr_l;
	unsigned int key;
	signed int sz;
	unsigned int qpn;
	unsigned int wqe_cnt;
	unsigned int cqn;
	unsigned int eqn;

    opcode = rx_ptr->wqe[0] & 0xFF;
    if (!(opcode & 0x80)){
		v_addr_l = rx_ptr->wqe[5];
		key = rx_ptr->wqe[6];
		sz = rx_ptr->wqe[8];
    	flush_dta(v_addr_l, key, sz, &rx_ptr->pkt.dta);

    	qpn = rx_ptr->qpn - 1;
		cqn = rx_ptr->cqn;
		eqn = create_cqe (qpn, cqn, 1);
		event_hndl(eqn, 0x00, cqn);

		rx_ptr->wqe[0] = opcode | 0x80;
		xmit_data(rx_ptr);
    } else {
		qpn = rx_ptr->qpn;
		cqn = rx_ptr->cqn;
		wqe_cnt = rx_ptr->wqe[1] & 0xFF;
		eqn = create_cqe (qpn, cqn, wqe_cnt);
		event_hndl(eqn, 0x00, cqn);
    }
*/
	return 0;
}

unsigned int recv_rdma_wr (struct nt_data_struct *rx_ptr)
{
	struct r_bth *bth_ptr;
	struct r_reth *reth_ptr;
	struct r_aeth *aeth_ptr;
	unsigned int v_addr_l;
	unsigned int key;
	signed int sz;
	enum rxe_hdr_mask mask;
	unsigned int apsn;
	unsigned int qpn;
	unsigned int cqn;
	unsigned int eqn;
	unsigned int offset;


	bth_ptr = &rx_ptr->pkt.bth;
	reth_ptr = &rx_ptr->pkt.reth;

	if(bth_ptr->opcode == IB_OPCODE_RC_RDMA_WRITE_FIRST)
	{
		dbg_offset = 2;
	}

	offset = (bth_ptr->apsn - 1) & BTH_PSN_MASK;
	v_addr_l = reth_ptr->va_l & 0xFFF;
	v_addr_l = v_addr_l + (offset * 0x400);

	if (!offset)
	{
		dbg_offset = 1;
	}

	key = reth_ptr->rkey;
	sz = reth_ptr->len;

	flush_dta(v_addr_l, key, sz, &rx_ptr->pkt.dta);

	qpn = bth_ptr->qpn;
	mask = pkt_opcode[bth_ptr->opcode].mask;
	if (mask & RXE_IMMDT_MASK){
		cqn = qp.qp_ctx[(qpn & 0x1F)][33];
		eqn = create_cqe (qpn, cqn, 1);
		event_hndl(eqn, 0x00, cqn);
	}

	if (mask & RXE_END_MASK){
		apsn = bth_ptr->apsn;
		apsn = apsn & ~BTH_ACK_MASK;

		bth_ptr->opcode = IB_OPCODE_RC_ACKNOWLEDGE;
		bth_ptr->flags = 0;
		bth_ptr->qpn = qpn + 1; // Temp
		bth_ptr->pkey = 0xFFFF;
		bth_ptr->apsn = apsn;

		aeth_ptr = (struct r_aeth *)(((unsigned char *)bth_ptr) + RXE_BTH_BYTES);
		aeth_ptr->smsn = 0;
		xmit_data(rx_ptr);
	}

/*
	unsigned int opcode;
	unsigned int v_addr_l;
	unsigned int key;
	signed int sz;
	unsigned int qpn;
	unsigned int wqe_cnt;
	unsigned int cqn;
	unsigned int eqn;

	opcode = rx_ptr->wqe[0] & 0xFF;
    if (!(opcode & 0x80)){
		v_addr_l = rx_ptr->wqe[5];
		key = rx_ptr->wqe[6];
		sz = rx_ptr->wqe[8];

		flush_dta(v_addr_l, key, sz, &rx_ptr->dta);

		rx_ptr->wqe[0] = opcode | 0x80;
		xmit_data(rx_ptr);
    } else {
		qpn = rx_ptr->qpn;
		cqn = rx_ptr->cqn;
		wqe_cnt = rx_ptr->wqe[1] & 0xFF;
		eqn = create_cqe (qpn, cqn, wqe_cnt);
		event_hndl(eqn, 0x00, cqn);
    }
*/
	return 0;
}

unsigned int recv_ack (struct nt_data_struct *rx_ptr)
{
	unsigned int qpn;
	unsigned int wqe_cnt;
	unsigned int cqn;
	unsigned int eqn;

	qpn = dbg_n_dta.qpn;
	cqn = dbg_n_dta.cqn;
	wqe_cnt = qp.wqe_cnt[(qpn & 0x1F)];//dbg_n_dta.wqe[1] & 0xFF;
	eqn = create_cqe (qpn, cqn, wqe_cnt);
	event_hndl(eqn, 0x00, cqn);

	return 0;
}

struct nt_data_struct nt_rx_process;
unsigned int recv_process (void)
{
	unsigned int sz;
	//struct nt_data_struct nt_rx;

	sz = recv_data(&nt_rx_process);
	if (!sz){
		return 0xFFFFFFFF;
	}

	switch(nt_rx_process.pkt.bth.opcode){
		case IB_OPCODE_RC_SEND_FIRST:
		case IB_OPCODE_RC_SEND_MIDDLE:
		case IB_OPCODE_RC_SEND_LAST:
		case IB_OPCODE_RC_SEND_ONLY:
		case IB_OPCODE_RC_SEND_ONLY_WITH_IMMEDIATE:
		case IB_OPCODE_RC_SEND_LAST_WITH_IMMEDIATE:
			recv_rdma_sd(&nt_rx_process);
			break;

		case IB_OPCODE_RC_RDMA_WRITE_FIRST:
		case IB_OPCODE_RC_RDMA_WRITE_MIDDLE:
		case IB_OPCODE_RC_RDMA_WRITE_LAST:
		case IB_OPCODE_RC_RDMA_WRITE_ONLY:
		case IB_OPCODE_RC_RDMA_WRITE_ONLY_WITH_IMMEDIATE:
		case IB_OPCODE_RC_RDMA_WRITE_LAST_WITH_IMMEDIATE:
			recv_rdma_wr(&nt_rx_process);
			break;

		case IB_OPCODE_RDMA_READ_REQUEST:
		case IB_OPCODE_RDMA_READ_RESPONSE_FIRST:
		case IB_OPCODE_RDMA_READ_RESPONSE_MIDDLE:
		case IB_OPCODE_RDMA_READ_RESPONSE_LAST:
		case IB_OPCODE_RDMA_READ_RESPONSE_ONLY:
			recv_rdma_rd(&nt_rx_process);
			break;

		case IB_OPCODE_RC_ACKNOWLEDGE:
			recv_ack(&nt_rx_process);
			break;

	}

	return 0;
}

unsigned int __recv_process (void)
{
	unsigned int sz;
	unsigned int opcode;
	struct nt_data_struct nt_rx;

	sz = recv_data(&nt_rx);
	if (!sz){
		return 0xFFFFFFFF;
	}

	opcode = nt_rx.wqe[0] & 0xFF;
	switch(opcode){
		case 0x08:
		case 0x88:
			recv_rdma_wr(&nt_rx);
			break;
		case 0x09:
		case 0x89:
			recv_rdma_wr_imm(&nt_rx);
			break;
		case 0x10:
		case 0x90:
			recv_rdma_rd(&nt_rx);
			break;
		case 0x0A:
		case 0x8A:
			recv_rdma_sd(&nt_rx);
			break;
		default:
			return 0xFFFFFFFF;
	}

	return 0;
}


int _recv_process (void)
{
        unsigned int sz;
        unsigned int *frm_ptr = (unsigned int *)0xC0040000;
        unsigned int *tx_frm_ptr = (unsigned int *)0xC0050000;
        unsigned int ethII_type;
        unsigned int frm[16];

        sz = eth_rx_active(0, 0x800);

        ethII_type = SWAP32(frm_ptr[3]) >> 16;
        switch(ethII_type){
			case 0x806:
				frm[0] = 0xC2723F04;
				frm[1] = 0x38386C51;
				frm[2] = 0x37343135;
				frm[3] = 0x01000608;
				frm[4] = 0x04060008;

				frm[5] = 0x38380200;
				frm[6] = 0x37343135;
				frm[7] = 0x6E05A8C0;
				frm[8] = 0xC2723F04;
				frm[9] = 0xA8C06C51;
				frm[10] = 0x00006F05;

				frm[11] = 0;
				frm[12] = 0;
				frm[13] = 0;
				frm[14] = 0;

				memcpy(tx_frm_ptr, &frm[0], sizeof(frm));
				eth_tx_active(0, 42);
				break;
			default:
				break;
        }

        return sz;
}

unsigned int uar_idx = 0x04;
unsigned int xrcd_idx = 0x10;
unsigned int srq_idx = 0x25;
unsigned int td_idx = 0x00;
unsigned int uar_idx_5 = 0;
unsigned int uar_idx_val = 0;
int run_mlx5_fw (void)
{
	unsigned int *cmd_ptr;
	unsigned int *rsp_ptr;
	unsigned int dbell = 0;
	unsigned int dbell_c = 0;
	unsigned int db_bit = 0;
	unsigned int dly = 0;
	unsigned int *cmd_r_ptr;
	unsigned int *rsp_w_ptr;
    unsigned int caddr_l;
    unsigned int caddr_h;
	unsigned int OpCode;
	unsigned int Ower;
	unsigned int status = 0;
	//unsigned int ila = 0;

    cmd_ptr = RD_CMD_RAM_PTR;
    rsp_ptr = WR_RSP_RAM_PTR;

	write32(&i_seg->fw_rev, 0x10001900);
    write32(&i_seg->cmdif_rev_fw_sub, 0x70170500);
    write32(&i_seg->cmdq_addr_h, 0x03000000);
    write32(&i_seg->cmdq_addr_l_sz, 0x56A0E1CD);
    write32(&i_seg->initializing, 0);

	init();

	while(1)
	{
		while(!dbell_c){
			delay(10);
			dbell = SWAP32(read32(&i_seg->cmd_dbell));
			if (dbell){
				dbell_c = dbell;
				write32(&i_seg->cmd_dbell, SWAP32(dbell));
				break;
			}

			recv_process();
			wqe_process();

			dly++;
			if (dly == 80)
			{
				dly = 0;
				health();
			}
		}

		health();

		db_bit = get_set_bit(dbell_c);

		caddr_h = SWAP32((read32(&i_seg->cmdq_addr_h)));
		caddr_l = SWAP32((read32(&i_seg->cmdq_addr_l_sz)));
		caddr_l = caddr_l | (db_bit << 6);
		h2c_activate(caddr_l, caddr_h, 0x40, 1);
		memcpy_ram(0x40, 1);

		cmd_r_ptr = cmd_ptr;
		rsp_w_ptr = rsp_ptr;
		Ower = read32(&cmd_r_ptr[15]);
		if (!(Ower & 0xFF000000)){
			//write32(&i_seg->cmd_dbell, (1 << (0x1F - db_bit)));
			//dbell = dbell & ~(1 << db_bit);
			continue;
		}

		status = 0;
		OpCode = SWAP32((read32(&cmd_r_ptr[4]))) >> 16;
		switch(OpCode)
		{
		case MLX5_CMD_OP_ENABLE_HCA:
			break;
		case MLX5_CMD_OP_QUERY_ISSI:
			break;
		case MLX5_CMD_OP_QUERY_PAGES:
			break;
		case MLX5_CMD_OP_MANAGE_PAGES:
			break;
		case MLX5_CMD_OP_ACCESS_REG:
			break;
		case MLX5_CMD_OP_QUERY_HCA_CAP:
			status = query_hca_cap();
			break;
		case MLX5_CMD_OP_SET_HCA_CAP:
			set_hca_cap();
			break;
		case MLX5_CMD_OP_INIT_HCA:
			break;
		case MLX5_CMD_OP_SET_DRIVER_VERSION:
			break;
		case MLX5_CMD_OP_QUERY_SPECIAL_CONTEXTS:
			status = query_special_ctx();
			break;
		case MLX5_CMD_OP_QUERY_NIC_VPORT_CONTEXT:
			status = query_nic_vport_ctx();
			break;
		case MLX5_CMD_OP_QUERY_ADAPTER:
			status = query_adapter();
			break;
		case MLX5_CMD_OP_ADD_VXLAN_UDP_DPORT:
			break;
		case MLX5_CMD_OP_ALLOC_UAR:
			write32(&rsp_w_ptr[10], SWAP32(uar_idx));
			uar_idx++;
			break;
		case MLX5_CMD_OP_CREATE_EQ:
			write32(&rsp_w_ptr[10], SWAP32(eq.eqn));
			create_eq();
			break;
		case 0x10e: // UNKNOWN COMMAND
			status = unknown_cmd_10e();
			break;
		case MLX5_CMD_OP_MODIFY_NIC_VPORT_CONTEXT:
			break;
		case MLX5_CMD_OP_ALLOC_PD:
			break;
		case MLX5_CMD_OP_CREATE_CQ:
			write32(&rsp_w_ptr[10], SWAP32(cq.cqn));
			create_cq();
			break;
		case MLX5_CMD_OP_ALLOC_XRCD:
			write32(&rsp_w_ptr[10], SWAP32(xrcd_idx));
			xrcd_idx++;
			break;
		case MLX5_CMD_OP_CREATE_SRQ:
			write32(&rsp_w_ptr[10], SWAP32(srq_idx));
			srq_idx++;
			break;
		case MLX5_CMD_OP_ALLOC_Q_COUNTER:
			break;
		case MLX5_CMD_OP_CREATE_UCTX:
			break;
		case MLX5_CMD_OP_QUERY_HCA_VPORT_PKEY:
			break;
		case MLX5_CMD_OP_QUERY_Q_COUNTER:
			break;
		case MLX5_CMD_OP_QUERY_CONG_STATISTICS:
			break;
		case MLX5_CMD_OP_CREATE_MKEY:
			write32(&rsp_w_ptr[10], SWAP32(mk.mkn));
			create_mkey();
			break;
		case MLX5_CMD_OP_CREATE_QP:
			write32(&rsp_w_ptr[10], SWAP32(qp.qpn));
			create_qp_ctx();
			break;
		case MLX5_CMD_OP_RST2INIT_QP:
			create_mod_qp();
			break;
		case MLX5_CMD_OP_INIT2RTR_QP:
			break;
		case MLX5_CMD_OP_RTR2RTS_QP:
			//uar_idx_5 = 1;
			break;
		case MLX5_CMD_OP_2RST_QP:
			break;
		case MLX5_CMD_OP_ALLOC_TRANSPORT_DOMAIN:
			write32(&rsp_w_ptr[10], SWAP32(td_idx));
			td_idx++;
			break;

		case MLX5_CMD_OP_DEALLOC_TRANSPORT_DOMAIN:
			break;
		case MLX5_CMD_OP_DEALLOC_XRCD:
			break;
		case MLX5_CMD_OP_DESTROY_QP:
			break;
		case MLX5_CMD_OP_DESTROY_MKEY:
			break;
		case MLX5_CMD_OP_DEALLOC_PD:
			break;
		case MLX5_CMD_OP_DESTROY_SRQ:
			break;
		case MLX5_CMD_OP_DESTROY_CQ:
			break;
		case MLX5_CMD_OP_DESTROY_EQ:
			eq.eqn--;
			if(eq.eqn < 5) eq.msix_mode = 0;
			break;
		case MLX5_CMD_OP_DEALLOC_UAR:
			break;
		case MLX5_CMD_OP_DELETE_VXLAN_UDP_DPORT:
			break;
		case MLX5_CMD_OP_TEARDOWN_HCA:
			break;
		case MLX5_CMD_OP_DISABLE_HCA:
			break;
		default:
			//default_hndl();
			status = 0xFFEE0000;
			break;
		}


		health();

		Ower = Ower & 0x00FFFFFF;
		write32(&rsp_w_ptr[15], Ower);

		if (status != 0xFFEE0000) {
			caddr_h = SWAP32((read32(&i_seg->cmdq_addr_h)));
			caddr_l = SWAP32((read32(&i_seg->cmdq_addr_l_sz)));
			caddr_l = caddr_l | (db_bit << 6);
			c2h_activate(caddr_l, caddr_h, 0x40, 1);
		}

		event_hndl(0, 0x0A,db_bit);

		dbell_c = dbell_c & ~(1 << db_bit);
	}

    return 0;
}

