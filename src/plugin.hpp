#pragma once
#include <rack.hpp>


using namespace rack;

// Declare the Plugin, defined in plugin.cpp
extern Plugin* pluginInstance;

// Declare each Model, defined in each module source file
// extern Model* modelMyModule;
#define TWOPIF 6.2831853f
#define MHEIGHT 128.5f

template<typename T>
struct DCBlocker {
  T x = 0.f;
  T y = 0.f;
  T process(T v) {
    y = v - x + y * 0.99f;
    x = v;
    return y;
  }
  void reset() {
    x = 0.f;
    y = 0.f;
  }
};
struct TrimbotWhite : SvgKnob {
  TrimbotWhite() {
    minAngle=-0.8*M_PI;
    maxAngle=0.8*M_PI;
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance,"res/TrimpotWhite.svg")));
  }
};

struct TrimbotWhite9 : SvgKnob {
  TrimbotWhite9() {
    minAngle=-0.8*M_PI;
    maxAngle=0.8*M_PI;
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance,"res/TrimpotWhite9mm.svg")));
  }
};

struct SmallPort : app::SvgPort {
  SmallPort() {
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance,"res/SmallPort.svg")));
  }
};
