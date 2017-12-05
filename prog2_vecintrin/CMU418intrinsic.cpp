#include <stdio.h>
#include <algorithm>
#include <getopt.h>
#include <math.h>
#include <cstdlib>
#include <cmath>
#include <vector>
#include <string.h>
using namespace std;

#define EXP_MAX 10

#define MAX_INST_LEN 32

// Define vector unit width here
#define VECTOR_WIDTH 4

// Declare a floating point vector register with __cmu418_vec_float
#define __cmu418_vec_float __cmu418_vec<float>

// Declare an integer vector register with __cmu418_vec_int
#define __cmu418_vec_int   __cmu418_vec<int>

template <typename T>
struct __cmu418_vec {
  T value[VECTOR_WIDTH];
};

// Declare a mask with __cmu418_mask
struct __cmu418_mask : __cmu418_vec<bool> {};


struct Log {
  char instruction[MAX_INST_LEN];
  unsigned long long mask; // support vector width up to 64
};

struct Statistics {
  unsigned long long utilized_lane;
  unsigned long long total_lane;
  unsigned long long total_instructions;
};

class Logger {
  private:
    vector<Log> log;
    Statistics stats;

  public:
    void addLog(const char * instruction, __cmu418_mask mask, int N = 0);
    void printStats();
    void printLog();
};




void Logger::addLog(const char * instruction, __cmu418_mask mask, int N) {
  Log newLog;
  strcpy(newLog.instruction, instruction);
  newLog.mask = 0;
  for (int i=0; i<N; i++) {
    if (mask.value[i]) {
      newLog.mask |= (((unsigned long long)1)<<i);
      stats.utilized_lane++;
    }
  }
  stats.total_lane += N;
  stats.total_instructions += (N>0);
  log.push_back(newLog);
}

void Logger::printStats() {
  printf("****************** Printing Vector Unit Statistics *******************\n");
  printf("Vector Width:              %d\n", VECTOR_WIDTH);
  printf("Total Vector Instructions: %lld\n", stats.total_instructions);
  printf("Vector Utilization:        %f%%\n", (double)stats.utilized_lane/stats.total_lane*100);
  printf("Utilized Vector Lanes:     %lld\n", stats.utilized_lane);
  printf("Total Vector Lanes:        %lld\n", stats.total_lane);
}

void Logger::printLog() {
  printf("***************** Printing Vector Unit Execution Log *****************\n");
  printf(" Instruction | Vector Lane Occupancy ('*' for active, '_' for inactive)\n");
  printf("------------- --------------------------------------------------------\n");
  for (int i=0; i<log.size(); i++) {
    printf("%12s | ", log[i].instruction);
    for (int j=0; j<VECTOR_WIDTH; j++) {
      if (log[i].mask & (((unsigned long long)1)<<j)) {
        printf("*");
      } else {
        printf("_");
      }
    }
    printf("\n");
  }
}

//***********************
//* Function Definition *
//***********************

// Return a mask initialized to 1 in the first N lanes and 0 in the others
__cmu418_mask _cmu418_init_ones(int first = VECTOR_WIDTH);

// Return the inverse of maska
__cmu418_mask _cmu418_mask_not(__cmu418_mask &maska);

// Return (maska | maskb)
__cmu418_mask _cmu418_mask_or(__cmu418_mask &maska, __cmu418_mask &maskb);

// Return (maska & maskb)
__cmu418_mask _cmu418_mask_and(__cmu418_mask &maska, __cmu418_mask &maskb);

// Count the number of 1s in maska
int _cmu418_cntbits(__cmu418_mask &maska);

// Set register to value if vector lane is active
//  otherwise keep the old value
void _cmu418_vset_float(__cmu418_vec_float &vecResult, float value, __cmu418_mask &mask);
void _cmu418_vset_int(__cmu418_vec_int &vecResult, int value, __cmu418_mask &mask);
// For user's convenience, returns a vector register with all lanes initialized to value
__cmu418_vec_float _cmu418_vset_float(float value);
__cmu418_vec_int _cmu418_vset_int(int value);

// Copy values from vector register src to vector register dest if vector lane active
// otherwise keep the old value
void _cmu418_vmove_float(__cmu418_vec_float &dest, __cmu418_vec_float &src, __cmu418_mask &mask);
void _cmu418_vmove_int(__cmu418_vec_int &dest, __cmu418_vec_int &src, __cmu418_mask &mask);

// Load values from array src to vector register dest if vector lane active
//  otherwise keep the old value
void _cmu418_vload_float(__cmu418_vec_float &dest, float* src, __cmu418_mask &mask);
void _cmu418_vload_int(__cmu418_vec_int &dest, int* src, __cmu418_mask &mask);

