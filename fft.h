

#include <ipps.h>
#include <ippdefs.h>
#include <stdexcept>

void ipp_fft_init(ipp_ptr<IppsFFTSpec_R_32f> &spec, ipp_ptr<Ipp8u> &buff, const Ipp32s length)
{
    Ipp32s order = Ipp32s(log2(Ipp64f(length)));
    Ipp32s specinit_sz = 0, specbuff_sz = 0, buff_sz = 0;
    
    IppStatus message = ippsFFTGetSize_R_32f(order, IPP_FFT_NODIV_BY_ANY, ippAlgHintFast, &specinit_sz, &specbuff_sz, &buff_sz);
    if(message != ippStsNoErr) throw std::runtime_error(ippGetStatusString(message));
    
    buff.reset(ippsMalloc_8u(buff_sz));
    
    ipp_ptr<Ipp8u> specInit(ippsMalloc_8u(specinit_sz));
    ipp_ptr<Ipp8u> specBuff(ippsMalloc_8u(specbuff_sz));
    
    IppsFFTSpec_R_32f *_spec = 0;
    
    message = ippsFFTInit_R_32f(&_spec, order, IPP_FFT_NODIV_BY_ANY, ippAlgHintFast, specInit.release(), specBuff.get());
    if(message != ippStsNoErr) throw std::runtime_error(ippGetStatusString(message));
    
    spec.reset(_spec);
}


inline void ipp_fft_fwd_r2ccs(const real32 *pSrc, real32 *pDst,
                              const IppsFFTSpec_R_32f *pSpec, Ipp8u *pBuffer)
{
    IppStatus message = ippsFFTFwd_RToCCS_32f(pSrc, pDst, pSpec, pBuffer);
    if(message != ippStsNoErr) throw std::runtime_error(ippGetStatusString(message));
}


inline void ipp_fft_inv_ccs2r(const Ipp32f *pSrc, Ipp32f *pDst,
                              const IppsFFTSpec_R_32f *pSpec, Ipp8u *pBuffer)
{
    IppStatus message = ippsFFTInv_CCSToR_32f(pSrc, pDst, pSpec, pBuffer);
    if(message != ippStsNoErr) throw std::runtime_error(ippGetStatusString(message));
}