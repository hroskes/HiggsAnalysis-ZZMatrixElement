import array
from collections import OrderedDict
import os
import ROOT
import tempfile

def include(filename):
  ROOT.gROOT.ProcessLine("#include <{}>".format(filename))

ROOT.gROOT.Macro(os.path.join(os.environ["CMSSW_BASE"], "src", "ZZMatrixElement", "MELA", "test", "loadMELA.C+"))
include("ZZMatrixElement/MELA/interface/Mela.h")

class MultiDimensionalCppArray(object):
  functionfiletemplate = """
    #include <type_traits>
    {includes}
    auto {name}_getitem({cppargs}) -> std::remove_reference<decltype({cppvariable}[item])>::type {{return {cppvariable}[item];}}
    void {name}_setitem({cppargs}, std::remove_reference<decltype({cppvariable}[item])>::type value) {{{cppvariable}[item] = value;}}
  """

  uniqueids = []
  functionfiles = {}
  getitems = {}
  setitems = {}

  def __init__(self, uniqueid, cppvariable, includes, othercppargs, *dimensions):

    self.uniqueid = uniqueid
    for i in self.uniqueids:
      if i == uniqueid:
        raise ValueError("Two MultiDimensionalCppArrays can't have the same id\n{}".format(i, self.uniqueid))
    self.uniqueids.append(uniqueid)

    self.cppvariable = cppvariable
    self.dimensions = dimensions
    self.ndim = len(self.dimensions)
    if self.ndim == 0:
      raise TypeError("Can't have a 0 dimensional array!")

    if self.ndim > 1:
      self.subarrays = []
      for i in range(dimensions[0]):
          othercppargs["int index{}".format(len(dimensions))] = i
          self.subarrays.append(
                                MultiDimensionalCppArray(
                                                         "{}_{}".format(self.uniqueid, i),
                                                         "{}[index{}]".format(cppvariable, len(dimensions)),
                                                         includes,
                                                         othercppargs,
                                                         *dimensions[1:]
                                                        )
                               )
    else:
      self.othercppargs = OrderedDict(othercppargs)
      self.functionfilecontents = self.functionfiletemplate.format(
                                                                   name="NAME",
                                                                   cppvariable=self.cppvariable,
                                                                   cppargs=",".join([key for key in self.othercppargs]+["int item"]),
                                                                   includes="\n".join("#include <{}>".format(_) for _ in includes),
                                                                  )
      self.includes = includes
      self.functionfile = self.getitem = self.setitem = None

  def writecpp(self, f=None):
    if self.ndim > 1:
      for subarray in self.subarrays:
        f = subarray.writecpp(f)
      return f

    if self.functionfilecontents not in self.functionfiles:
      if f is None:
        f = tempfile.NamedTemporaryFile(suffix=".C", bufsize=0)
      f.write(self.functionfilecontents.replace("NAME", self.uniqueid))
      return f
    else:
      return self.functionfiles[self.functionfilecontents]

  def compilecpp(self, f):
    if self.ndim > 1:
      for subarray in self.subarrays:
        subarray.compilecpp(f)
      return

    if self.functionfilecontents not in self.functionfiles:
      ROOT.gROOT.ProcessLine(".L {}+".format(f.name))
      self.functionfiles[self.functionfilecontents] = f
      self.getitems[self.functionfilecontents] = getattr(ROOT, "{}_getitem".format(self.uniqueid))
      self.setitems[self.functionfilecontents] = getattr(ROOT, "{}_setitem".format(self.uniqueid))

    self.functionfile = self.functionfiles[self.functionfilecontents]
    self.getitem = self.getitems[self.functionfilecontents]
    self.setitem = self.setitems[self.functionfilecontents]


  def __getitem__(self, item):
    if self.ndim > 1:
      return self.subarrays[item]
    else:
      if self.getitem is None: self.compilecpp(self.writecpp())
      if item >= self.dimensions[0]:
        raise IndexError("Index {} out of range (0-{})".format(item, self.dimensions[0]))
      return self.getitem(*(self.othercppargs.values()+[item]))

  def __setitem__(self, item, value):
    if self.ndim > 1:
      raise TypeError("Need to specify all indices to write to the array.")
    else:
      if self.setitem is None: self.compilecpp()
      if item >= self.dimensions[0]:
        raise IndexError("Index {} out of range (0-{})".format(item, self.dimensions[0]))
      self.setitem(*(self.othercppargs.values()+[item, value]))