// Store values from vector register src to array dest if vector lane active
//  otherwise keep the old value
void _cmu418_vstore_float(float* dest, __cmu418_vec_float &src, __cmu418_mask &mask);
void _cmu418_vstore_int(int* dest, __cmu418_vec_int &src, __cmu418_mask &mask);

// Return calculation of (veca + vecb) if vector lane active
//  otherwise keep the old value
void _cmu418_vadd_float(__cmu418_vec_float &vecResult, __cmu418_vec_float &veca, __cmu418_vec_float &vecb, __cmu418_mask &mask);
void _cmu418_vadd_int(__cmu418_vec_int &vecResult, __cmu418_vec_int &veca, __cmu418_vec_int &vecb, __cmu418_mask &mask);

// Return calculation of (veca - vecb) if vector lane active
//  otherwise keep the old value
void _cmu418_vsub_float(__cmu418_vec_float &vecResult, __cmu418_vec_float &veca, __cmu418_vec_float &vecb, __cmu418_mask &mask);
void _cmu418_vsub_int(__cmu418_vec_int &vecResult, __cmu418_vec_int &veca, __cmu418_vec_int &vecb, __cmu418_mask &mask);

// Return calculation of (veca * vecb) if vector lane active
//  otherwise keep the old value
void _cmu418_vmult_float(__cmu418_vec_float &vecResult, __cmu418_vec_float &veca, __cmu418_vec_float &vecb, __cmu418_mask &mask);
void _cmu418_vmult_int(__cmu418_vec_int &vecResult, __cmu418_vec_int &veca, __cmu418_vec_int &vecb, __cmu418_mask &mask);

// Return calculation of (veca / vecb) if vector lane active
//  otherwise keep the old value
void _cmu418_vdiv_float(__cmu418_vec_float &vecResult, __cmu418_vec_float &veca, __cmu418_vec_float &vecb, __cmu418_mask &mask);
void _cmu418_vdiv_int(__cmu418_vec_int &vecResult, __cmu418_vec_int &veca, __cmu418_vec_int &vecb, __cmu418_mask &mask);


// Return calculation of absolute value abs(veca) if vector lane active
//  otherwise keep the old value
void _cmu418_vabs_float(__cmu418_vec_float &vecResult, __cmu418_vec_float &veca, __cmu418_mask &mask);
void _cmu418_vabs_int(__cmu418_vec_int &vecResult, __cmu418_vec_int &veca, __cmu418_mask &mask);

// Return a mask of (veca > vecb) if vector lane active
//  otherwise keep the old value
void _cmu418_vgt_float(__cmu418_mask &vecResult, __cmu418_vec_float &veca, __cmu418_vec_float &vecb, __cmu418_mask &mask);
void _cmu418_vgt_int(__cmu418_mask &vecResult, __cmu418_vec_int &veca, __cmu418_vec_int &vecb, __cmu418_mask &mask);

// Return a mask of (veca < vecb) if vector lane active
//  otherwise keep the old value
void _cmu418_vlt_float(__cmu418_mask &vecResult, __cmu418_vec_float &veca, __cmu418_vec_float &vecb, __cmu418_mask &mask);
void _cmu418_vlt_int(__cmu418_mask &vecResult, __cmu418_vec_int &veca, __cmu418_vec_int &vecb, __cmu418_mask &mask);

// Return a mask of (veca == vecb) if vector lane active
//  otherwise keep the old value
void _cmu418_veq_float(__cmu418_mask &vecResult, __cmu418_vec_float &veca, __cmu418_vec_float &vecb, __cmu418_mask &mask);
void _cmu418_veq_int(__cmu418_mask &vecResult, __cmu418_vec_int &veca, __cmu418_vec_int &vecb, __cmu418_mask &mask);

// Adds up adjacent pairs of elements, so
//  [0 1 2 3] -> [0+1 0+1 2+3 2+3]
void _cmu418_hadd_float(__cmu418_vec_float &vecResult, __cmu418_vec_float &vec);

// Performs an even-odd interleaving where all even-indexed elements move to front half
//  of the array and odd-indexed to the back half, so
//  [0 1 2 3 4 5 6 7] -> [0 2 4 6 1 3 5 7]
void _cmu418_interleave_float(__cmu418_vec_float &vecResult, __cmu418_vec_float &vec);

// Add a customized log to help debugging
void addUserLog(const char * logStr);


Logger CMU418Logger;

//******************
//* Implementation *
//******************

__cmu418_mask _cmu418_init_ones(int first) {
  __cmu418_mask mask;
  for (int i=0; i<VECTOR_WIDTH; i++) {
    mask.value[i] = (i<first) ? true : false;
  }
  return mask;
}

__cmu418_mask _cmu418_mask_not(__cmu418_mask &maska) {
  __cmu418_mask resultMask;
  for (int i=0; i<VECTOR_WIDTH; i++) {
    resultMask.value[i] = !maska.value[i];
  }
  CMU418Logger.addLog("masknot", _cmu418_init_ones(), VECTOR_WIDTH);
  return resultMask;
}

