/* 
   Test Decryption by using a previously generated cipher file as the input.
   The decrypted (recovered) arrays are stored in a binary file, and max error 
   is computed between the original and recovered files.
*/

#include "../src/HEAAN.h"

using namespace std;
using namespace NTL;

#define NUM_PTHREADS        8
#define COMPUTE_ERROR       1

int main(int argc, char *argv[]) 
{
  /* Parameters */
  long logq;          /* Ciphertext modulus */
  long logp;           /* Scaling Factor */
  long slots;
  long nrows, row, i;

  const char* orgfilename = "orgdblfile";       /* original data file */
  const char* oupfilename = "recdblfile";       /* decryted output file */
  const char* cipherfilename = "cipherfile";    /* encrypted file */
  const char* seckeyfilename = "seckeyfile";    /* secret key file */
  FILE* fp;
	
  SetNumThreads(NUM_PTHREADS);
  Ring ring;
  SecretKey secretKey(ring);
  complex<double>* dvec1;
  Ciphertext cipher1;

  /* Read scheme parameters */
  fp = fopen(cipherfilename, "rb");
  if(fp == NULL) return -1;
  if(fread(&slots, sizeof(long), 1, fp) != 1) {
      fclose(fp);
      return -1;              
  }
  if(fread(&logp, sizeof(long), 1, fp) != 1) {
      fclose(fp);
      return -1;              
  }
  if(fread(&logq, sizeof(long), 1, fp) != 1) {
      fclose(fp);
      return -1;              
  }
  fclose(fp);

  /* check if secret key file exists */
  ifstream testfile;
  testfile.open(seckeyfilename);
  if(!testfile){
      return -1;
  }

  /* read the secret key */
  SerializationUtils::readSecKey(secretKey, logq, seckeyfilename);

  Scheme scheme(secretKey, ring);

  /* if recovered file exits, delete it */
  testfile.open(oupfilename);
  if(testfile && remove(oupfilename)){
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

  double temparr[2*slots];

  fp = fopen(oupfilename, "wb");
  if(fp == NULL) return -1;

  for(row = 0; row < nrows; row++){
    /* read cipher */      
    SerializationUtils::readCiphertext(cipher1, cipherfilename, row == 0, row == nrows-1);

    /* Decrypt it */
    dvec1 = scheme.decrypt(secretKey, cipher1);

    /* put it in a double array for storing */
    for(i = 0; i < slots; i++){
      temparr[2*i] = dvec1[i].real();
      temparr[2*i+1] = dvec1[i].imag();
    }

    /* store the recovered message */
    if(fwrite(temparr, sizeof(double), 2*slots, fp) != 2*slots) {
      fclose(fp);
      return -1;              
    }
  }
  fclose(fp);

  cout << "\nDecrypted " << nrows << " rows.\n";

  if(COMPUTE_ERROR){
    FILE *fp1, *fp2;
    double maxerr = 0, temp1, temp2;

    fp1 = fopen(orgfilename, "rb");
    fp2 = fopen(oupfilename, "rb");
    for(row = 0; row < nrows; row++){
      for(i = 0; i < 2*slots; i++){
        if(fread(&temp1, sizeof(double), 1, fp1) != 1){
          fclose(fp1);
          return -1;
        }
        if(fread(&temp2, sizeof(double), 1, fp2) != 1){
          fclose(fp1);
          return -1;
        }

        maxerr = max(maxerr, abs(temp1-temp2));
        //if(maxerr > 0.5)
          //cout << temp1 << " " << temp2 << "\n";
      }
    }

    fclose(fp1);
    fclose(fp2);

    cout << "Max error = " << maxerr << ".\n";
  }

  return 0;
}
