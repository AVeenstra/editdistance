/**********************************************************************
Copyright ©2013 Advanced Micro Devices, Inc. All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

	Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
	Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or
 other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
********************************************************************/

// For clarity,error checking has been omitted.

#include <CL/cl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <fstream>

#define SUCCESS 0
#define FAILURE 1

using namespace std;

/* return the maximum of two sizes */
int getMax(size_t a_, size_t b_) {
	int a = static_cast<int>(a_);
	int b = static_cast<int>(b_);
    int c = a - b;
    int k = (c >> 31) & 0x1;
    int max = a - k * c;
    return max;
}

/* convert the kernel file into a string */
int convertToString(const char *filename, std::string& s)
{
	size_t size;
	char*  str;
	std::fstream f(filename, (std::fstream::in | std::fstream::binary));

	if(f.is_open())
	{
		size_t fileSize;
		f.seekg(0, std::fstream::end);
		size = fileSize = (size_t)f.tellg();
		f.seekg(0, std::fstream::beg);
		str = new char[size+1];
		if(!str)
		{
			f.close();
			return 0;
		}

		f.read(str, fileSize);
		f.close();
		str[size] = '\0';
		s = str;
		delete[] str;
		return 0;
	}
	cout<<"Error: failed to open file\n:"<<filename<<endl;
	return FAILURE;
}