__cmu418_mask _cmu418_mask_or(__cmu418_mask &maska, __cmu418_mask &maskb) {
  __cmu418_mask resultMask;
  for (int i=0; i<VECTOR_WIDTH; i++) {
    resultMask.value[i] = maska.value[i] | maskb.value[i];
  }
  CMU418Logger.addLog("maskor", _cmu418_init_ones(), VECTOR_WIDTH);
  return resultMask;
}

__cmu418_mask _cmu418_mask_and(__cmu418_mask &maska, __cmu418_mask &maskb) {
  __cmu418_mask resultMask;
  for (int i=0; i<VECTOR_WIDTH; i++) {
    resultMask.value[i] = maska.value[i] && maskb.value[i];
  }
  CMU418Logger.addLog("maskand", _cmu418_init_ones(), VECTOR_WIDTH);
  return resultMask;
}

int _cmu418_cntbits(__cmu418_mask &maska) {
  int count = 0;
  for (int i=0; i<VECTOR_WIDTH; i++) {
    if (maska.value[i]) count++;
  }
  CMU418Logger.addLog("cntbits", _cmu418_init_ones(), VECTOR_WIDTH);
  return count;
}

template <typename T>
void _cmu418_vset(__cmu418_vec<T> &vecResult, T value, __cmu418_mask &mask) {
  for (int i=0; i<VECTOR_WIDTH; i++) {
    vecResult.value[i] = mask.value[i] ? value : vecResult.value[i];
  }
  CMU418Logger.addLog("vset", mask, VECTOR_WIDTH);
}

template void _cmu418_vset<float>(__cmu418_vec_float &vecResult, float value, __cmu418_mask &mask);
template void _cmu418_vset<int>(__cmu418_vec_int &vecResult, int value, __cmu418_mask &mask);

void _cmu418_vset_float(__cmu418_vec_float &vecResult, float value, __cmu418_mask &mask) { _cmu418_vset<float>(vecResult, value, mask); }
void _cmu418_vset_int(__cmu418_vec_int &vecResult, int value, __cmu418_mask &mask) { _cmu418_vset<int>(vecResult, value, mask); }

__cmu418_vec_float _cmu418_vset_float(float value) {
  __cmu418_vec_float vecResult;
  __cmu418_mask mask = _cmu418_init_ones();
  _cmu418_vset_float(vecResult, value, mask);
  return vecResult;
}
__cmu418_vec_int _cmu418_vset_int(int value) {
  __cmu418_vec_int vecResult;
  __cmu418_mask mask = _cmu418_init_ones();
  _cmu418_vset_int(vecResult, value, mask);
  return vecResult;
}

template <typename T>
void _cmu418_vmove(__cmu418_vec<T> &dest, __cmu418_vec<T> &src, __cmu418_mask &mask) {
    for (int i = 0; i < VECTOR_WIDTH; i++) {
        dest.value[i] = mask.value[i] ? src.value[i] : dest.value[i];
    }
    CMU418Logger.addLog("vmove", mask, VECTOR_WIDTH);
}

template void _cmu418_vmove<float>(__cmu418_vec_float &dest, __cmu418_vec_float &src, __cmu418_mask &mask);
template void _cmu418_vmove<int>(__cmu418_vec_int &dest, __cmu418_vec_int &src, __cmu418_mask &mask);

void _cmu418_vmove_float(__cmu418_vec_float &dest, __cmu418_vec_float &src, __cmu418_mask &mask) { _cmu418_vmove<float>(dest, src, mask); }
void _cmu418_vmove_int(__cmu418_vec_int &dest, __cmu418_vec_int &src, __cmu418_mask &mask) { _cmu418_vmove<int>(dest, src, mask); }

template <typename T>
void _cmu418_vload(__cmu418_vec<T> &dest, T* src, __cmu418_mask &mask) {
  for (int i=0; i<VECTOR_WIDTH; i++) {
    dest.value[i] = mask.value[i] ? src[i] : dest.value[i];
  }
  CMU418Logger.addLog("vload", mask, VECTOR_WIDTH);
}

template void _cmu418_vload<float>(__cmu418_vec_float &dest, float* src, __cmu418_mask &mask);
template void _cmu418_vload<int>(__cmu418_vec_int &dest, int* src, __cmu418_mask &mask);

void _cmu418_vload_float(__cmu418_vec_float &dest, float* src, __cmu418_mask &mask) { _cmu418_vload<float>(dest, src, mask); }
void _cmu418_vload_int(__cmu418_vec_int &dest, int* src, __cmu418_mask &mask) { _cmu418_vload<int>(dest, src, mask); }

