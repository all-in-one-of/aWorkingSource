#include <iostream>
#include <math.h>
// Kernel definition
__global__
void VecAdd(float* A, float* B, float* C,int* msg)
{
    int i = threadIdx.x;
    C[i] = A[i] + B[i];
    *msg = i;
}

int main()
{
	int N = 256;
	float *A = new float[N];
	float *B = new float[N];
	float *C = new float[N];
	int *msg;
	cudaMallocManaged(&A, N*sizeof(float));
	cudaMallocManaged(&B, N*sizeof(float));
	cudaMallocManaged(&C, N*sizeof(float));
	cudaMallocManaged(&msg, sizeof(int));
	for(int i=0;i<N;i++)
	{
		A[i] = 0.1;
		B[i] = 0.1;
	}

	// Kernel invocation with N threads
	int blockSize = 256;
	VecAdd<<<1, blockSize>>>(A, B, C, msg);

	// Wait for GPU to finish before accessing on host
	cudaDeviceSynchronize();

	std::cout << *msg << std::endl;

	cudaFree(A);
	cudaFree(B);
	cudaFree(C);
	cudaFree(msg);
}
