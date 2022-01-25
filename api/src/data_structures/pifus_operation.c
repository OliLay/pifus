/* local includes */
#include "data_structures/pifus_operation.h"

#define C(k, v) [v] = #k,    
const char * const operation_name[] = { OPERATIONS };

const char* operation_str(enum operation_code operation_code) {
    return operation_name[operation_code];
}

bool is_tcp_operation(struct pifus_operation* operation) {
    return operation->op < UDP_BIND;
}