template <typename T>
void _cmu418_vstore(T* dest, __cmu418_vec<T> &src, __cmu418_mask &mask) {
  for (int i=0; i<VECTOR_WIDTH; i++) {
    dest[i] = mask.value[i] ? src.value[i] : dest[i];
  }
  CMU418Logger.addLog("vstore", mask, VECTOR_WIDTH);
}

template void _cmu418_vstore<float>(float* dest, __cmu418_vec_float &src, __cmu418_mask &mask);
template void _cmu418_vstore<int>(int* dest, __cmu418_vec_int &src, __cmu418_mask &mask);

void _cmu418_vstore_float(float* dest, __cmu418_vec_float &src, __cmu418_mask &mask) { _cmu418_vstore<float>(dest, src, mask); }
void _cmu418_vstore_int(int* dest, __cmu418_vec_int &src, __cmu418_mask &mask) { _cmu418_vstore<int>(dest, src, mask); }

template <typename T>
void _cmu418_vadd(__cmu418_vec<T> &vecResult, __cmu418_vec<T> &veca, __cmu418_vec<T> &vecb, __cmu418_mask &mask) {
  for (int i=0; i<VECTOR_WIDTH; i++) {
    vecResult.value[i] = mask.value[i] ? (veca.value[i] + vecb.value[i]) : vecResult.value[i];
  }
  CMU418Logger.addLog("vadd", mask, VECTOR_WIDTH);
}

template void _cmu418_vadd<float>(__cmu418_vec_float &vecResult, __cmu418_vec_float &veca, __cmu418_vec_float &vecb, __cmu418_mask &mask);
template void _cmu418_vadd<int>(__cmu418_vec_int &vecResult, __cmu418_vec_int &veca, __cmu418_vec_int &vecb, __cmu418_mask &mask);

void _cmu418_vadd_float(__cmu418_vec_float &vecResult, __cmu418_vec_float &veca, __cmu418_vec_float &vecb, __cmu418_mask &mask) { _cmu418_vadd<float>(vecResult, veca, vecb, mask); }
void _cmu418_vadd_int(__cmu418_vec_int &vecResult, __cmu418_vec_int &veca, __cmu418_vec_int &vecb, __cmu418_mask &mask) { _cmu418_vadd<int>(vecResult, veca, vecb, mask); }

template <typename T>
void _cmu418_vsub(__cmu418_vec<T> &vecResult, __cmu418_vec<T> &veca, __cmu418_vec<T> &vecb, __cmu418_mask &mask) {
  for (int i=0; i<VECTOR_WIDTH; i++) {
    vecResult.value[i] = mask.value[i] ? (veca.value[i] - vecb.value[i]) : vecResult.value[i];
  }
  CMU418Logger.addLog("vsub", mask, VECTOR_WIDTH);
}

template void _cmu418_vsub<float>(__cmu418_vec_float &vecResult, __cmu418_vec_float &veca, __cmu418_vec_float &vecb, __cmu418_mask &mask);
template void _cmu418_vsub<int>(__cmu418_vec_int &vecResult, __cmu418_vec_int &veca, __cmu418_vec_int &vecb, __cmu418_mask &mask);

void _cmu418_vsub_float(__cmu418_vec_float &vecResult, __cmu418_vec_float &veca, __cmu418_vec_float &vecb, __cmu418_mask &mask) { _cmu418_vsub<float>(vecResult, veca, vecb, mask); }
void _cmu418_vsub_int(__cmu418_vec_int &vecResult, __cmu418_vec_int &veca, __cmu418_vec_int &vecb, __cmu418_mask &mask) { _cmu418_vsub<int>(vecResult, veca, vecb, mask); }

template <typename T>
void _cmu418_vmult(__cmu418_vec<T> &vecResult, __cmu418_vec<T> &veca, __cmu418_vec<T> &vecb, __cmu418_mask &mask) {
  for (int i=0; i<VECTOR_WIDTH; i++) {
    vecResult.value[i] = mask.value[i] ? (veca.value[i] * vecb.value[i]) : vecResult.value[i];
  }
  CMU418Logger.addLog("vmult", mask, VECTOR_WIDTH);
}

template void _cmu418_vmult<float>(__cmu418_vec_float &vecResult, __cmu418_vec_float &veca, __cmu418_vec_float &vecb, __cmu418_mask &mask);
template void _cmu418_vmult<int>(__cmu418_vec_int &vecResult, __cmu418_vec_int &veca, __cmu418_vec_int &vecb, __cmu418_mask &mask);

void _cmu418_vmult_float(__cmu418_vec_float &vecResult, __cmu418_vec_float &veca, __cmu418_vec_float &vecb, __cmu418_mask &mask) { _cmu418_vmult<float>(vecResult, veca, vecb, mask); }
void _cmu418_vmult_int(__cmu418_vec_int &vecResult, __cmu418_vec_int &veca, __cmu418_vec_int &vecb, __cmu418_mask &mask) { _cmu418_vmult<int>(vecResult, veca, vecb, mask); }

