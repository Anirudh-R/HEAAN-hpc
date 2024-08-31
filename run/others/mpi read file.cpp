/* 
   Test Encyption with a variable number of 32Ki complex<double> arrays.
   The encrypted arrays are stored in a cipher file.
*/

#include "../src/HEAAN.h"
#include <mpi.h>

using namespace std;
using namespace NTL;

#define NUM_PTHREADS        8

#define CHECK_ERR(func) { \
    if (err != MPI_SUCCESS) { \
        int errorStringLen; \
        char errorString[MPI_MAX_ERROR_STRING]; \
        MPI_Error_string(err, errorString, &errorStringLen); \
        printf("Error at line %d: calling %s (%s)\n",__LINE__, #func, errorString); \
    } \
}

int main(int argc, char *argv[]) 
{
  /* Parameters */
  long logq = 300;          /* Ciphertext modulus (this value should be <= logQ in "scr/Params.h") */
  long logp = 30;           /* Scaling Factor (larger logp will give you more accurate value) */
  long logn = 15;           /* number of slots (this value should be < logN in "src/Params.h") */
  long slots = 1 << logn;
  long nrows, row, i;
  double temparr[2*slots];

  const char* inpfilename = "/opt/cloud/HEAAN-v4/run/orgdblfile";       /* file to be encrypted */
  const char* cipherfilename = "cipherfile";    /* encrypted file */
  const char* seckeyfilename = "seckeyfile";    /* secret key file */
  FILE* fp;

  int pid, np, cmode, err, count;
  MPI_File fh;
  MPI_Status status;
  MPI_Offset offset;
  MPI_Init(&argc, &argv);

  /* find our process ID and the total number of processes */
  err = MPI_Comm_rank(MPI_COMM_WORLD, &pid);
  MPI_Comm_size(MPI_COMM_WORLD, &np);
  CHECK_ERR(MPI_Comm_rank);

  cout << "My rank is " << pid << "\n";

  cmode = MPI_MODE_RDONLY;
  err = MPI_File_open(MPI_COMM_WORLD, inpfilename, cmode, MPI_INFO_NULL, &fh);
  CHECK_ERR(MPI_File_open);

  double buf[10];
  offset = 10 * pid * sizeof(double);
  MPI_File_read_at(fh, offset, buf, 10, MPI_DOUBLE, &status);

  MPI_Get_count(&status, MPI_DOUBLE, &count);
  cout << "process " << pid << " read " << count << " doubles" << "\n";

  for(int i = 0; i < 10; i++){
    cout << buf[i] << "\n";
  }

  MPI_File_close(&fh);

  MPI_Finalize();

  #if 0
  /* Construct and Generate Public Keys */
  srand(time(NULL));
  SetNumThreads(NUM_PTHREADS);
  Ring ring;
  SecretKey secretKey(ring);

  Scheme scheme(secretKey, ring);
  
  complex<double>* mvec1 = new complex<double>[slots];
  Ciphertext cipher1;

  /* if cipherfile exits, delete it */
  ifstream testfile;
  testfile.open(cipherfilename);
  if(testfile && remove(cipherfilename)){
      return -1;
  }

  /* if seckeyfile exits, delete it */
  testfile.open(seckeyfilename);
  if(testfile && remove(seckeyfilename)){
      return -1;
  }

  /* get the no. of 32Ki arrays to encrypt */
  if(argc > 1){
    sscanf(argv[1], "%ld", &nrows);
  }
  else{
    cout << "Error: Enter no. of rows\n";
    return -1;
  }

  fp = fopen(inpfilename, "rb");
  if(fp == NULL) return -1;

  for(row = 0; row < nrows; row++){  
    /* read-in a single row */
    if(fread(temparr, sizeof(double), 2*slots, fp) != 2*slots) {
        fclose(fp);
        return -1;              
    }

    /* fill it into the message vector */
    for(i = 0; i < slots; i++){
        mvec1[i].real(temparr[2*i]);
        mvec1[i].imag(temparr[2*i+1]);
    }

    /* Encrypt the message */
    scheme.encrypt(cipher1, mvec1, slots, logp, logq);

    /* store the cipher */
    SerializationUtils::writeCiphertext(cipher1, cipherfilename);
  }
  fclose(fp);

  /* store the secret key */
  SerializationUtils::writeSecKey(secretKey, logq, seckeyfilename);

  cout << "\nEncrypted " << nrows << " rows.\n";
  #endif

  return 0;
}

