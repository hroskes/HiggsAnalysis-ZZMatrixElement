#ifndef EvtProb_VAR
#define EvtProb_VAR

#include <cstring>
#include <string>
#include <vector>
#include <utility>
#include "TCouplings.hh"
#include "MelaIO.h"
#include "TString.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TLorentzVector.h"

#define fbGeV2 0.389379E12
#define smallnumber 1e-15
#define sixteen_2Pi_to_8 3.88650230418250561e+07
#define eight_2Pi_to_5 7.83410393050320417e+04
#define four_2Pi_to_2 39.478417604357432

// typedefs for use in simple_event_record
typedef std::pair<int, TLorentzVector> SimpleParticle_t;
typedef std::vector<SimpleParticle_t> SimpleParticleCollection_t;


class TVar{
public:

  enum{
    kNoAssociated=1,
    kUseAssociated_Leptons=2, // l or nu
    kUseAssociated_Photons=3,
    kUseAssociated_Jets=5,
    kUseAssociated_UnstableTops=7,
    kUseAssociated_StableTops=11
  };
  enum CandidateDecayMode{
    CandidateDecay_Stable,
    CandidateDecay_ff,
    CandidateDecay_WW,
    CandidateDecay_ZZ,
    CandidateDecay_ZG,
    CandidateDecay_GG,
    CandidateDecay_ZW // Untested
  };
  enum VerbosityLevel {
    SILENT = 0,
    ERROR = 1,
    INFO = 2,
    DEBUG = 3,
    DEBUG_VERBOSE = 4,
    DEBUG_MECHECK = 5
  };
  enum MatrixElement{
    MCFM = 0,
    JHUGen = 1,
    ANALYTICAL = 2
  };
  enum Production{
    ZZGG,
    ZZQQB,
    ZZQQB_STU, // Should be the same as ZZQQB, just for crosscheck
    ZZINDEPENDENT,

    ttH, // ttH
    bbH, // bbH

    JQCD, // ? + 1 jet

    JJQCD, // SBF
    JJVBF, // VBF
    JJEW, // VBF+VH (had.)
    JJEWQCD, // VBF+VH+QCD, all hadronic
    Had_ZH, // ZH, Z->uu/dd
    Had_WH, // W(+/-)H, W->ud
    Lep_ZH, // ZH, Z->ll/nunu
    Lep_WH, // W(+/-)H, W->lnu

    // s-channel contributions
    ZZQQB_S,
    JJQCD_S,
    JJVBF_S,
    JJEW_S,
    JJEWQCD_S,
    Had_ZH_S,
    Had_WH_S,
    Lep_ZH_S,
    Lep_WH_S,

    // t+u-channel contributions
    ZZQQB_TU,
    JJQCD_TU,
    JJVBF_TU,
    JJEW_TU,
    JJEWQCD_TU,
    Had_ZH_TU,
    Had_WH_TU,
    Lep_ZH_TU,
    Lep_WH_TU,

    GammaH, // gammaH, stable A (could implement S and TU in the future
    //
    nProductions
  };
  enum LeptonInterference{
    DefaultLeptonInterf,
    InterfOn,
    InterfOff
  };
  enum FermionMassRemoval{
    NoRemoval,
    ConserveDifermionMass,
    MomentumToEnergy,
    nFermionMassRemovalSchemes
  };
  enum ResonancePropagatorScheme{ // Assigned specific integer value on purpose, translated directly to the JHUGen propagator indices
    NoPropagator=0,
    RunningWidth=1,
    FixedWidth=2,
    CPS=3
  };

  enum Process{
    HSMHiggs,    //0+, call this for MCFM |H|**2.
    H0minus,    //0-
    H0hplus,    //0h+
    H0_g1prime2,   //g1=0, g1prime2=-12046.01
    H0_Zgs,
    H0_gsgs,
    H0_Zgs_PS,
    H0_gsgs_PS,
    H0_Zgsg1prime2,

