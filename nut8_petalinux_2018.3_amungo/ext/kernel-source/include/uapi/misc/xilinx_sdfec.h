/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Xilinx SD-FEC
 *
 * Copyright (C) 2016 - 2017 Xilinx, Inc.
 *
 * Description:
 * This driver is developed for SDFEC16 IP. It provides a char device
 * in sysfs and supports file operations like open(), close() and ioctl().
 */
#ifndef __XILINX_SDFEC_H__
#define __XILINX_SDFEC_H__

/* Shared LDPC Tables */
#define XSDFEC_LDPC_SC_TABLE_ADDR_BASE		(0x10000)
#define XSDFEC_LDPC_SC_TABLE_ADDR_HIGH		(0x103FC)
#define XSDFEC_LDPC_LA_TABLE_ADDR_BASE		(0x18000)
#define XSDFEC_LDPC_LA_TABLE_ADDR_HIGH		(0x18FFC)
#define XSDFEC_LDPC_QC_TABLE_ADDR_BASE		(0x20000)
#define XSDFEC_LDPC_QC_TABLE_ADDR_HIGH		(0x27FFC)

/**
 * enum xsdfec_code - Code Type.
 * @XSDFEC_CODE_INVALID: Driver is in a invalid mode.
 * @XSDFEC_TURBO_CODE: Driver is configured for Turbo mode.
 * @XSDFEC_LDPC_CODE: Driver is configured for LDPC mode.
 *
 * This enum is used to indicate the mode of the driver. The mode is determined
 * by checking which codes are set in the driver. Note that the mode cannot be
 * changed by the driver.
 */
enum xsdfec_code {
	XSDFEC_CODE_INVALID = 0,
	XSDFEC_TURBO_CODE,
	XSDFEC_LDPC_CODE,
};

/**
 * enum xsdfec_order - Order
 * @XSDFEC_INVALID_ORDER: Invalid order value
 * @XSDFEC_MAINTAIN_ORDER: Maintain order execution of blocks.
 * @XSDFEC_OUT_OF_ORDER: Out-of-order execution of blocks.
 * @XSDFEC_ORDER_MAX: Used to indicate out of bound order values.
 *
 * This enum is used to indicate whether the order of blocks can change from
 * input to output.
 */
enum xsdfec_order {
	XSDFEC_INVALID_ORDER = 0,
	XSDFEC_MAINTAIN_ORDER,
	XSDFEC_OUT_OF_ORDER,
	XSDFEC_ORDER_MAX,
};

/**
 * enum xsdfec_turbo_alg - Turbo Algorithm Type.
 * @XSDFEC_MAX_SCALE: Max Log-Map algorithm with extrinsic scaling. When
 *		      scaling is set to this is equivalent to the Max Log-Map
 *		      algorithm.
 * @XSDFEC_MAX_STAR: Log-Map algorithm.
 * @XSDFEC_TURBO_ALG_MAX: Used to indicate out of bound Turbo algorithms.
 *
 * This enum specifies which Turbo Decode algorithm is in use.
 */
enum xsdfec_turbo_alg {
	XSDFEC_MAX_SCALE = 0,
	XSDFEC_MAX_STAR,
	XSDFEC_TURBO_ALG_MAX,
};

/**
 * enum xsdfec_state - State.
 * @XSDFEC_INIT: Driver is initialized.
 * @XSDFEC_STARTED: Driver is started.
 * @XSDFEC_STOPPED: Driver is stopped.
 * @XSDFEC_NEEDS_RESET: Driver needs to be reset.
 * @XSDFEC_PL_RECONFIGURE: Programmable Logic needs to be recofigured.
 *
 * This enum is used to indicate the state of the driver.
 */
enum xsdfec_state {
	XSDFEC_INIT = 0,
	XSDFEC_STARTED,
	XSDFEC_STOPPED,
	XSDFEC_NEEDS_RESET,
	XSDFEC_PL_RECONFIGURE,
};

/**
 * enum xsdfec_axis_width - AXIS_WIDTH.DIN Setting for 128-bit width.
 * @XSDFEC_1x128b: DIN data input stream consists of a 128-bit lane
 * @XSDFEC_2x128b: DIN data input stream consists of two 128-bit lanes
 * @XSDFEC_4x128b: DIN data input stream consists of four 128-bit lanes
 *
 * This enum is used to indicate the AXIS_WIDTH.DIN setting for 128-bit width.
 * The number of lanes of the DIN data input stream depends upon the
 * AXIS_WIDTH.DIN parameter.
 */
