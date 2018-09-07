#include "MCParticleSelection.h"

MCParticleSelection aMCParticleSelection;


MCParticleSelection::MCParticleSelection() : Processor("MCParticleSelection") {


  // register steering parameters: name, description, class-variable, default value

	registerProcessorParameter( "Printing" ,
	                            "Print certain messages"  ,
	                             _printing,
	                             (int)5 ) ;

	std::vector<int> pdgs{};
	pdgs.push_back(0);
	registerProcessorParameter("Pdgs",
				   "PDG codes of particles to be picked out",
				    _pdgs,
				    pdgs);

	

   	std::string inputTrackCollectionName = "x";
  	registerInputCollection( LCIO::TRACK,
                                 "InputTrackCollectionName" ,
                                 "Input Track Collection Name " ,
                                 _inputTrackCollectionName,
                                 inputTrackCollectionName);  

	std::string inputMcParticleCollectionName = "x";
	registerInputCollection( LCIO::MCPARTICLE,
				"McParticleCollectionName" ,
				"Name of the MCParticle input collection" ,
				_inputMcParticleCollectionName,
				inputMcParticleCollectionName);

  	std::string outputParticleCollectionName = "x";
  	registerOutputCollection( LCIO::RECONSTRUCTEDPARTICLE,
                             	"OutputParticleCollectionName" ,
			     	"Output Particle Collection Name "  ,
                             	_outputParticleCollectionName,
                             	outputParticleCollectionName);

}

void MCParticleSelection::init() {

  streamlog_out(DEBUG) << "   init called  " << std::endl;
                   
  // usually a good idea to
  printParameters() ;
  nEvt = 0;
 
  BField = marlin::Global::GEAR->getBField().at(gear::Vector3D(0.,0.,0.)).z();

}

void MCParticleSelection::processRunHeader( LCRunHeader* run) {
  streamlog_out(MESSAGE) << " processRunHeader "  << run->getRunNumber() << std::endl ;
}

bool MCParticleSelection::FindTracks( LCEvent* evt ) {

  bool tf = false;

  // clear old vector
  _trackvec.clear();
  typedef const std::vector<std::string> StringVec ;
  StringVec* strVec = evt->getCollectionNames() ;
  for(StringVec::const_iterator itname=strVec->begin(); itname!=strVec->end(); itname++){
    if(*itname==_inputTrackCollectionName){
      LCCollection* col = evt->getCollection(*itname);
      unsigned int nelem = col->getNumberOfElements();
      tf = true;
      for(unsigned int i=0;i<nelem;i++){
        Track* track = dynamic_cast<Track*>(col->getElementAt(i));
        _trackvec.push_back(track);
      }
    }
  }

  if(_printing>1)std::cout << "FindTracks : " << tf << std::endl;

  return tf;
}
bool MCParticleSelection::FindMCParticles( LCEvent* evt ){
   
	bool collectionFound = false;

  	// clear old global MCParticle vector
  	_mcpartvec.clear();
  	typedef const std::vector<std::string> StringVec ;
  	StringVec* strVec = evt->getCollectionNames() ;

	//iterate over collections, find the matching name
  	for(StringVec::const_iterator itname=strVec->begin(); itname!=strVec->end(); itname++){    
    
		//if found print name and number of elements 
		if(*itname==_inputMcParticleCollectionName){
      			LCCollection* collection = evt->getCollection(*itname);
     			std::cout<< "Located MC Collection "<< *itname<< " with "<< collection->getNumberOfElements() << " elements " <<std::endl;
      			collectionFound = true;
      
			//add the collection elements to the global vector
			for(int i=0;i<collection->getNumberOfElements();i++){
				MCParticle* mcPart = dynamic_cast<MCParticle*>(collection->getElementAt(i));
				_mcpartvec.push_back(mcPart);

       
      			}
    		}
  	}

  	if(!collectionFound){
		std::cout<<"MC Collection "<< _inputMcParticleCollectionName << "not found"<<std::endl;
	}
  
  	return collectionFound;
}

void MCParticleSelection::processEvent( LCEvent * evt ) {
 FindMCParticles(evt);


  
  
 // LCCollectionVec * partCollection = new LCCollectionVec(LCIO::RECONSTRUCTEDPARTICLE);
 //EVENT::LCCollection* partCollection = evt->getCollection("NewPfoCol");

	//loop over mcparts see if there are any d0 or d0 bar
	int pdg;
	for(int i=0; i< _mcpartvec.size(); i++){
		pdg = _mcpartvec.at(i)->getPDG();
		for(int j=0; j< _pdgs.size(); j++){
			if(pdg == _pdgs.at(j)){
				//std::cout<<"found PDG: "<<pdg<<std::endl;

				//hacks make sure we find a kaon + pion now
				std::vector<MCParticle*> daughters = _mcpartvec.at(i)->getDaughters();
				for(int k=0; k < daughters.size(); k++){
					if(daughters.at(i) == 321 || daughters.at(i) == -321){
						std::cout<<"found d0->K"<<std::endl;
					}
				}
			}
		}
	}
  

  // Add new collection to event
//comment this next line when appending to collection
 // evt->addCollection(partCollection , _outputParticleCollectionName.c_str() ); 
 std::cout << "======================================== event " << nEvt << std::endl ;
nEvt++;
}
void MCParticleSelection::end(){
	
}

