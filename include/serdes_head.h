/**
 * @file   serdes_head.h
 * @author DAB-Embedded
 * @date   21 Nov 2024
 * @brief  Header.
 *
 */

#ifndef __SERDES_HEAD__H__
#define __SERDES_HEAD__H__

#include <stdio.h>
#include <stdint.h>

#define SERIALIZER_MAX_PORTS      (4)
#define DESERIALIZER_MAX_PORTS    (4)

#define FEATURE_DES_3GBPS         (1 << 0)
#define FEATURE_DES_6GBPS         (1 << 1)
#define FEATURE_DES_12GBPS        (1 << 2)

#define ARRAY_SIZE(x)       (sizeof(x)/sizeof(x[0]))

typedef struct st_app_params_ {
    int             i2c_port;
    int             mipi_rx_lanes;
    uint16_t        mipi_rx_map;
    uint16_t        mipi_rx_pol;
    int             mipi_rx_skew_en;
    int             mipi_tx_lanes;
    uint16_t        mipi_tx_map;
    uint16_t        mipi_tx_pol;
    int             mipi_tx_deskew_en;
    int             mipi_tx_out_freq;
    int             stats_flags;
    int             ser_i2c_sa;
    int             deser_i2c_sa;
    int             no_init;
} st_app_params, *pst_app_params;

typedef struct serializer_stats_ {
    int global_status;
    int global_pkt_count;
    int mipi_rx_l_lp_errors;
    int mipi_rx_l_hs_errors;
    int mipi_rx_h_lp_errors;
    int mipi_rx_h_hs_errors;
    int ctrl1_csi_l_errors;
    int ctrl1_csi_h_errors;
    int mipi_dphy_rx_count;
    int mipi_pkt_processed;
    int mipi_clk_rx_count;
    int is_tunnel_mode;
    int is_tunnel_overflow;
    int tunnel_pkt_processed;
    int tx_fifo_warn_flag;
    int tx_fifo_overflow_flag;
    int tx_pclk_drift_flag;
    int tx_pclk_det_flag;
} serializer_stats, *pserializer_stats;

typedef struct deserializer_stats_ {
    int global_status;
    int global_pkt_count;
    int remote_error_flag;
    int video_rx_blk_len_err_flag;
    int video_pipeline_locked_flag;
    int sufficient_video_rx_thr_flag;
    int video_seq_error_flag;
    int video_tunnel_flag;
    int video_rx_overflow_flag;
    int video_rx_tun_overflow_flag;
    int mipi_phy_packets_count;
    int csi2_tx_packets_count;
    int lcrc_error_flag;
} deserializer_stats, *pdeserializer_stats;

typedef struct serializer_ctx_ {
    uint8_t             i2c_slave_address;
    serializer_stats    ser_stats[SERIALIZER_MAX_PORTS];
} serializer_ctx, *pserializer_ctx;

typedef struct deserializer_ctx_ {
    uint8_t             i2c_slave_address;
    uint32_t            features;
    deserializer_stats  deser_stats[DESERIALIZER_MAX_PORTS];
} deserializer_ctx, *pdeserializer_ctx;

typedef struct serializer_entry_ {
    uint8_t ser_devid;
    char    ser_name[32];
    int     rx_ports;
    /* Functions */
    int (*init)(pserializer_ctx pctx);
    int (*start)(pserializer_ctx pctx);
    int (*set_mipi_rx_params)(pserializer_ctx pctx,
            int port, int lanes, uint16_t lane_mapping, uint16_t lane_polarity, int skew_en, int tunnel_mode_en);
    int (*reset_link)(pserializer_ctx pctx);
    int (*wait_for_link)(pserializer_ctx pctx);
    int (*set_link_speed_gbps)(pserializer_ctx pctx, int speed);
    int (*get_stats)(pserializer_ctx pctx);
} serializer_entry;

typedef struct deserializer_entry_ {
    uint8_t deser_devid;
    char    deser_name[32];
    int     tx_ports;
    /* Functions */
    int (*init)(pdeserializer_ctx pctx);
    int (*start)(pdeserializer_ctx pctx);
    int (*set_mipi_tx_params)(pdeserializer_ctx pctx,
            int port, int lanes, uint16_t lane_mapping, uint16_t lane_polarity, int deskew_en,
            int out_freq, int tunnel_mode_en);
    int (*reset_link)(pdeserializer_ctx pctx);
    int (*wait_for_link)(pdeserializer_ctx pctx);
    int (*set_link_speed_gbps)(pdeserializer_ctx pctx, int speed);
    int (*get_stats)(pdeserializer_ctx pctx);
} deserializer_entry;