enum xsdfec_axis_width {
	XSDFEC_1x128b = 1,
	XSDFEC_2x128b = 2,
	XSDFEC_4x128b = 4,
};

/**
 * enum xsdfec_axis_word_include - Words Configuration.
 * @XSDFEC_FIXED_VALUE: Fixed, the DIN_WORDS AXI4-Stream interface is removed
 *			from the IP instance and is driven with the specified
 *			number of words.
 * @XSDFEC_IN_BLOCK: In Block, configures the IP instance to expect a single
 *		     DIN_WORDS value per input code block. The DIN_WORDS
 *		     interface is present.
 * @XSDFEC_PER_AXI_TRANSACTION: Per Transaction, configures the IP instance to
 * expect one DIN_WORDS value per input transaction on the DIN interface. The
 * DIN_WORDS interface is present.
 * @XSDFEC_AXIS_WORDS_INCLUDE_MAX: Used to indicate out of bound Words
 *				   Configurations.
 *
 * This enum is used to specify the DIN_WORDS configuration.
 */
enum xsdfec_axis_word_include {
	XSDFEC_FIXED_VALUE = 0,
	XSDFEC_IN_BLOCK,
	XSDFEC_PER_AXI_TRANSACTION,
	XSDFEC_AXIS_WORDS_INCLUDE_MAX,
};

/**
 * struct xsdfec_turbo - User data for Turbo codes.
 * @alg: Specifies which Turbo decode algorithm to use
 * @scale: Specifies the extrinsic scaling to apply when the Max Scale algorithm
 *	   has been selected
 *
 * Turbo code structure to communicate parameters to XSDFEC driver.
 */
struct xsdfec_turbo {
	enum xsdfec_turbo_alg alg;
	u8 scale;
};

/**
 * struct xsdfec_ldpc_params - User data for LDPC codes.
 * @n: Number of code word bits
 * @k: Number of information bits
 * @psize: Size of sub-matrix
 * @nlayers: Number of layers in code
 * @nqc: Quasi Cyclic Number
 * @nmqc: Number of M-sized QC operations in parity check matrix
 * @nm: Number of M-size vectors in N
 * @norm_type: Normalization required or not
 * @no_packing: Determines if multiple QC ops should be performed
 * @special_qc: Sub-Matrix property for Circulant weight > 0
 * @no_final_parity: Decide if final parity check needs to be performed
 * @max_schedule: Experimental code word scheduling limit
 * @sc_off: SC offset
 * @la_off: LA offset
 * @qc_off: QC offset
 * @sc_table: SC Table
 * @la_table: LA Table
 * @qc_table: QC Table
 * @code_id: LDPC Code
 *
 * This structure describes the LDPC code that is passed to the driver by the
 * application.
 */
struct xsdfec_ldpc_params {
	u32 n;
	u32 k;
	u32 psize;
	u32 nlayers;
	u32 nqc;
	u32 nmqc;
	u32 nm;
	u32 norm_type;
	u32 no_packing;
	u32 special_qc;
	u32 no_final_parity;
	u32 max_schedule;
	u32 sc_off;
	u32 la_off;
	u32 qc_off;
	u32 sc_table[XSDFEC_LDPC_SC_TABLE_ADDR_HIGH -
			XSDFEC_LDPC_SC_TABLE_ADDR_BASE];
	u32 la_table[XSDFEC_LDPC_LA_TABLE_ADDR_HIGH -
			XSDFEC_LDPC_LA_TABLE_ADDR_BASE];
	u32 qc_table[XSDFEC_LDPC_QC_TABLE_ADDR_HIGH -
			XSDFEC_LDPC_QC_TABLE_ADDR_BASE];
	u16 code_id;
};

/**
 * struct xsdfec_status - Status of SD-FEC core.
 * @fec_id: ID of SD-FEC instance
 * @state: State of the SD-FEC core
 * @activity: Describes if the SD-FEC instance is Active
 */
struct xsdfec_status {
	s32 fec_id;
	enum xsdfec_state state;
	bool activity;
};

/**
 * struct xsdfec_irq - Enabling or Disabling Interrupts.
 * @enable_isr: If true enables the ISR
 * @enable_ecc_isr: If true enables the ECC ISR
 */
struct xsdfec_irq {
	bool enable_isr;
	bool enable_ecc_isr;
};

