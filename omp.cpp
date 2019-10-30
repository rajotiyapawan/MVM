#include <fstream>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#include <string.h>

using namespace std;

void RandomDataInitialization(double* pMatrix, double* pVector, int ro, int col){
	int i, j;
	srand(clock());
	for(i=0; i<ro; i++){
		pVector[i] = rand()/1000000;//printf("%7.4lf\n",pVector[i] );
		for(j=0; j<col; j++){
			pMatrix[i*col+j] = rand()/1000000;
		}
	}
}

void RowCal(double* pMatrix, double* pVector, double* pResult, int ro, int col){
	#pragma omp parallel for num_threads(ro/2)
	for(int i=0;i<ro;i++){
		#pragma parallel for num_threads(col/2)
		for(int j=0;j<col;j++){
			pResult[j] += pMatrix[i*ro + j]*pVector[i];
		}
	}
}

void ColCal(double* pMatrix, double* pVector, double* pResult, int ro, int col){
	int j, k;
	#pragma omp parallel for num_threads(col/2)
	for(k=0;k<col;k++){
		pResult[k] = 0;
		#pragma omp parallel for num_threads(ro/2)
			for(j=0; j<ro; j++){//printf("while");
				pResult[k] += pMatrix[j*col +k]*pVector[j];
			}
	}
}

void ChCal(double* pMatrix, double* pVector, double* pResult, int ro, int col){
	/*int j, k;
	#pragma omp parallel for
	for(k=0;k<col;k++){
		pResult[k] = 0;
		#pragma omp parallel for num_threads(ro/2)
		for(j=0; j<ro; j++){//printf("while");
			pResult[k] += pMatrix[j*col +k]*pVector[j];
		}
	}*/
	
	if(ro%2==0){int ind =0;
		for(int k=0;k<2;k++){
			#pragma omp parallel for num_threads(ro/4)
			for(int i=ind;i<ind+ro/2;i++){
				#pragma parallel for num_threads(col/2)
				for(int j=0;j<col;j++){
					pResult[j] += pMatrix[i*ro + j]*pVector[i];
				}
			}
			ind = ind+ro/2;
		}
	}
	else{int ind=0;
		for(int k=0;k<2;k++){
			#pragma omp parallel for num_threads(ro/4)
			for(int i=ind;i<=ind+ro/2;i++){
				#pragma parallel for num_threads(col/2)
				for(int j=0;j<col;j++){
					pResult[j] += pMatrix[i*ro + j]*pVector[i];
				}
			}
			ind = ind+ro/2+1;
		}
	}
}

void SerialCal(double* pMatrix, double* pVector, double* pResult, int ro, int col){
	int j, k;
	for(k=0;k<col;k++){
		pResult[k] = 0;
			for(j=0; j<ro; j++){//printf("while");
				pResult[k] += pMatrix[j*col +k]*pVector[j];
			}
	}
}

void PrintMatrix(double* pMatrix, int RowCount, int ColCount){
	int i, j;//printf("%d",RowCount);
	ofstream out("matrix.txt");

	for(i=0; i<RowCount; i++){
		for (j = 0; j < ColCount; j++){
			out << pMatrix[i*ColCount+j]<<" ";
		}out << ("\n");
	}
	out.close();
}

void ResultVector(double* pVector, int ro){
	int i;
	ofstream out("result.txt");

	for(i=0; i<ro; i++){
		out <<pVector[i] <<" ";
	}
	out.close();
}

void PrintVector(double* pVector, int ro){
	int i;
	ofstream out("vector.txt");

	for(i=0; i<ro; i++){
		out <<pVector[i] <<" ";
	}
	out.close();
}

int main(int argc, char* args[])
{



	time_t start, finish;
	double duration, stime;
	double* pMatrix;
	double* pVector;
	double* pResult;
	int ro, col;
	if(argc == 4){
		ro = atoi(args[2]);//printf("%d\n",ro);
		col = atoi(args[3]);//printf("%d\n",col );
	}
	else{
		printf("Please provide row and columns\n");
		return -1;
	}

	printf("OpenMP matrix-vector multiplication program\n");

	pMatrix = new double[ro*col];
	pVector = new double[ro];
	pResult = new double[col];

	//printf("%d",(int) sizeof(pMatrix));

	RandomDataInitialization(pMatrix, pVector, ro, col);

	//printf("%d",Size);


	start = clock();
		SerialCal(pMatrix, pVector, pResult, ro, col);
		finish = clock();
		stime = (finish-start)/CLOCKS_PER_SEC;
		printf("\nTime of Serial Execution:%f\n", duration);


	if(atoi(args[1])==1){
		start = clock();
		RowCal(pMatrix, pVector, pResult, ro, col);
		finish = clock();
		duration = (finish-start)/CLOCKS_PER_SEC;
		printf("\nTime of Row Execution:%f\n", duration);
		printf("\nSpeedup: %f\n",stime/duration);
		PrintMatrix(pMatrix, ro, col);
		ResultVector(pResult, col);
		PrintVector(pVector, ro);
	}
	else if(atoi(args[1])==2){
		start = clock();
		ColCal(pMatrix, pVector, pResult, ro, col);
		finish = clock();
		duration = (finish-start)/CLOCKS_PER_SEC;
		printf("\nTime of Column Execution:%f\n", duration);
		printf("\nSpeedup: %f\n",stime/duration);
		PrintMatrix(pMatrix, ro, col);
		ResultVector(pResult, col);
		PrintVector(pVector, ro);
	}
	else if(atoi(args[1])==3){
		start = clock();
		ChCal(pMatrix, pVector, pResult, ro, col);
		finish = clock();
		duration = (finish-start)/CLOCKS_PER_SEC;
		printf("\nTime of Checkrboard Execution:%f\n", duration);
		printf("\nSpeedup: %f\n",stime/duration);
		PrintMatrix(pMatrix, ro, col);
		ResultVector(pResult, col);
		PrintVector(pVector, ro);
	}
	else if(atoi(args[1])==0){
		/*ofstream out("omp_r.txt");
		for(int i=1; i<=16;i=2*i){
			start = clock();
			RowCal(pMatrix, pVector, pResult, ro, col);
			finish = clock();
			duration = (finish-start)/CLOCKS_PER_SEC;
			out << i <<" "<< stime/duration;
		}
		out.close();
		ofstream out1("omp_c.txt");
		for(int i=1; i<=16;i=2*i){
			start = clock();
			ColCal(pMatrix, pVector, pResult, ro, col);
			finish = clock();
			duration = (finish-start)/CLOCKS_PER_SEC;
			out1 << i <<" "<< stime/duration;
		}
		out1.close();
		ofstream out2("omp_ch.txt");
		for(int i=1; i<=16;i=2*i){
			start = clock();
			ChCal(pMatrix, pVector, pResult, ro, col);
			finish = clock();
			duration = (finish-start)/CLOCKS_PER_SEC;
			out2 << i <<" "<< stime/duration;
		}
		out2.close();*/
	}

	//printf("\nTime of Execution:%f\n", duration);


	

	return 0;
}