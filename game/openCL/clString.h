

//make a string as a local variable (non constant)
#define LOCAL_STR(STRNAME, STR) int STRNAME##_len=sizeof(STR); char STRNAME[sizeof(STR)+1] = STR;  STRNAME[sizeof(STR)] = '\0';  
#define LOCAL_STRL(STRNAME, STR, STRLEN) int STRLEN=sizeof(STR);  char STRNAME[sizeof(STR)+1] = STR;  STRNAME[sizeof(STR)] = '\0';  


// #define LOCAL_STR_INT(STRNAME, VALUE, STRLEN) \
//  int STRLEN=CL_DIGITS_FOR_VALUE(VALUE, 10); \
//  char STRNAME[STRLEN+1]; \
//  CL_ITOA(VALUE, STRNAME, STRLEN, 10);  \
//  STRNAME[STRLEN] = '\0';  





// int CL_DIGITS_FOR_VALUE(int value, int base)
// {
//     int nDigits = 0;
//     int origValue = value;
//     while(value)
//     {
//         nDigits++;
//         value /= base;
//     }
//     if(origValue < 0)
//         return nDigits+1;
//     else 
//         return nDigits;
// }

char* CL_ITOA(int value, char* result, int resultSize, int base) {

    char* ptr = result, *ptr1 = result, tmp_char;
    int tmp_value;
    int counter = 0;
    do {
        tmp_value = value;
        value /= base;
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
        counter++;

    } while ( value && counter < resultSize );


    //return if out of bounds of result
    if(counter >= resultSize)
    {
        printf("ERROR: CL_ITOA Overflow\n");
        return result;
    }

    
    // Apply negative sign
    if (tmp_value < 0) 
        *ptr++ = '-';

    *ptr-- = '\0';
    while(ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr--= *ptr1;
        *ptr1++ = tmp_char;
    }

    return result;
}














