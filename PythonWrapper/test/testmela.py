"""
Simple example of reweighting using the python Mela and LHEFile classes
Also see the bottom of mela.py for more examples of the python Mela class
"""

from ZZMatrixElement.PythonWrapper.mela import LHEFile, TVar
import ROOT
import style

h = [ROOT.TH1F("h{}".format(i), "h", 20, -1, 1) for i in range(4)]

with LHEFile("0+.lhe") as f:   #have to provide this file yourself
    for i, event in enumerate(f, start=1):
        costheta1 = event.computeDecayAngles().costheta1
        event.setProcess(TVar.SelfDefine_spin0, TVar.JHUGen, TVar.ZZINDEPENDENT)
        event.ghz1 = 1
        p0plus = event.computeP(False)
        event.setProcess(TVar.SelfDefine_spin0, TVar.JHUGen, TVar.ZZINDEPENDENT)
        event.ghz4 = 1
        p0minus = event.computeP(False)
        h[0].Fill(costheta1)
        h[1].Fill(costheta1, p0minus / p0plus)
        if i%1000 == 0: print "Processed", i, "events"

with LHEFile("0-.lhe") as f:   #have to provide this file yourself
    for i, event in enumerate(f, start=1):
        costheta1 = event.computeDecayAngles().costheta1
        event.setProcess(TVar.SelfDefine_spin0, TVar.JHUGen, TVar.ZZINDEPENDENT)
        event.ghz1 = 1
        p0plus = event.computeP(False)
        event.setProcess(TVar.SelfDefine_spin0, TVar.JHUGen, TVar.ZZINDEPENDENT)
        event.ghz4 = 1
        p0minus = event.computeP(False)
        h[2].Fill(costheta1, p0plus / p0minus)
        h[3].Fill(costheta1)
        if i%1000 == 0: print "Processed", i, "events"

hstack = ROOT.THStack("hstack", "")
for i, hist in enumerate(h):
    hist.SetLineColor(i+1)
    hist.Scale(1/hist.Integral())
    hstack.Add(hist)

legend = ROOT.TLegend(.5, .2, .9, .4)
legend.SetBorderSize(0)
legend.SetFillStyle(0)
legend.AddEntry(h[0], "0^{+}", "l")
legend.AddEntry(h[1], "0^{+} reweighted to 0^{-}", "l")
legend.AddEntry(h[2], "0^{-}", "l")
legend.AddEntry(h[3], "0^{-} reweighted to 0^{+}", "l")

c = ROOT.TCanvas()
hstack.Draw("hist nostack")
hstack.GetXaxis().SetTitle("cos#theta_{1}")
legend.Draw()
c.SaveAs("test.pdf")
