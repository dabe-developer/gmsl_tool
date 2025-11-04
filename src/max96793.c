/**
 * @file   max96793.c
 * @author DAB-Embedded
 * @date   28 Oct 2025
 * @brief  MAX96793 function source.
 *
 */

#include <stdio.h>
#include <linux/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "serdes_head.h"

#define SER_CTX_CHECK(x) if (x == NULL) { return -EFAULT; }

#define MIPI_RX_DELAY     (100 * 1000)
#define MIPI_RST_TIME     (150 * 1000)
#define LINK_WAIT_TIME    (20)

int max96793_init(pserializer_ctx pctx)
{
    uint8_t reg8 = 0;

    SER_CTX_CHECK(pctx);

    /* Coax drive */
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x0011, 0x03);

    /* DEV : REG2 | VID_TX_EN_Z (VID_TX_EN_Z): Disabled */
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x0002, 0x03);

    /* Enable LOCK_EN */
    i2c_read_reg8a16(pctx->i2c_slave_address, 0x0005, &reg8);
    reg8 |= (1 << 7);
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x0005, reg8);

    /* MIPI_RX : MIPI_RX0 : Continious clock */
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x0330, 0x00);

    /* 400kHz I2C, 16ms timeout */
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x40, 0x15);
    /* 400kHz I2C, 16ms timeout */
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x41, 0x55);

    /* Stop heartbeat */
    //i2c_read_reg8a16(pctx->i2c_slave_address, 0x0112, &reg8);
    ///reg8 &= 0xFB;
    //i2c_write_reg8a16(pctx->i2c_slave_address, 0x0112, reg8);

    /* GPIO4 */
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x0570, 0x00);
    /* No pull, GPIO_RX_EN=0 */
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x02CA, 0x01);
    /* Pull none, Push-pull, GPIO ID disabled */
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x02CB, 0x20);
    /* Override, Disable GMSL RX */
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x02CC, 0x00);

    return 0;
}

int max96793_start(pserializer_ctx pctx)
{
    //uint8_t reg8 = 0;

    SER_CTX_CHECK(pctx);

    /* FRONTTOP : FRONTTOP_0 | (Default) RSVD (CLK_SELZ): Port B | (Default) START_PORTB (START_PORTB): Enabled */
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x0308, 0x64);
    /* FRONTTOP : FRONTTOP_9 | (Default) START_PORTBZ (START_PORTBZ): Start Video */
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x0311, 0x40);
    /* CFGV__VIDEO_Z : TX3 | TX_STR_SEL (TX_STR_SEL Pipe Z): 0x0 */
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x005B, 0x00);

    usleep(MIPI_RX_DELAY);

    /* DEV : REG2 | VID_TX_EN_Z (VID_TX_EN_Z): Enabled */
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x0002, 0x43);

    /* START_PORTBZ to 1 */
    //i2c_read_reg8a16(pctx->i2c_slave_address, 0x0311, &reg8);
    //reg8 |= (1 << 6);
    //i2c_write_reg8a16(pctx->i2c_slave_address, 0x0311, reg8);

    /* MIPI RX reset */
    //i2c_write_reg8a16(pctx->i2c_slave_address, 0x0330, 0x08);
    //usleep(MIPI_RX_DELAY);
    //i2c_write_reg8a16(pctx->i2c_slave_address, 0x0330, 0x00);

    return 0;
}

