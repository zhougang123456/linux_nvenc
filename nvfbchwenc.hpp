#ifndef NVFBCHWENC_HPP
#define NVFBCHWENC_HPP
#include "nvfbc.hpp"
#include "nvfbcutils.hpp"
class NvFBCHwEnc
{
public:
    NvFBCHwEnc();
    ~NvFBCHwEnc();
    bool init();
    void gpu_capture();
private:
    NVFBC_DESTROY_CAPTURE_SESSION_PARAMS destroyCaptureParams;
    NVFBC_DESTROY_HANDLE_PARAMS destroyHandleParams;
    NVFBCSTATUS fbcStatus;
    NVFBC_API_FUNCTION_LIST pFn;
    NVFBC_SESSION_HANDLE fbcHandle;
    NVFBC_CREATE_HANDLE_PARAMS createHandleParams;
    NVFBC_GET_STATUS_PARAMS statusParams;
    NVFBC_CREATE_CAPTURE_SESSION_PARAMS createCaptureParams;
    NVFBC_HWENC_CONFIG encoderConfig;
    NVFBC_TOHWENC_SETUP_PARAMS setupParams;
};
#endif // NVFBCHWENC_HPP
