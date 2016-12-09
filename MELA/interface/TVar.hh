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
    HSMHiggs, // Call this for any MCFM |H|**2-only ME.
    H0_g1prime2,
    H0hplus,
    H0minus,
    H0_Zgsg1prime2,
    H0_Zgs,
    H0_Zgs_PS,
    H0_gsgs,
    H0_gsgs_PS,

    D_g1g1prime2,
    D_g1g2,
    D_g1g2_pi_2,
    D_g1g4,
    D_g1g4_pi_2,
    D_zzzg,
    D_zzgg,
    D_zzzg_PS,
    D_zzgg_PS,
    D_zzzg_g1prime2,
    D_zzzg_g1prime2_pi_2,

    H1minus, // 1-
    H1plus, // 1+

    H2_g1, // 2m+, Zg, gg
    H2_g2, // 2h2+
    H2_g3, // 2h3+
    H2_g4, // 2h+
    H2_g5, // 2b+
    H2_g1g5, // 2m+
    H2_g6, // 2h6+
    H2_g7, // 2h7+
    H2_g8, // 2h-
    H2_g9, // 2h9-
    H2_g10, // 2h10-

    bkgZGamma, // Z+gamma cont.
    bkgZJets, // Z + 0/1/2 jets (ZZGG, JQCD, JJQCD)
    bkgZZ, // qq/gg->ZZ cont.
    bkgWW, // qq/gg->WW cont.
    bkgWWZZ, // gg->ZZ+WW cont.

    bkgZZ_SMHiggs, // ggZZ cont. + SMHigg
    bkgWW_SMHiggs, // ggWW cont. + SMHiggs
    bkgWWZZ_SMHiggs, // ggZZ+WW cont. + SMHiggs

    HSMHiggs_WWZZ, // MCFM |H|**2 ZZ+WW with ZZ-WW interference

    /**** For width ***/
    D_gg10,

    /***** Self Defined******/
    SelfDefine_spin0,
    SelfDefine_spin1,
    SelfDefine_spin2,

    nProcesses
  };
  enum SuperMelaSyst{
    // Nominal value
    SMSyst_None      = 0,
    // Scale uncertainties
    SMSyst_ScaleUp   = 1,
    SMSyst_ScaleDown = 2,
    // Resolution uncertainties
    SMSyst_ResUp     = 3,
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

  static TString ProductionName(TVar::Production temp){
    if (temp==TVar::ZZGG) return TString("ZZGG");
    else if (temp==TVar::ZZQQB) return TString("ZZQQB");
    else if (temp==TVar::ZZQQB_STU) return TString("ZZQQB_STU");
    else if (temp==TVar::ZZINDEPENDENT) return TString("ZZINDEPENDENT");

    else if (temp==TVar::ttH) return TString("ttH");
    else if (temp==TVar::bbH) return TString("bbH");

    else if (temp==TVar::JQCD) return TString("JQCD");

    else if (temp==TVar::JJQCD) return TString("JJQCD");
    else if (temp==TVar::JJVBF) return TString("JJVBF");
    else if (temp==TVar::JJEW) return TString("JJEW");
    else if (temp==TVar::JJEWQCD) return TString("JJEWQCD");
    else if (temp==TVar::Had_ZH) return TString("Had_ZH");
    else if (temp==TVar::Had_WH) return TString("Had_WH");
    else if (temp==TVar::Lep_ZH) return TString("Lep_ZH");
    else if (temp==TVar::Lep_WH) return TString("Lep_WH");

    else if (temp==TVar::ZZQQB_S) return TString("ZZQQB_S");
    else if (temp==TVar::JJQCD_S) return TString("JJQCD_S");
    else if (temp==TVar::JJVBF_S) return TString("JJVBF_S");
    else if (temp==TVar::JJEW_S) return TString("JJEW_S");
    else if (temp==TVar::JJEWQCD_S) return TString("JJEWQCD_S");
    else if (temp==TVar::Had_ZH_S) return TString("Had_ZH_S");
    else if (temp==TVar::Had_WH_S) return TString("Had_WH_S");
    else if (temp==TVar::Lep_ZH_S) return TString("Lep_ZH_S");
    else if (temp==TVar::Lep_WH_S) return TString("Lep_WH_S");

    else if (temp==TVar::ZZQQB_TU) return TString("ZZQQB_TU");
    else if (temp==TVar::JJQCD_TU) return TString("JJQCD_TU");
    else if (temp==TVar::JJVBF_TU) return TString("JJVBF_TU");
    else if (temp==TVar::JJEW_TU) return TString("JJEW_TU");
    else if (temp==TVar::JJEWQCD_TU) return TString("JJEWQCD_TU");
    else if (temp==TVar::Had_ZH_TU) return TString("Had_ZH_TU");
    else if (temp==TVar::Had_WH_TU) return TString("Had_WH_TU");
    else if (temp==TVar::Lep_ZH_TU) return TString("Lep_ZH_TU");
    else if (temp==TVar::Lep_WH_TU) return TString("Lep_WH_TU");

    else if (temp==TVar::GammaH) return TString("GammaH");

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
