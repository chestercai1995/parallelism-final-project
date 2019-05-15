#include "shm.h"
#include "sched.h"


#define BSIZE1 2 
#define BSIZE2 16
#define BSIZE3 32


float** A;
float** B;
float** C;
int M = 256;
int N = 256;
int P = 256;


void matmul_aware() {
  int i, j, k;
  int ii, jj, kk;
  int iii, jjj, kkk;
  int iiii=0, jjjj=0, kkkk=0;
  float sum;
  int kb;
  int kkb = 0;
  int kkkb = 0;
  int kkkkb = 0;
  for(i = 0; i < M; i += BSIZE3){
    for(j = 0; j < N; j += BSIZE3){
      if(!kb){
      for(k = 0; k < P; k += BSIZE3){
        //first block
        for(ii = i; ii < (i + BSIZE3); ii += BSIZE2){
          for(jj = j; jj < (j + BSIZE3); jj += BSIZE2){
            if(!kkb){
            for(kk = k; kk < (k + BSIZE3); kk += BSIZE2){
              //second block
              for(iii = ii; iii < (ii + BSIZE2); iii += BSIZE1){
                for(jjj = jj; jjj < (jj + BSIZE2); jjj += BSIZE1){
                  if(!kkkb){
                  for(kkk = kk; kkk < (kk + BSIZE2); kkk += BSIZE1){
                    //third block
            			  for(iiii = iii; iiii < iii + BSIZE1; iiii++){
                        for(jjjj = jjj; jjjj < jjj + BSIZE1; jjjj++){
            			        sum = 0;
                          if(!kkkkb){
            			          for(kkkk = kkk; kkkk < kkk + BSIZE1; kkkk++){
            			            sum += A[iiii][kkkk] * B[jjjj][kkkk];
            			          }
                            kkkkb = 1;
                          }
                          else{
            			          for(kkkk = kkk + BSIZE1 - 1; kkkk >=  kkk; kkkk--){
            			            sum += A[iiii][kkkk] * B[jjjj][kkkk];
            			          }
                            kkkkb = 0;
                          }
            			        C[iiii][jjjj] += sum;
            			      }
            			  }
                  }
                  kkkb = 1;
                  }
                  else{
                  for(kkk = kk + BSIZE2 - BSIZE1; kkk >= kk; kkk -= BSIZE1){
                    //third block
            			  for(iiii = iii; iiii < iii + BSIZE1; iiii++){
                        for(jjjj = jjj; jjjj < jjj + BSIZE1; jjjj++){
            			        sum = 0;
                          if(!kkkkb){
            			          for(kkkk = kkk; kkkk < kkk + BSIZE1; kkkk++){
            			            sum += A[iiii][kkkk] * B[jjjj][kkkk];
            			          }
                            kkkkb = 1;
                          }
                          else{
            			          for(kkkk = kkk + BSIZE1 - 1; kkkk >=  kkk; kkkk--){
            			            sum += A[iiii][kkkk] * B[jjjj][kkkk];
            			          }
                            kkkkb = 0;
                          }
            			        C[iiii][jjjj] += sum;
            			      }
            			  }
                  }
                  kkkb = 0;
                  }
                }
              }
            }
            kkb = 1;
            }
            else{
            for(kk = k + BSIZE3 - BSIZE2; kk >= k; kk -= BSIZE2){
              //second block
              for(iii = ii; iii < (ii + BSIZE2); iii += BSIZE1){
                for(jjj = jj; jjj < (jj + BSIZE2); jjj += BSIZE1){
                  if(!kkkb){
                  for(kkk = kk; kkk < (kk + BSIZE2); kkk += BSIZE1){
                    //third block
            			  for(iiii = iii; iiii < iii + BSIZE1; iiii++){
                        for(jjjj = jjj; jjjj < jjj + BSIZE1; jjjj++){
            			        sum = 0;
                          if(!kkkkb){
            			          for(kkkk = kkk; kkkk < kkk + BSIZE1; kkkk++){
            			            sum += A[iiii][kkkk] * B[jjjj][kkkk];
            			          }
                            kkkkb = 1;
                          }
                          else{
            			          for(kkkk = kkk + BSIZE1 - 1; kkkk >=  kkk; kkkk--){
            			            sum += A[iiii][kkkk] * B[jjjj][kkkk];
            			          }
                            kkkkb = 0;
                          }
            			        C[iiii][jjjj] += sum;
            			      }
            			  }
                  }
                  kkkb = 1;
                  }
                  else{
                  for(kkk = kk + BSIZE2 - BSIZE1; kkk >= kk; kkk -= BSIZE1){
                    //third block
            			  for(iiii = iii; iiii < iii + BSIZE1; iiii++){
                        for(jjjj = jjj; jjjj < jjj + BSIZE1; jjjj++){
            			        sum = 0;
                          if(!kkkkb){
            			          for(kkkk = kkk; kkkk < kkk + BSIZE1; kkkk++){
            			            sum += A[iiii][kkkk] * B[jjjj][kkkk];
            			          }
                            kkkkb = 1;
                          }
                          else{
            			          for(kkkk = kkk + BSIZE1 - 1; kkkk >=  kkk; kkkk--){
            			            sum += A[iiii][kkkk] * B[jjjj][kkkk];
            			          }
                            kkkkb = 0;
                          }
            			        C[iiii][jjjj] += sum;
            			      }
            			  }
                  }
                  kkkb = 0;
                  }
                }
              }
            }
            kkb = 0;
            }
          }
        }
      }
      kb = 1;
      }
      else{
      for(k = P - BSIZE3; k >= 0; k -= BSIZE3){
        //first block
        for(ii = i; ii < (i + BSIZE3); ii += BSIZE2){
          for(jj = j; jj < (j + BSIZE3); jj += BSIZE2){
            if(!kkb){
            for(kk = k; kk < (k + BSIZE3); kk += BSIZE2){
              //second block
              for(iii = ii; iii < (ii + BSIZE2); iii += BSIZE1){
                for(jjj = jj; jjj < (jj + BSIZE2); jjj += BSIZE1){
                  if(!kkkb){
                  for(kkk = kk; kkk < (kk + BSIZE2); kkk += BSIZE1){
                    //third block
            			  for(iiii = iii; iiii < iii + BSIZE1; iiii++){
                        for(jjjj = jjj; jjjj < jjj + BSIZE1; jjjj++){
            			        sum = 0;
                          if(!kkkkb){
            			          for(kkkk = kkk; kkkk < kkk + BSIZE1; kkkk++){
            			            sum += A[iiii][kkkk] * B[jjjj][kkkk];
            			          }
                            kkkkb = 1;
                          }
                          else{
            			          for(kkkk = kkk + BSIZE1 - 1; kkkk >=  kkk; kkkk--){
            			            sum += A[iiii][kkkk] * B[jjjj][kkkk];
            			          }
                            kkkkb = 0;
                          }
            			        C[iiii][jjjj] += sum;
            			      }
            			  }
                  }
                  kkkb = 1;
                  }
                  else{
                  for(kkk = kk + BSIZE2 - BSIZE1; kkk >= kk; kkk -= BSIZE1){
                    //third block
            			  for(iiii = iii; iiii < iii + BSIZE1; iiii++){
                        for(jjjj = jjj; jjjj < jjj + BSIZE1; jjjj++){
            			        sum = 0;
                          if(!kkkkb){
            			          for(kkkk = kkk; kkkk < kkk + BSIZE1; kkkk++){
            			            sum += A[iiii][kkkk] * B[jjjj][kkkk];
            			          }
                            kkkkb = 1;
                          }
                          else{
            			          for(kkkk = kkk + BSIZE1 - 1; kkkk >=  kkk; kkkk--){
            			            sum += A[iiii][kkkk] * B[jjjj][kkkk];
            			          }
                            kkkkb = 0;
                          }
            			        C[iiii][jjjj] += sum;
            			      }
            			  }
                  }
                  kkkb = 0;
                  }
                }
              }
            }
            kkb = 1;
            }
            else{
            for(kk = k + BSIZE3 - BSIZE2; kk >= k; kk -= BSIZE2){
              //second block
              for(iii = ii; iii < (ii + BSIZE2); iii += BSIZE1){
                for(jjj = jj; jjj < (jj + BSIZE2); jjj += BSIZE1){
                  if(!kkkb){
                  for(kkk = kk; kkk < (kk + BSIZE2); kkk += BSIZE1){
                    //third block
            			  for(iiii = iii; iiii < iii + BSIZE1; iiii++){
                        for(jjjj = jjj; jjjj < jjj + BSIZE1; jjjj++){
            			        sum = 0;
                          if(!kkkkb){
            			          for(kkkk = kkk; kkkk < kkk + BSIZE1; kkkk++){
            			            sum += A[iiii][kkkk] * B[jjjj][kkkk];
            			          }
                            kkkkb = 1;
                          }
                          else{
            			          for(kkkk = kkk + BSIZE1 - 1; kkkk >=  kkk; kkkk--){
            			            sum += A[iiii][kkkk] * B[jjjj][kkkk];
            			          }
                            kkkkb = 0;
                          }
            			        C[iiii][jjjj] += sum;
            			      }
            			  }
                  }
                  kkkb = 1;
                  }
                  else{
                  for(kkk = kk + BSIZE2 - BSIZE1; kkk >= kk; kkk -= BSIZE1){
                    //third block
            			  for(iiii = iii; iiii < iii + BSIZE1; iiii++){
                        for(jjjj = jjj; jjjj < jjj + BSIZE1; jjjj++){
            			        sum = 0;
                          if(!kkkkb){
            			          for(kkkk = kkk; kkkk < kkk + BSIZE1; kkkk++){
            			            sum += A[iiii][kkkk] * B[jjjj][kkkk];
            			          }
                            kkkkb = 1;
                          }
                          else{
            			          for(kkkk = kkk + BSIZE1 - 1; kkkk >=  kkk; kkkk--){
            			            sum += A[iiii][kkkk] * B[jjjj][kkkk];
            			          }
                            kkkkb = 0;
                          }
            			        C[iiii][jjjj] += sum;
            			      }
            			  }
                  }
                  kkkb = 0;
                  }
                }
              }
            }
            kkb = 0;
            }
          }
        }
      }
      kb = 0;
      }
    }
  }
}
// function to allocate a matrix on the heap
// creates an mXn matrix and returns the pointer.
//
// the matrices are in row-major order.
void create_matrix(float*** Q, int m, int n) {
  float **T = 0;
  int i;

  T = (float**)malloc( m*sizeof(float*));
  for ( i=0; i<m; i++ ) {
     T[i] = (float*)malloc(n*sizeof(float));
  }
  *Q = T;
}

