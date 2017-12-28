#define LEN(arr) ((int) (sizeof (arr) / sizeof (arr)[0]))
enum Col {
  Id = 0, Data0, Data1, Data2,
  
  #ifdef USE_DIAGNOSTICS
    Time, SampleLength, InterruptLength, 
  #endif
  
  NCOLS
};
const int NWORDS = Col::NCOLS;
const int WSIZE = sizeof(uint16_t);
const int NBYTES = NWORDS * WSIZE;
typedef union { uint16_t words[NWORDS]; uint8_t bytes[NBYTES]; } MODEL;

