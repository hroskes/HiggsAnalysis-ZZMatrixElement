//
// MELA root package loader - see testKD.C for instructions
//
{
  TString LIBMCFM="libmcfm_702.so";
  gSystem->AddIncludePath("-I$ROOFITSYS/include/");
  gSystem->AddIncludePath("-I$CMSSW_BASE/interface/");
  gSystem->AddIncludePath("-I$CMSSW_BASE/src/");
  gSystem->Load("libZZMatrixElementMELA.so");
  gSystem->Load("$CMSSW_BASE/src/ZZMatrixElement/MELA/data/$SCRAM_ARCH/" + LIBMCFM);
}