void intialize_matrix(float** Q, int m, int n) {

  for(int i=0; i<m; i++) {
    for(int j=0; j<n; j++) {
      //Q[i][j] = (float) rand(); 
      Q[i][j] = (float) 1.1; 
    }	
  }
}

int main(int argc, char *argv[]) {

	if(argc!=2) {
		printf("Agrument for multiples of runtime needed\n");
		return 1;
	}
	
    char * filename = argv[0];
	int shmid1;
	stats_ptrs = (stats_struct *) get_shared_ptr("stats_pt", sizeof(stats_struct)*68, SHM_W, &shmid1);
	
    int shmid2;
    core_mapping = (core_write_struct *) get_shared_ptr(filename, sizeof(core_write_struct), SHM_W, &shmid2);


  setup_timer();
  setup_papi();
	
  int rt_mul = atoi(argv[1]);

  create_matrix(&A, M, P);
  create_matrix(&B, P, N);
  create_matrix(&C, M, N);
  intialize_matrix(A, M, P);
  intialize_matrix(B, P, N);
  intialize_matrix(C, M, N);

  int rep_count = 11*rt_mul;

  for(int k=0; k<rep_count; k++) {
	  int i, j;
	  for(i=0; i<N; i++)
	  {
		for(j=0; j<N; j++)
		{
		  A[i][j] = 1.1;
		  B[i][j] = 1.1;    
		}
	  }
	  // assume some initialization of A and B
	  // think of this as a library where A and B are
	  // inputs in row-major format, and C is an output
	  // in row-major.
	  //flip B
	  for(i = 0; i < P; i++){
		for(j = 0; j < M; j++){
		  B[i][j] = B[j][i];
		}
	  }
	  matmul_aware();
  }
  detach_shared_mem(stats_ptrs);
  detach_shared_mem(core_mapping);
  PAPI_shutdown();
  exit(0);
}
