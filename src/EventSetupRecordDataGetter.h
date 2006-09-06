#ifndef Modules_EventSetupRecordDataGetter_h
#define Modules_EventSetupRecordDataGetter_h
// -*- C++ -*-
//
// Package:     Modules
// Class  :     EventSetupRecordDataGetter
// 
/**\class EventSetupRecordDataGetter EventSetupRecordDataGetter.h FWCore/Modules/interface/EventSetupRecordDataGetter.h

 Description: Can be configured to 'get' any Data in any EventSetup Record.  Primarily used for testing.

 Usage:
    <usage>

*/
//
// Original Author:  Chris Jones
//         Created:  Tue Jun 28 13:45:06 EDT 2005
// $Id: EventSetupRecordDataGetter.h,v 1.1 2005/10/11 17:09:22 wmtan Exp $
//

// system include files
#include <map>
#include <vector>
#include <boost/shared_ptr.hpp>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "FWCore/Framework/interface/EventSetupRecord.h"
#include "FWCore/Framework/interface/IOVSyncValue.h"

// forward declarations
namespace edm {
   class EventSetupRecordDataGetter : public edm::EDAnalyzer {
public:
      explicit EventSetupRecordDataGetter(const edm::ParameterSet&);
      ~EventSetupRecordDataGetter();
      
      
      virtual void analyze(const edm::Event&, const edm::EventSetup&);
private:
         // ----------member data ---------------------------
      ParameterSet pSet_;
      
      typedef std::map<edm::eventsetup::EventSetupRecordKey, std::vector<edm::eventsetup::DataKey> > RecordToDataKeys;
      RecordToDataKeys recordToDataKeys_;
      std::map<eventsetup::EventSetupRecordKey, unsigned long long > recordToCacheIdentifier_;
      bool verbose_;
   };
}


#endif