class Mela(object):
  counter = 0
  doneinit = False
  computeptemplate = """
    #include <ZZMatrixElement/MELA/interface/Mela.h>
    float computeP(Mela& mela, bool useconstant) {
      float result;
      mela.computeP(result, useconstant);
      return result;
    }
    float computeProdP(Mela& mela, bool useconstant) {
      float result;
      mela.computeProdP(result, useconstant);
      return result;
    }
  """
  def __init__(self, *args, **kwargs):
    self.__mela = ROOT.Mela(*args, **kwargs)
    self.index = self.counter
    type(self).counter += 1

    arrays  = (
               ("selfDHqqcoupl", (2, 2)),
               ("selfDHggcoupl", (3, 2)),
               ("selfDHzzcoupl", (2, 39, 2)),
               ("selfDHwwcoupl", (2, 39, 2)),
               ("selfDHzzLambda_qsq", (2, 4, 3)),
               ("selfDHwwLambda_qsq", (2, 4, 3)),
               ("selfDHzzCLambda_qsq", (2, 3)),
               ("selfDHwwCLambda_qsq", (2, 3)),
               ("selfDZqqcoupl", (2, 2)),
               ("selfDZvvcoupl", (2, 2)),
               ("selfDGqqcoupl", (2, 2)),
               ("selfDGggcoupl", (5, 2)),
               ("selfDGvvcoupl", (10, 2)),
              )

    f = None
    for name, dimensions in arrays:
        setattr(
                self,
                name,
                MultiDimensionalCppArray(
                                         "mela{}{}".format(self.index, name),
                                         "mela.{}".format(name),
                                         ["ZZMatrixElement/MELA/interface/Mela.h"],
                                         {"Mela& mela": self.__mela},
                                         *dimensions
                                        )
               )
        f = getattr(self, name).writecpp(f)
    bkpgErrorIgnoreLevel, ROOT.gErrorIgnoreLevel = ROOT.gErrorIgnoreLevel, ROOT.kInfo+1
    f.write(self.computeptemplate)
    for name, dimensions in arrays:
        getattr(self, name).compilecpp(f)
    ROOT.gErrorIgnoreLevel = bkpgErrorIgnoreLevel
    self.doneinit = True

  def __getattr__(self, name):
    return getattr(self.__mela, name)

  def __setattr__(self, name, value):
    if self.doneinit:
      return setattr(self.__mela, name, value)
    else:
      super(Mela, self).__setattr__(name, value)

  def setInputEvent_fromLHE(self, event, isgen):
    lines = event.split("\n")
    lines = [line for line in lines if not ("<event>" in line or "</event>" in line or not line.split("#")[0].strip())]
    nparticles, _, _, _, _, _ = lines[0].split()
    nparticles = int(nparticles)
    if nparticles != len(lines)-1:
      raise ValueError("Wrong number of particles! Should be {}, have {}".replace(nparticles, len(lines)-1))
    daughters, mothers, associated = [], [], []
    ids = [None]
    mother1s = [None]
    mother2s = [None]
    for line in lines[1:]:
      id, status, mother1, mother2 = (int(_) for _ in line.split()[0:4])
      ids.append(id)
      mother1s.append(mother1)
      mother2s.append(mother2)
      if status == -1:
        mothers.append(line)
      elif status == 1 and (1 <= abs(id) <= 6 or 11 <= abs(id) <= 16 or abs(id) == 22):
        while True:
          if mother1 != mother2 or mother1 is None:
            associated.append(line)
            break
          if ids[mother1] == 25:
            daughters.append(line)
            break
          mother2 = mother2s[mother1]
          mother1 = mother1s[mother1]
    #print "mothers"
    #for _ in mothers: print _
    #print "daughters"
    #for _ in daughters: print _
    #print "associated"
    #for _ in associated: print _
    self.setInputEvent(SimpleParticleCollection_t(daughters), SimpleParticleCollection_t(associated), SimpleParticleCollection_t(mothers), isgen)

  def computeP(self, useconstant):
    return ROOT.computeP(self.__mela, useconstant)

  def computeProdP(self, useconstant):
    return ROOT.computeProdP(self.__mela, useconstant)

def SimpleParticleCollection_t(iterable=None):
  if iterable is None: return ROOT.SimpleParticleCollection_t()
  result = ROOT.SimpleParticleCollection_t()
  for _ in iterable:
    result.push_back(SimpleParticle_t(_))
  return result

