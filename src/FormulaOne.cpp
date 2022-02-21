#include "plugin.hpp"


#include "FormulaOne.hpp"


void FormulaOne::process(const ProcessArgs &args) {
  if(compiled) {
    sr=args.sampleRate;
    smpTime=args.sampleTime;
    a=params[A_PARAM].getValue();
    b=params[B_PARAM].getValue();
    c=params[C_PARAM].getValue();
    d=params[D_PARAM].getValue();
    int channels=1;
    if(inputs[P_INPUT].isConnected()) {
      channels=inputs[P_INPUT].getChannels();
    } else {
      channels=inputs[W_INPUT].getChannels();
      channels=std::max(channels,inputs[X_INPUT].getChannels());
      channels=std::max(channels,inputs[Y_INPUT].getChannels());
      channels=std::max(channels,inputs[Z_INPUT].getChannels());
      if(channels==0)
        channels=1;
    }
    for(int c=0;c<channels;c++) {
      channel=c;
      if(inputs[P_INPUT].isConnected())
        t=(inputs[P_INPUT].getVoltage(c)+5.f)/10.f; // normalize to [0,1]
      if(inputs[X_INPUT].isConnected())
        x=inputs[X_INPUT].getPolyVoltage(c);
      if(inputs[Y_INPUT].isConnected())
        y=inputs[Y_INPUT].getPolyVoltage(c);
      if(inputs[Z_INPUT].isConnected())
        z=inputs[Z_INPUT].getPolyVoltage(c);
      if(inputs[W_INPUT].isConnected())
        w=inputs[W_INPUT].getPolyVoltage(c);
      float out=expression.value();
      outputs[V_OUTPUT].setVoltage(std::isnan(out)?0.f:out,c);
      outputs[V1_OUTPUT].setVoltage(std::isnan(out1)?0.f:out1,c);
      outputs[V2_OUTPUT].setVoltage(std::isnan(out2)?0.f:out2,c);
    }
    outputs[V_OUTPUT].setChannels(channels);
    outputs[V1_OUTPUT].setChannels(channels);
    outputs[V2_OUTPUT].setChannels(channels);
  }
  blinkPhase+=args.sampleTime;
  if(blinkPhase>=1.0f)
    blinkPhase-=1.0f;
  if(compiled) {
    lights[OK_LIGHT].value=0;
    lights[ERROR_LIGHT].value=1;
  } else {
    lights[OK_LIGHT].value=(blinkPhase<0.5f)?1.0f:0.0f;
    lights[ERROR_LIGHT].value=0;
  }
}

struct FormulaTextField : MTextField {
  FormulaOne *module;

  FormulaTextField() : MTextField() {
    //bgColor=nvgRGB(0x30,0x30,0x30);
  }

  void setModule(FormulaOne *module) {
    this->module=module;
  }

  void onChange(const event::Change &e) override {
    if(module) {
      module->formula=getText();
      module->compile();
      module->extDirty=true;
    }
  }

  void draw(const DrawArgs &args) override {
    if(module&&(module->dirty)) {
      int cursorSave=cursor;
      setText(module->formula);
      cursor=selection=cursorSave;
      module->dirty=false;
      module->extDirty=true;
    }
    MTextField::draw(args);
  }
};

struct FormulaOneWidget : ModuleWidget {
  float ms=mm2px(Vec(5.08f,0)).x;

  FormulaOneWidget(FormulaOne *module) {
    ScrollWidget *scrollWidget;
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance,"res/FormulaOne.svg")));

    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH,0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x-2*RACK_GRID_WIDTH,0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH,RACK_GRID_HEIGHT-RACK_GRID_WIDTH)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x-2*RACK_GRID_WIDTH,RACK_GRID_HEIGHT-RACK_GRID_WIDTH)));
    //auto textDisplay = createWidget<LedDisplay>(mm2px(Vec(3, 13)));
    //textDisplay->box.size = mm2px(Vec(85, 51));
    //addChild(textDisplay);
    scrollWidget=new ScrollWidget();
    scrollWidget->box.size=mm2px(Vec(85,68));
    scrollWidget->box.pos=mm2px(Vec(3.5f,MHEIGHT-54-68));
    addChild(scrollWidget);
    auto textField=createWidget<FormulaTextField>(Vec(0,0));
    textField->setModule(module);
    textField->box.size=mm2px(Vec(200,160));
    textField->multiline=true;
    scrollWidget->container->addChild(textField);
    addChild(createLight<MediumLight<GreenRedLight>>(mm2px(Vec(45,MHEIGHT-29)),module,FormulaOne::ERROR_LIGHT));
    addParam(createParam<TrimbotWhite9>(mm2px(Vec(8.75,MHEIGHT-34.5-8.985f)),module,FormulaOne::A_PARAM));
    addParam(createParam<TrimbotWhite9>(mm2px(Vec(21.5,MHEIGHT-34.5-8.985f)),module,FormulaOne::B_PARAM));
    addParam(createParam<TrimbotWhite9>(mm2px(Vec(34.5,MHEIGHT-34.5-8.985f)),module,FormulaOne::C_PARAM));
    addParam(createParam<TrimbotWhite9>(mm2px(Vec(47.5,MHEIGHT-34.5-8.985f)),module,FormulaOne::D_PARAM));
    addParam(createParam<TrimbotWhite9>(mm2px(Vec(60.5,MHEIGHT-34.5-8.985f)),module,FormulaOne::E_PARAM));
    addInput(createInput<SmallPort>(mm2px(Vec(10,MHEIGHT-10-6.27f)),module,FormulaOne::P_INPUT));
    addInput(createInput<SmallPort>(mm2px(Vec(23,MHEIGHT-10-6.27f)),module,FormulaOne::W_INPUT));
    addInput(createInput<SmallPort>(mm2px(Vec(36,MHEIGHT-10-6.27f)),module,FormulaOne::X_INPUT));
    addInput(createInput<SmallPort>(mm2px(Vec(49,MHEIGHT-10-6.27f)),module,FormulaOne::Y_INPUT));
    addInput(createInput<SmallPort>(mm2px(Vec(62,MHEIGHT-10-6.27f)),module,FormulaOne::Z_INPUT));

    addOutput(createOutput<SmallPort>(mm2px(Vec(79.212,MHEIGHT-10-6.27f)),module,FormulaOne::V_OUTPUT));
    addOutput(createOutput<SmallPort>(mm2px(Vec(79.212,MHEIGHT-24-6.27f)),module,FormulaOne::V1_OUTPUT));
    addOutput(createOutput<SmallPort>(mm2px(Vec(79.212,MHEIGHT-38-6.27f)),module,FormulaOne::V2_OUTPUT));
  }

};


Model *modelFormulaOne=createModel<FormulaOne,FormulaOneWidget>("FormulaOne");