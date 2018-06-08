#include <iostream>
#include "utils.h"
#include <mpi.h>
#include <fstream>

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

const char* get_error_string_opencl(cl_int error) {
	switch (error) {
		// run-time and JIT compiler errors
	case 0:
		return "CL_SUCCESS";
	case 1:
		return "CL_DEVICE_NOT_FOUND";
	case 2:
		return "CL_DEVICE_NOT_AVAILABLE";
	case 3:
		return "CL_COMPILER_NOT_AVAILABLE";
	case 4:
		return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
	case 5:
		return "CL_OUT_OF_RESOURCES";
	case 6:
		return "CL_OUT_OF_HOST_MEMORY";
	case 7:
		return "CL_PROFILING_INFO_NOT_AVAILABLE";
	case 8:
		return "CL_MEM_COPY_OVERLAP";
	case 9:
		return "CL_IMAGE_FORMAT_MISMATCH";
	case 10:
		return "CL_IMAGE_FORMAT_NOT_SUPPORTED";
	case 11:
		return "CL_BUILD_PROGRAM_FAILURE";
	case 12:
		return "CL_MAP_FAILURE";
	case 13:
		return "CL_MISALIGNED_SUB_BUFFER_OFFSET";
	case 14:
		return "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";
	case 15:
		return "CL_COMPILE_PROGRAM_FAILURE";
	case 16:
		return "CL_LINKER_NOT_AVAILABLE";
	case 17:
		return "CL_LINK_PROGRAM_FAILURE";
	case 18:
		return "CL_DEVICE_PARTITION_FAILED";
	case 19:
		return "CL_KERNEL_ARG_INFO_NOT_AVAILABLE";

		// compile-time errors
	case 30:
		return "CL_INVALID_VALUE";
	case 31:
		return "CL_INVALID_DEVICE_TYPE";
	case 32:
		return "CL_INVALID_PLATFORM";
	case 33:
		return "CL_INVALID_DEVICE";
	case 34:
		return "CL_INVALID_CONTEXT";
	case 35:
		return "CL_INVALID_QUEUE_PROPERTIES";
	case 36:
		return "CL_INVALID_COMMAND_QUEUE";
	case 37:
		return "CL_INVALID_HOST_PTR";
	case 38:
		return "CL_INVALID_MEM_OBJECT";
	case 39:
		return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
	case 40:
		return "CL_INVALID_IMAGE_SIZE";
	case 41:
		return "CL_INVALID_SAMPLER";
	case 42:
		return "CL_INVALID_BINARY";
	case 43:
		return "CL_INVALID_BUILD_OPTIONS";
	case 44:
		return "CL_INVALID_PROGRAM";
	case 45:
		return "CL_INVALID_PROGRAM_EXECUTABLE";
	case 46:
		return "CL_INVALID_KERNEL_NAME";
	case 47:
		return "CL_INVALID_KERNEL_DEFINITION";
	case 48:
		return "CL_INVALID_KERNEL";
	case 49:
		return "CL_INVALID_ARG_INDEX";
	case 50:
		return "CL_INVALID_ARG_VALUE";
	case 51:
		return "CL_INVALID_ARG_SIZE";
	case 52:
		return "CL_INVALID_KERNEL_ARGS";
	case 53:
		return "CL_INVALID_WORK_DIMENSION";
	case 54:
		return "CL_INVALID_WORK_GROUP_SIZE";
	case 55:
		return "CL_INVALID_WORK_ITEM_SIZE";
	case 56:
		return "CL_INVALID_GLOBAL_OFFSET";
	case 57:
		return "CL_INVALID_EVENT_WAIT_LIST";
	case 58:
		return "CL_INVALID_EVENT";
	case 59:
		return "CL_INVALID_OPERATION";
	case 60:
		return "CL_INVALID_GL_OBJECT";
	case 61:
		return "CL_INVALID_BUFFER_SIZE";
	case 62:
		return "CL_INVALID_MIP_LEVEL";
	case 63:
		return "CL_INVALID_GLOBAL_WORK_SIZE";
	case 64:
		return "CL_INVALID_PROPERTY";
	case 65:
		return "CL_INVALID_IMAGE_DESCRIPTOR";
	case 66:
		return "CL_INVALID_COMPILER_OPTIONS";
	case 67:
		return "CL_INVALID_LINKER_OPTIONS";
	case 68:
		return "CL_INVALID_DEVICE_PARTITION_COUNT";

		// extension errors
	case 1000:
		return "CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR";
	case 1001:
		return "CL_PLATFORM_NOT_FOUND_KHR";
	case 1002:
		return "CL_INVALID_D3D10_DEVICE_KHR";
	case 1003:
		return "CL_INVALID_D3D10_RESOURCE_KHR";
	case 1004:
		return "CL_D3D10_RESOURCE_ALREADY_ACQUIRED_KHR";
	case 1005:
		return "CL_D3D10_RESOURCE_NOT_ACQUIRED_KHR";
	default:
		return "Unknown OpenCL error";
	}
}

