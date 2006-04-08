//----------------------------------------------------------------------
// EdmCollUtil.cpp
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
#include "PhysicsTools/FWLite/src/AutoLibraryLoader.h"
#include "TFile.h"
#include "TUUID.h"

int main(int argc, char* argv[]) {

  // Add options here

  boost::program_options::options_description desc("Allowed options");
  desc.add_options()
    ("help,h", "print help message")
    ("file,f", boost::program_options::value<std::string>(), "data file (Required)")
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
  expectedTrees.push_back("ParameterSets");
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

  std::string datafile = vm["file"].as<std::string>(); 

  AutoLibraryLoader::enable();

  // open a data file
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
    if ( tfile ) tfile->ls();
  }

  // Print out each tree
  if ( vm.count("print") ) {
    edm::printTrees(tfile);
  }
  
  if ( vm.count("uuid") ) {
    TUUID uuid=tfile->GetUUID();
    std::cout << "TFile UUID: ";
    uuid.Print();
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

  return 0;

}

