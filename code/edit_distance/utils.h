#include <iostream>
#include <mpi.h>

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

struct String {
	size_t size;
	char* text;
};

struct BufferElement {
	cl_uint* data;
	BufferElement* next = nullptr;
};

class Buffer {
	BufferElement* first = nullptr;
	BufferElement* last = nullptr;
public:
	cl_uint* pop(int cluster_id) {
        if (first == nullptr) {
            std::cout << cluster_id << ": Pop without element?" << std::endl;
            exit(0);
        }
		cl_uint* data = first->data;
        if (first->next == nullptr) {
            first = nullptr;
            last = nullptr;
        } else {
            first = first->next;
        }
		return data;
	}

    cl_uint* peek(cl_uint size) {
        if (first != nullptr) {
           return first->data;
        } else {
            return nullptr;
        }
    }

	void put(cl_uint* data) {
		BufferElement *temp = new BufferElement();
		temp->data = data;
		if (last != nullptr) {
			last->next = temp;
		}
		else {
			first = temp;
		}
		last = temp;
	}
};

std::ifstream *open_file(char* filename);
cl_int check_success_opencl(cl_int result);
int check_success_mpi(int result);
String *load_file(char* filename);
String *load_file_part(char* filename, int start, size_t size);
cl_uint get_file_size(char* filename);
bool arg_equal(char* arg, char* value);
bool send_complete(MPI_Request *request);
