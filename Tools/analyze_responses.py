import numpy as np, csv, pathlib, json
from scipy.interpolate import interp1d
N=32768; fs=48000
freq=np.array([11,13,15,17,20,24,28,33,39,46,54,63,75,88,100,120,140,170,200,240,280,330,420,570,125,150,180,210,250,300,350,420,500,600,700,850,1000,1200,1400,1700,2000,2400,2800,3400,4000,4800,5700,6800,3400,3700,4000,4400,4800,5200,5700,6200,6800,7400,8000,8800,9600,10500,11500,12500,13600,14800,16000,17000,18500,20000,22500,25000]).reshape(3,24)
gains=[-9,-8,-7,-6,-5,-4,-3,-2.5,-2,-1.5,-1,-.5,0,.5,1,1.5,2,2.5,3,4,5,6,7,8,9]
q1=[.67,1.03,1.77,2.30,4.38];q6=[.53,.82,1.39,1.83,3.05]
axis=np.fft.rfftfreq(N,1/fs)
test=np.geomspace(20,20000,240)
dtype=np.dtype([('key','<i4',(4,)),('ir','<f4',(N,))])
raw=np.memmap('responses.bin',dtype=dtype,mode='r')
out=pathlib.Path('validation');out.mkdir(exist_ok=True)
summary=[]; anchors=[]; seen=set()
with (out/'all-settings.csv').open('w',newline='') as f, (out/'frequency-response.csv').open('w',newline='') as ff:
 w=csv.writer(f);w.writerow(['band','frequency_index','shape_index','gain_db','peak_or_trough_hz','peak_or_trough_db','finite','reference_type','max_amplitude_error_pct','rms_db','pass_5pct'])
 ww=csv.writer(ff);ww.writerow(['band','frequency_index','shape_index','gain_db']+[f'{x:.3f}' for x in test])
 for rec in raw:
  b,k,q,g=map(int,rec['key']);db=gains[g];seen.add((b,k,q,g))
  ir=rec['ir'];finite=bool(np.isfinite(ir).all());H=np.fft.rfft(ir);mag=np.abs(H);actual=np.interp(test,axis,mag);actualdb=20*np.log10(np.maximum(actual,1e-30))
  idx=np.argmax(mag) if db>0 else np.argmin(mag)
  ref=None;kind='unmeasured hardware setting'
  # Only the measured +/-1 dB shelf at 50 Hz / fixed high shelf has a full published fit.
  if abs(db)==1 and ((b==0 and k==0) or b==4):
   fc,A={(0,1):(104.9220527,.96276648),(0,-1):(107.91641041,-1.02533953),(4,1):(4099.32098,.926376903),(4,-1):(3805.34622,-.948855989)}[b,db]
   s=1j*test/fc;ref=np.abs(1+(10**(A/20)-1)*(1/(1+s) if b==0 else s/(1+s)));kind='published graph analog fit'
  err=rms=passed=''
  if ref is not None:
   err=float(np.max(np.abs(actual/ref-1))*100);rms=float(np.sqrt(np.mean((actualdb-20*np.log10(ref))**2)));passed=err<=5
   anchors.append(dict(band=b,gain=db,max_amplitude_error_pct=err,rms_db=rms,passed=passed))
  w.writerow([b,k,q,db,axis[idx],20*np.log10(max(mag[idx],1e-30)),finite,kind,err,rms,passed]);ww.writerow([b,k,q,db]+list(np.round(actualdb,5)))
  summary.append(finite)
expected={(b,k,q,g) for b in range(5) for k in range(2 if b==0 else 1 if b==4 else 24) for q in range(5 if 0<b<4 else 1) for g in range(25) if g!=12}
report=dict(records=len(raw),unique=len(seen),expected=len(expected),missing=len(expected-seen),all_finite=all(summary),hardware_curve_comparisons=anchors,unverified_individual_settings=len(seen)-len(anchors))
(out/'summary.json').write_text(json.dumps(report,indent=2));print(json.dumps(report,indent=2))
if seen!=expected or not all(summary):raise SystemExit(1)
