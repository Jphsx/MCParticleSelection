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

	
	registerProcessorParameter("PDG",
				   "PDG code of particle to be picked out",
				    _PDG,
				    0);
	

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

  	std::string outputTrkCollectionName = "x";
  	registerOutputCollection( LCIO::TRACK,
                             	"OutputTrackCollectionName" ,
			     	"Output Track Collection Name "  ,
                             	_outputTrkCollectionName,
                             	outputTrkCollectionName);

	std::string outputMCCollectionName = "y";
  	registerOutputCollection( LCIO::MCPARTICLE,
                             	"OutputMCCollectionName" ,
			     	"Output MC Collection Name "  ,
                             	_outputMCCollectionName,
                             	outputMCCollectionName);

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
 


  
  
  LCCollectionVec * trkCollection = new LCCollectionVec(LCIO::TRACK);
  LCCollectionVec * mcCollection = new LCCollectionVec(LCIO::MCPARTICLE);
   mcCollection->setSubset(true);
 //EVENT::LCCollection* partCollection = evt->getCollection("NewPfoCol");

	//loop over mcparts see if there are any d0 or d0 bar
	int pdg;
	for(int i=0; i< _mcpartvec.size(); i++){
		pdg = _mcpartvec.at(i)->getPDG();
		
		if(pdg == _PDG){
				//std::cout<<"found PDG: "<<pdg<<std::endl;

				//hacks make sure we find a kaon + pion now
			std::vector<MCParticle*> daughters = _mcpartvec.at(i)->getDaughters();
			bool flag1 = false;
			bool flag2 = false;
			bool flag3 = false;
			for(int k=0; k < daughters.size(); k++){
				if( daughters.size() == 2){ 
					flag1=true; 
				}
				if(daughters.at(k)->getPDG() == 321 || daughters.at(k)->getPDG() == -321){
					flag2 = true;
				}
				if(daughters.at(k)->getPDG() == 211 || daughters.at(k)->getPDG() == -211){
					flag3 = true;
				}
			}//end daughter loop

			if(flag1 && flag2 && flag3 ){
				std::cout<<"found d0->Kpi"<<std::endl;
				//first add the mc particle 
				mcCollection->addElement( _mcpartvec.at(i) );
				//also add daughters
				for(int j=0; j< daughters.size(); j++){
					mcCollection->addElement( daughters.at(j) );
				}
				//now loop and select tracks of this event with dislaced vertices only > 0.5mm

				FindTracks(evt);
				for(int j=0; j<_trackvec.size(); j++){
					if( (_trackvec.at(j)->getD0() >= 0.01) || (_trackvec.at(j)->getZ0() >= 0.01) ){
						std::cout<<"found a displaced track"<<std::endl;
						Track* T =  _trackvec.at(j);
						TrackImpl* t = new TrackImpl();
						t->setD0( T->getD0() );
						t->setPhi( T->getPhi() );
						t->setOmega( T->getOmega() );
						t->setZ0( T->getZ0() );
						t->setTanLambda( T->getTanLambda() );
						t->setCovMatrix( T->getCovMatrix() );
						t->setReferencePoint( T->getReferencePoint() );

						trkCollection->addElement( t );
					}
				}
			}
			
		}//end d0 check
		
	}//end mcpartvec loop
  

  // Add new collection to event
//comment this next line when appending to collection
  
  evt->addCollection(trkCollection , _outputTrkCollectionName.c_str() ); 
  evt->addCollection(mcCollection , _outputMCCollectionName.c_str() );

 std::cout << "======================================== event " << nEvt << std::endl ;
nEvt++;
}
void MCParticleSelection::end(){
	
}

