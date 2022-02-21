#include "plugin.hpp"


Plugin* pluginInstance;

extern Model* modelFormulaOne;
extern Model* modelFormulaOneEdit;

void init(Plugin* p) {
	pluginInstance = p;

	// Add modules here
	p->addModel(modelFormulaOne);
  p->addModel(modelFormulaOneEdit);

	// Any other plugin initialization may go here.
	// As an alternative, consider lazy-loading assets and lookup tables when your module is created to reduce startup times of Rack.
}
