/*
* Copyright (c) by CryptoLab inc.
* This program is licensed under a
* Creative Commons Attribution-NonCommercial 3.0 Unported License.
* You should have received a copy of the license along with this
* work.  If not, see <http://creativecommons.org/licenses/by-nc/3.0/>.
*/
#include "Ciphertext.h"

#include <NTL/BasicThreadPool.h>
#include <NTL/tools.h>

Ciphertext::Ciphertext(long logp, long logq, long n) : logp(logp), logq(logq), n(n) {
}

Ciphertext::Ciphertext(const Ciphertext& o) : logp(o.logp), logq(o.logq), n(o.n) {
	//for (long i = 0; i < N; ++i) {
	NTL_EXEC_RANGE(N, first, last);
		for (long i = first; i < last; ++i) {
			ax[i] = o.ax[i];
			bx[i] = o.bx[i];
		}
	NTL_EXEC_RANGE_END;
}

void Ciphertext::copyParams(Ciphertext& o) {
	logp = o.logp;
	logq = o.logq;
	n = o.n;
}

void Ciphertext::copy(Ciphertext& o) {
	copyParams(o);
	//for (long i = 0; i < N; ++i) {
	NTL_EXEC_RANGE(N, first, last);
	for (long i = first; i < last; ++i) {
		ax[i] = o.ax[i];
		bx[i] = o.bx[i];
	}
	NTL_EXEC_RANGE_END;
}

void Ciphertext::free() {
	//for (long i = 0; i < N; ++i) {
	NTL_EXEC_RANGE(N, first, last);
	for (long i = first; i < last; ++i) {
		clear(ax[i]);
		clear(bx[i]);
	}
	NTL_EXEC_RANGE_END;
}

Ciphertext::~Ciphertext() {
	delete[] ax;
	delete[] bx;
}