int max96793_set_mipi_rx_params(pserializer_ctx pctx,
        int port, int lanes, uint16_t lane_mapping, uint16_t lane_polarity, int skew_en, int tunnel_mode_en)
{
    uint8_t reg8 = 0;

    SER_CTX_CHECK(pctx);

    //if ((lanes == 0) && (lanes != 1) && (lanes != 2) && (lanes != 4)) lanes = 4;

    if (tunnel_mode_en != 0)
    {
        /* MIPI_RX_EXT : EXT11 | (Default) Tun_Mode (Tunnel Mode): Enabled */
        i2c_write_reg8a16(pctx->i2c_slave_address, 0x0383, 0x80);
    } else {

        /* Turn off Tunnel mode */
        i2c_read_reg8a16(pctx->i2c_slave_address, 0x0383, &reg8);
        reg8 &= 0x7F;
        i2c_write_reg8a16(pctx->i2c_slave_address, 0x0383, reg8);

    }

    reg8 = ((lanes - 1) << 4);
    if (skew_en != 0) reg8 |= (1 << 6);
    /* MIPI_RX : MIPI_RX1 | (Default) ctrl1_num_lanes (Port B - Lane Count): set */
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x0331, reg8);

    /* MIPI_RX : MIPI_RX2 | (Default) phy1_lane_map (Lane Map - PHY1 D0): Lane 2 | (Default) phy1_lane_map (Lane Map - PHY1 D1): Lane 3 */
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x0332, (lane_mapping >> 8) & 0xFF);
    /* MIPI_RX : MIPI_RX3 | (Default) phy2_lane_map (Lane Map - PHY2 D0): Lane 0 | (Default) phy2_lane_map (Lane Map - PHY2 D1): Lane 1 */
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x0333, lane_mapping & 0xFF);
    /* MIPI_RX : MIPI_RX4 | (Default) phy1_pol_map (Polarity - PHY1 Lane 0): Normal | (Default) phy1_pol_map (Polarity - PHY1 Lane 1): Normal */
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x0334, (lane_polarity >> 8) & 0xFF);
    /* MIPI_RX : MIPI_RX5 | (Default) phy2_pol_map (Polarity - PHY2 Lane 0): Normal | (Default) phy2_pol_map (Polarity - PHY2 Lane 1): Normal | (Default) phy2_pol_map (Polarity - PHY2 Clock Lane): Normal */
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x0335, lane_polarity & 0xFF);

    /* LCRC on */
    i2c_read_reg8a16(pctx->i2c_slave_address, 0x0110, &reg8);
    reg8 |= (1 << 6);
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x0110, reg8);

    usleep(MIPI_RX_DELAY);

    return 0;
}

int max96793_wait_for_link(pserializer_ctx pctx)
{
    uint8_t reg8 = 0;
    int i;

    SER_CTX_CHECK(pctx);

    i2c_read_reg8a16(pctx->i2c_slave_address, 0x0013, &reg8);

    if (reg8 & 0x08) return 0;

    i = 0;
    while(i++ < LINK_WAIT_TIME)
    {
        i2c_read_reg8a16(pctx->i2c_slave_address, 0x0013, &reg8);

        if (reg8 & 0x08) return 0;

        usleep(100 * 1000);
    }

    return -1;
}

int max96793_set_link_speed_gbps(pserializer_ctx pctx, int speed)
{
    uint8_t reg8 = 0;

    SER_CTX_CHECK(pctx);

    switch(speed)
    {
    case 3:
        /* Enable 3G */
        i2c_read_reg8a16(pctx->i2c_slave_address, 0x0001, &reg8);
        reg8 &= ~0xC;
        reg8 |= 0x4;
        i2c_write_reg8a16(pctx->i2c_slave_address, 0x0001, reg8);

        printf("MAX96793 prepared for 3G.\r\n");
        break;
    case 6:
        /* Enable 6G */
        i2c_read_reg8a16(pctx->i2c_slave_address, 0x0001, &reg8);
        reg8 &= ~0xC;
        reg8 |= 0x8;
        i2c_write_reg8a16(pctx->i2c_slave_address, 0x0001, reg8);

        /* Disable FEC */
        i2c_read_reg8a16(pctx->i2c_slave_address,  0x0028, &reg8);
        reg8 &= ~(1 << 1);
        i2c_write_reg8a16(pctx->i2c_slave_address, 0x0028, reg8);

        /* Enable GMSL2 */
        i2c_read_reg8a16(pctx->i2c_slave_address,  0x0006, &reg8);
        reg8 |= (1 << 7);
        i2c_write_reg8a16(pctx->i2c_slave_address, 0x0006, reg8);
        
        printf("MAX96793 prepared for 6G.\r\n");
        break;
    case 12:
        /* Enable 12G */
        i2c_read_reg8a16(pctx->i2c_slave_address, 0x0001, &reg8);
        reg8 |= 0xC;
        i2c_write_reg8a16(pctx->i2c_slave_address, 0x0001, reg8);

        /* Enable FEC */
        i2c_read_reg8a16(pctx->i2c_slave_address,  0x0028, &reg8);
        reg8 |= (1 << 1);
        i2c_write_reg8a16(pctx->i2c_slave_address, 0x0028, reg8);

        /* Disable GMSL2 */
        i2c_read_reg8a16(pctx->i2c_slave_address,  0x0006, &reg8);
        reg8 &= ~(1 << 7);
        i2c_write_reg8a16(pctx->i2c_slave_address, 0x0006, reg8);

        printf("MAX96793 prepared for 12G.\r\n");
        break;
    default:
        break;
    }

    return 0;
}