/**
 * struct xsdfec_config - Configuration of SD-FEC core.
 * @fec_id: ID of SD-FEC instance
 * @code: The codes being used by the SD-FEC instance
 * @order: Order of Operation
 * @bypass: Is the core being bypassed
 * @code_wr_protect: Is write protection of LDPC codes enabled
 * @din_width: Width of the DIN AXI4-Stream
 * @din_word_include: How DIN_WORDS are inputted
 * @dout_width: Width of the DOUT AXI4-Stream
 * @dout_word_include: HOW DOUT_WORDS are outputted
 * @irq: Enabling or disabling interrupts
 */
struct xsdfec_config {
	s32 fec_id;
	enum xsdfec_code code;
	enum xsdfec_order order;
	bool bypass;
	bool code_wr_protect;
	enum xsdfec_axis_width din_width;
	enum xsdfec_axis_word_include din_word_include;
	enum xsdfec_axis_width dout_width;
	enum xsdfec_axis_word_include dout_word_include;
	struct xsdfec_irq irq;
};

/**
 * struct xsdfec_stats - Stats retrived by ioctl XSDFEC_GET_STATS. Used
 *			 to buffer atomic_t variables from struct
 *			 xsdfec_dev.
 * @isr_err_count: Count of ISR errors
 * @cecc_count: Count of Correctable ECC errors (SBE)
 * @uecc_count: Count of Uncorrectable ECC errors (MBE)
 */
struct xsdfec_stats {
	u32 isr_err_count;
	u32 cecc_count;
	u32 uecc_count;
};

/**
 * struct xsdfec_ldpc_param_table_sizes - Used to store sizes of SD-FEC table
 *					  entries for an individual LPDC code
 *					  parameter.
 * @sc_size: Size of SC table used
 * @la_size: Size of LA table used
 * @qc_size: Size of QC table used
 */
struct xsdfec_ldpc_param_table_sizes {
	u32 sc_size;
	u32 la_size;
	u32 qc_size;
};

/**
 * xsdfec_calculate_shared_ldpc_table_entry_size - Calculates shared code
 * table sizes.
 * @ldpc: Pointer to the LPDC Code Parameters
 * @table_sizes: Pointer to structure containing the calculated table sizes
 *
 * Calculates the size of shared LDPC code tables used for a specified LPDC code
 * parameters.
 */
inline void xsdfec_calculate_shared_ldpc_table_entry_size(
	struct xsdfec_ldpc_params *ldpc,
	struct xsdfec_ldpc_param_table_sizes *table_sizes)
{
	/* Calculate the sc_size in 32 bit words */
	table_sizes->sc_size = (ldpc->nlayers + 3) >> 2;
	/* Calculate the la_size in 256 bit words */
	table_sizes->la_size = ((ldpc->nlayers << 2) + 15) >> 4;
	/* Calculate the qc_size in 256 bit words */
	table_sizes->qc_size = ((ldpc->nqc << 2) + 15) >> 4;
}

/*
 * XSDFEC IOCTL List
 */
#define XSDFEC_MAGIC		'f'
/**
 * DOC: XSDFEC_START_DEV
 *
 * @Description
 *
 * ioctl to start SD-FEC core
 *
 * This fails if the XSDFEC_SET_ORDER ioctl has not been previously called
 */
#define XSDFEC_START_DEV	_IO(XSDFEC_MAGIC, 0)
/**
 * DOC: XSDFEC_STOP_DEV
 *
 * @Description
 *
 * ioctl to stop the SD-FEC core
 */
#define XSDFEC_STOP_DEV		_IO(XSDFEC_MAGIC, 1)
/**
 * DOC: XSDFEC_GET_STATUS
 *
 * @Description
 *
 * ioctl that returns status of SD-FEC core
 */
#define XSDFEC_GET_STATUS	_IOR(XSDFEC_MAGIC, 3, struct xsdfec_status *)
/**
 * DOC: XSDFEC_SET_IRQ
 * @Parameters
 *
 * @struct xsdfec_irq *
 *	Pointer to the &struct xsdfec_irq that contains the interrupt settings
 *	for the SD-FEC core
 *
 * @Description
 *
 * ioctl to enable or disable irq
 */
#define XSDFEC_SET_IRQ		_IOW(XSDFEC_MAGIC, 4, struct xsdfec_irq *)
/**
 * DOC: XSDFEC_SET_TURBO
 * @Parameters
 *
 * @struct xsdfec_turbo *
 *	Pointer to the &struct xsdfec_turbo that contains the Turbo decode
 *	settings for the SD-FEC core
 *
 * @Description
 *
 * ioctl that sets the SD-FEC Turbo parameter values
 *
 * This can only be used when the driver is in the XSDFEC_STOPPED state
 */