template <typename T>
void _cmu418_vdiv(__cmu418_vec<T> &vecResult, __cmu418_vec<T> &veca, __cmu418_vec<T> &vecb, __cmu418_mask &mask) {
  for (int i=0; i<VECTOR_WIDTH; i++) {
    vecResult.value[i] = mask.value[i] ? (veca.value[i] / vecb.value[i]) : vecResult.value[i];
  }
  CMU418Logger.addLog("vdiv", mask, VECTOR_WIDTH);
}

template void _cmu418_vdiv<float>(__cmu418_vec_float &vecResult, __cmu418_vec_float &veca, __cmu418_vec_float &vecb, __cmu418_mask &mask);
template void _cmu418_vdiv<int>(__cmu418_vec_int &vecResult, __cmu418_vec_int &veca, __cmu418_vec_int &vecb, __cmu418_mask &mask);

void _cmu418_vdiv_float(__cmu418_vec_float &vecResult, __cmu418_vec_float &veca, __cmu418_vec_float &vecb, __cmu418_mask &mask) { _cmu418_vdiv<float>(vecResult, veca, vecb, mask); }
void _cmu418_vdiv_int(__cmu418_vec_int &vecResult, __cmu418_vec_int &veca, __cmu418_vec_int &vecb, __cmu418_mask &mask) { _cmu418_vdiv<int>(vecResult, veca, vecb, mask); }

template <typename T>
void _cmu418_vabs(__cmu418_vec<T> &vecResult, __cmu418_vec<T> &veca, __cmu418_mask &mask) {
  for (int i=0; i<VECTOR_WIDTH; i++) {
    vecResult.value[i] = mask.value[i] ? (abs(veca.value[i])) : vecResult.value[i];
  }
  CMU418Logger.addLog("vabs", mask, VECTOR_WIDTH);
}

template void _cmu418_vabs<float>(__cmu418_vec_float &vecResult, __cmu418_vec_float &veca, __cmu418_mask &mask);
template void _cmu418_vabs<int>(__cmu418_vec_int &vecResult, __cmu418_vec_int &veca, __cmu418_mask &mask);

void _cmu418_vabs_float(__cmu418_vec_float &vecResult, __cmu418_vec_float &veca, __cmu418_mask &mask) { _cmu418_vabs<float>(vecResult, veca, mask); }
void _cmu418_vabs_int(__cmu418_vec_int &vecResult, __cmu418_vec_int &veca, __cmu418_mask &mask) { _cmu418_vabs<int>(vecResult, veca, mask); }

template <typename T>
void _cmu418_vgt(__cmu418_mask &maskResult, __cmu418_vec<T> &veca, __cmu418_vec<T> &vecb, __cmu418_mask &mask) {
  for (int i=0; i<VECTOR_WIDTH; i++) {
    maskResult.value[i] = mask.value[i] ? (veca.value[i] > vecb.value[i]) : maskResult.value[i];
  }
  CMU418Logger.addLog("vgt", mask, VECTOR_WIDTH);
}

template void _cmu418_vgt<float>(__cmu418_mask &maskResult, __cmu418_vec_float &veca, __cmu418_vec_float &vecb, __cmu418_mask &mask);
template void _cmu418_vgt<int>(__cmu418_mask &maskResult, __cmu418_vec_int &veca, __cmu418_vec_int &vecb, __cmu418_mask &mask);

void _cmu418_vgt_float(__cmu418_mask &maskResult, __cmu418_vec_float &veca, __cmu418_vec_float &vecb, __cmu418_mask &mask) { _cmu418_vgt<float>(maskResult, veca, vecb, mask); }
void _cmu418_vgt_int(__cmu418_mask &maskResult, __cmu418_vec_int &veca, __cmu418_vec_int &vecb, __cmu418_mask &mask) { _cmu418_vgt<int>(maskResult, veca, vecb, mask); }

template <typename T>
void _cmu418_vlt(__cmu418_mask &maskResult, __cmu418_vec<T> &veca, __cmu418_vec<T> &vecb, __cmu418_mask &mask) {
  for (int i=0; i<VECTOR_WIDTH; i++) {
    maskResult.value[i] = mask.value[i] ? (veca.value[i] < vecb.value[i]) : maskResult.value[i];
  }
  CMU418Logger.addLog("vlt", mask, VECTOR_WIDTH);
}

template void _cmu418_vlt<float>(__cmu418_mask &maskResult, __cmu418_vec_float &veca, __cmu418_vec_float &vecb, __cmu418_mask &mask);
template void _cmu418_vlt<int>(__cmu418_mask &maskResult, __cmu418_vec_int &veca, __cmu418_vec_int &vecb, __cmu418_mask &mask);

