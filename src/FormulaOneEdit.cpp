#include "plugin.hpp"
#include "textfield.hpp"
#include "FormulaOne.hpp"
extern Model *modelFormulaOne;
struct FormulaOneEdit : Module {
	enum ParamId {
		PARAMS_LEN
	};
	enum InputId {
		INPUTS_LEN
	};
	enum OutputId {
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHTS_LEN
	};
  FormulaOne *formel1=nullptr;

  FormulaOneEdit() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
	}

	void process(const ProcessArgs& args) override {
    if(leftExpander.module) {
      if(leftExpander.module->model==modelFormulaOne) {
        if(formel1==nullptr) {
          formel1=reinterpret_cast<FormulaOne *>(leftExpander.module);
          formel1->extDirty=true;
        }
      } else {
        formel1=nullptr;
      }
    } else {
      formel1=nullptr;
    }
	}
};
struct TestTextField : MTextField {

  TestTextField() : MTextField() {
  }

  void onChange(const event::Change &e) override {
  }

  void draw(const DrawArgs& args) override {
    MTextField::draw(args);
  }
};

struct ExtFormulaTextField : MTextField {
  FormulaOneEdit *module;

  ExtFormulaTextField() : MTextField() {
    //bgColor=nvgRGB(0x30,0x30,0x30);
  }

  void setModule(FormulaOneEdit *module) {
    this->module=module;
  }

  void onChange(const event::Change &e) override {
    if(module&&module->formel1) {
      module->formel1->formula=getText();
      module->formel1->compile();
      module->formel1->dirty=true;
    }
  }

  void draw(const DrawArgs &args) override {
    if(module&&module->formel1&&module->formel1->extDirty) {
      int cursorSave=cursor;
      setText(module->formel1->formula.substr(0));
      cursor=selection=cursorSave;
      module->formel1->extDirty=false;
    } else {

    }
    MTextField::draw(args);
  }
};

struct FormulaOneEditWidget : ModuleWidget {
  ScrollWidget *scrollWidget;
  FormulaOneEditWidget(FormulaOneEdit* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/FormulaOneEdit.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    scrollWidget = new TextScrollWidget();
    scrollWidget->box.size=mm2px(Vec(176,115));
    scrollWidget->box.pos = mm2px(Vec(3.5f,MHEIGHT-7-115));
    INFO("%f",scrollWidget->box.size.y);
    addChild(scrollWidget);
    auto textField=createWidget<ExtFormulaTextField>(Vec(0,0));
    textField->box.size=mm2px(Vec(200,400));
    textField->multiline=true;
    textField->setModule(module);
    textField->scroll = scrollWidget;

    scrollWidget->container->addChild(textField);
	}
};


Model* modelFormulaOneEdit = createModel<FormulaOneEdit, FormulaOneEditWidget>("FormulaOneEdit");