/*----------------------------------------------------------------------
$Id: AsciiOutputModule.h,v 1.1 2005/10/11 17:09:22 wmtan Exp $
----------------------------------------------------------------------*/

#include <iostream>

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/OutputModule.h"

namespace edm {
  class ParameterSet;
  class AsciiOutputModule : public OutputModule {
  public:
    // We do not take ownership of passed stream.
    explicit AsciiOutputModule(ParameterSet const& pset, std::ostream* os = &std::cout);
    virtual ~AsciiOutputModule();
    virtual void write(const EventPrincipal& e);

  private:
    int prescale_;
    int verbosity_;
    int counter_;
    std::ostream* pout_;
  };
}