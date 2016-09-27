from collections import OrderedDict
import os
import ROOT
import tempfile

def include(filename):
  ROOT.gROOT.ProcessLine("#include <{}>".format(filename))

ROOT.gROOT.Macro(os.path.join("..", "test", "loadMELA.C+"))
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
      functionfilecontents = self.functionfiletemplate.format(
                                                              name="NAME",
                                                              cppvariable=self.cppvariable,
                                                              cppargs=",".join([key for key in self.othercppargs]+["int item"]),
                                                              includes="\n".join("#include <{}>".format(_) for _ in includes),
                                                             )
      if functionfilecontents not in self.functionfiles:
        f = tempfile.NamedTemporaryFile(suffix=".C", bufsize=0)
        self.includes = includes
        f.write(functionfilecontents.replace("NAME", self.uniqueid))
        ROOT.gROOT.ProcessLine(".L {}+".format(f.name))
        self.functionfiles[functionfilecontents] = f
        self.getitems[functionfilecontents] = getattr(ROOT, "{}_getitem".format(self.uniqueid))
        self.setitems[functionfilecontents] = getattr(ROOT, "{}_setitem".format(self.uniqueid))

      self.functionfile = self.functionfiles[functionfilecontents]
      self.getitem = self.getitems[functionfilecontents]
      self.setitem = self.setitems[functionfilecontents]
      

  def __getitem__(self, item):
    if self.ndim > 1:
      return self.subarrays[item]
    else:
      if item >= self.dimensions[0]:
        raise IndexError("Index {} out of range (0-{})".format(item, self.dimensions[0]))
      return self.getitem(*(self.othercppargs.values()+[item]))

  def __setitem__(self, item, value):
    if self.ndim > 1:
      raise TypeError("Need to specify all indices to write to the array.")
    else:
      if item >= self.dimensions[0]:
        raise IndexError("Index {} out of range (0-{})".format(item, self.dimensions[0]))
      self.setitem(*(self.othercppargs.values()+[item, value]))

class Mela(object):
  counter = 0
  def __init__(self, *args):
    ROOT.gROOT.ProcessLine("Mela mela{}{};".format(self.counter, "("+", ".join(args)+")" if args else ""))
    self.__mela = getattr(ROOT, "mela{}".format(self.counter))
    self.index = self.counter
    type(self).counter += 1

    for name, dimensions in (
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
                            ):

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

  def __getattr__(self, name):
    return getattr(self.__mela, name)