void _cmu418_vlt_float(__cmu418_mask &maskResult, __cmu418_vec_float &veca, __cmu418_vec_float &vecb, __cmu418_mask &mask) { _cmu418_vlt<float>(maskResult, veca, vecb, mask); }
void _cmu418_vlt_int(__cmu418_mask &maskResult, __cmu418_vec_int &veca, __cmu418_vec_int &vecb, __cmu418_mask &mask) { _cmu418_vlt<int>(maskResult, veca, vecb, mask); }

template <typename T>
void _cmu418_veq(__cmu418_mask &maskResult, __cmu418_vec<T> &veca, __cmu418_vec<T> &vecb, __cmu418_mask &mask) {
  for (int i=0; i<VECTOR_WIDTH; i++) {
    maskResult.value[i] = mask.value[i] ? (veca.value[i] == vecb.value[i]) : maskResult.value[i];
  }
  CMU418Logger.addLog("veq", mask, VECTOR_WIDTH);
}

template void _cmu418_veq<float>(__cmu418_mask &maskResult, __cmu418_vec_float &veca, __cmu418_vec_float &vecb, __cmu418_mask &mask);
template void _cmu418_veq<int>(__cmu418_mask &maskResult, __cmu418_vec_int &veca, __cmu418_vec_int &vecb, __cmu418_mask &mask);

void _cmu418_veq_float(__cmu418_mask &maskResult, __cmu418_vec_float &veca, __cmu418_vec_float &vecb, __cmu418_mask &mask) { _cmu418_veq<float>(maskResult, veca, vecb, mask); }
void _cmu418_veq_int(__cmu418_mask &maskResult, __cmu418_vec_int &veca, __cmu418_vec_int &vecb, __cmu418_mask &mask) { _cmu418_veq<int>(maskResult, veca, vecb, mask); }

template <typename T>
void _cmu418_hadd(__cmu418_vec<T> &vecResult, __cmu418_vec<T> &vec) {
  for (int i=0; i<VECTOR_WIDTH/2; i++) {
    T result = vec.value[2*i] + vec.value[2*i+1];
    vecResult.value[2 * i] = result;
    vecResult.value[2 * i + 1] = result;
  }
}

template void _cmu418_hadd<float>(__cmu418_vec_float &vecResult, __cmu418_vec_float &vec);

void _cmu418_hadd_float(__cmu418_vec_float &vecResult, __cmu418_vec_float &vec) { _cmu418_hadd<float>(vecResult, vec); }

template <typename T>
void _cmu418_interleave(__cmu418_vec<T> &vecResult, __cmu418_vec<T> &vec) {
  for (int i=0; i<VECTOR_WIDTH; i++) {
    int index = i < VECTOR_WIDTH/2 ? (2 * i) : (2 * (i - VECTOR_WIDTH/2) + 1);
    vecResult.value[i] = vec.value[index];
  }
}

template void _cmu418_interleave<float>(__cmu418_vec_float &vecResult, __cmu418_vec_float &vec);

void _cmu418_interleave_float(__cmu418_vec_float &vecResult, __cmu418_vec_float &vec) { _cmu418_interleave<float>(vecResult, vec); }

void addUserLog(const char * logStr) {
  CMU418Logger.addLog(logStr, _cmu418_init_ones(), 0);
}


void usage(const char* progname);
void initValue(float* values, int* exponents, float* output, float* gold, unsigned int N);
void absSerial(float* values, float* output, int N);
void absVector(float* values, float* output, int N);
void clampedExpSerial(float* values, int* exponents, float* output, int N);
void clampedExpVector(float* values, int* exponents, float* output, int N);
float arraySumSerial(float* values, int N);
float arraySumVector(float* values, int N);
bool verifyResult(float* values, int* exponents, float* output, float* gold, int N);

