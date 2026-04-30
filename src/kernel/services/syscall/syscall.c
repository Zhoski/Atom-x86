#include "syscall.h"

void syscall_handler(int eax, int ebx,int ecx, int edx) {
    /*
     * kwrite_string("---SYSCALL---");
     * kwrite_string("\nEAX  (num): ");
     * kwrite_int(eax);
     * kwrite_string("\nEBX (arg1): ");
     * kwrite_int(ebx);
     * kwrite_string("\nECX (arg2): ");
     * kwrite_int(ecx);
     * kwrite_string("\nEDX (arg3): ");
     * kwrite_int(edx);
     * kwrite_string("\n"); 
    */
    switch(eax) {
        case SYSCALL_WRITE:  
            switch(ebx) {
                case WRITE_TEXT:
                    kwrite_string((uint8_t*)ecx); 
                    break;
                case WRITE_INT:
                    kwrite_int(ecx);
                    break;
            } 
            break;
        default:
            break;
    } 
}
