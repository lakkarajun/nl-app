/* SPDX-License-Identifier: (GPL-2.0 OR MIT) */
#ifndef __PHY_NETLINK_H
#define __PHY_NETLINK_H

#ifndef u8
typedef unsigned char u8;
#endif

#define CABDIAG_PAIR_A_MASK 0x0001
#define CABDIAG_PAIR_B_MASK 0x0002
#define CABDIAG_PAIR_C_MASK 0x0004
#define CABDIAG_PAIR_D_MASK 0x0008

/* Genetlink setup */
enum {
	PHYNL_CMD_NOOP,
	PHYNL_CMD_CABDIAG,

	__PHYNL_CMD_CNT,
	PHYNL_CMD_MAX = (__PHYNL_CMD_CNT - 1)
};

enum {
	CABDIAG_OP_ATTR_NOOP,
	CABDIAG_OP_ATTR_REQUEST,
	CABDIAG_OP_ATTR_STATUS,

	__CABDIAG_OP_ATTR_CNT,
	CABDIAG_OP_ATTR_MAX = (__CABDIAG_OP_ATTR_CNT - 1)
};

enum {
	CABDIAG_REQ_ATTR_NOOP,
	CABDIAG_REQ_ATTR_IFNAME,
	CABDIAG_REQ_ATTR_CMD,
	CABDIAG_REQ_ATTR_PAIRS_MASK,
	CABDIAG_REQ_ATTR_TIMEOUT,

	__CABDIAG_REQ_ATTR_CNT,
	CABDIAG_REQ_ATTR_MAX = (__CABDIAG_REQ_ATTR_CNT - 1)
};

enum {
	CABDIAG_PAIR_STA_ATTR_NOOP,
	CABDIAG_PAIR_STA_ATTR_CODE,
	CABDIAG_PAIR_STA_ATTR_LENGTH,

	__CABDIAG_PAIR_STA_ATTR_CNT,
	CABDIAG_PAIR_STA_ATTR_MAX = (__CABDIAG_PAIR_STA_ATTR_CNT - 1)
};

enum {
	CABDIAG_STA_ATTR_NOOP,
	CABDIAG_STA_ATTR_IFNAME,
	CABDIAG_STA_ATTR_PAIRS_MASK,
	CABDIAG_STA_ATTR_STATUS,

	__CABDIAG_STA_ATTR_CNT,
	CABDIAG_STA_ATTR_MAX = (__CABDIAG_STA_ATTR_CNT - 1)
};

/**
 * CMD_CABDIAG_NONE: Not valid command
 * CMD_CABDIAG_STOP: Cable diagnostics stp command
 * CMD_CABDIAG_START: Cable diagnostics start command
 */
enum phy_cabdiag_cmd {
	CMD_CABDIAG_NONE,
	CMD_CABDIAG_STOP,
	CMD_CABDIAG_START
};

/**
 * struct phy_cabdiag_req - PHY diagnostics request command
 * @cmd: Diagnostics Command
 * @pairs_bitmask: Allows settings diag request just for a pair
 * @timeout: Timeout in seconds
 */
struct phy_cabdiag_req {
	enum phy_cabdiag_cmd cmd;
	u8 pairs_bitmask;
	u8 timeout;
};

#define CABDIAG_PAIR_A_MASK 0x0001
#define CABDIAG_PAIR_B_MASK 0x0002
#define CABDIAG_PAIR_C_MASK 0x0004
#define CABDIAG_PAIR_D_MASK 0x0008

enum {
	CABDIAG_PAIR_A,
	CABDIAG_PAIR_B,
	CABDIAG_PAIR_C,
	CABDIAG_PAIR_D,

	CABDIAG_PAIR_CNT,
};

/**
 * phy_cabdiag_sta_code - Cable diagnostics fault codes
 * b0000 - 0100: Individual cable pair fault codes
 * b10xx       : Cross pair short to pair 'xx'
 * b11xx       : Abnormal Cross pair coupling with pair 'xx'
 *    xx       : b00 - Pair-A,
 *             : b01 - Pair-B,
 *             : b10 - Pair-C,
 *             : b11 - Pair-D
 */
enum phy_cabdiag_sta_code {
	CD_NORMAL_PAIR            = 0x0,
	CD_OPEN_PAIR              = 0x1,
	CD_SHORTED_PAIR           = 0x2,
	CD_ABNORMAL_TERMINATION   = 0x4,
	CD_X_PAIR_SHORT_TO_PAIR_A = 0x8,
	CD_X_PAIR_SHORT_TO_PAIR_B = 0x9,
	CD_X_PAIR_SHORT_TO_PAIR_C = 0xA,
	CD_X_PAIR_SHORT_TO_PAIR_D = 0xB,
	CD_ABNORMAL_X_PAIR_A      = 0xC,
	CD_ABNORMAL_X_PAIR_B      = 0xD,
	CD_ABNORMAL_X_PAIR_C      = 0xE,
	CD_ABNORMAL_X_PAIR_D      = 0xF,
	CD_NOT_SUPPORT            = 0xFF
};

/**
 * struct phy_cabdiag_pair_sta - PHY diagnostics pair status
 * @code: Fault codes
 * @length: Length in meters
 */
struct phy_cabdiag_pair_sta {
	enum phy_cabdiag_sta_code status;
	u8 length;
};

/**
 * struct phy_cabdiag_sta - PHY diagnostics status
 * @pairs_bitmask: Allows settings diag request just for a pair
 * @pairs: Status of each pair in cable
 */
struct phy_cabdiag_sta {
	u8 pairs_bitmask;
	struct phy_cabdiag_pair_sta pairs[CABDIAG_PAIR_CNT];
};

#endif /* __PHY_NETLINK_H */
