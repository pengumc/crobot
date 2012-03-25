/**
 * @file Report.c
 */

#include "Report.h"


/*================================= REPORT STD =============================*/
/** notify the user of something.
 * @param text The text you want the user to see.
 */
void Report_std(const char* text){
    printf("%s\n", text);
}


/*================================= REPORT ERR =============================*/
/** notify the user of an error.
 * @param text The text you want the user to see.
 */
void Report_err(const char* text){
    fprintf(stderr, "%s\n", text);

}