int main(int argc, char * argv[]) {
  int N = 16;
  bool printLog = false;

  // parse commandline options ////////////////////////////////////////////
  int opt;
  static struct option long_options[] = {
    {"size", 1, 0, 's'},
    {"log", 0, 0, 'l'},
    {"help", 0, 0, '?'},
    {0 ,0, 0, 0}
  };

  while ((opt = getopt_long(argc, argv, "s:l?", long_options, NULL)) != EOF) {

    switch (opt) {
      case 's':
        N = atoi(optarg);
        if (N <= 0) {
          printf("Error: Workload size is set to %d (<0).\n", N);
          return -1;
        }
        break;
      case 'l':
        printLog = true;
        break;
      case '?':
      default:
        usage(argv[0]);
        return 1;
    }
  }


  float* values = new float[N+VECTOR_WIDTH];
  int* exponents = new int[N+VECTOR_WIDTH];
  float* output = new float[N+VECTOR_WIDTH];
  float* gold = new float[N+VECTOR_WIDTH];
  initValue(values, exponents, output, gold, N);

  clampedExpSerial(values, exponents, gold, N);
  clampedExpVector(values, exponents, output, N);

  absSerial(values, gold, N);
  absVector(values, output, N);


  printf("\e[1;31mCLAMPED EXPONENT\e[0m (required) \n");
  bool clampedCorrect = verifyResult(values, exponents, output, gold, N);
  if (printLog) CMU418Logger.printLog();
  CMU418Logger.printStats();

  printf("************************ Result Verification *************************\n");
  if (!clampedCorrect) {
    printf("@@@ Failed!!!\n");
  } else {
    printf("Passed!!!\n");
  }

  printf("\n\e[1;31mARRAY SUM\e[0m (bonus) \n");
  if (N % VECTOR_WIDTH == 0) {
    float sumGold = arraySumSerial(values, N);
    float sumOutput = arraySumVector(values, N);
    float epsilon = 0.1;
    bool sumCorrect = abs(sumGold - sumOutput) < epsilon * 2;
    if (!sumCorrect) {
      printf("Expected %f, got %f\n.", sumGold, sumOutput);
      printf("@@@ Failed!!!\n");
    } else {
      printf("Passed!!!\n");
    }
  } else {
    printf("Must have N % VECTOR_WIDTH == 0 for this problem (VECTOR_WIDTH is %d)\n", VECTOR_WIDTH);
  }

  delete[] values;
  delete[] exponents;
  delete[] output;
  delete gold;

  return 0;
}

void usage(const char* progname) {
  printf("Usage: %s [options]\n", progname);
  printf("Program Options:\n");
  printf("  -s  --size <N>     Use workload size N (Default = 16)\n");
  printf("  -l  --log          Print vector unit execution log\n");
  printf("  -?  --help         This message\n");
}

void initValue(float* values, int* exponents, float* output, float* gold, unsigned int N) {

  for (unsigned int i=0; i<N+VECTOR_WIDTH; i++)
  {
    // random input values
    values[i] = -1.f + 4.f * static_cast<float>(rand()) / RAND_MAX;
    exponents[i] = rand() % EXP_MAX;
    output[i] = 0.f;
    gold[i] = 0.f;
  }

}

bool verifyResult(float* values, int* exponents, float* output, float* gold, int N) {
  int incorrect = -1;
  float epsilon = 0.00001;
  for (int i=0; i<N+VECTOR_WIDTH; i++) {
    if ( abs(output[i] - gold[i]) > epsilon ) {
      incorrect = i;
      break;
    }
  }

  if (incorrect != -1) {
    if (incorrect >= N)
      printf("You have written to out of bound value!\n");
    printf("Wrong calculation at value[%d]!\n", incorrect);
    printf("value  = ");
    for (int i=0; i<N; i++) {
      printf("% f ", values[i]);
    } printf("\n");

    printf("exp    = ");
    for (int i=0; i<N; i++) {
      printf("% 9d ", exponents[i]);
    } printf("\n");

    printf("output = ");
    for (int i=0; i<N; i++) {
      printf("% f ", output[i]);
    } printf("\n");

    printf("gold   = ");
    for (int i=0; i<N; i++) {
      printf("% f ", gold[i]);
    } printf("\n");
    return false;
  }
  printf("Results matched with answer!\n");
  return true;
}

void absSerial(float* values, float* output, int N) {
  for (int i=0; i<N; i++) {
    float x = values[i];
    if (x < 0) {
      output[i] = -x;
    } else {
      output[i] = x;
    }
  }
}

// implementation of absolute value using 15418 instrinsics
void absVector(float* values, float* output, int N) {
  __cmu418_vec_float x;
  __cmu418_vec_float result;
  __cmu418_vec_float zero = _cmu418_vset_float(0.f);
  __cmu418_mask maskAll, maskIsNegative, maskIsNotNegative;

//  Note: Take a careful look at this loop indexing.  This example
//  code is not guaranteed to work when (N % VECTOR_WIDTH) != 0.
//  Why is that the case?
  for (int i=0; i<N; i+=VECTOR_WIDTH) {
    // All ones
    maskAll = _cmu418_init_ones();

    // All zeros
    maskIsNegative = _cmu418_init_ones(0);

    // Load vector of values from contiguous memory addresses
    _cmu418_vload_float(x, values+i, maskAll);               // x = values[i];
	
    // Set mask according to predicate
    _cmu418_vlt_float(maskIsNegative, x, zero, maskAll);     // if (x < 0) {

    // Execute instruction using mask ("if" clause)
    _cmu418_vsub_float(result, zero, x, maskIsNegative);      //   output[i] = -x;

    // Inverse maskIsNegative to generate "else" mask
    maskIsNotNegative = _cmu418_mask_not(maskIsNegative);     // } else {

    // Execute instruction ("else" clause)
    _cmu418_vload_float(result, values+i, maskIsNotNegative); //   output[i] = x; }
	
    // Write results back to memory
    _cmu418_vstore_float(output+i, result, maskAll);
	
  }
}