    D_g1g4, // D_CP
    D_g1g4_pi_2, // D_CP_T
    D_g1g2,   // D_int
    D_g1g2_pi_2, // D_int_T
    D_g1g1prime2, // D_int_lambda1
    D_zzzg,
    D_zzgg,
    D_zzzg_PS,
    D_zzgg_PS,
    D_zzzg_g1prime2,
    D_zzzg_g1prime2_pi_2,

    H1minus,    //1-
    H1plus,    //1+

    H2_g1, //2m+, Zg, gg
    H2_g2, // 2h2+
    H2_g3, // 2h3+
    H2_g4, //2h+
    H2_g5, //2b+
    H2_g1g5, //2m+
    H2_g6, // 2h6+
    H2_g7, // 2h7+
    H2_g8, //2h-
    H2_g9, // 2h9-
    H2_g10, // 2h10-

    bkgZGamma,    //Z+gamma
    bkgZJets,    //Z + 0/1/2 jets (ZZGG, JQCD, JJQCD)
    bkgZZ,    //qq/gg->ZZ
    bkgWW,    //qq/gg->WW
    bkgWWZZ,    //gg->ZZ+WW

    bkgZZ_SMHiggs,    //ggZZ+SMHigg
    bkgWW_SMHiggs,    //ggWW+SMHiggs
    bkgWWZZ_SMHiggs,    //ggZZ+WW+SMHiggs

    HSMHiggs_WWZZ,    //MCFM |H|**2 ZZ+WW with interference

    /**** For width ***/
    D_gg10,

    /***** Self Defined******/
    SelfDefine_spin0,
    SelfDefine_spin1,
    SelfDefine_spin2,

    nProcesses
  };
  enum SuperMelaSyst{
    SMSyst_None      = 0, // nominal value
    SMSyst_ScaleUp   = 1, //Scale Uncertaintie
    SMSyst_ScaleDown = 2,
    SMSyst_ResUp     = 3, // Resolution Uncertainty
    SMSyst_ResDown   = 4
  };
  enum EventScaleScheme{
    DefaultScaleScheme,
    Fixed_mH,
    Fixed_mW,
    Fixed_mZ,
    Fixed_mWPlusmH,
    Fixed_mZPlusmH,
    Fixed_TwomtPlusmH,
    Fixed_mtPlusmH,
    Dynamic_qH,
    Dynamic_qJJH,
    Dynamic_qJJ_qH,
    Dynamic_qJ_qJ_qH,
    Dynamic_HT,

    nEventScaleSchemes
  };

