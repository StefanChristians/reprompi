/*  ReproMPI Benchmark
 *
 *  Copyright 2015 Alexandra Carpen-Amarie, Sascha Hunold
    Research Group for Parallel Computing
    Faculty of Informatics
    Vienna University of Technology, Austria
 *
 * Copyright (c) 2021 Stefan Christians
 *
<license>
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
</license>
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include "mpi.h"

#include "buf_manager/mem_allocation.h"
#include "collectives.h"

#include "contrib/intercommunication/intercommunication.h"


/***************************************/
// MPI_Allreduce with Reduce + Bcast

inline void execute_GL_Allreduce_as_ReduceBcast(collective_params_t* params) {
    MPI_Reduce(params->sbuf, params->rbuf, params->scount, params->datatype,
            params->op, params->root, params->communicator);
    MPI_Bcast(params->rbuf, params->rcount, params->datatype,
            params->root, params->communicator);
}



void initialize_data_GL_Allreduce_as_ReduceBcast(const basic_collective_params_t info, const long count, collective_params_t* params) {
    initialize_common_data(info, params);

    params->count = count;

    params->scount = count;
    params->rcount = count;

    assert (params->scount < INT_MAX);
    assert (params->rcount < INT_MAX);

    params->sbuf = (char*)reprompi_calloc(params->scount, params->datatype_extent);
    params->rbuf = (char*)reprompi_calloc(params->rcount, params->datatype_extent);

}


void cleanup_data_GL_Allreduce_as_ReduceBcast(collective_params_t* params) {
    free(params->sbuf);
    free(params->rbuf);
    params->sbuf = NULL;
    params->rbuf = NULL;

}
/***************************************/


/***************************************/
// MPI_Allreduce with Reduce_scatter and Allgather
inline void execute_GL_Allreduce_as_ReducescatterAllgather(collective_params_t* params) {

    MPI_Reduce_scatter(params->sbuf, params->tmp_buf, params->counts_array,
            params->datatype, params->op, params->communicator);

    MPI_Allgather(params->tmp_buf, params->count, params->datatype,
            params->rbuf, params->count, params->datatype,
            params->communicator);

}


void initialize_data_GL_Allreduce_as_ReducescatterAllgather(const basic_collective_params_t info,
        const long count, collective_params_t* params) {
    int i;

    initialize_common_data(info, params);

    params->count = count / params->remote_size; // block size per process

    params->scount = count;
    params->rcount = count;

    assert (params->scount < INT_MAX);
    assert (params->rcount < INT_MAX);

    // we send the same number of elements to all processes
    params->counts_array = (int*)reprompi_calloc(params->remote_size, sizeof(int));
    for (i=0; i< params->remote_size; i++) {
        params->counts_array[i] = params->count;
    }

    params->sbuf = (char*)reprompi_calloc(params->scount, params->datatype_extent);
    params->rbuf = (char*)reprompi_calloc(params->rcount, params->datatype_extent);
    params->tmp_buf = (char*)reprompi_calloc(params->scount, params->datatype_extent);

}


void cleanup_data_GL_Allreduce_as_ReducescatterAllgather(collective_params_t* params) {
    free(params->sbuf);
    free(params->rbuf);
    free(params->tmp_buf);
    free(params->counts_array);
    params->sbuf = NULL;
    params->rbuf = NULL;
    params->tmp_buf = NULL;
    params->counts_array = NULL;

}
/***************************************/



/***************************************/
// MPI_Allreduce with Reduce_scatterblock and Allgather
inline void execute_GL_Allreduce_as_ReducescatterblockAllgather(collective_params_t* params) {


    MPI_Reduce_scatter_block(params->sbuf, params->tmp_buf, params->count,
            params->datatype, params->op, params->communicator);

    MPI_Allgather(params->tmp_buf, params->count, params->datatype,
            params->rbuf, params->count, params->datatype,
            params->communicator);

}


void initialize_data_GL_Allreduce_as_ReducescatterblockAllgather(const basic_collective_params_t info,
        const long count, collective_params_t* params) {
    initialize_common_data(info, params);


    params->count = count / params->remote_size; // block size per process

    params->scount = count;
    params->rcount = count;

    assert (params->scount < INT_MAX);
    assert (params->rcount < INT_MAX);

    params->sbuf = (char*)reprompi_calloc(params->scount, params->datatype_extent);
    params->rbuf = (char*)reprompi_calloc(params->rcount, params->datatype_extent);
    params->tmp_buf = (char*)reprompi_calloc(params->scount, params->datatype_extent);

}


void cleanup_data_GL_Allreduce_as_ReducescatterblockAllgather(collective_params_t* params) {
    free(params->sbuf);
    free(params->rbuf);
    free(params->tmp_buf);
    params->sbuf = NULL;
    params->rbuf = NULL;
    params->tmp_buf = NULL;

}
/***************************************/


/***************************************/
// MPI_Allreduce with Reduce_scatter and Allgatherv
inline void execute_GL_Allreduce_as_ReducescatterAllgatherv(collective_params_t* params) {

    MPI_Reduce_scatter(params->sbuf, params->tmp_buf, params->counts_array,
            params->datatype, params->op, params->communicator);

    MPI_Allgatherv(params->tmp_buf, params->count, params->datatype,
            params->rbuf, params->counts_array, params->displ_array, params->datatype,
            params->communicator);

}


void initialize_data_GL_Allreduce_as_ReducescatterAllgatherv(const basic_collective_params_t info,
        const long count, collective_params_t* params) {
    int i;

    initialize_common_data(info, params);

    // set block size per process according to rank
    if (params->rank < count % params->remote_size) {
        params->count = count / params->remote_size + 1;
    }
    else {
        params->count = count / params->remote_size;
    }

    // total count for the initial message and the final result
    params->scount = count;
    params->rcount = count;

    assert (params->scount < INT_MAX);
    assert (params->rcount < INT_MAX);

    // each process receives a different number of elements according to its rank
    params->counts_array = (int*)reprompi_calloc(params->remote_size, sizeof(int));
    params->displ_array = (int*)reprompi_calloc(params->remote_size, sizeof(int));

    for (i=0; i< params->remote_size; i++) {
        if (i < count % params->remote_size) {
            params->counts_array[i] = count / params->remote_size + 1;
        }
        else {
            params->counts_array[i] = count / params->remote_size;
        }


        if (i==0) {
            params->displ_array[0] = 0;
        }
        else {
            params->displ_array[i] = params->displ_array[i-1] + params->counts_array[i-1];
        }
    }


    // source and destination buffers have the same size - the total amount of reduced data
    params->sbuf = (char*)reprompi_calloc(params->scount, params->datatype_extent);
    params->rbuf = (char*)reprompi_calloc(params->rcount, params->datatype_extent);

    // initialize with the message size communicated to the current process
    params->tmp_buf = (char*)reprompi_calloc(params->count, params->datatype_extent);

}


void cleanup_data_GL_Allreduce_as_ReducescatterAllgatherv(collective_params_t* params) {
    free(params->sbuf);
    free(params->rbuf);
    free(params->tmp_buf);
    free(params->counts_array);
    params->sbuf = NULL;
    params->rbuf = NULL;
    params->tmp_buf = NULL;
    params->counts_array = NULL;

}
/***************************************/
