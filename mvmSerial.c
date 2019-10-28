#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void DummyDataInitialization(double* pMatrix, double* pVector, int Size){
	int i, j; //printf("%d\n",Size);
	for(i=0; i<Size; i++){
		pVector[i] = 1;//printf("%d\n",Size);
		for(j=0; j<Size; j++){
			pMatrix[i*Size +j] = i;//printf("%7.4f ",pMatrix[i*Size +j]);
		}
	}
}

/*void RandomDataInitialization(double* pMatrix, double* pVector, int Size){
	int i, j;
	srand(clock());
	for(i=0; i<Size; i++){
		pVector[i] = rand()/1000;
		for(j=0; j<Size; j++){
			pMatrix[i*Size+j] = rand()/1000;
		}
	}
}

void ProcessInitialization(double* pMatrix, double* pVector, double* pResult, int Size){

	pMatrix = (double *)malloc(Size*Size*sizeof(double));
	pVector = (double *)malloc(Size*sizeof(double));
	pResult = (double *)malloc(Size*sizeof(double));

	//DummyDataInitialization(pMatrix, pVector, Size);

	int i, j; //printf("%d\n",Size);
	for(i=0; i<Size; i++){
		pVector[i] = 1;printf("%d\n",Size);
		for(j=0; j<Size; j++){
			pMatrix[i*Size +j] = i;printf("%7.4f ",pMatrix[i*Size +j]);
		}
	}
}*/

void ResultCalculation(double* pMatrix, double* pVector, double* pResult, int Size){
	int j, k;
	for(k=0;k<Size;k++){
		pResult[k] = 0;
			for(j=0; j<Size; j++){//printf("while");
				pResult[k] += pMatrix[j*Size +k]*pVector[j];
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

int main(int argc, char const *argv[])
{
	int Size;
	time_t start, finish;
	double duration;

	printf("Serial matrix-vector multiplication program\n");

	while(Size<=0){
		printf("\nEnter the size of the initial objects: ");
		scanf("%d", &Size);
		//printf("\nChosen objects size = %d\n", Size);
		if(Size <=0){
			printf("\nSize of objects must be greater than 0!\n");
		}
	}

	double* pMatrix = (double *)malloc(Size*Size*sizeof(double));
	double* pVector = (double *)malloc(Size*sizeof(double));
	double* pResult = (double *)malloc(Size*sizeof(double));

	//printf("%d",(int) sizeof(pMatrix));

	DummyDataInitialization(pMatrix, pVector, Size);

	//printf("%d",Size);

	//PrintMatrix(pMatrix, Size, Size);

	start = clock();
	ResultCalculation(pMatrix, pVector, pResult, Size);
	finish = clock();
	duration = (finish-start)/CLOCKS_PER_SEC;

	printf("\nTime of Execution:%f\n", duration);

	//PrintVector(pResult, Size);

	return 0;
}