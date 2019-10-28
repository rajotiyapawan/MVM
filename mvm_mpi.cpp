#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

int ProcNum = 0;
int ProcRank = 0;

void DummyDataInitialization(double* pMatrix, double* pVector, int rows, int cols){
	int i, j; //printf("%d\n",Size);
	for(i=0; i<rows; i++){
		pVector[i] = 1;//printf("%d\n",Size);
		for(j=0; j<cols; j++){
			pMatrix[i*cols +j] = i;//printf("%7.4f ",pMatrix[i*Size +j]);
		}
	}
}

void RandomDataInitialization(double* pMatrix, double* pVector, int Size){
	int i, j;
	srand(clock());
	for(i=0; i<Size; i++){//printf("hello\n");
		pVector[i] = rand()/1000;//printf("pVector\n");
		for(j=0; j<Size; j++){
			pMatrix[i*Size+j] = rand()/1000;//printf("%7.4f\n",pMatrix[i*Size+j]);
		}
	}
}

void ProcessInitialization(double* &pMatrix, double* &pVector, double* &pResult, double* &pProcRows, double* &pProcResult, int &Size, int &RowNum){
	int RestRows;
	int i;
	setvbuf(stdout, 0, _IONBF, 0);
	if(ProcRank==0){
		do{
			printf("\nEnter the size of the matrix and vector: ");
			scanf("%d", &Size);//printf("%d\n", Size);
			if(Size < ProcNum){
				printf("Size of the objects must be greater than the processes!\n");
			}
		}while(Size<ProcNum);
	}
	MPI_Bcast(&Size, 1, MPI_INT, 0, MPI_COMM_WORLD);
	RestRows = Size;
	for(i=0; i<ProcRank; i++){
		RestRows = RestRows - RestRows/(ProcNum-i);
	}
		RowNum = RestRows/(ProcNum-ProcRank);
		//}
		pVector = new double[Size];
		pResult = new double[Size];
		pProcRows = new double[RowNum*Size];//printf("Rownum is %d\n",RowNum);
		pProcResult = new double[RowNum];

	//}

		if(ProcRank==0){
			pMatrix = new double[Size*Size];
			RandomDataInitialization(pMatrix, pVector, Size);
		}
	//}

}

void DataDistribution(double* pMatrix, double* pProcRows, double* pVector, int Size, int RowNum){
	int *pSendNum;
	int *pSendInd;
	int RestRows=Size;
	MPI_Bcast(pVector, Size, MPI_DOUBLE, 0, MPI_COMM_WORLD);

	pSendInd = new int [ProcNum];
	pSendNum = new int [ProcNum];

	RowNum = (Size/ProcNum);
	pSendNum[0] = RowNum*Size;//printf("%d\n",pSendNum[0]);
	pSendInd[0] = 0;

	for(int i=1; i<ProcNum; i++){//printf("done\n");
		RestRows -= RowNum;
		RowNum = RestRows/(ProcNum-i);
		pSendNum[i] = RowNum*Size;//printf("\nhello %d\n",pSendNum[i]);
		pSendInd[i] = pSendInd[i-1]+pSendNum[i-1];//printf("\nhello\n");
	}

	//printf("hello hello %ld\n",sizeof(pProcRows));

	MPI_Scatterv(pMatrix, pSendNum, pSendInd, MPI_DOUBLE, pProcRows, pSendNum[ProcRank], MPI_DOUBLE, 0, MPI_COMM_WORLD);
	//printf("data Distributed\n");


	delete [] pSendNum;
	delete [] pSendInd;
}

void ResultReplication(double* pProcResult, double* pResult, int Size, int RowNum){
	int *pReceiveNum;
	int *pReceiveInd;

	int RestRows = Size;
	int i;

	pReceiveNum = new int [ProcNum];
	pReceiveInd = new int [ProcNum];

	pReceiveInd[0] = 0;
	pReceiveNum[0] = Size/ProcNum;

	for(i=1; i<ProcNum; i++){
		RestRows -= pReceiveNum[i-1];
		pReceiveNum[i] = RestRows/(ProcNum-i);
		pReceiveInd[i] = pReceiveInd[i-1]+pReceiveNum[i-1];
	}

	MPI_Allgatherv(pProcResult, pReceiveNum[ProcRank],MPI_DOUBLE, pResult, pReceiveNum, pReceiveInd,MPI_DOUBLE,MPI_COMM_WORLD);

	delete [] pReceiveInd;
	delete [] pReceiveNum;
}

