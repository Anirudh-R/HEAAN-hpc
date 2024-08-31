/*
* Copyright (c) by CryptoLab inc.
* This program is licensed under a
* Creative Commons Attribution-NonCommercial 3.0 Unported License.
* You should have received a copy of the license along with this
* work.  If not, see <http://creativecommons.org/licenses/by-nc/3.0/>.
*/
#include "SerializationUtils.h"

/* anirudhr: changed to fopen in append mode (to store multiple ciphers in a file) */
void SerializationUtils::writeCiphertext(Ciphertext& cipher, string path) {
	fstream fout;
	fout.open(path, ios::binary|ios::out|ios::app);

	long n = cipher.n;
	long logp = cipher.logp;
	long logq = cipher.logq;
	fout.write(reinterpret_cast<char*>(&n), sizeof(long));
	fout.write(reinterpret_cast<char*>(&logp), sizeof(long));
	fout.write(reinterpret_cast<char*>(&logq), sizeof(long));

	long np = ceil(((double)logq + 1)/8);
	ZZ q = conv<ZZ>(1) << logq;
	unsigned char* bytes = new unsigned char[np];
	for (long i = 0; i < N; ++i) {
		cipher.ax[i] %= q;
		BytesFromZZ(bytes, cipher.ax[i], np);
		fout.write(reinterpret_cast<char*>(bytes), np);
	}
	for (long i = 0; i < N; ++i) {
		cipher.bx[i] %= q;
		BytesFromZZ(bytes, cipher.bx[i], np);
		fout.write(reinterpret_cast<char*>(bytes), np);
	}
	fout.close();
}

void SerializationUtils::readCiphertext(Ciphertext& cipher, string path, int isBegin, int isEnd) {
	long n, logp, logq;
	static fstream fin;
	if(isBegin)
		fin.open(path, ios::binary|ios::in);

	fin.read(reinterpret_cast<char*>(&n), sizeof(long));
	fin.read(reinterpret_cast<char*>(&logp), sizeof(long));
	fin.read(reinterpret_cast<char*>(&logq), sizeof(long));

	long np = ceil(((double)logq + 1)/8);
	unsigned char* bytes = new unsigned char[np];
	cipher.n = n;
	cipher.logp = logp;
	cipher.logq = logq;

	for (long i = 0; i < N; ++i) {
		fin.read(reinterpret_cast<char*>(bytes), np);
		ZZFromBytes(cipher.ax[i], bytes, np);
	}
	for (long i = 0; i < N; ++i) {
		fin.read(reinterpret_cast<char*>(bytes), np);
		ZZFromBytes(cipher.bx[i], bytes, np);
	}

	if(isEnd)
		fin.close();
}

void SerializationUtils::writeKey(Key* key, string path) {
	fstream fout;
	fout.open(path, ios::binary|ios::out);
	fout.write(reinterpret_cast<char*>(key->rax), Nnprimes*sizeof(uint64_t));
	fout.write(reinterpret_cast<char*>(key->rbx), Nnprimes*sizeof(uint64_t));
	fout.close();
}

Key* SerializationUtils::readKey(string path) {
	//Key key;
	Key* key = new Key();
	fstream fin;
	fin.open(path, ios::binary|ios::in);
	fin.read(reinterpret_cast<char*>(key->rax), Nnprimes*sizeof(uint64_t));
	fin.read(reinterpret_cast<char*>(key->rbx), Nnprimes*sizeof(uint64_t));
	fin.close();
	return key;
}

/* Write secretKey to a file */
void SerializationUtils::writeSecKey(SecretKey& secretKey, long logq, string path) {
	fstream fout;
	fout.open(path, ios::binary|ios::out);

	fout.write(reinterpret_cast<char*>(&logq), sizeof(long));

	long np = ceil(((double)logq + 1)/8);
	ZZ q = conv<ZZ>(1) << logq;
	unsigned char* bytes = new unsigned char[np];
	for (long i = 0; i < N; ++i) {
		secretKey.sx[i] %= q;
		BytesFromZZ(bytes, secretKey.sx[i], np);
		fout.write(reinterpret_cast<char*>(bytes), np);
	}

	fout.close();
}

/* Read secretKey from a file */
void SerializationUtils::readSecKey(SecretKey& secretKey, long& logq, string path) {
	fstream fin;
	fin.open(path, ios::binary|ios::in);

	const ZZ minus1 = conv<ZZ>("2037035976334486086268445688409378161051468393665936250636140449354381299763336706183397375");	/* value for -1 */

	fin.read(reinterpret_cast<char*>(&logq), sizeof(long));

	long np = ceil(((double)logq + 1)/8);
	unsigned char* bytes = new unsigned char[np];
	for (long i = 0; i < N; ++i) {
		fin.read(reinterpret_cast<char*>(bytes), np);
		ZZFromBytes(secretKey.sx[i], bytes, np);

		if(secretKey.sx[i] == minus1)
			secretKey.sx[i] = -1;
	}

	fin.close();
}
