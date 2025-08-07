/**
 * @file   args.c
 * @author DAB-Embedded
 * @date   21 Nov 2024
 * @brief  Parsing command line source.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include "serdes_head.h"

st_app_params app_params;

// Function to display usage information
void display_usage(const char *prog_name) {
    printf("Usage: %s [options]\n", prog_name);
    printf("Options:\n");
    printf("  -i, --i2c <busnumber>   Specify the i2c bus number [default 4]\n");
    printf("  -m, --maprx <hex>       Specify MIPI RX pins mapping for serializer\n");
    printf("  -p, --polarityrx <hex>  Specify MIPI RX pins polarity for serializer\n");
    printf("  -l, --lanesrx <val>     Specify MIPI RX lanes for serializer [default 4]\n");
    printf("  -k, --maptx <hex>       Specify MIPI TX pins mapping for deserializer\n");
    printf("  -o, --polaritytx <hex>  Specify MIPI TX pins polarity for deserializer\n");
    printf("  -r, --lanestx <val>     Specify MIPI TX lanes for serializer [default 4]\n");
    printf("  -t, --ratetx <val>      Specify MIPI TX transfer rate\n");
    printf("  -s, --stats             Statistics only\n");
    printf("  -n, --noinit            Without init\n");
    printf("  -a, --ssa <address>     Specify serializer I2C slave address [default - autoscan]\n");
    printf("  -b, --dsa <address>     Specify deserializer I2C slave address [default - autoscan]\n");
    printf("  -h, --help              Show this help message and exit\n");
}

int application_opt_parsing(int argc, char *argv[]) {
    int opt;

    // Define long options
    static struct option long_options[] = {
        {"i2c",         required_argument,  0, 'i'},
        {"maprx",       required_argument,  0, 'm'},
        {"polarityrx",  required_argument,  0, 'p'},
        {"lanesrx",     required_argument,  0, 'l'},
        {"maptx",       required_argument,  0, 'k'},
        {"polaritytx",  required_argument,  0, 'o'},
        {"lanestx",     required_argument,  0, 'r'},
        {"ratetx",      required_argument,  0, 't'},
        {"stats",       no_argument,        0, 's'},
        {"noinit",      no_argument,        0, 'n'},
        {"ssa",         required_argument,  0, 'a'},
        {"dsa",         required_argument,  0, 'b'},
        {"help",        no_argument,        0, 'h'},
        {0, 0, 0, 0}  // Terminator
    };

    memset(&app_params, 0, sizeof(app_params));
    app_params.i2c_port          = 4;
    app_params.mipi_rx_lanes     = 4;
    app_params.mipi_rx_map       = 0x400E;
    app_params.mipi_rx_pol       = 0x3402;
    app_params.mipi_tx_lanes     = 4;
    app_params.mipi_tx_map       = 0xE4;
    app_params.mipi_tx_pol       = 0x00;
    app_params.mipi_tx_out_freq  = 1500;

    while ((opt = getopt_long(argc, argv, "a:b:i:m:p:l:k:o:r:t:hsn", long_options, NULL)) != -1) {
        // String in optarg
        switch (opt) {
            case 'i':
                app_params.i2c_port = atol(optarg);
                break;
            case 'm':
                app_params.mipi_rx_map = strtoul(optarg, NULL, 16);
                break;
            case 'p':
                app_params.mipi_rx_pol = strtoul(optarg, NULL, 16);
                break;
            case 'l':
                app_params.mipi_rx_lanes = strtoul(optarg, NULL, 16);
                break;
            case 'k':
                app_params.mipi_tx_map = strtoul(optarg, NULL, 16);
                break;
            case 'o':
                app_params.mipi_tx_pol = strtoul(optarg, NULL, 16);
                break;
            case 'r':
                app_params.mipi_tx_lanes = atol(optarg);
                break;
            case 't':
                app_params.mipi_tx_out_freq = atol(optarg);
                break;
            case 's':
                app_params.stats_flags = 1;
                break;
            case 'n':
                app_params.no_init = 1;
                break;
            case 'a':
                app_params.ser_i2c_sa = strtoul(optarg, NULL, 16);
                break;
            case 'b':
                app_params.deser_i2c_sa = strtoul(optarg, NULL, 16);
                break;
            case 'h':
                display_usage(argv[0]);
                return 1;
            default: // '?' for unknown options
                display_usage(argv[0]);
                return 1;
        }
    }

    // Display the parsed options
    printf("I2C bus: %d\n", app_params.i2c_port);

    printf("Serializer MIPI lanes: %d\r\n", app_params.mipi_rx_lanes);

    // Handle remaining arguments (non-option arguments)
    if (optind < argc) {
        printf("Non-option arguments:\n");
        while (optind < argc) {
            printf("  %s\n", argv[optind++]);
        }
    }

    return 0;
}
