// -*- C++ -*-
//
// Package:    EventSetupRecordDataGetter
// Class:      EventSetupRecordDataGetter
// 
/**\class EventSetupRecordDataGetter EventSetupRecordDataGetter.cc src/EventSetupRecordDataGetter/src/EventSetupRecordDataGetter.cc

 Description: Can be configured to 'get' any Data in any EventSetup Record.  Primarily used for testing.

 Implementation:
     <Notes on implementation>
*/
//
// Original Author:  Chris Jones
//         Created:  Tue Jun 28 11:10:24 EDT 2005
// $Id: EventSetupRecordDataGetter.cc,v 1.3 2006/08/26 18:41:19 chrjones Exp $
//
//


// system include files
#include <memory>
#include <map>
#include <vector>
#include <iostream>

// user include files
#include "FWCore/Modules/src/EventSetupRecordDataGetter.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/EventSetupRecord.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"


//
// class decleration
//
namespace edm {
//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//
   EventSetupRecordDataGetter::EventSetupRecordDataGetter(const edm::ParameterSet& iConfig):
   pSet_(iConfig),
   recordToDataKeys_(),
   recordToCacheIdentifier_(),
   verbose_(iConfig.getUntrackedParameter("verbose",false))
{
}

EventSetupRecordDataGetter::~EventSetupRecordDataGetter()
{
 
   // do anything here that needs to be done at desctruction time
   // (e.g. close files, deallocate resources etc.)

}


//
// member functions
//

// ------------ method called to produce the data  ------------
void
EventSetupRecordDataGetter::analyze(const edm::Event& /*iEvent*/, const edm::EventSetup& iSetup)
{
   if(0 == recordToDataKeys_.size()) {
      typedef std::vector< ParameterSet > Parameters;
      Parameters toGet = pSet_.getParameter<Parameters>("toGet");
      
      for(Parameters::iterator itToGet = toGet.begin(); itToGet != toGet.end(); ++itToGet) {
         std::string recordName = itToGet->getParameter<std::string>("record");
         
         eventsetup::EventSetupRecordKey recordKey(eventsetup::EventSetupRecordKey::TypeTag::findType(recordName));
         if(recordKey.type() == eventsetup::EventSetupRecordKey::TypeTag()) {
            //record not found
            std::cout <<"Record \""<< recordName <<"\" does not exist "<<std::endl;
            continue;
         }
         typedef std::vector< std::string > Strings;
         Strings dataNames = itToGet->getParameter< Strings >("data");
         std::vector< eventsetup::DataKey > dataKeys;
         for(Strings::iterator itDatum = dataNames.begin(); itDatum != dataNames.end(); ++itDatum) {
            std::string datumName(*itDatum, 0, itDatum->find_first_of("/"));
            std::string labelName;
            if(itDatum->size() != datumName.size()) {
               labelName = std::string(*itDatum, datumName.size()+1);
            }
            eventsetup::TypeTag datumType = eventsetup::TypeTag::findType(datumName);
            if(datumType == eventsetup::TypeTag()) {
               //not found
               std::cout <<"data item of type \""<< datumName <<"\" does not exist"<<std::endl;
               continue;
            }
            eventsetup::DataKey datumKey(datumType, labelName.c_str());
            dataKeys.push_back(datumKey); 
         }
         recordToDataKeys_.insert(std::make_pair(recordKey, dataKeys));
         recordToCacheIdentifier_.insert(std::make_pair(recordKey, 0));
      }
   }
   
   using namespace edm;
   using namespace edm::eventsetup;

   //For each requested Record get the requested data only if the Record is in a new IOV
   
   for(RecordToDataKeys::iterator itRecord = recordToDataKeys_.begin();
        itRecord != recordToDataKeys_.end();
        ++itRecord) {
      const EventSetupRecord* pRecord = iSetup.find(itRecord->first);
      
      if(0 != pRecord && pRecord->cacheIdentifier() != recordToCacheIdentifier_[itRecord->first]) {
         recordToCacheIdentifier_[itRecord->first] = pRecord->cacheIdentifier();
         typedef std::vector< DataKey > Keys;
         const Keys& keys = itRecord->second;
         for(Keys::const_iterator itKey = keys.begin();
              itKey != keys.end();
              ++itKey) {
            if(! pRecord->doGet(*itKey)) {
               std::cout << "No data of type \""<<itKey->type().name() <<"\" with name \""<< itKey->name().value()<<"\" in record "<<itRecord->first.type().name() <<" found "<< std::endl;
            } else {
               if(verbose_) {
                  std::cout << "got data of type \""<<itKey->type().name() <<"\" with name \""<< itKey->name().value()<<"\" in record "<<itRecord->first.type().name() << std::endl;
               }
            }
         }
      }
   }
}
}
//define this as a plug-in
//DEFINE_FWK_MODULE(EventSetupRecordDataGetter)