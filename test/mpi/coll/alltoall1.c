/*
 * Copyright (C) by Argonne National Laboratory
 *     See COPYRIGHT in top-level directory
 */

#include "mpitest.h"
#include "mpicolltest.h"

#ifdef MULTI_TESTS
#define run coll_alltoall1
int run(const char *arg);
#endif

/*
static char MTEST_Descrip[] = "";
*/

int run(const char *arg)
{
    int errs = 0;
    int rank, size;
    int minsize = 2, count;
    MPI_Comm comm;
    int *sendbuf, *recvbuf, *p;
    int sendcount, recvcount;
    int i, j;
    MPI_Datatype sendtype, recvtype;

    int is_blocking = 1;

    MTestArgList *head = MTestArgListCreate_arg(arg);
    if (MTestArgListGetInt_with_default(head, "nonblocking", 0)) {
        is_blocking = 0;
    }
    MTestArgListDestroy(head);

    /* The following illustrates the use of the routines to
     * run through a selection of communicators and datatypes.
     * Use subsets of these for tests that do not involve combinations
     * of communicators, datatypes, and counts of datatypes */
    while (MTestGetIntracommGeneral(&comm, minsize, 1)) {
        if (comm == MPI_COMM_NULL)
            continue;

        /* Determine the sender and receiver */
        MPI_Comm_rank(comm, &rank);
        MPI_Comm_size(comm, &size);

        /* printf("Size of comm = %d\n", size); */
        for (count = 1; count < 65000; count = count * 2) {

            /* Create a send buf and a receive buf suitable for testing
             * all to all.  */
            sendcount = count;
            recvcount = count;
            sendbuf = (int *) malloc(count * size * sizeof(int));
            recvbuf = (int *) malloc(count * size * sizeof(int));
            sendtype = MPI_INT;
            recvtype = MPI_INT;

            if (!sendbuf || !recvbuf) {
                errs++;
                fprintf(stderr, "Failed to allocate sendbuf and/or recvbuf\n");
                MPI_Abort(MPI_COMM_WORLD, 1);
            }
            for (i = 0; i < count * size; i++)
                recvbuf[i] = -1;
            p = sendbuf;
            for (j = 0; j < size; j++) {
                for (i = 0; i < count; i++) {
                    *p++ = j * size + rank + i;
                }
            }

            MTest_Alltoall(is_blocking, sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype,
                           comm);

            p = recvbuf;
            for (j = 0; j < size; j++) {
                for (i = 0; i < count; i++) {
                    if (*p != rank * size + j + i) {
                        errs++;
                        if (errs < 10) {
                            fprintf(stderr, "Error with communicator %s and size=%d count=%d\n",
                                    MTestGetIntracommName(), size, count);
                            fprintf(stderr, "recvbuf[%d,%d] = %d, should be %d\n",
                                    j, i, *p, rank * size + j + i);
                        }
                    }
                    p++;
                }
            }

#if MTEST_HAVE_MIN_MPI_VERSION(2,2)
            /* check MPI_IN_PLACE, added in MPI-2.2 */
            p = recvbuf;
            for (j = 0; j < size; j++) {
                for (i = 0; i < count; i++) {
                    *p++ = j * size + rank + i;
                }
            }
            MPI_Alltoall(MPI_IN_PLACE, -1 /*ignored */ , MPI_DATATYPE_NULL /*ignored */ ,
                         recvbuf, recvcount, recvtype, comm);
            p = recvbuf;
            for (j = 0; j < size; j++) {
                for (i = 0; i < count; i++) {
                    if (*p != rank * size + j + i) {
                        errs++;
                        if (errs < 10) {
                            fprintf(stderr,
                                    "Error (MPI_IN_PLACE) with communicator %s and size=%d count=%d\n",
                                    MTestGetIntracommName(), size, count);
                            fprintf(stderr, "recvbuf[%d,%d] = %d, should be %d\n", j, i, *p,
                                    rank * size + j + i);
                        }
                    }
                    p++;
                }
            }
#endif

            free(recvbuf);
            free(sendbuf);
        }
        MTestFreeComm(&comm);
    }

    return errs;
}
