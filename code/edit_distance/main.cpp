// https://stackoverflow.com/questions/24326432/convenient-way-to-show-opencl-error-codes

#include <iostream>
#include <fstream>
#include "utils.h"
#include <mpi.h>
#include <algorithm>
#include <iterator>
#include <chrono>
#include <queue>

#ifdef __APPLE__

#include <OpenCL/opencl.h>

#else

#include <CL/cl.h>

#endif

#define KERNEL_FILE "kernel.cl"
#define MEMORY_OBJECTS_SIZE 3
#define SEQUENCE_A_FILENAME "sequences/Homo_sapiens.GRCh38.dna.chromosome.1.M.txt"
#define SEQUENCE_B_FILENAME "sequences/Homo_sapiens.GRCh38.dna.chromosome.2.M.txt"
#define ROWS_PER_ITERATION 15

void init_opencl(
        cl_program *program,
        cl_context *context,
        cl_command_queue *command_queue,
        cl_uint *max_group_size,
        cl_uint *max_compute_units,
        cl_kernel *kernel_main,
        cl_kernel *kernel_fill_row,
        cl_kernel *kernel_fill_defaults_fill_row,
        cl_kernel *kernel_fill_defaults_main,
        int group_id
) {
    cl_int result;
    cl_device_id device_ids[group_id + 1];

    // Load kernel file
    auto kernel_source = load_file((char *) KERNEL_FILE);

    // Load (the first) platform.
    cl_platform_id platform_id = nullptr;
    check_success_opencl(clGetPlatformIDs(1, &platform_id, nullptr));

    // Load (the first) device
    cl_uint num_devices = 0;
    check_success_opencl(clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_ALL, (cl_uint) group_id + 1, device_ids, &num_devices));

    if (num_devices < group_id + 1) {
        std::cout << "Not enough OpenCL devices found!" << std::endl;
        std::cout << "The program has finished" << std::endl << "Press enter to quit the program" << std::endl;
        MPI_Abort(MPI_COMM_WORLD, 1);
        exit(1);
    }

    auto device_id = device_ids[group_id];

    // Get device information
    size_t temp = 0;
    check_success_opencl(clGetDeviceInfo(device_id, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(size_t), &temp, NULL));
    *max_group_size = cl_uint(temp);
    check_success_opencl(clGetDeviceInfo(device_id, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cl_uint), max_compute_units, nullptr));

    // Create a context
    *context = clCreateContext(nullptr, 1, &device_id, nullptr, nullptr, &result);
    check_success_opencl(result);

    // Create command queue for the device
    *command_queue = clCreateCommandQueue(*context, device_id, 0, &result);
    check_success_opencl(result);

    // Create kernel program from the source
    *program = clCreateProgramWithSource(*context, 1, const_cast<const char **>(&kernel_source->text), &kernel_source->size, &result);
    check_success_opencl(result);


    // Build kernel program
    result = clBuildProgram(*program, 1, &device_id, nullptr, nullptr, nullptr);

    // Get the build log
    size_t log_size;
    check_success_opencl(clGetProgramBuildInfo(*program, device_id, CL_PROGRAM_BUILD_LOG, 0, nullptr, &log_size));
    if (2 < log_size) {
        auto log = new char[log_size];
        check_success_opencl(clGetProgramBuildInfo(*program, device_id, CL_PROGRAM_BUILD_LOG, log_size, log, nullptr));
        std::cout << "The following warnings where encountered while compiling the OpenCL kernel:" << std::endl << log << std::endl;
        std::cout << "<-- end -->" << std::endl << std::endl;
        delete[] log;
    }

    // Check if the build was succesfull
    check_success_opencl(result);

    delete[] kernel_source->text;
    delete kernel_source;

    // Create kernels
    *kernel_main = clCreateKernel(*program, "calc_diagonals", &result);
    check_success_opencl(result);
    *kernel_fill_row = clCreateKernel(*program, "fill_row", &result);
    check_success_opencl(result);
    *kernel_fill_defaults_fill_row = clCreateKernel(*program, "fill_defaults_fill_row", &result);
    check_success_opencl(result);
    *kernel_fill_defaults_main = clCreateKernel(*program, "fill_defaults_calc_diagonals", &result);
    check_success_opencl(result);
}

