/* 
   Test Encyption with a variable number of 32Ki complex<double> arrays.
   The encrypted arrays are stored in a cipher file.

   Credits: Adapted from the work of Cryptography LAB, Seoul National University, https://github.com/snucrypto/HEAAN

   License: https://creativecommons.org/licenses/by-nc/3.0/
 */

#include "../src/HEAAN.h"
#include <mpi.h>

using namespace std;
using namespace NTL;

#define NUM_PTHREADS        1

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
	long nrows, nrowsperproc, row, i;
	complex<double>* mvec1 = new complex<double>[slots];
	Ciphertext cipher1;
	double temparr[2*slots];
	int rank, np, err, count;
	MPI_File fh;
	MPI_Status status;
	MPI_Offset offset;

	const char* inpfilename = "/opt/cloud/HEAAN-v4/run/orgdblfile";           /* file to be encrypted */
	const char* finalcipherfilename = "cipherfile";   /* final encrypted file */
	const char* seckeyfilename = "seckeyfile";        /* secret key file */

	/* Construct and Generate Public Keys */
	srand(time(NULL));
	SetNumThreads(NUM_PTHREADS);
	MPI_Init(&argc, &argv);

	/* find our process ID and the total number of processes */
	err = MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	CHECK_ERR(MPI_Comm_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &np);

	/* get the no. of 32Ki arrays to encrypt */
	if(argc > 1){
		sscanf(argv[1], "%ld", &nrows);
	}
	else{
		cout << "Error: Enter no. of rows\n";
		return -1;
	}

	nrowsperproc = nrows/np;

	/* master process */
	if(rank == 0){
		Ring ring;
		SecretKey secretKey(ring);
		Scheme scheme(secretKey, ring, true, true);

		system("rm cipherfile*");

		/* if final cipherfile exits, delete it */
		ifstream testfile;
		testfile.open(finalcipherfilename);
		if(testfile && remove(finalcipherfilename)){
			return -1;
		}

		/* if seckeyfile exits, delete it */
		testfile.open(seckeyfilename);
		if(testfile && remove(seckeyfilename)){
			return -1;
		}

		err = MPI_File_open(MPI_COMM_WORLD, inpfilename, MPI_MODE_RDONLY, MPI_INFO_NULL, &fh);
		CHECK_ERR(MPI_File_open);

		offset = rank * nrowsperproc * slots * 2 * sizeof(double);

		for(row = 0; row < nrowsperproc; row++){
			MPI_File_read_at(fh, offset + row * slots * 2 * sizeof(double), temparr, 2*slots, MPI_DOUBLE, &status);
			MPI_Get_count(&status, MPI_DOUBLE, &count);
			cout << "Rank " << rank << ": read " << count << " doubles" << "\n";

			/* fill it into the message vector */
			for(i = 0; i < slots; i++){
				mvec1[i].real(temparr[2*i]);
				mvec1[i].imag(temparr[2*i+1]);
			}

			/* Encrypt the message */
			scheme.encrypt(cipher1, mvec1, slots, logp, logq);

			/* store the cipher */
			string localcipherfilename = "cipherfile" + to_string(rank);
			SerializationUtils::writeCiphertext(cipher1, localcipherfilename);
		}

		cout << "Rank " << rank << ": Encrypted " << nrowsperproc << " rows.\n";

		MPI_File_close(&fh);

		/* store the secret key */
		SerializationUtils::writeSecKey(secretKey, logq, seckeyfilename);
	}
	
	/* Slave */
	else{
		Ring ring;
		SecretKey secretKey(ring);
		Scheme scheme(secretKey, ring, true, false);

		err = MPI_File_open(MPI_COMM_WORLD, inpfilename, MPI_MODE_RDONLY, MPI_INFO_NULL, &fh);
		CHECK_ERR(MPI_File_open);

		offset = rank * nrowsperproc * slots * 2 * sizeof(double);

		for(row = 0; row < nrowsperproc; row++){
			MPI_File_read_at(fh, offset + row * slots * 2 * sizeof(double), temparr, 2*slots, MPI_DOUBLE, &status);
			MPI_Get_count(&status, MPI_DOUBLE, &count);
			cout << "Rank " << rank << ": read " << count << " doubles" << "\n";

			/* fill it into the message vector */
			for(i = 0; i < slots; i++){
				mvec1[i].real(temparr[2*i]);
				mvec1[i].imag(temparr[2*i+1]);
			}

			/* Encrypt the message */
			scheme.encrypt(cipher1, mvec1, slots, logp, logq);

			/* store the cipher */
			string localcipherfilename = "cipherfile" + to_string(rank);
			SerializationUtils::writeCiphertext(cipher1, localcipherfilename);
		}

		cout << "Rank " << rank << ": Encrypted " << nrowsperproc << " rows.\n";

		MPI_File_close(&fh);
	}

	MPI_Barrier(MPI_COMM_WORLD);

	/* Combine individual encrypted files */
	if(rank == 0){
		for(i = 0; i < np; i++){
			string cmd = "dd if=cipherfile" + to_string(i) + " bs=4k of=cipherfile oflag=append conv=notrunc";
			system(cmd.c_str());	
		}

		system("rm cipherfile?");
	}

	MPI_Finalize();

	return 0;
}