int max96793_reset_link(pserializer_ctx pctx)
{
    uint8_t reg8 = 0;

    SER_CTX_CHECK(pctx);

    /* Reset one shot */
    i2c_read_reg8a16(pctx->i2c_slave_address,  0x0010, &reg8);
    reg8 |= (1 << 5);
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x0010, reg8);

    return 0;
}

int max96793_get_stats(pserializer_ctx pctx)
{
    uint8_t reg8 = 0;

    SER_CTX_CHECK(pctx);

    /* RX0 Counting Video packets only */
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x002C, 0x01);

    i2c_read_reg8a16(pctx->i2c_slave_address, 0x0013, &reg8);
    pctx->ser_stats[0].global_status = reg8 & 0x04 ? -1 : 0;

    i2c_read_reg8a16(pctx->i2c_slave_address, 0x033B, &reg8);
    pctx->ser_stats[0].mipi_rx_l_lp_errors = reg8;

    i2c_read_reg8a16(pctx->i2c_slave_address, 0x033C, &reg8);
    pctx->ser_stats[0].mipi_rx_l_hs_errors = reg8;

    i2c_read_reg8a16(pctx->i2c_slave_address, 0x033D, &reg8);
    pctx->ser_stats[0].mipi_rx_h_lp_errors = reg8;

    i2c_read_reg8a16(pctx->i2c_slave_address, 0x033E, &reg8);
    pctx->ser_stats[0].mipi_rx_h_hs_errors = reg8;

    i2c_read_reg8a16(pctx->i2c_slave_address, 0x0343, &reg8);
    pctx->ser_stats[0].ctrl1_csi_l_errors = reg8;

    i2c_read_reg8a16(pctx->i2c_slave_address, 0x0344, &reg8);
    pctx->ser_stats[0].ctrl1_csi_h_errors = reg8;

    i2c_read_reg8a16(pctx->i2c_slave_address, 0x038D, &reg8);
    pctx->ser_stats[0].mipi_dphy_rx_count = reg8;

    i2c_read_reg8a16(pctx->i2c_slave_address, 0x038E, &reg8);
    pctx->ser_stats[0].mipi_pkt_processed = reg8;

    i2c_read_reg8a16(pctx->i2c_slave_address, 0x0390, &reg8);
    pctx->ser_stats[0].mipi_clk_rx_count = reg8;

    i2c_read_reg8a16(pctx->i2c_slave_address, 0x0383, &reg8);
    pctx->ser_stats[0].is_tunnel_mode = reg8 & 0x80 ? 1 : 0;

    if (reg8 & 0x80)
    {
        i2c_read_reg8a16(pctx->i2c_slave_address, 0x0380, &reg8);
        pctx->ser_stats[0].is_tunnel_overflow = reg8 & 0x01 ? 1 : 0;

        i2c_read_reg8a16(pctx->i2c_slave_address, 0x038F, &reg8);
        pctx->ser_stats[0].tunnel_pkt_processed = reg8;
    }

    i2c_read_reg8a16(pctx->i2c_slave_address, 0x0112, &reg8);
    pctx->ser_stats[0].tx_fifo_warn_flag        = reg8 & (1 << 4) ? 1 : 0;
    pctx->ser_stats[0].tx_fifo_overflow_flag    = reg8 & (1 << 5) ? 1 : 0;
    pctx->ser_stats[0].tx_pclk_drift_flag       = reg8 & (1 << 6) ? 1 : 0;
    pctx->ser_stats[0].tx_pclk_det_flag         = reg8 & (1 << 7) ? 1 : 0;

    i2c_read_reg8a16(pctx->i2c_slave_address, 0x0025, &reg8);
    pctx->ser_stats[0].global_pkt_count = reg8;
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x0025, 0x00);

    return 0;
}
