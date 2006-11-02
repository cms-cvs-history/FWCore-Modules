//----------------------------------------------------------------------
// EdmCollUtil.cpp
//
// $Id: EdmCollUtil.cpp,v 1.5 2006/08/24 19:27:17 elmer Exp $
//
// Author: Chih-hsiang Cheng, LLNL
//         Chih-Hsiang.Cheng@cern.ch
//
// March 13, 2006
//

#include <exception>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <boost/program_options.hpp>
#include "FWCore/Modules/bin/CollUtil.h"
#include "DataFormats/Common/interface/ParameterSetBlob.h"
#include "FWCore/FWLite/src/AutoLibraryLoader.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Framework/interface/FileCatalog.h"
#include "IOPool/Common/interface/PoolDataSvc.h"
#include "Cintex/Cintex.h"
#include "FWCore/Utilities/interface/Exception.h"
#include "FWCore/Utilities/interface/ProblemTracker.h"
#include "FWCore/Utilities/interface/Presence.h"
#include "FWCore/Utilities/interface/PresenceFactory.h"
#include "FWCore/ServiceRegistry/interface/ServiceRegistry.h"

#include "TFile.h"

int main(int argc, char* argv[]) {

  // Add options here

  boost::program_options::options_description desc("Allowed options");
  desc.add_options()
    ("help,h", "print help message")
    ("file,f", boost::program_options::value<std::vector<std::string> >(), "data file (Required)")
    ("catalog,c", boost::program_options::value<std::string>(), "catalog")
    ("ls,l", "list file content")
    ("print,P", "Print all")
    ("uuid,u", "Print uuid")
    ("verbose,v","Verbose printout")
    ("allowRecovery","Allow root to auto-recover corrupted files") 
    ("events,e",boost::program_options::value<std::string>(), 
     "Show event ids for events within a range or set of ranges , e.g., 5-13,30,60-90 ");

  // What trees do we require for this to be a valid collection?
  std::vector<std::string> expectedTrees;
  expectedTrees.push_back("MetaData");
  expectedTrees.push_back("##Params");
  expectedTrees.push_back("Events");


  boost::program_options::positional_options_description p;
  p.add("file", -1);


  boost::program_options::variables_map vm;

  boost::program_options::store(boost::program_options::command_line_parser(argc, argv).
          options(desc).positional(p).run(), vm);

  boost::program_options::notify(vm);    

  bool verbose= vm.count("verbose") > 0 ? true : false;

  if (vm.count("help")) {
    std::cout << desc << "\n";
    return 1;
  }

  if (!vm.count("file")) {
    std::cout << "Data file not set.\n";
    std::cout << desc << "\n";
    return 1;
  }

  //dl  std::string datafile = vm["file"].as<std::string>(); 

  AutoLibraryLoader::enable();

  int rc = 0;
  try {
    std::string config =
      "process EdmCollUtil = {";
    config += "service = SiteLocalConfigService{}"
      "}";

    //create the services
    edm::ServiceToken tempToken = edm::ServiceRegistry::createServicesFromConfig(config);

    //make the services available
    edm::ServiceRegistry::Operate operate(tempToken);

    // now run..
    edm::ParameterSet pset;
    std::vector<std::string> in = vm["file"].as<std::vector<std::string> >();
    std::string catalogIn = (vm.count("catalog") ? vm["catalog"].as<std::string>() : std::string());
    
    std::cout << in[0] << "\n"; 
    pset.addUntrackedParameter<std::vector<std::string> >("fileNames", in);
    pset.addUntrackedParameter<std::string>("catalog", catalogIn);
    
    edm::InputFileCatalog catalog(pset);
    std::vector<std::string> const& filesIn = catalog.fileNames();
    
    // open a data file
    std::string datafile=filesIn[0];
    TFile *tfile= edm::openFileHdl(datafile);
    
    
    
    if ( tfile == 0 ) return 1;
    if ( verbose ) std::cout << "ECU:: Opened " << datafile << std::endl;
    
    // First check that this file is not auto-recovered
    // Stop the job unless specified to do otherwise
    
    bool isRecovered = tfile->TestBit(TFile::kRecovered);
    if ( isRecovered ) {
      std::cout << datafile << " appears not to have been closed correctly and has been autorecovered \n";
      if ( vm.count("allowRecovery") ) {
	std::cout << "Proceeding anyway\n";
      }
      else{
	std::cout << "Stopping. Use --allowRecovery to try ignoring this\n";
	return 1;
      }
    }
    else{
      if ( verbose ) std::cout << "ECU:: Collection not autorecovered. Continuing\n";
    }
    
    // Ok. Do we have the expected trees?
    for ( unsigned int i=0; i<expectedTrees.size(); i++) {
      TTree *t= (TTree*) tfile->Get(expectedTrees[i].c_str());
      if ( t==0 ) {
	std::cout << "Tree " << expectedTrees[i] << " appears to be missing. Not a valid collection\n";
	std::cout << "Exiting\n";
	return 1;
      }
      else{
	if ( verbose ) std::cout << "ECU:: Found Tree " << expectedTrees[i] << std::endl;
      }
    }
    
    if ( verbose ) std::cout << "ECU:: Found all expected trees\n"; 
    
    // Ok. How many events?
    long int nevts= edm::numEntries(tfile,"Events");
    std::cout << tfile->GetName() << " ( " << nevts << " events, " 
	      << tfile->GetSize() << " bytes )" << std::endl;
    
    // Look at the collection contents
    if ( vm.count("ls")) {
      if ( tfile != 0 ) tfile->ls();
    }
    
    // Print out each tree
    if ( vm.count("print") ) {
      edm::printTrees(tfile);
    }

    if ( vm.count("uuid") ) {
      TTree *paramsTree=(TTree*)tfile->Get("##Params");

      char uuidCA[1024];
      paramsTree->SetBranchAddress("db_string",uuidCA);
      paramsTree->GetEntry(0);

      // Then pick out relevent piece of this string
      // 9A440868-8058-DB11-85E3-00304885AB94 from
      // [NAME=FID][VALUE=9A440868-8058-DB11-85E3-00304885AB94]

      std::string uuidStr(uuidCA);
      std::string::size_type start=uuidStr.find("VALUE=");
      if ( start == std::string::npos ) {
	std::cout << "Seemingly invalid db_string entry in ##Params tree?\n";
	std::cout << uuidStr << std::endl;
      }
      else{
	std::string::size_type stop=uuidStr.find("]",start);
	if ( stop == std::string::npos ) {
	  std::cout << "Seemingly invalid db_string entry in ##Params tree?\n";
	  std::cout << uuidStr << std::endl;
	}
	else{
	  //Everything is Ok - just proceed...
	  std::string result=uuidStr.substr(start+6,stop-start-6);
	  std::cout << "UUID: " << result << std::endl;
	}
      }
    }
    
    // Print out event lists 
    if ( vm.count("events") ) {
      bool keepgoing=true;  
      std::string remainingStr=vm["events"].as<std::string>();
      while ( keepgoing ) {
	long int iLow(-1),iHigh(-2);
	// split by commas
	std::string::size_type pos= remainingStr.find_first_of(",");
	std::string evtstr=remainingStr;
	
	if ( pos == std::string::npos ) {
	  keepgoing=false;
	}
	else{
	  evtstr=remainingStr.substr(0,pos);
	  remainingStr=remainingStr.substr(pos+1);
	}
	
	pos= evtstr.find_first_of("-");
	if ( pos == std::string::npos ) {
	  iLow= (int)atof(evtstr.c_str());
	  iHigh= iLow;
	} else {
	  iLow= (int)atof(evtstr.substr(0,pos).c_str());
	  iHigh= (int)atof(evtstr.substr(pos+1).c_str());
	}
	
	//    edm::showEvents(tfile,"Events",vm["events"].as<std::string>());
	if ( iLow < 1 ) iLow=1;
	if ( iHigh > nevts ) iHigh=nevts;
	
	// shift by one.. C++ starts at 0
	iLow--;
	iHigh--;
	edm::showEvents(tfile,"Events",iLow,iHigh);
      }
    }
    
  }
  catch (cms::Exception& e) {
    std::cout << "cms::Exception caught in "
              <<"EdmCollUtil"
              << '\n'
              << e.explainSelf();
    rc = 1;
  }
  catch (seal::Error& e) {
    std::cout << "Exception caught in "
              << "EdmCollUtil"
              << '\n'
              << e.explainSelf();
    rc = 1;
  }
  catch (std::exception& e) {
    std::cout << "Standard library exception caught in "
              << "EdmCollUtil"
              << '\n'
              << e.what();
    rc = 1;
  }
  catch (...) {
    std::cout << "Unknown exception caught in "
              << "EdmCollUtil";
    rc = 2;
  }



  

  return 0;

}

