#ifndef _TMODKINEMATICS_HH_
#define _TMODKINEMATICS_HH_

extern "C" {
  void __modkinematics_MOD_setrunningscales(double p[3][4], int id[4]);
  void __modkinematics_MOD_setpdfs(double* x1, double* x2, double pdf[2][13]);
  void __modkinematics_MOD_evalalphas();
}

#endif

