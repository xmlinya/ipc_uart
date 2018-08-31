/*
 * Copyright (c) 2013-2015 Texas Instruments Incorporated - http://www.ti.com
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ======== main_host.c ========
 *
 */

/* cstdlib header files */
#include <stdio.h>
#include <stdlib.h>

/* package header files */
#include <ti/ipc/Std.h>
#include <ti/ipc/Ipc.h>
#include <ti/ipc/transports/TransportRpmsg.h>

#include <ti/ipc/MultiProc.h>

/* local header files */
#include "App.h"

/* private functions */
static Int Main_main(Void);
static Int Main_parseArgs(Int argc, Char *argv[]);


#define Main_USAGE "\
Usage:\n\
    app_host [options] procName\n\
\n\
Arguments:\n\
    procName      : the name of the remote processor\n\
\n\
Options:\n\
    h   : print this help message\n\
    l   : list the available remote names\n\
\n\
Examples:\n\
    app_host DSP\n\
    app_host -l\n\
    app_host -h\n\
\n"

/* private data */
static String   Main_remoteProcName = NULL;


/*
 *  ======== main ========
 */
Int main(Int argc, Char* argv[])
{
    Int status;

    printf("--> main:\n");

    /* configure the transport factory */
    Ipc_transportConfig(&TransportRpmsg_Factory);

    /* parse command line */
    status = Main_parseArgs(argc, argv);

    if (status < 0) {
        goto leave;
    }

    /* Ipc initialization */
    status = Ipc_start();

    if (status >= 0) {
        /* application create, exec, delete */
        status = Main_main();

        /* Ipc finalization */
        Ipc_stop();
    }
    else {
        printf("Ipc_start failed: status = %d\n", status);
        goto leave;
    }

leave:
    printf("<-- main:\n");
    status = (status >= 0 ? 0 : status);

    return (status);
}


/*
 *  ======== Main_main ========
 */
Int Main_main(Void)
{
    UInt16      remoteProcId;
    Int         status = 0;

    printf("--> Main_main:\n");

    remoteProcId = MultiProc_getId(Main_remoteProcName);

    /* application create phase */
    status = App_create(remoteProcId);

    if (status < 0) {
        goto leave;
    }

    /* application execute phase */
    status = App_exec();

    if (status < 0) {
        goto leave;
    }

    /* application delete phase */
    status = App_delete();

    if (status < 0) {
        goto leave;
    }

leave:
    printf("<-- Main_main:\n");

    status = (status >= 0 ? 0 : status);
    return (status);
}


/*
 *  ======== Main_parseArgs ========
 */
Int Main_parseArgs(Int argc, Char *argv[])
{
    Int             x, cp, opt, argNum;
    UInt16          i, numProcs;
    String          name;
    Int             status = 0;


    /* parse the command line options */
    for (opt = 1; (opt < argc) && (argv[opt][0] == '-'); opt++) {
        for (x = 0, cp = 1; argv[opt][cp] != '\0'; cp++) {
            x = (x << 8) | (int)argv[opt][cp];
        }

        switch (x) {
            case 'h': /* -h */
                printf("%s", Main_USAGE);
                exit(0);
                break;

            case 'l': /* -l */
                printf("Processor List\n");
                status = Ipc_start();
                if (status >= 0) {
                    numProcs = MultiProc_getNumProcessors();
                    for (i = 0; i < numProcs; i++) {
                        name = MultiProc_getName(i);
                        printf("    procId=%d, procName=%s\n", i, name);
                    }
                    Ipc_stop();
                }
                else {
                    printf(
                        "Error: %s, line %d: Ipc_start failed\n",
                        __FILE__, __LINE__);
                    goto leave;
                }
                exit(0);
                break;

            default:
                printf(
                    "Error: %s, line %d: invalid option, %c\n",
                    __FILE__, __LINE__, (Char)x);
                printf("%s", Main_USAGE);
                status = -1;
                goto leave;
        }
    }

    /* parse the command line arguments */
    for (argNum = 1; opt < argc; argNum++, opt++) {

        switch (argNum) {
            case 1: /* name of proc #1 */
                Main_remoteProcName = argv[opt];
                break;

            default:
                printf(
                    "Error: %s, line %d: too many arguments\n",
                    __FILE__, __LINE__);
                printf("%s", Main_USAGE);
                status = -1;
                goto leave;
        }
    }

    /* validate command line arguments */
    if (Main_remoteProcName == NULL) {
        printf("Error: missing procName argument\n");
        printf("%s", Main_USAGE);
        status = -1;
        goto leave;
    }

leave:
    return(status);
}
