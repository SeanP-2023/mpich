/*
 * Copyright (C) by Argonne National Laboratory
 *     See COPYRIGHT in top-level directory
 */

#include "mpitest.h"

#ifdef MULTI_TESTS
#define run attr_keyval_double_free_win
int run(const char *arg);
#endif

#define NUM_WIN 2
#define DATA_SZ sizeof(int)

/* tests multiple invocations of MPI_Win_free_keyval on the same keyval */

static int delete_fn(MPI_Win win, int keyval, void *attr, void *extra)
{
    MPI_Win_free_keyval(&keyval);
    return MPI_SUCCESS;
}

int run(const char *arg)
{
    void *base_ptr[NUM_WIN];
    MPI_Win windows[NUM_WIN];
    int keyval = MPI_KEYVAL_INVALID;
    int keyval_copy = MPI_KEYVAL_INVALID;
    int errs = 0;

    MPI_Alloc_mem(DATA_SZ, MPI_INFO_NULL, &base_ptr[0]);
    MPI_Win_create(base_ptr[0], DATA_SZ, 1, MPI_INFO_NULL, MPI_COMM_WORLD, &windows[0]);
    MPI_Alloc_mem(DATA_SZ, MPI_INFO_NULL, &base_ptr[1]);
    MPI_Win_create(base_ptr[1], DATA_SZ, 1, MPI_INFO_NULL, MPI_COMM_WORLD, &windows[1]);

    MPI_Win_create_keyval(MPI_WIN_NULL_COPY_FN, delete_fn, &keyval, NULL);
    keyval_copy = keyval;

    MPI_Win_set_attr(windows[0], keyval, NULL);
    MPI_Win_set_attr(windows[1], keyval, NULL);

    MPI_Win_free(&windows[0]);  /* first MPI_Win_free_keyval */
    MPI_Free_mem(base_ptr[0]);
    MPI_Win_free_keyval(&keyval);       /* second MPI_Win_free_keyval */
    MPI_Win_free_keyval(&keyval_copy);  /* third MPI_Win_free_keyval */
    MPI_Win_free(&windows[1]);  /* fourth MPI_Win_free_keyval */
    MPI_Free_mem(base_ptr[1]);
    return errs;
}
