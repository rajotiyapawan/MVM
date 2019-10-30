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

void RandomDataInitialization(double* pMatrix, double* pVector, int ro, int col){
	int i, j;
	srand(clock());
	for(i=0; i<ro; i++){//printf("hello\n");
		pVector[i] = rand()/1000000;//printf("pVector\n");
		for(j=0; j<col; j++){
			pMatrix[i*col+j] = rand()/1000000;//printf("%7.4f\n",pMatrix[i*Size+j]);
		}
	}
}

void ProcessInitialization(double* &pMatrix, double* &pVector, double* &pResult, double* &pProcRows, double* &pProcResult, int &ro, int &col, int &RowNum){
	int RestRows;
	int i;
	/*setvbuf(stdout, 0, _IONBF, 0);
	if(ProcRank==0){
		do{
			printf("\nEnter the size of the matrix and vector: ");
			scanf("%d", &Size);//printf("%d\n", Size);
			if(Size < ProcNum){
				printf("Size of the objects must be greater than the processes!\n");
			}
		}while(Size<ProcNum);
	}*/
	MPI_Bcast(&ro, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&col, 1, MPI_INT, 0, MPI_COMM_WORLD);

	RestRows = ro;
	for(i=0; i<ProcRank; i++){
		RestRows = RestRows - RestRows/(ProcNum-i);
	}
		RowNum = RestRows/(ProcNum-ProcRank);
		//}
		pVector = new double[ro];
		pResult = new double[col];
		pProcRows = new double[RowNum*col];//printf("Rownum is %d\n",RowNum);
		pProcResult = new double[RowNum];

	//}

		if(ProcRank==0){
			pMatrix = new double[ro*col];
			RandomDataInitialization(pMatrix, pVector, ro, col);
		}
	//}

}

void DataDistribution(double* pMatrix, double* pProcRows, double* pVector, int ro, int col, int RowNum){
	int *pSendNum;
	int *pSendInd;
	int RestRows=ro;
	MPI_Bcast(pVector, ro, MPI_DOUBLE, 0, MPI_COMM_WORLD);

	pSendInd = new int [ProcNum];
	pSendNum = new int [ProcNum];

	RowNum = (ro/ProcNum);
	pSendNum[0] = RowNum*col;//printf("%d\n",pSendNum[0]);
	pSendInd[0] = 0;

	for(int i=1; i<ProcNum; i++){//printf("done\n");
		RestRows -= RowNum;
		RowNum = RestRows/(ProcNum-i);
		pSendNum[i] = RowNum*col;//printf("\nhello %d\n",pSendNum[i]);
		pSendInd[i] = pSendInd[i-1]+pSendNum[i-1];//printf("\nhello\n");
	}

	//printf("hello hello %ld\n",sizeof(pProcRows));

	MPI_Scatterv(pMatrix, pSendNum, pSendInd, MPI_DOUBLE, pProcRows, pSendNum[ProcRank], MPI_DOUBLE, 0, MPI_COMM_WORLD);
	//printf("data Distributed\n");


	delete [] pSendNum;
	delete [] pSendInd;
}

void ResultReplication(double* pProcResult, double* pResult, int ro, int col, int RowNum){
	int *pReceiveNum;
	int *pReceiveInd;

	int RestRows = ro;
	int i;

	pReceiveNum = new int [ProcNum];
	pReceiveInd = new int [ProcNum];

	pReceiveInd[0] = 0;
	pReceiveNum[0] = ro/ProcNum;

	for(i=1; i<ProcNum; i++){
		RestRows -= pReceiveNum[i-1];
		pReceiveNum[i] = RestRows/(ProcNum-i);
		pReceiveInd[i] = pReceiveInd[i-1]+pReceiveNum[i-1];
	}

	MPI_Allgatherv(pProcResult, pReceiveNum[ProcRank],MPI_DOUBLE, pResult, pReceiveNum, pReceiveInd,MPI_DOUBLE,MPI_COMM_WORLD);

	delete [] pReceiveInd;
	delete [] pReceiveNum;
}

void SerailResultCalculation(double* pMatrix, double* pVector, double* pResult, int ro, int col){
	int i, j;
	for(i=0; i<col;i++){
		pResult[i]=0;
		for(j=0;j<ro;j++){
			pResult[i] += pMatrix[j*col+i]*pVector[j];
		}
	}
}

void ParallelResultCalculation(double* pProcRows, double* pVector, double* pProcResult, int ro, int col, int RowNum){
	int i, j;
	for(i=0; i<RowNum; i++){
		pProcResult[i] = 0;
		for(j=0;j<col;j++){
			pProcResult[i] += pProcRows[j*col +i]*pVector[j];
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

void TestResult(double* pMatrix, double* pVector, double* pResult, int ro, int col){
	double* pSerialResult;
	int equal = 0;
	int i;
	if(ProcRank==0){
		pSerialResult = new double[col];
		pSerialResult = pResult;
		SerailResultCalculation(pMatrix, pVector, pSerialResult, ro, col);
		for(i=0; i<ro; i++){
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

int main(int argc, char *args[])
{
	int RowNum, Size, ro, col;
	time_t start, finish;
	double duration;

	double* pMatrix;
	double* pVector;
	double* pResult;
	double* pProcRows;
	double* pProcResult;

	if(ProcRank==0){
		if(argc == 3){
			ro = atoi(args[1]);//printf("%d\n",ro);
			col = atoi(args[2]);//printf("%d\n",col );
		}
		else{
			printf("Please provide row and columns\n");
			return -1;
		}
	}

	MPI_Init(&argc, &args);
	MPI_Comm_size(MPI_COMM_WORLD, &ProcNum);
	MPI_Comm_rank(MPI_COMM_WORLD, &ProcRank);

	if(ProcRank==0){
		printf("Parallel matrix-vector multiplication program\n");
	}

	ProcessInitialization(pMatrix, pVector, pResult, pProcRows, pProcResult, ro, col, RowNum);

	start = MPI_Wtime();

	DataDistribution(pMatrix, pProcRows, pVector, ro, col, RowNum);

	ParallelResultCalculation(pProcRows, pVector, pProcResult, ro, col, RowNum);

	ResultReplication(pProcResult, pResult, ro, col, RowNum);

	finish = MPI_Wtime();

	duration = finish-start;

	TestResult(pMatrix, pVector, pResult, ro, col);

	if(ProcRank==0){
		printf("Time of Execution = %f\n", duration);
	}

	ProcessTermination(pMatrix, pVector, pResult, pProcRows, pProcResult);

	MPI_Finalize();
}