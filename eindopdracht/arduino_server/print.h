// *.h file
// ...
#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC void print(int number);

#undef EXTERNC