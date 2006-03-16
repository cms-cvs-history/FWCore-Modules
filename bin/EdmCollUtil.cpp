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

int main(int argc, char* argv[]) {

  boost::program_options::options_description desc("Allowed options");
  desc.add_options()
    ("help,h", "print help message")
    ("file,f", boost::program_options::value<std::string>(), "data file (Required)")
    ("ls,l", "list file content")
    ("print,P", "Print all")
    ("events,e",boost::program_options::value<std::string>(), 
     "Show event ids for events within a range, e.g., 5-13 ");


  boost::program_options::positional_options_description p;
  p.add("file", -1);


  boost::program_options::variables_map vm;

  boost::program_options::store(boost::program_options::command_line_parser(argc, argv).
          options(desc).positional(p).run(), vm);

  boost::program_options::notify(vm);    

  if (vm.count("help")) {
    std::cout << desc << "\n";
    return 1;
  }

  if (!vm.count("file")) {
    std::cout << "data file not set.\n";
    std::cout << desc << "\n";
    return 1;
  }

  std::string datafile = vm["file"].as<std::string>(); 

  AutoLibraryLoader::enable();

  // open a data file
  TFile *tfile= edm::openFileHdl(datafile);
  if ( !tfile ) return 1;
  long int nevts= edm::numEntries(tfile,"Events");
  std::cout << tfile->GetName() << " (" << nevts << " events, " 
	    << tfile->GetSize() << "bytes)" << std::endl;

  if ( vm.count("ls")) {
    if ( tfile ) tfile->ls();
  }

  if ( vm.count("print") ) {
    edm::printTrees(tfile);
  }
  


  if ( vm.count("events") ) {
    long int iLow(-1),iHigh(-2);
    std::string evtstr=vm["events"].as<std::string>();
    std::string::size_type pos= evtstr.find_first_of("-");
    if ( pos == std::string::npos ) {
      iLow= (int)atof(evtstr.c_str());
      iHigh= iLow;
    } else {
      iLow= (int)atof(evtstr.substr(0,pos).c_str());
      iHigh= (int)atof(evtstr.substr(pos+1).c_str());
    }
    
    //    edm::showEvents(tfile,"Events",vm["events"].as<std::string>());
    edm::showEvents(tfile,"Events",iLow,iHigh);

  }

  return 0;

}