  //---------------------------------
  // Function
  //---------------------------------
  static TString ProcessName(TVar::Process temp){
    if (temp==TVar::HSMHiggs) return TString("HSMHiggs");
    else if (temp==TVar::H0minus) return TString("H0minus");
    else if (temp==TVar::H0hplus) return TString("H0hplus");
    else if (temp==TVar::H0_g1prime2) return TString("H0_g1prime2");
    else if (temp==TVar::H0_Zgs) return TString("H0_Zgs");
    else if (temp==TVar::H0_gsgs) return TString("H0_gsgs");
    else if (temp==TVar::H0_Zgs_PS) return TString("H0_Zgs_PS");
    else if (temp==TVar::H0_gsgs_PS) return TString("H0_gsgs_PS");
    else if (temp==TVar::H0_Zgsg1prime2) return TString("H0_Zgsg1prime2");

    else if (temp==TVar::D_g1g4) return TString("D_g1g4");
    else if (temp==TVar::D_g1g4_pi_2) return TString("D_g1g4_pi_2");
    else if (temp==TVar::D_g1g2) return TString("D_g1g2");
    else if (temp==TVar::D_g1g2_pi_2) return TString("D_g1g2_pi_2");
    else if (temp==TVar::D_g1g1prime2) return TString("D_g1g1prime2");
    else if (temp==TVar::D_zzzg) return TString("D_zzzg");
    else if (temp==TVar::D_zzgg) return TString("D_zzgg");
    else if (temp==TVar::D_zzzg_PS) return TString("D_zzzg_PS");
    else if (temp==TVar::D_zzgg_PS) return TString("D_zzgg_PS");
    else if (temp==TVar::D_zzzg_g1prime2) return TString("D_zzzg_g1prime2");
    else if (temp==TVar::D_zzzg_g1prime2_pi_2) return TString("D_zzzg_g1prime2_pi_2");

    else if (temp==TVar::H1minus) return TString("H1minus");
    else if (temp==TVar::H1plus) return TString("H1plus");

    else if (temp==TVar::H2_g1) return TString("H2_g1");
    else if (temp==TVar::H2_g2) return TString("H2_g2");
    else if (temp==TVar::H2_g3) return TString("H2_g3");
    else if (temp==TVar::H2_g4) return TString("H2_g4");
    else if (temp==TVar::H2_g5) return TString("H2_g5");
    else if (temp==TVar::H2_g1g5) return TString("H2_g1g5");
    else if (temp==TVar::H2_g6) return TString("H2_g6");
    else if (temp==TVar::H2_g7) return TString("H2_g7");
    else if (temp==TVar::H2_g8) return TString("H2_g8");
    else if (temp==TVar::H2_g9) return TString("H2_g9");
    else if (temp==TVar::H2_g10) return TString("H2_g10");

    else if (temp==TVar::bkgZGamma) return TString("bkgZGamma");
    else if (temp==TVar::bkgZJets) return TString("bkgZJets");
    else if (temp==TVar::bkgZZ) return TString("bkgZZ");
    else if (temp==TVar::bkgWW) return TString("bkgWW");
    else if (temp==TVar::bkgWWZZ) return TString("bkgWWZZ");
    else if (temp==TVar::bkgZZ_SMHiggs) return TString("bkgZZ_SMHiggs");
    else if (temp==TVar::bkgWW_SMHiggs) return TString("bkgWW_SMHiggs");
    else if (temp==TVar::bkgWWZZ_SMHiggs) return TString("bkgWWZZ_SMHiggs");
    else if (temp==TVar::HSMHiggs_WWZZ) return TString("HSMHiggs_WWZZ");

    else if (temp==TVar::D_gg10) return TString("D_gg10");

    else if (temp==TVar::SelfDefine_spin0) return TString("SelfDefine_spin0");
    else if (temp==TVar::SelfDefine_spin1) return TString("SelfDefine_spin1");
    else if (temp==TVar::SelfDefine_spin2) return TString("SelfDefine_spin2");

    else return TString("Unknown");
  };

  inline virtual ~TVar(){};
  ClassDef(TVar, 0)
};

struct simple_event_record{ // Somewhat not-so-simple particles
  int AssociationCode;
  int AssociationVCompatibility; // Z=23, W+-=|+-24| or none=0
  int nRequested_AssociatedJets;
  int nRequested_AssociatedLeptons;
  int nRequested_AssociatedPhotons;
  int nRequested_Tops;
  int nRequested_Antitops;

  // Output 4-vectors
  std::vector<int> intermediateVid; // Origin of daughters, not associated particles
  SimpleParticleCollection_t pDaughters;
  SimpleParticleCollection_t pAssociated;
  SimpleParticleCollection_t pMothers;

  std::vector<SimpleParticleCollection_t> pTopDaughters;
  std::vector<SimpleParticleCollection_t> pAntitopDaughters;
  SimpleParticleCollection_t pStableTops;
  SimpleParticleCollection_t pStableAntitops;

  // Constructor
  simple_event_record() :
    AssociationCode(TVar::kNoAssociated),
    AssociationVCompatibility(0),
    nRequested_AssociatedJets(0),
    nRequested_AssociatedLeptons(0),
    nRequested_AssociatedPhotons(0),
    nRequested_Tops(0),
    nRequested_Antitops(0)
  {}

};

struct event_scales_type{
  TVar::EventScaleScheme renomalizationScheme;
  TVar::EventScaleScheme factorizationScheme;
  double ren_scale_factor;
  double fac_scale_factor;
};



#endif
