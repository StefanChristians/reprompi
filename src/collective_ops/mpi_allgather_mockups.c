/*  ReproMPI Benchmark
 *
 *  Copyright 2015 Alexandra Carpen-Amarie, Sascha Hunold
    Research Group for Parallel Computing
    Faculty of Informatics
    Vienna University of Technology, Austria

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
#include "mpi.h"
#include "collectives.h"


/***************************************/
// MPI_Allgather with Alltoall

inline void execute_GL_Allgather_as_Alltoall(collective_params_t* params) {

#ifdef COMPILE_BENCH_TESTS
    // replicate data in the source buffer nprocs times
    {
        int i;
        for (i=1; i<params->nprocs; i++) {
            memcpy((char*)params->sbuf + i * params->msize * params->datatypesize, (char*)params->sbuf,
                    params->msize * params->datatypesize);
        }
    }
#endif

    MPI_Alltoall(params->sbuf, params->msize, params->datatype,
            params->rbuf, params->msize, params->datatype,
            MPI_COMM_WORLD);
}



void initialize_data_GL_Allgather_as_Alltoall(const basic_collective_params_t info, const long msize, collective_params_t* params) {
    initialize_common_data(info, params);

    params->msize = msize; // size of the buffer for each process

    // source buffer must contain msize elements repeated nprocs times
    params->scount = msize * params->nprocs;
    params->rcount = msize * params->nprocs;

    params->sbuf = (char*) malloc(params->scount * params->datatypesize);
    params->rbuf = (char*) malloc(params->rcount * params->datatypesize);

}


void cleanup_data_GL_Allgather_as_Alltoall(collective_params_t* params) {
    free(params->sbuf);
    free(params->rbuf);
    params->sbuf = NULL;
    params->rbuf = NULL;
}
/***************************************/



/***************************************/
// MPI_Allgather with Allreduce

inline void execute_GL_Allgather_as_Allreduce(collective_params_t* params) {

#ifdef COMPILE_BENCH_TESTS
    memcpy((char*)params->tmp_buf + params->rank * params->scount * params->datatypesize, (char*)params->sbuf,
            params->scount * params->datatypesize);
#endif

    MPI_Allreduce(params->tmp_buf, params->rbuf, params->rcount, params->datatype,
            params->op, MPI_COMM_WORLD);

}


void initialize_data_GL_Allgather_as_Allreduce(const basic_collective_params_t info, const long msize, collective_params_t* params) {
    int num_ints, num_adds, num_dtypes, combiner;
    int i;

    initialize_common_data(info, params);

    params->msize = msize;

    params->scount = msize;
    params->rcount = msize * params->nprocs;

    params->sbuf = (char*) malloc(params->scount * params->datatypesize);
    params->rbuf = (char*) malloc(params->rcount * params->datatypesize);
    params->tmp_buf = (char*) malloc(params->rcount * params->datatypesize);

    // set identity operand for different operations
    if (params->op == MPI_BAND || params->op == MPI_PROD) {
        MPI_Type_get_envelope(params->datatype,
                &num_ints, &num_adds, &num_dtypes, &combiner);

        if (combiner == MPI_COMBINER_NAMED) {
            if (params->datatype == MPI_INT) {
                for (i=0; i<params->rcount; i++) {
                    ((int*)params->tmp_buf)[i] = 1;
                }
            }
            else {
                if (params->datatype == MPI_DOUBLE) {
                    for (i=0; i<params->rcount; i++) {
                        ((double*)params->tmp_buf)[i] = 1;
                    }
                }
                else {
                    memset( params->tmp_buf, 0xFF, params->rcount * params->datatypesize);
                }
            }
        }
        else {
            memset( params->tmp_buf, 0xFF, params->rcount * params->datatypesize);
        }
    }
    else {
        memset(params->tmp_buf, 0, params->rcount * params->datatypesize);
    }
}


void cleanup_data_GL_Allgather_as_Allreduce(collective_params_t* params) {
    free(params->sbuf);
    free(params->rbuf);
    free(params->tmp_buf);
    params->sbuf = NULL;
    params->rbuf = NULL;
    params->tmp_buf = NULL;

}
/***************************************/




/***************************************/
// MPI_Allgather with Gather + Bcast

inline void execute_GL_Allgather_as_GatherBcast(collective_params_t* params) {
    MPI_Gather(params->sbuf, params->msize, params->datatype,
            params->rbuf, params->msize, params->datatype,
            params->root, MPI_COMM_WORLD);

    MPI_Bcast(params->rbuf, params->rcount, params->datatype,
            params->root, MPI_COMM_WORLD);
}


void initialize_data_GL_Allgather_as_GatherBcast(const basic_collective_params_t info, const long msize, collective_params_t* params) {
    initialize_common_data(info, params);

    params->msize = msize;

    params->scount = msize;
    params->rcount = msize * params->nprocs;

    params->sbuf = (char*) malloc(params->scount * params->datatypesize);
    params->rbuf = (char*) malloc(params->rcount * params->datatypesize);

}


void cleanup_data_GL_Allgather_as_GatherBcast(collective_params_t* params) {
    free(params->sbuf);
    free(params->rbuf);
    params->sbuf = NULL;
    params->rbuf = NULL;

}
/***************************************/