int  i2c_init(int i2c_bus_num);
int  i2c_exit(void);
int  i2c_read_reg8a8(unsigned char dev_addr,unsigned char reg_addr,unsigned char *buf);
int  i2c_read_reg8a16(unsigned char dev_addr,unsigned short reg_addr,unsigned char *buf);
int  i2c_write_reg8a16(unsigned char dev_addr,unsigned short reg_addr,unsigned char value_byte);
int  i2c_write_reg8a8(unsigned char dev_addr,unsigned char reg_addr,unsigned char value_byte);
int  i2c_write_buffer(unsigned char dev_addr,unsigned char *pbuf,size_t length);

int  max96717_init(pserializer_ctx pctx);
int  max96717_start(pserializer_ctx pctx);
int  max96717_set_mipi_rx_params(pserializer_ctx pctx,
        int port, int lanes, uint16_t lane_mapping, uint16_t lane_polarity, int skew_en, int tunnel_mode_en);
int  max96717_reset_link(pserializer_ctx pctx);
int  max96717_wait_for_link(pserializer_ctx pctx);
int  max96717_set_link_speed_gbps(pserializer_ctx pctx, int speed);
int  max96717_get_stats(pserializer_ctx pctx);

int  max9295d_init(pserializer_ctx pctx);
int  max9295d_start(pserializer_ctx pctx);
int  max9295d_set_mipi_rx_params(pserializer_ctx pctx,
        int port, int lanes, uint16_t lane_mapping, uint16_t lane_polarity, int skew_en, int tunnel_mode_en);
int  max9295d_reset_link(pserializer_ctx pctx);
int  max9295d_wait_for_link(pserializer_ctx pctx);
int  max9295d_set_link_speed_gbps(pserializer_ctx pctx, int speed);
int  max9295d_get_stats(pserializer_ctx pctx);

int  max96714_init(pdeserializer_ctx pctx);
int  max96714_start(pdeserializer_ctx pctx);
int  max96714_set_mipi_tx_params(pdeserializer_ctx pctx,
        int port, int lanes, uint16_t lane_mapping, uint16_t lane_polarity, int deskew_en, int out_freq, int tunnel_mode_en);
int  max96714_reset_link(pdeserializer_ctx pctx);
int  max96714_wait_for_link(pdeserializer_ctx pctx);
int  max96714_set_link_speed_gbps(pdeserializer_ctx pctx, int speed);
int  max96714_get_stats(pdeserializer_ctx pctx);

int  max96792_init(pdeserializer_ctx pctx);
int  max96792_start(pdeserializer_ctx pctx);
int  max96792_set_mipi_tx_params(pdeserializer_ctx pctx,
        int port, int lanes, uint16_t lane_mapping, uint16_t lane_polarity, int deskew_en, int out_freq, int tunnel_mode_en);
int  max96792_reset_link(pdeserializer_ctx pctx);
int  max96792_wait_for_link(pdeserializer_ctx pctx);
int  max96792_set_link_speed_gbps(pdeserializer_ctx pctx, int speed);
int  max96792_get_stats(pdeserializer_ctx pctx);

int  max96712_init(pdeserializer_ctx pctx);
int  max96712_start(pdeserializer_ctx pctx);
int  max96712_set_mipi_tx_params(pdeserializer_ctx pctx,
        int port, int lanes, uint16_t lane_mapping, uint16_t lane_polarity, int deskew_en, int out_freq, int tunnel_mode_en);
int  max96712_reset_link(pdeserializer_ctx pctx);
int  max96712_wait_for_link(pdeserializer_ctx pctx);
int  max96712_set_link_speed_gbps(pdeserializer_ctx pctx, int speed);
int  max96712_get_stats(pdeserializer_ctx pctx);

int  application_opt_parsing(int argc, char *argv[]);

int  serializer_search_chip(void);
int  serializer_init(uint32_t features);
int  serializer_get_stat(void);
int  deserializer_init(void);
int  deserializer_search_for_serializer(void);
uint32_t deserializer_get_features(void);
int  deserializer_set_link_speed(int speed);
int  deserializer_start(void);
int  deserializer_get_stat(void);

extern st_app_params app_params;

#endif
