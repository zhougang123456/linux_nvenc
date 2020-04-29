#include "capture.hpp"
int main(){
    Capture* capture = new Capture();
    capture->init();
    while (1){
        capture->do_capture();
    }
    return 0;
}