// accepts and array of values and an array of exponents
//
// For each element, compute values[i]^exponents[i] and clamp value to
// 9.999.  Store result in outputs.
void clampedExpSerial(float* values, int* exponents, float* output, int N) {
  for (int i=0; i<N; i++) {
    float x = values[i];
    int y = exponents[i];
    if (y == 0) {
      output[i] = 1.f;
    } else {
      float result = x;
      int count = y - 1;
      while (count > 0) {
        result *= x;
        count--;
      }
      if (result > 9.999999f) {
        result = 9.999999f;
      }
      output[i] = result;
    }
  }
}

void clampedExpVector(float* values, int* exponents, float* output, int N) {
  // Implement my vectorized version of clampedExpSerial here
  __cmu418_vec_float x;
  __cmu418_vec_int y;
  __cmu418_vec_int count;
  __cmu418_vec_float result;
  __cmu418_vec_int zero = _cmu418_vset_int(0);
  __cmu418_vec_int intOne = _cmu418_vset_int(1);
  __cmu418_vec_float floatOne = _cmu418_vset_float(1.f);
  __cmu418_vec_float clampVal = _cmu418_vset_float(9.999999f);
  __cmu418_mask maskAll, maskIsZero, maskIsNotZero, maskAllZero, maskIsExceed;

  for (int i=0; i<N; i+=VECTOR_WIDTH) {
    // All ones
    maskAll = _cmu418_init_ones();

    // Load vector of values from contiguous memory addresses
    _cmu418_vload_float(x, values+i, maskAll);                        // x = values[i];
    // Load vector of exponents from contiguous memory addresses
    _cmu418_vload_int(y, exponents+i, maskAll);                       // y = exponents[i];

    // Set mask according to predicate
    _cmu418_veq_int(maskIsZero, y, zero, maskAll);                    // if (y == 0) {

    // Execute instruction using mask ("if" clause)
    _cmu418_vload_float(result, floatOne.value, maskIsZero);          //   output[i] = 1.f;

    // Inverse maskIsNegative to generate "else" mask
    maskIsNotZero = _cmu418_mask_not(maskIsZero);                     // } else {

    // Execute instruction using mask ("else" clause)
    _cmu418_vload_float(result, values+i, maskIsNotZero);             //   float result = x;
    _cmu418_vsub_int(count, y, intOne, maskIsNotZero);                //   int count = y - 1;
    _cmu418_veq_int(maskAllZero, count, zero, maskIsNotZero);
    while (_cmu418_cntbits(maskAllZero)) {                            //   while (count > 0) {
      _cmu418_vmult_float(result, result, x, maskIsNotZero);          //     result *= x;
      _cmu418_vsub_int(count, count, intOne, maskIsNotZero);          //     count--;
      _cmu418_veq_int(maskAllZero, count, zero, maskIsNotZero);
    }                                                                 //   }

    _cmu418_vgt_float(maskIsExceed, result, clampVal, maskIsNotZero); //   if (result > 9.999999f) {
    _cmu418_vload_float(result, clampVal.value, maskIsExceed);        //     result = 9.999999f; }

    // Write results back to memory
    _cmu418_vstore_float(output+i, result, maskAll);                  //   output[i] = result;
    
  }
}

float arraySumSerial(float* values, int N) {
  float sum = 0;
  for (int i=0; i<N; i++) {
    sum += values[i];
  }

  return sum;
}

// Assume N is a power VECTOR_WIDTH == 0
// Assume VECTOR_WIDTH is a power of 2
float arraySumVector(float* values, int N) {
  // Implement my vectorized version of arraySumSerial here
  __cmu418_vec_float currentVec;
  __cmu418_vec_float tmpResult1;
  __cmu418_vec_float tmpResult2;
  __cmu418_vec_float result;
  __cmu418_mask maskAll, maskAdd;

  for (int i=0; i<N; i+=VECTOR_WIDTH) {
    maskAll = _cmu418_init_ones();
    _cmu418_vload_float(currentVec, values+i, maskAll);

    _cmu418_hadd_float(tmpResult1, currentVec);
    _cmu418_interleave_float(tmpResult2, tmpResult1);

    maskAdd = _cmu418_init_ones(VECTOR_WIDTH / 2);
    if (i != 0) { maskAdd = _cmu418_mask_not(maskAdd); }
    _cmu418_vmove_float(result, tmpResult2, maskAdd);
    if (i != 0) {
      _cmu418_hadd_float(tmpResult1, result);
      _cmu418_interleave_float(result, tmpResult1);
    }
  }

  for (int i = VECTOR_WIDTH / 2; i > 1; i /= 2) {
    _cmu418_hadd_float(tmpResult1, result);
    _cmu418_interleave_float(result, tmpResult1);
  }

  return result.value[0];
}