void init_MPI(
        int *argc,
        char ***argv,
        int *cluster_size,
        int *cluster_id,
        int *group_id
) {
    MPI_Init(argc, argv);
    check_success_mpi(MPI_Comm_size(MPI_COMM_WORLD, cluster_size));
    check_success_mpi(MPI_Comm_rank(MPI_COMM_WORLD, cluster_id));

    char buff[MPI_MAX_PROCESSOR_NAME];
    int bufflen;
    MPI_Get_processor_name(buff, &bufflen);

    unsigned int hash = 37;
    for (int i = 0; i < bufflen; i++) {
        hash = ((hash << 3) + hash) + buff[i] - 'a';
    }

    MPI_Comm group;
    MPI_Comm_split(MPI_COMM_WORLD, *cluster_id, *cluster_id, &group);
    MPI_Comm_rank(group, group_id);
}

void get_cluster_info(
        int cluster_size,
        int cluster_id,
        cl_uint max_group_size,
        cl_uint *cluster_total_max_group_size,
        cl_uint *cluster_min_max_group_size,
        cl_uint *starting_column
) {
    MPI_Allreduce(&max_group_size, cluster_total_max_group_size, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
    MPI_Allreduce(&max_group_size, cluster_min_max_group_size, 1, MPI_INT, MPI_MIN, MPI_COMM_WORLD);

    if (cluster_id == 0) {
        if (1 < cluster_size) {
            check_success_mpi(MPI_Send(&max_group_size, 1, MPI_UNSIGNED, 1, 0, MPI_COMM_WORLD));
        }
    } else {
        check_success_mpi(MPI_Recv(starting_column, 1, MPI_UNSIGNED, cluster_id - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE));
        if (cluster_id < cluster_size - 1) {
            auto temp = *starting_column + max_group_size;
            check_success_mpi(MPI_Send(&temp, 1, MPI_UNSIGNED, cluster_id + 1, 0, MPI_COMM_WORLD));
        }
    }
}

void init_opencl_mem(
        cl_context context,
        cl_command_queue command_queue,
        cl_mem *memory_objects,
        int cluster_size,
        cl_uint max_group_size,
        cl_uint *row_height,
        cl_mem **row_sequences,
        char *sequence_b_filename,
        cl_uint *sequence_b_total_size,
        cl_uint *rows_size
) {
    cl_int result;
    memory_objects[0] = clCreateBuffer(context, CL_MEM_READ_ONLY, max_group_size, nullptr, &result);
    check_success_opencl(result);

    *sequence_b_total_size = get_file_size(sequence_b_filename);
    if (1 < cluster_size && *sequence_b_total_size < *row_height * 2) {
        *row_height = *sequence_b_total_size / 2 + 1;
        if (*row_height < max_group_size * 2) {
            std::cout << "Sequence b not big enough!" << std::endl;
            MPI_Abort(MPI_COMM_WORLD, 1);
            exit(1);
        }
    }
    *rows_size = *sequence_b_total_size / *row_height + 1;

    memory_objects[1] = clCreateBuffer(context, CL_MEM_READ_ONLY, max_group_size + *rows_size * *row_height, nullptr, &result);
    check_success_opencl(result);

    auto stream = open_file(sequence_b_filename);
    stream->seekg(0, std::ios::beg);
    char buffer[*row_height];
    *row_sequences = new cl_mem[*rows_size];
    for (int i = 0; i < *rows_size; i++) {
        stream->read(buffer, *row_height);
        check_success_opencl(clEnqueueWriteBuffer(command_queue, memory_objects[1], CL_TRUE, i * *row_height, (size_t) stream->gcount(), buffer, 0, nullptr, nullptr));

        cl_buffer_region temp;
        temp.origin = i * *row_height;
        temp.size = *row_height + max_group_size;
        (*row_sequences)[i] = clCreateSubBuffer(memory_objects[1], CL_MEM_READ_ONLY, CL_BUFFER_CREATE_TYPE_REGION, &temp, &result);
        check_success_opencl(result);
    }
    stream->close();
    delete stream;

    memory_objects[2] = clCreateBuffer(context, CL_MEM_READ_WRITE, (*row_height + max_group_size) * 2 * sizeof(cl_uint), nullptr, &result);
    check_success_opencl(result);
}

void execute_kernel(
        cl_kernel kernel,
        cl_mem *memory_objects,
        cl_command_queue command_queue,
        cl_uint row_height,
        cl_uint max_group_size,
        cl_uint offset_a,
        cl_uint offset_b,
        String *sequence_a,
        cl_mem sequence_b,
        cl_uint *column_current,
        cl_uint *column_next,
        cl_uint height
) {

    if (offset_b != 0) {
        // copy diagonal from last run
        auto diagonal_size = (sequence_a->size * 2 - 1) * sizeof(cl_uint);
        check_success_opencl(clEnqueueCopyBuffer(command_queue, memory_objects[2], memory_objects[2], row_height * sizeof(cl_uint), 0, diagonal_size, 0, nullptr, nullptr));

        if (offset_a != 0) {
            // copy column data
            auto column_size = (row_height - sequence_a->size + 2) * sizeof(cl_uint);
            check_success_opencl(clEnqueueWriteBuffer(command_queue, memory_objects[2], CL_FALSE, diagonal_size, column_size, &column_current[sequence_a->size - 2], 0, nullptr, nullptr));
            if (column_next != nullptr) {
                check_success_opencl(clEnqueueWriteBuffer(command_queue, memory_objects[2], CL_FALSE, diagonal_size + column_size, (sequence_a->size + 1) * sizeof(cl_uint), column_next, 0, nullptr, nullptr));
            }
        }
    } else if (offset_a != 0) {
        // copy column data
        check_success_opencl(clEnqueueWriteBuffer(command_queue, memory_objects[2], CL_FALSE, (sequence_a->size + 1) * sizeof(cl_uint), row_height * sizeof(cl_uint), column_current, 0, nullptr, nullptr));
        check_success_opencl(clEnqueueWriteBuffer(command_queue, memory_objects[2], CL_FALSE, (sequence_a->size + 1 + row_height) * sizeof(cl_uint), row_height * sizeof(cl_uint), column_next, 0, nullptr, nullptr));
    }

    // Set kernel arguments
    check_success_opencl(clSetKernelArg(kernel, 0, sizeof(cl_mem), static_cast<void *>(&memory_objects[0])));
    check_success_opencl(clSetKernelArg(kernel, 1, sizeof(cl_mem), static_cast<void *>(&sequence_b)));
    check_success_opencl(clSetKernelArg(kernel, 2, sizeof(cl_uint), static_cast<void *>(&height)));
    check_success_opencl(clSetKernelArg(kernel, 3, sizeof(cl_uint), static_cast<void *>(&offset_a)));
    check_success_opencl(clSetKernelArg(kernel, 4, sizeof(cl_uint), static_cast<void *>(&offset_b)));
    check_success_opencl(clSetKernelArg(kernel, 5, sizeof(cl_mem), static_cast<void *>(&memory_objects[2])));

    size_t work_size[1] = {sequence_a->size};

    check_success_opencl(clEnqueueNDRangeKernel(command_queue, kernel, 1, nullptr, work_size, work_size, 0, nullptr, nullptr));

    // Copy results from the memory buffer
    check_success_opencl(clEnqueueReadBuffer(command_queue, memory_objects[2], CL_TRUE, sizeof(cl_uint), row_height * sizeof(cl_uint), column_current, 0, nullptr, nullptr));
}

void receive_column(Buffer *buffer, cl_uint row_height, int cluster_size, int cluster_id) {
    auto temp = new cl_uint[row_height];
    check_success_mpi(MPI_Recv(temp, row_height, MPI_UNSIGNED, (cluster_id + cluster_size - 1) % cluster_size, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE));
    buffer->put(temp);
}

int main(int argc, char **argv) {
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    char *sequence_a_filename = (char *) SEQUENCE_A_FILENAME;
    char *sequence_b_filename = (char *) SEQUENCE_B_FILENAME;
    char *seperator = (char *) "--";

    if (3 < argc && arg_equal(argv[argc - 3], seperator)) {
        sequence_a_filename = argv[argc - 2];
        sequence_b_filename = argv[argc - 1];
        argc -= 3;
    } else {
        for (int i = 1; i < argc; i++) {
            std::cout << argv[i] << std::endl;
        }
    }

    cl_context context = nullptr;
    cl_command_queue command_queue = nullptr;
    cl_mem memory_objects[MEMORY_OBJECTS_SIZE];
    cl_program program = nullptr;
    cl_kernel kernel_main, kernel_fill_row, kernel_fill_defaults_fill_row, kernel_fill_defaults_main, kernel_start, kernel_continue;
    cl_uint max_group_size, max_compute_units, cluster_min_max_group_size, cluster_total_max_group_size;
    cl_uint starting_column = 0, *column = nullptr, sequence_b_total_size, rows_size;
    int cluster_size, cluster_id, group_id = 0;

    // init MPI
    init_MPI(&argc, &argv, &cluster_size, &cluster_id, &group_id);

    // init opencl
    init_opencl(&program, &context, &command_queue, &max_group_size, &max_compute_units, &kernel_main, &kernel_fill_row, &kernel_fill_defaults_fill_row, &kernel_fill_defaults_main, group_id);

    // communicate cluster info
    get_cluster_info(cluster_size, cluster_id, max_group_size, &cluster_total_max_group_size, &cluster_min_max_group_size, &starting_column);

    // Memory allocation and retrieving some info
    cl_uint row_height = cluster_min_max_group_size * ROWS_PER_ITERATION;
    auto sequence_a_total_size = get_file_size(sequence_a_filename);

    cl_mem *row_sequences = nullptr;

    // init the memeory on the OpenCL device
    init_opencl_mem(context, command_queue, memory_objects, cluster_size, max_group_size, &row_height, &row_sequences, sequence_b_filename, &sequence_b_total_size, &rows_size);

    Buffer buffer;

    // setting correct kernels
    if (cluster_id == 0) {
        kernel_start = kernel_fill_defaults_fill_row;
        kernel_continue = kernel_fill_defaults_main;
    } else {
        kernel_start = kernel_fill_row;
        kernel_continue = kernel_main;
    }

    std::queue<MPI_Request *> request_queue;
    std::queue<cl_uint *> column_queue;

    for (auto c = starting_column; c < sequence_a_total_size; c = c + cluster_total_max_group_size) {
        auto sequence_a = load_file_part(sequence_a_filename, c, max_group_size);
        check_success_opencl(clEnqueueWriteBuffer(command_queue, memory_objects[0], CL_FALSE, 0, sequence_a->size, sequence_a->text, 0, nullptr, nullptr));
        auto kernel = kernel_start;

        if (1 < cluster_size && c != 0) {
            receive_column(&buffer, row_height, cluster_size, cluster_id);
        }

        for (cl_uint r = 0; r < rows_size; r++) {

            // get column
            if (1 < cluster_size && c != 0 && 1 < rows_size - r) {
                receive_column(&buffer, row_height, cluster_size, cluster_id);
            }

            if (c != 0) {
                column = buffer.pop(cluster_id);
            } else {
                column = new cl_uint[row_height];
            }

            auto height = std::min(row_height, sequence_b_total_size - r * row_height);

            execute_kernel(
                    kernel, memory_objects, command_queue, row_height, max_group_size, c, row_height * r, sequence_a, row_sequences[r], column, buffer.peek(row_height), height
            );

            if (c + max_group_size < sequence_a_total_size) {
                if (1 < cluster_size) {
                    MPI_Request *request = new MPI_Request;
                    check_success_mpi(MPI_Isend(column, row_height, MPI_UNSIGNED, (cluster_id + 1) % cluster_size, 1, MPI_COMM_WORLD, request));
                    column_queue.push(column);
                    request_queue.push(request);

                    while (!request_queue.empty() && send_complete(request_queue.front())) {
                        delete request_queue.front();
                        request_queue.pop();
                        delete[] column_queue.front();
                        column_queue.pop();
                    }
                } else {
                    buffer.put(column);
                }
            } else {
                if (rows_size - r == 1) {
                    std::cout << "Result: " << column[sequence_b_total_size - (rows_size - 1) * row_height - 1] << std::endl;
                }
                delete[] column;
            }

            kernel = kernel_continue;
        }

        delete[] sequence_a->text;
        delete sequence_a;
        kernel_start = kernel_fill_row;
        kernel_continue = kernel_main;
    }

    // Finalization
//    check_success_opencl(clFlush(command_queue));
//    check_success_opencl(clFinish(command_queue));
    check_success_opencl(clReleaseKernel(kernel_main));
    check_success_opencl(clReleaseKernel(kernel_fill_row));
    check_success_opencl(clReleaseKernel(kernel_fill_defaults_fill_row));
    check_success_opencl(clReleaseProgram(program));
    for (auto i = 0; i < MEMORY_OBJECTS_SIZE; i++) {
        check_success_opencl(clReleaseMemObject(memory_objects[i]));
    }
//    check_success_opencl(clReleaseCommandQueue(command_queue));
    check_success_opencl(clReleaseContext(context));

    delete[] row_sequences;

    MPI_Barrier(MPI_COMM_WORLD);

    while (!request_queue.empty()) {
        delete request_queue.front();
        request_queue.pop();
        delete[] column_queue.front();
        column_queue.pop();
    }

    MPI_Finalize();

    if (cluster_id == 0) {
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        std::cout << "Time elapsed (seconds) = "
                  << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() / 1000000.0
                  << std::endl;
    }
    return 0;
}