int main(int argc, char* argv[])
{

	/*Step1: Getting platforms and choose an available one.*/
	cl_uint numPlatforms;	//the NO. of platforms
	cl_platform_id platform = NULL;	//the chosen platform
	cl_int	status = clGetPlatformIDs(0, NULL, &numPlatforms);
	if (status != CL_SUCCESS)
	{
		cout << "Error: Getting platforms!" << endl;
		return FAILURE;
	}

	/*For clarity, choose the first available platform. */
	if(numPlatforms > 0)
	{
		cl_platform_id* platforms = (cl_platform_id* )malloc(numPlatforms* sizeof(cl_platform_id));
		status = clGetPlatformIDs(numPlatforms, platforms, NULL);
		if(status != CL_SUCCESS) 
   	{ 
        	std::cout << "Error: platform error \n";
        	return 0;
    	}
		platform = platforms[0];
		free(platforms);
	}

	/*Step 2:Query the platform and choose the first GPU device if has one.Otherwise use the CPU as device.*/
	cl_uint				numDevices = 0;
	cl_device_id        *devices;
	status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 0, NULL, &numDevices);
	if (numDevices == 0)	//no GPU available.
	{
		cout << "No GPU device available." << endl;
		cout << "Choose CPU as default device." << endl;
		status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 0, NULL, &numDevices);	
		if(status != CL_SUCCESS) { 
        	std::cout << "Error: device error. \n";
        	return 0;
    	}
		devices = (cl_device_id*)malloc(numDevices * sizeof(cl_device_id));
		status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, numDevices, devices, NULL);
		if(status != CL_SUCCESS) 
		{ 
        	std::cout << "Error: device error2. \n";
        	return 0;
    	}
	}
	else
	{
		devices = (cl_device_id*)malloc(numDevices * sizeof(cl_device_id));
		status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, numDevices, devices, NULL);
		if(status != CL_SUCCESS) 
		{ 
        	std::cout << "Error: device error 3. \n";
        	return 0;
    	}
	}
	

	/*Step 3: Create context.*/
	cl_context context = clCreateContext(NULL,1, devices,NULL,NULL,NULL);
	
	/*Step 4: Creating command queue associate with the context.*/
	cl_command_queue commandQueue = clCreateCommandQueue(context, devices[0], 0, NULL);

	/*Step 5: Create program object */
	const char *filename = "EditDistance_Kernel.cl";
	string sourceStr;
	status = convertToString(filename, sourceStr);
	if(status != CL_SUCCESS) 
   	{ 
        	std::cout << "Error: conversion error. \n";
        	return 0;
    	}
	const char *source = sourceStr.c_str();
	size_t sourceSize[] = {strlen(source)};
	cl_program program = clCreateProgramWithSource(context, 1, &source, sourceSize, NULL);
	
	/*Step 6: Build program. */
	status = clBuildProgram(program, 1,devices,NULL,NULL,NULL);
	if(status != CL_SUCCESS) 
   	{ 
        	std::cout << "Error: Compiling kernel. \n";
		size_t length;
    		char buffer[10000];
   		clGetProgramBuildInfo(program, devices[0], CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &length);
    cout<<"--- Build log ---\n "<<buffer<<endl;
        	return 0;
    	}

	/*Step 7: Initial input,output for the host and create memory objects for the kernel*/
	const char* input1 = "saturday";
	const char* input2 = "sunday";	
	int input4[3] = {1, 1, 1};
	size_t strlength1 = strlen(input1);
	size_t strlength2 = strlen(input2);
	int input3[(strlength1) * (strlength2)];
	size_t matsize = (strlength1)*(strlength2);	
	size_t costSize = 3;
	cout << "input strings:" << endl;
	cout << input1 << endl;
	cout << input2 << endl;
	int *output = (int*) malloc(sizeof(int));	

	cl_mem inputBuffer1 = clCreateBuffer(context, CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR, (strlength1 + 1) * sizeof(char),(void *) input1, NULL);
	cl_mem inputBuffer2 = clCreateBuffer(context, CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR, (strlength2 + 2) * sizeof(char),(void *) input2, NULL);
	cl_mem inputBuffer3 = clCreateBuffer(context, CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR, matsize*(sizeof(cl_int)),(void *) input3, NULL);
	cl_mem inputBuffer4 = clCreateBuffer(context, CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR, costSize*(sizeof(cl_int)),(void *) input4, NULL);
	cout << strlength1 << endl;

	cl_mem inputBuffer5 = clCreateBuffer(context, CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR, sizeof(int),(void *) strlength1, NULL);
	cl_mem inputBuffer6 = clCreateBuffer(context, CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR, sizeof(int),(void *) strlength2, NULL);

	cl_mem outputBuffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY , (1) * sizeof(int), NULL, NULL);

	/*Step 8: Create kernel object */
	cout << "1" << endl;
	cl_kernel kernel = clCreateKernel(program,"editdistance", NULL);
	
	/*Step 9: Sets Kernel arguments.*/
	status = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&inputBuffer1);
	if(status != CL_SUCCESS) 
   	{ 
        	std::cout << "Error: Setting first kernel argument. \n";
        	return 0;
    	}
	status = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&inputBuffer2);
	if(status != CL_SUCCESS) 
   	{ 
        	std::cout << "Error: Setting second kernel argument. \n";
        	return 0;
    	}
	status = clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *)&inputBuffer3);
	if(status != CL_SUCCESS) 
   	{ 
        	std::cout << "Error: Setting third kernel argument. \n";
        	return 0;
    	}
	status = clSetKernelArg(kernel, 3, sizeof(cl_mem), (void *)&inputBuffer4);
	if(status != CL_SUCCESS) 
   	{ 
        	std::cout << "Error: Setting fourth kernel argument. \n";
        	return 0;
    	}
	status = clSetKernelArg(kernel, 4, sizeof(cl_mem), (void *)&inputBuffer5);
	if(status != CL_SUCCESS) 
   	{ 
        	std::cout << "Error: Setting fourth kernel argument. \n";
        	return 0;
    	}
	status = clSetKernelArg(kernel, 5, sizeof(cl_mem), (void *)&inputBuffer6);
	if(status != CL_SUCCESS) 
   	{ 
        	std::cout << "Error: Setting fourth kernel argument. \n";
        	return 0;
    	}
	status = clSetKernelArg(kernel, 6, sizeof(cl_mem), (void *)&outputBuffer);
	if(status != CL_SUCCESS) 
   	{ 
        	std::cout << "Error: Setting kernel output. \n";
        	return 0;
    	}

	/*Step 10: Running the kernel.*/
	size_t global_work_size[1] = {(strlength1 + 1)*(strlength2 + 1) + getMax(strlength1, strlength2)};
	size_t bla[1] = {10};
	status = clEnqueueNDRangeKernel(commandQueue, kernel, 1, NULL, bla, bla, 0, NULL, NULL);

	/*Step 11: Read the cout put back to host memory.*/
	status = clEnqueueReadBuffer(commandQueue, outputBuffer, CL_TRUE, 0, 1 * sizeof(int), output, 0, NULL, NULL);

//status = clEnqueueReadBuffer(commandQueue, outputBuffer, CL_TRUE, 0, strlength * sizeof(char), output, 0, NULL, NULL);

	
	output[sizeof(int)] = '\0';	//Add the terminal character to the end of output.
	//output[0] =0;
	cout << "\nedit distance:" << endl;
	cout << output[0] << endl;
	if (output[0] == 0) {
		cout << "\nsuccess!" << endl;	
	} else {
		cout << "\nfailure" << endl;
	}

	/*Step 12: Clean the resources.*/
	status = clReleaseKernel(kernel);				//Release kernel.
	status = clReleaseProgram(program);				//Release the program object.
	status = clReleaseMemObject(inputBuffer1);		//Release mem object.
	status = clReleaseMemObject(inputBuffer2);		//Release mem object.
	status = clReleaseMemObject(inputBuffer3);
	status = clReleaseMemObject(outputBuffer);
	status = clReleaseCommandQueue(commandQueue);	//Release  Command queue.
	status = clReleaseContext(context);				//Release context.

	if (output != NULL)
	{
		free(output);
		output = NULL;
	}

	if (devices != NULL)
	{
		free(devices);
		devices = NULL;
	}

	std::cout<<"Passed!\n";
	return SUCCESS;
}