void SerailResultCalculation(double* pMatrix, double* pVector, double* pResult, int Size){
	int i, j;
	for(i=0; i<Size;i++){
		pResult[i]=0;
		for(j=0;j<Size;j++){
			pResult[i] += pMatrix[i*Size+j]*pVector[j];
		}
	}
}

void ParallelResultCalculation(double* pProcRows, double* pVector, double* pProcResult, int Size, int RowNum){
	int i, j;
	for(i=0; i<RowNum; i++){
		pProcResult[i] = 0;
		for(j=0;j<Size;j++){
			pProcResult[i] += pProcRows[i*Size +j]*pVector[j];
		}
	}
}

void PrintMatrix(double* pMatrix, int RowCount, int ColCount){
	int i, j;//printf("%d",RowCount);
	for(i=0; i<RowCount; i++){
		for (j = 0; i < ColCount; j++){
			printf("%7.4f ", pMatrix[i*RowCount+j]);
		}
	}
}

void PrintVector(double* pVector, int Size){
	int i;
	for(i=0; i<Size; i++){
		printf("%7.4f ", pVector[i]);
	}
}

void TestDistribution(double* pMatrix, double* pVector, double* pProcRows, int Size, int RowNum){
	if(ProcRank==0){
		printf("Initial Matrix: \n");
		PrintMatrix(pMatrix, Size, Size);
		printf("Initial vector:\n");
		PrintVector(pVector, Size);
	}
	MPI_Barrier(MPI_COMM_WORLD);
	for(int i=0;i<ProcNum;i++){
		if(ProcRank==i){
			printf("Matrix Stripe:\n");
			PrintMatrix(pProcRows, RowNum, Size);
			printf("Vector:\n");
			PrintVector(pVector, Size);
		}
	}
	MPI_Barrier(MPI_COMM_WORLD);
}

void TestResult(double* pMatrix, double* pVector, double* pResult, int Size){
	double* pSerialResult;
	int equal = 0;
	int i;
	if(ProcRank==0){
		pSerialResult = new double[Size];
		SerailResultCalculation(pMatrix, pVector, pSerialResult, Size);
		for(i=0; i<Size; i++){
			if(pResult[i]!=pSerialResult[i]){
				equal=1;
			}
		}
		if(equal==1){printf("Results are not equal\n");}
		else{printf("Results are identical\n");}
	}
}

void ProcessTermination(double* pMatrix,double* pVector, double* pResult, double* pProcRows, double* pProcResult){
	if(ProcRank==0){
		delete [] pMatrix;
		delete [] pVector;
		delete [] pResult;
		delete [] pProcResult;
		delete [] pProcRows;
	}
}

int main(int argc, char *argv[])
{
	int RowNum, Size;
	time_t start, finish;
	double duration;

	double* pMatrix;
	double* pVector;
	double* pResult;
	double* pProcRows;
	double* pProcResult;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &ProcNum);
	MPI_Comm_rank(MPI_COMM_WORLD, &ProcRank);

	if(ProcRank==0){
		printf("Parallel matrix-vector multiplication program\n");
	}

	ProcessInitialization(pMatrix, pVector, pResult, pProcRows, pProcResult, Size, RowNum);

	start = MPI_Wtime();

	DataDistribution(pMatrix, pProcRows, pVector, Size, RowNum);

	ParallelResultCalculation(pProcRows, pVector, pProcResult, Size, RowNum);

	ResultReplication(pProcResult, pResult, Size, RowNum);

	finish = MPI_Wtime();

	duration = finish-start;

	TestResult(pMatrix, pVector, pResult, Size);

	if(ProcRank==0){
		printf("Time of Execution = %f\n", duration);
	}

	ProcessTermination(pMatrix, pVector, pResult, pProcRows, pProcResult);

	MPI_Finalize();
}