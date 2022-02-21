
# Formula One

A script/formula evaluation module for VCVRack. This module can do almost anything, and it is quite fast.

![](images/FormulaOne.png?raw=true)

It is based on the [exprtk expression library](http://www.partow.net/programming/exprtk/index.html) 
which has very good [benchmarks](https://github.com/ArashPartow/math-parser-benchmark-project#the-rounds).

## Examples and Usecases
<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->

- [A simple polyphonic oscillator](#a-simple-polyphonic-oscillator)
  - [Step by Step](#step-by-step)
- [An external phase controlled Oscillator](#an-external-phase-controlled-oscillator)
- [A Wave Folder](#a-wave-folder)
- [A Simple Polyphonic Filter](#a-simple-polyphonic-filter)
  - [Step by Step](#step-by-step-1)
- [A Polyphonic Comb Filter (Chorus,Flanger or whatever)](#a-polyphonic-comb-filter-chorusflanger-or-whatever)
  - [Step by Step](#step-by-step-2)
- [A Stereo Delay](#a-stereo-delay)
- [A Polyphonic Random and Hold](#a-polyphonic-random-and-hold)
- [A Simple Sequencer](#a-simple-sequencer)
- [A Chord Sequencer](#a-chord-sequencer)
- [A Drum Sequencer](#a-drum-sequencer)
- [A Line Segement Envelope Generator](#a-line-segement-envelope-generator)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

### A simple polyphonic oscillator

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

### An external phase controlled Oscillator
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

### A Wave Folder
![](images/Wavefolder.png?raw=true)

`sin((c*5)*t*2*pi+b*5)*a*5`

The same principle can be used to make a wave folder.
The knob `c` controls the depth and the knob `b` controls the offset.

### A Simple Polyphonic Filter

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

### A Polyphonic Comb Filter (Chorus,Flanger or whatever)

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

### A Stereo Delay

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


### A Polyphonic Random and Hold

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

### A Simple Sequencer

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

### A Chord Sequencer

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
                 7,11,14,18,  7,11,12,16};
if(st(0,z)>0) {v1+=1; if(v1>=4) v1:=0;}
var out: = bufr(chord[chn+v1*4]);

```

All variables and buffers are cleared if the script is compiled due to a change.
Here the variable v3 is used to fill a buffer with note values (V/Oct) only once.
(otherwise it costs much CPU).

Similar to the RndH example, a trigger on the input `z` advances the sequence. The notes of a single
chord are distributed in 4 channels.

### A Drum Sequencer

![](images/Drums.png?raw=true)

```Tcl
var bd[16] := { 1,0,0,1,0,0,0,0,1,0,0,0,1,0,0,1};
var sn[16] := { 0,1,0,0,1,0,0,0,0,0,0,0,1,0,0,0};
var hh[16] := { 0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,1};
var dur := 10; // pulse duration in samples
var p:=0; 
if(chn==0) {
   if(st(chn,z)>0) {
     if(bd[v1]>0)  v2:=dur; else v2:=0;
   };
   if(v2>0) { p:=10; v2-=1;};
};  
if(chn==1) {
   if(st(chn,z)>0) {
     if(sn[v1]>0)  v3:=dur; else v3:=0;
   };
    if(v3>0) { p:=10; v3-=1;}
};
if(chn==2) {
   if(st(chn,z)>0) {
     if(hh[v1]>0)  v4:=dur; else v4:=0;
   };
   if(st1(0,z)>0) {
    v1+=1;
    if(v1>=16) v1:=0;
   }
   if(v4>0) { p:=10; v4-=1;}
};

var out:= p;
```
This example shows how to make pulses. The duration is defined in samples.
When a trigger arrives and the current position (v1) has a one, a state variable 
(here v2,v3,v4) is set to the duration and decremented
on each run until it is zero. While greater than zero the output is set to 10V.
NB the values for bd,sn and hh can be changed while running, but on every change the
state variables are reset to zero (especially the sequence position v1) 
and the sequence starts from the beginning.

Further ideas: set the parameters for bd,sn,hh to values in between zero and one and
output the current value on output 1 for using as a velocity.

### A Line Segement Envelope Generator

![](images/LineSeg.png?raw=true)

```Tcl
var vals[6]:={0,5,3,2,2,0};
var durs[5]:={0.1,0.2,0.5,0.3,0.2};
if(st(0,w)>0) v1:=0;
var pos:= v1; var idx :=-1;
var ps := 0;var i:=0;
repeat
  ps+=durs[i];
  i+=1;
until ((i>durs[]) or (pos<ps));
if(i<=durs[]) idx:=i-1;
var o; var pct;
if(idx==-1) o:=vals[5]; else {
  pct:= (durs[idx]-(ps-pos))/durs[idx];
  o:= vals[idx]+(vals[idx+1]-vals[idx])*pct;
};
v1+=stim;
var out:=o;  

```
Outputs line segments according to the values and duration arrays. It is triggered via the
`w` input.  NB and TBD: this algorithm can be made much better and faster.

