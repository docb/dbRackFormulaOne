
# Formula One

A script/formula evaluation module for VCVRack. This module can do almost anything, and it is quite fast.

![](images/FormulaOne.png?raw=true)

It is based on the [exprtk expression library](http://www.partow.net/programming/exprtk/index.html) 
which has very good [benchmarks](https://github.com/ArashPartow/math-parser-benchmark-project#the-rounds).

It is not intended for replacing native modules because they still will be faster i.e. consume less CPU 
and provide more usability. It is for experimenting,learning and making special things. 

## Examples and Usecases

The examples are available as factory presets.

<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->

- [Simple polyphonic oscillator](#simple-polyphonic-oscillator)
  - [Step by Step](#step-by-step)
- [External Phase Controlled Oscillator](#external-phase-controlled-oscillator)
- [Wave Folder](#wave-folder)
- [Simple Polyphonic Filter](#simple-polyphonic-filter)
  - [Step by Step](#step-by-step-1)
- [Polyphonic Comb Filter (Chorus,Flanger or whatever)](#polyphonic-comb-filter-chorusflanger-or-whatever)
  - [Step by Step](#step-by-step-2)
- [Stereo Delay](#stereo-delay)
- [Polyphonic Random and Hold](#polyphonic-random-and-hold)
- [Simple Sequencer](#simple-sequencer)
- [Chord Sequencer](#chord-sequencer)
- [Gate Sequencer](#gate-sequencer)
- [A Line Segment Envelope Generator](#a-line-segment-envelope-generator)
- [CZ-Series PD Filter sweep](#cz-series-pd-filter-sweep)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

### Simple polyphonic oscillator

![](images/Osc.png?raw=true)

```
var freq := 261.626 * pow(2,w);
var p:= bufr(chn);
var o:=a*4*(
   sin(2*pi*p)+
   sin(6*pi*p)/3+
   sin(10*pi*p)/5+
   sin(14*pi*p)/7
);
var phs:= p+stim*freq;
phs-=trunc(phs);
bufw(chn,phs);
var out:=o; 
```
#### Step by Step
`var freq := 261.626 * pow(2,w);`  Declare a variable freq and assign 
the computed frequency from the input `w` (V/Oct).

`var p:= bufr(chn);`  Read the current phase for `chn` from the buffer 
into the variable p. The script is evaluated for every
channel which is detected in the `w` input and provides the
 value of the input `w` for the current channel via the global variable `w`.
The current channel number is set in the global variable `chn`.

`var o:=a*4*(sin(2*pi*p)+sin(6*pi*p)/3+sin(10*pi*p)/5+sin(14*pi*p)/7);` 
compute the output sample from the current phase. the global variable `a` holds the current
value of the knob `a`;

`var phs:= p+stim*freq;  phs-=trunc(phs);` compute the next phase using the global variable
`stim` which holds the current sample time (`1/sampleRate`).

`bufw(chn,phs);`  write the new phase into the buffer.

`var out:=o;` the last expression value in the script is returned to the module and written
into the output `out`.

### External Phase Controlled Oscillator
![](images/PulseWave.png?raw=true)

```Tcl
a*5*(
   sin(2*pi*t)+
   sin(6*pi*t)/3+
   sin(10*pi*t)/5+
   sin(14*pi*t)/7
)
```

This example does the same as the one above but uses the special input `t` as phase.
The values of `t` are normalized from -5V/5V to 0V/1V.

### Wave Folder
![](images/Wavefolder.png?raw=true)

`sin((c*5)*t*2*pi+b*5)*a*5`

The same principle can be used to make a wave folder.
The knob `c` controls the depth and the knob `b` controls the offset.

### Simple Polyphonic Filter

This example shows how to implement a simple LP filter. The algorithm is directly taken from the VCV Rack
[source](https://github.com/VCVRack/Rack/blob/v2/include/dsp/filter.hpp)

![](images/Filter.png?raw=true)

```Tcl
var f := clamp(2^(c*8),0.001,0.5);
var fc := 2/f;
var out:= (x + bufr(chn) - buf1r(chn) * (1 - fc)) /(1 + fc);
bufw(chn,x); 
buf1w(chn,out);
dcb(chn,out);
```
#### Step by Step

`var f := clamp(2^(c*8),0.001,0.5); var fc := 2/f;` Compute the cutoff frequency from the knob `c`.

```
var out:= (x + bufr(chn) - buf1r(chn) * (1 - fc)) /(1 + fc);
bufw(chn,x); 
buf1w(chn,out);
```
Compute the next filter sample. The two needed state variables are stored per channel in the 
buffer (0) and buffer 1. There are 4 buffers of size 4096 available which can be read 
via bufr,buf1r,buf2r,buf3r and written via bufw,buf1w,buf2w,buf3w

`dcb(chn,out);`  output the dc blocked sample.

### Polyphonic Comb Filter (Chorus,Flanger or whatever)

![](images/Comb.png?raw=true)
```Tcl
var delay_ms:= 2.1;
var len:= delay_ms/1000*sr;
rblen(chn,len);
var c0 := clamp((c+1)/2+y/10*a,0,0.99);
var x0 := rbget(chn,c0*len); 
var nx := x/2 + x0 * d;
rbpush(chn,nx);
var out:=x0+nx;
var x1 := rbget(chn,c0*len/2);
out1:=dcb2(chn,x1+nx);
dcb(chn,out);
```
This example shows the use of the provided ring buffers.

#### Step by Step

`var delay_ms:= 2.1; var len:= delay_ms/1000*sr;` Define the delay length and compute
the buffer size in samples using the global variabel `sr` (sample rate).

`rblen(chn,len);`  set the ring buffer length for each channel. There are 16 ring buffers available with
maximum length of 48000. 

`var c0 := clamp((c+1)/2+y/10*a,0,0.99);`. Compute the relative read position of the buffer
using the knob value c and the input `y` attenuated with the knob value `a`.

`var x0 := rbget(chn,c0*len);` read the buffer value at the computed index.

`var nx := x/2 + x0 * d; rbpush(chn,nx);` mix with the input, 
add feedback attenuated with knob `d`and write it into the ring buffer.

`var out:=x0+nx`; store the output in the variable `out`.

`var x1 := rbget(chn,c0*len/2);`  read a second value from the buffer for a stereo effect.

`out1:=dcb2(chn,x1+nx);` output the 'right' stereo channel in output `out1` via 
setting the global variable `out1`. There are two dc blockers available per channel. 


`dcb(chn,out)`; output the left channel in `out`. (The value of the last expression is
always written into the output `out`).

### Stereo Delay

![](images/StereoDelay.png?raw=true)

```Tcl
var delayR:= 0.375;
var delayL:= 0.750;
var lenR:= delayR*sr;
var lenL:= delayL*sr;
rblen(0,lenL);
rblen(1,lenR);

var xL := rbget(0,0);
var xR := rbget(1,0);
rbpush(0,x+a*xL);
rbpush(1,x+a*xR);
out1:=dcb(0,(xR*b)+(x*(1-b)));
dcb(1,xL*b+x*(1-b));
```

Similar to the last example. The knob `a` controls the feedback and the knob `b` dry/wet.
The position in the call rbget is relative to the last write position + 1. With position 0 it returns the
sample which was written length samples before. `rbget(0,-1)` would return the last written sample.


### Polyphonic Random and Hold

![](images/RndH.png?raw=true)

```Tcl
if(st(chn,z)>0) {
  bufw(chn,rnd());
}
var out := bufr(chn)*10*a;
```
There are 4 Schmitt Triggers available per channel (st,st1,st2,st3).
The buffer is used to hold the value until the next trigger arrives.
NB the polyphony is determined by the input channels in the following way:
If the input t is connected the channels of t is used, otherwise the maximum of the
channels of the inputs w,x,y,z. The image above shows the case with one channel.

### Simple Sequencer

![](images/Sequencer.png?raw=true)

```Tcl
var seq[8] := { 0,3/12,5/12,
             7/12,10/12,7/12,
             5/12,3/12};

if(st(0,z)>0) { 
 v1:=v1+1;
 if(v1>=8) v1:=0;
} 
if(st1(0,y)>0) v1:=0;
var out := seq[v1];
```

This example shows the use of one of the 8 global variables v1-v8.
`v1` is used as the current position of the sequence which is advanced on arrival
on a trigger on input `z`. The input `y` is used for resetting the sequence.

### Chord Sequencer

![](images/Chords.png?raw=true)

```Tcl
if(v3==0) {
  var n := -1;
  for(var i:=0;i<36;i+=1) {
    bufw(i,n);
    n+=1/12;
  };
  v3:=1;
};
var chord[16]:={9,12,16,19, 9,12,14,18,
                7,11,14,18, 7,11,12,16};
if(st(0,z)>0) {v1+=1; if(v1>=4) v1:=0;}
var out: = bufr(chord[chn+v1*4]);

```

All variables and buffers are cleared if the script is compiled due to a change.
Here the variable v3 is used to fill a buffer with note values (V/Oct) only once.
(otherwise it costs much CPU).

Similar to the RndH example, a trigger on the input `z` advances the sequence. The notes of a single
chord are distributed in 4 channels.

### Gate Sequencer

![](images/GateSequencer.png?raw=true)

```Tcl
var data[16] := { 
1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,1};
var dur := 10; var gate:=0;
if(st(chn,z)>0) {
    if(data[v1]>0) {
      bufw(chn,dur);
    } else {
      bufw(chn,0);
    } 
};
if(bufr(chn)>0) { 
  gate:=10; bufw(chn,bufr(chn)-1);
};
if(st1(0,z)>0) {
  v1+=1;
  if(v1>=16) v1:=0;
};
var out:= gate;
```
This example shows how to make pulses. The duration is defined in samples.
When a trigger arrives and the current position (v1) has a one, a state variable 
 is set to the duration and decremented on each run until it is zero. 
While greater than zero the output is set to 10V.

### A Line Segment Envelope Generator

![](images/LineSeg.png?raw=true)

```Tcl
var vals[6]:={0,5,3,2,2,0};
var durs[5]:={0.3,0.2,0.5,0.3,0.2};
if(st(0,w)>0) { v1:=0; v2:=0; v3:=0; }
var end:=0;
if(v1>=v2+durs[v3]) { // advance
  if(v3>=5) end:=1;
  else { v2+=durs[v3]; v3+=1; }
};
var o;var pct;
if(end==1) {
  o:=vals[5]; 
} else {
  pct:= (v1-v2)/durs[v3];
  o:=vals[v3]+(vals[v3+1]-vals[v3])*pct;  
};
v1+=stim;
o;
```
Outputs line segments according to the values and duration arrays. It is triggered via the
`w` input.
As this is a common usecase the example can be simplified as so:
```Tcl
if(st(0,w)>0) { v1:=0; }
var o:=lseg(v1,0,0.3,5,0.2,3,0.5,2,0.3,
        2,0.2,0);
v1+=stim;
o;
```
The function `lseg` takes as shown the phase as first parameter then alternating value
and duration. If the last value is missing the first value is used instead.

Additionally there is a function eseg providing exponential segments

![](images/ExpSeg.png?raw=true)

```Tcl
var o:=eseg(v1, 0,0.3,-5, 5,0.5,-4,
               2,0.3,0, 2,0.5,-3, 0);
if(st(0,w)>0) v1:=0;

v1+=stim;
o;
```

The `eseg` function takes three alternating parameters value, duration, bending.
The sign of the bending parameter determines if the curve 
is fast decaying/raising (<0) or slow decaying/raising (>0). 


### Waveshaping
![](images/waveshaping1.png?raw=true)

This example shows the definition and use of a user defined function.
```Tcl
function sign(f) {
   f<0?-1:(f>0?1:0);
}
var p:= scl1(-a,0.01,10);
// waveshaping function
sign(x)*(1-p/(abs(x)+p))*5;
```

Functions must be defined always on top, before the normal code starts.
The return value of a function is always the value of the last expression.


There are some convenient functions provided for scaling:

`scl(input,minInput,maxInput,minOutput,maxOutput)`

this function scales the input expected in the range minInput,maxInput to the
target range minOutput,maxOutput.

`scl1` as shown in the example is a shortcut for 

`scl(input,-1,1,minOutput,maxOutput)`

-- suited for the knob inputs.

#### Wavshaping with exp segemnts

![](images/Waveshaping2.png?raw=true)

```Tcl
eseg(t,-1,0.5,a*10,1,0.5,b*10,-1)*5
```
This example shows the use of the `eseg` function (see above) for waveshaping.
Play around with knob a and b.

### CZ-Series PD Filter sweep

![](images/PDFilterSweep.png?raw=true)

```Tcl
var freq := 261.626 * pow(2,w);
var p:= bufr(chn);
var phs:= p+stim*freq;
if(phs>=1) phs:=0;
bufw(chn,phs);
var inv:=1-p;
out1:=p;
var freq2 := 261.626 * pow(2,a*5);
var p2:= buf2r(chn);
var o:= sin(2*pi*p2)*inv;
var phs2:= p2+stim*freq2;
phs2-=trunc(phs2);
if(phs==0) phs2:=0;
buf2w(chn,phs2);

var out:=o*5;
```

This script implements the
[Resonant Filter Simulation](https://en.wikipedia.org/wiki/Phase_distortion_synthesis)
from the Casio CZ Series. The knob `a` controls the resonant frequency.

