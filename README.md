# Accelerated HEAAN library suitable for compute clusters

pthreads and MPI-parallelized HEAAN library (https://heaan.it/) with added serialization functions to store/read ciphertexts and secret (private) keys. The `for` loops in the library are parallelized using the NTL pthreads macros.  

The test `test_enc` encrypts a specified no. of blocks of an input file `orgdblfile`, and stores the encrypted file and secret key file. Each 'block' contains 32Ki complex elements, and the no. of blocks to encrypt/decrypt is specified using a cmd line argument. 

The test `test_dec` takes the encrypted file and secret key file as inputs, decrypts it, and stores the recovered file. It also computes the maximum absolute error between the original and recovered files.

The helper function `rndfilegen` outputs a complex double data file of the specified number of elements.

The parallelized library has been tested with 1GiB input files on a 3-PC cluster, yielding acceptable maximum absolute errors. The project was developed using VS Code.

