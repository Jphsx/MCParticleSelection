#include "marlin/Processor.h"
#include "EVENT/MCParticle.h"
#include "EVENT/Track.h"
#include "lcio.h"
#include "TFile.h"
#include <vector>
#include "IMPL/LCCollectionVec.h"
#include "IMPL/ParticleIDImpl.h"
#include "IMPL/ReconstructedParticleImpl.h"

#include <marlin/Global.h>
#include "gear/BField.h"
#include "TLorentzVector.h"
#include <string>
#include <iostream>
#include <fstream>

using namespace lcio;

	/** MCParticleSelection:<br>
 *
 * 
 * @author Justin Anguiano, University of Kansas
 * 
 */

 class MCParticleSelection : public marlin::Processor {

 public:

 virtual marlin::Processor*  newProcessor() { return new MCParticleSelection ; }

  MCParticleSelection(const MCParticleSelection&) = delete ;
  MCParticleSelection& operator=(const MCParticleSelection&) = delete ;

  MCParticleSelection() ;

  /** Called at the beginning of the job before anything is read.
   *  Use to initialize the proscessor, e.g. book histograms.
   */
  virtual void init() ;
  /** Called for every run.
   */
  virtual void processRunHeader( LCRunHeader* run ) ;

  /** Called for every event - the working horse.
   */
  virtual void processEvent( LCEvent * evt ) ;


  /** Called after data processing for clean up.
   */
  virtual void end() ;

  bool FindTracks(LCEvent* evt);
  bool FindMCParticles( LCEvent* evt );

  protected:
  int nEvt{};
  
  //vector to hold the tracks for the event
  std::vector<Track*> _trackvec{};
  std::vector<MCParticle*> _mcpartvec{};
  int   _printing{};
  std::vector<int> _pdgs{};

   //need to no BField to calculate stuff
   double BField{};
 
// _inputTrackCollectionName 
  std::string _outputParticleCollectionName{};
  std::string _inputTrackCollectionName{};
  std::string _inputMcParticleCollectionName{};
//  std::string m_rootFile{};
};
