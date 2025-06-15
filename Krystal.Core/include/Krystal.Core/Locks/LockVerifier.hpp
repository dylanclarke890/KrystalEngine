#pragma once  

namespace Krys  
{  
  class LockVerifier  
  {  
  private:  
    volatile bool _locked;  

  public:  
    LockVerifier() noexcept;  
    void Acquire() noexcept;  
    void Release() noexcept;  
  };  

#ifdef KRYS_ENABLE_ASSERTS  
  #define KRYS_SCOPED_LOCK_VERIFIER(lock) ScopedLock<LockVerifier> scopedLockVerifier##__LINE__(lock)  
#else  
  #define KRYS_SCOPED_LOCK_VERIFIER(lockVerifier)  
#endif  
}