const char* get_error_string_mpi(int error) {
	char* result = nullptr;
	if (MPI_ERR_ARG == MPI_Error_string(error, result, nullptr)) {
		return "Unknown MPI error";
	}
	return result;
}

cl_int check_success_opencl(cl_int result) {
	if (result != CL_SUCCESS) {
		printf("Unfortunately an OpenCL error occured:\n%d\t%s\n", -result, get_error_string_opencl(-result));

		std::cout << "The program has finished" << std::endl << "Press enter to quit the program" << std::endl;
        MPI_Abort(MPI_COMM_WORLD, (-result) & 255);
		exit((-result) & 255);
	}
	return result;
}

int check_success_mpi(int result) {
	if (result != MPI_SUCCESS) {
		printf("Unfortunately an MPI error occured:\n%d\t%s\n", result, get_error_string_mpi(result));

        std::cout << "The program has finished" << std::endl << "Press enter to quit the program" << std::endl;
        MPI_Abort(MPI_COMM_WORLD, result & 255);
		exit(result & 255);
	}
	return result;
}

std::ifstream *open_file(char* filename) {
    std::ifstream *stream = new std::ifstream(filename, std::ios::ate | std::ios::binary);

	if (!stream->is_open()) {
		fprintf(stderr, "Could not find \"%s\".\n", filename);

        std::cout << "The program has finished" << std::endl << "Press enter to quit the program" << std::endl;
        MPI_Abort(MPI_COMM_WORLD, 1);
		exit(1);
	}
	return stream;
}

String *load_file(char* filename) {
	String *result = new String();
	auto stream = open_file(filename);
	result->size = (size_t) stream->tellg();
	stream->seekg(0, std::ios::beg);
	result->text = new char[result->size+1];
	result->text[result->size] = 0;
	stream->read(result->text, result->size);
	stream->close();
	delete stream;
	return result;
}

String *load_file_part(char* filename, int start, size_t size) {
	String *result = new String();
	auto stream = open_file(filename);
	stream->seekg(start, std::ios::beg);
	result->text = new char[size];
	stream->read(result->text, size);
	result->size = (size_t) stream->gcount();
	stream->close();
	delete stream;
	return result;
}

cl_uint get_file_size(char* filename) {
	auto stream = open_file(filename);
	cl_uint result = (cl_uint) stream->tellg();
	stream->close();
	return result;
}

bool arg_equal(char* arg, char* value) {
	do {
		if (arg[0] != value[0]) {
			return false;
		}
		arg++;
		value++;
	} while (arg[0] != 0);
	return arg[0] == value[0];
}

bool send_complete(MPI_Request *request) {
    int result = 0;
    if (request != nullptr) {
        check_success_mpi(MPI_Test(request, &result, MPI_STATUS_IGNORE));
    }
    return result != 0;
}
