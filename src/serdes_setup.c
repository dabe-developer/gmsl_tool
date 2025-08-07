/**
 * @file   serdes_setup.c
 * @author DAB-Embedded
 * @date   25 Nov 2024
 * @brief  Application entry.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "serdes_head.h"
#include "version.h"

int main(int argc, char *argv[]) {
    int ret = EXIT_SUCCESS;

    printf("GMSL link setup application ver %d.%d (%s %s)\r\n",
            APP_VERSION_MAJOR, APP_VERSION_MINOR, __DATE__, __TIME__);

    if (application_opt_parsing(argc, argv) != 0)
    {
        return EXIT_FAILURE;
    }

    i2c_init(app_params.i2c_port);

    if (deserializer_init() < 0)
    {
        printf("Error: Deserializer not found!\r\n");
        ret = EXIT_FAILURE;
        goto app_close_routine;
    }

    if (app_params.no_init == 0)
    {
        if (deserializer_search_for_serializer() < 0)
        {
            printf("Error: Serializer not found!\r\n");
            ret = EXIT_FAILURE;
            goto app_close_routine;
        }
    }

    serializer_search_chip();

    if (app_params.stats_flags != 0)
    {
        serializer_get_stat();
        deserializer_get_stat();
        goto app_close_routine;
    }

    serializer_init(deserializer_get_features());

    deserializer_start();

app_close_routine:
    i2c_exit();

    return ret;
}