def SimpleParticle_t(lineorid, pxortlv=None, py=None, pz=None, e=None):
  if pxortlv is py is pz is e is None:
    if isinstance(lineorid, basestring):
      lineorid = lineorid.split()
    if len(lineorid) == 13:
      id, status, mother1, mother2, color1, color2, px, py, pz, e, m, lifetime, spin = (f(_) for f, _ in zip((int, int, int, int, int, int, float, float, float, float, float, float, float), lineorid))
    elif len(lineorid) == 5:
      id, px, py, pz, e = (f(_) for f, _ in zip((int, float, float, float, float), lineorid))
    else:
      raise ValueError("len(lineorid) has to be 5 or 13, not {}".format(len(lineorid)))
  else:
    id = lineorid
    px = pxortlv

  if py is pz is e is None:
    tlv = pxortlv
  else:
    tlv = ROOT.TLorentzVector(px, py, pz, e)

  return ROOT.SimpleParticle_t(id, tlv)

if __name__ == "__main__":
  m = Mela()
  event1 = """
<event>
12  50   1.0000000E+00   1.2500000E+02   7.8125000E-03   1.2380607E-01
        2   -1    0    0  503    0  0.00000000000E+00  0.00000000000E+00  1.65430825479E+03  1.65430825479E+03  0.00000000000E+00 0.00000000000E+00  1.
       -1   -1    0    0    0  503  0.00000000000E+00  0.00000000000E+00 -1.42857195999E+01  1.42857195999E+01  0.00000000000E+00 0.00000000000E+00  1.
       24    2    1    2    0    0 -2.29473352103E+00 -1.04804828491E+02  4.95291431378E+02  5.12296652853E+02  7.83896718395E+01 0.00000000000E+00  1.
       25    2    1    2    0    0  2.29473352103E+00  1.04804828491E+02  1.14473110382E+03  1.15629732154E+03  1.24999511524E+02 0.00000000000E+00  1.
       14    1    3    3    0    0  4.42035961901E+00 -5.60456350211E+01  4.09886160671E+02  4.13723721213E+02  8.42936970218E-06 0.00000000000E+00  1.
      -13    1    3    3    0    0 -6.71509314004E+00 -4.87591934698E+01  8.54052707068E+01  9.85729316407E+01  1.05660000144E-01 0.00000000000E+00  1.
       23    2    4    4    0    0 -2.00748771644E+01  3.21702667586E+01  3.27018956548E+02  3.30034988785E+02  2.33188576920E+01 0.00000000000E+00  1.
       23    2    4    4    0    0  2.23696106855E+01  7.26345617324E+01  8.17712147272E+02  8.26262332755E+02  9.09950970840E+01 0.00000000000E+00  1.
      -11    1    7    7    0    0 -1.74223737299E+01  9.11950220870E+00  1.06644211152E+02  1.08442114510E+02  5.11001208360E-04 0.00000000000E+00  1.
       11    1    7    7    0    0 -2.65250343458E+00  2.30507645499E+01  2.20374745396E+02  2.21592874275E+02  5.10994690391E-04 0.00000000000E+00  1.
      -13    1    8    8    0    0  8.81223774828E+00  8.87930337607E+01  5.03683096793E+02  5.11525690007E+02  1.05660000328E-01 0.00000000000E+00  1.
       13    1    8    8    0    0  1.35573729372E+01 -1.61584720283E+01  3.14029050479E+02  3.14736642748E+02  1.05659999907E-01 0.00000000000E+00  1.
</event>
  """
  event2 = """
<event>
12  50   1.0000000E+00   1.2500000E+02   7.8125000E-03   1.2380607E-01
        1   -1    0    0  503    0  0.00000000000E+00  0.00000000000E+00  1.58591490197E+03  1.58591490197E+03  0.00000000000E+00 0.00000000000E+00  1.
       -1   -1    0    0    0  503  0.00000000000E+00  0.00000000000E+00 -8.99084923758E+00  8.99084923758E+00  0.00000000000E+00 0.00000000000E+00  1.
       23    2    1    2    0    0  4.31808951699E+01  1.18843550193E+01  8.22005355890E+02  8.28398612649E+02  9.24425698805E+01 0.00000000000E+00  1.
       25    2    1    2    0    0 -4.31808951699E+01 -1.18843550193E+01  7.54918696840E+02  7.66507138556E+02  1.25000508063E+02 0.00000000000E+00  1.
       11    1    3    3    0    0 -1.35803884002E+01 -5.28931958672E+00  5.41360784563E+02  5.41556924907E+02  5.11072900539E-04 0.00000000000E+00  1.
      -11    1    3    3    0    0  5.67612835701E+01  1.71736746060E+01  2.80644571326E+02  2.86841687743E+02  5.11012071458E-04 0.00000000000E+00  1.
       23    2    4    4    0    0 -2.43038338852E+01  5.06442605250E+00  2.48359236741E+02  2.53284239962E+02  4.30612469142E+01 0.00000000000E+00  1.
       23    2    4    4    0    0 -1.88770612847E+01 -1.69487810718E+01  5.06559460099E+02  5.13222898594E+02  7.84324703350E+01 0.00000000000E+00  1.
      -13    1    7    7    0    0 -3.25370809281E+01 -6.79837669312E+00  2.02354268485E+02  2.05066186143E+02  1.05659999991E-01 0.00000000000E+00  1.
       13    1    7    7    0    0  8.23324704291E+00  1.18628027456E+01  4.60049682560E+01  4.82180538193E+01  1.05659999989E-01 0.00000000000E+00  1.
      -13    1    8    8    0    0  4.59433181687E+00 -3.18015647781E+01  4.39027117172E+02  4.40201395027E+02  1.05659999655E-01 0.00000000000E+00  1.
       13    1    8    8    0    0 -2.34713931016E+01  1.48527837063E+01  6.75323429266E+01  7.30215035668E+01  1.05660000010E-01 0.00000000000E+00  1.
</event>
  """
  event3 = """
<event>
11  60   1.0000000E+00   1.2500000E+02   7.8125000E-03   1.2380607E-01
        1   -1    0    0  501    0  0.00000000000E+00  0.00000000000E+00  8.38349783822E+01  8.38349783822E+01  0.00000000000E+00 0.00000000000E+00  1.
        2   -1    0    0  502    0  0.00000000000E+00  0.00000000000E+00 -8.69647303563E+02  8.69647303563E+02  0.00000000000E+00 0.00000000000E+00  1.
        4    1    1    2  501    0  4.93534233194E+01 -7.45486758049E+00  2.54822242213E+01  5.60417629563E+01  0.00000000000E+00 0.00000000000E+00  1.
        1    1    1    2  502    0 -4.29482465415E+01  4.39907893858E+01 -7.51475061906E+02  7.53985749267E+02  0.00000000000E+00 0.00000000000E+00  1.
       25    2    1    2    0    0 -6.40517677787E+00 -3.65359218053E+01 -5.98194874970E+01  1.43454769722E+02  1.25000000000E+02 0.00000000000E+00  1.
       23    2    5    5    0    0 -1.61638014503E+01 -3.55963825472E+01 -2.51394501445E+01  1.03431837860E+02  9.24001201399E+01 0.00000000000E+00  1.
       23    2    5    5    0    0  9.75862467247E+00 -9.39539258134E-01 -3.46800373525E+01  4.00229318615E+01  1.74073718437E+01 0.00000000000E+00  1.
      -11    1    6    6    0    0  3.37109433312E+01 -2.97615359833E+01  4.38251799494E+00  4.51816687231E+01  5.11000134768E-04 0.00000000000E+00  1.
       11    1    6    6    0    0 -4.98747447816E+01 -5.83484656388E+00 -2.95219681394E+01  5.82501691374E+01  5.11001208360E-04 0.00000000000E+00  1.
      -13    1    7    7    0    0  1.46596263059E+01  5.33582780943E-01 -2.31337995488E+01  2.73929406894E+01  1.05660000000E-01 0.00000000000E+00  1.
       13    1    7    7    0    0 -4.90100163341E+00 -1.47312203908E+00 -1.15462378037E+01  1.26299911721E+01  1.05660000000E-01 0.00000000000E+00  1.
</event>
  """

  m.setInputEvent_fromLHE(event1, True)

  m.selfDHzzcoupl[0][0][0] = 1
  m.selfDHzzcoupl[0][3][0] = 1
  m.setProcess(ROOT.TVar.SelfDefine_spin0, ROOT.TVar.JHUGen, ROOT.TVar.Lep_WH)
  print m.computeProdP(False)
  m.selfDHzzcoupl[0][0][0] = 1
  m.selfDHzzcoupl[0][3][0] = 1
  m.setProcess(ROOT.TVar.SelfDefine_spin0, ROOT.TVar.JHUGen, ROOT.TVar.ZZINDEPENDENT)
  print m.computeP(False)