#define XSDFEC_SET_TURBO	_IOW(XSDFEC_MAGIC, 5, struct xsdfec_turbo *)
/**
 * DOC: XSDFEC_ADD_LDPC_CODE_PARAMS
 * @Parameters
 *
 * @struct xsdfec_ldpc_params *
 *	Pointer to the &struct xsdfec_ldpc_params that contains the LDPC code
 *	parameters to be added to the SD-FEC Block
 *
 * @Description
 * ioctl to add an LDPC code to the SD-FEC LDPC codes
 *
 * This can only be used when:
 *
 * - Driver is in the XSDFEC_STOPPED state
 *
 * - SD-FEC core is configured as LPDC
 *
 * - SD-FEC Code Write Protection is disabled
 */
#define XSDFEC_ADD_LDPC_CODE_PARAMS	\
	_IOW(XSDFEC_MAGIC, 6, struct xsdfec_ldpc_params *)
/**
 * DOC: XSDFEC_GET_CONFIG
 * @Parameters
 *
 * @struct xsdfec_config *
 *	Pointer to the &struct xsdfec_config that contains the current
 *	configuration settings of the SD-FEC Block
 *
 * @Description
 *
 * ioctl that returns SD-FEC core configuration
 */
#define XSDFEC_GET_CONFIG	_IOR(XSDFEC_MAGIC, 7, struct xsdfec_config *)
/**
 * DOC: XSDFEC_GET_TURBO
 * @Parameters
 *
 * @struct xsdfec_turbo *
 *	Pointer to the &struct xsdfec_turbo that contains the current Turbo
 *	decode settings of the SD-FEC Block
 *
 * @Description
 *
 * ioctl that returns SD-FEC turbo param values
 */
#define XSDFEC_GET_TURBO	_IOR(XSDFEC_MAGIC, 8, struct xsdfec_turbo *)
/**
 * DOC: XSDFEC_SET_ORDER
 * @Parameters
 *
 * @struct unsigned long *
 *	Pointer to the unsigned long that contains a value from the
 *	@enum xsdfec_order
 *
 * @Description
 *
 * ioctl that sets order, if order of blocks can change from input to output
 *
 * This can only be used when the driver is in the XSDFEC_STOPPED state
 */
#define XSDFEC_SET_ORDER	_IOW(XSDFEC_MAGIC, 10, unsigned long *)
/**
 * DOC: XSDFEC_SET_BYPASS
 * @Parameters
 *
 * @struct bool *
 *	Pointer to bool that sets the bypass value, where false results in
 *	normal operation and false results in the SD-FEC performing the
 *	configured operations (same number of cycles) but output data matches
 *	the input data
 *
 * @Description
 *
 * ioctl that sets bypass.
 *
 * This can only be used when the driver is in the XSDFEC_STOPPED state
 */
#define XSDFEC_SET_BYPASS	_IOW(XSDFEC_MAGIC, 11, bool *)
/**
 * DOC: XSDFEC_IS_ACTIVE
 * @Parameters
 *
 * @struct bool *
 *	Pointer to bool that returns true if the SD-FEC is processing data
 *
 * @Description
 *
 * ioctl that determines if SD-FEC is processing data
 */
#define XSDFEC_IS_ACTIVE	_IOR(XSDFEC_MAGIC, 12, bool *)
/**
 * DOC: XSDFEC_CLEAR_STATS
 *
 * @Description
 *
 * ioctl that clears error stats collected during interrupts
 */
#define XSDFEC_CLEAR_STATS	_IO(XSDFEC_MAGIC, 13)
/**
 * DOC: XSDFEC_GET_STATS
 * @Parameters
 *
 * @struct xsdfec_stats *
 *	Pointer to the &struct xsdfec_stats that will contain the updated stats
 *	values
 *
 * @Description
 *
 * ioctl that returns SD-FEC core stats
 *
 * This can only be used when the driver is in the XSDFEC_STOPPED state
 */
#define XSDFEC_GET_STATS	_IOR(XSDFEC_MAGIC, 14, struct xsdfec_stats *)
/**
 * DOC: XSDFEC_SET_DEFAULT_CONFIG
 *
 * @Description
 *
 * ioctl that returns SD-FEC core to default config, use after a reset
 *
 * This can only be used when the driver is in the XSDFEC_STOPPED state
 */
#define XSDFEC_SET_DEFAULT_CONFIG _IO(XSDFEC_MAGIC, 15)

#endif /* __XILINX_SDFEC_H__ */
