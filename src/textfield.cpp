#include "textfield.hpp"
#include <helpers.hpp>
#include <context.hpp>

struct MTextFieldCopyItem : ui::MenuItem {
  WeakPtr <MTextField> textField;

  void onAction(const ActionEvent &e) override {
    if(!textField)
      return;
    textField->copyClipboard();
    APP->event->setSelectedWidget(textField);
  }
};


struct MTextFieldCutItem : ui::MenuItem {
  WeakPtr <MTextField> textField;

  void onAction(const ActionEvent &e) override {
    if(!textField)
      return;
    textField->cutClipboard();
    APP->event->setSelectedWidget(textField);
  }
};


struct MTextFieldPasteItem : ui::MenuItem {
  WeakPtr <MTextField> textField;

  void onAction(const ActionEvent &e) override {
    if(!textField)
      return;
    textField->pasteClipboard();
    APP->event->setSelectedWidget(textField);
  }
};


struct MTextFieldSelectAllItem : ui::MenuItem {
  WeakPtr <MTextField> textField;

  void onAction(const ActionEvent &e) override {
    if(!textField)
      return;
    textField->selectAll();
    APP->event->setSelectedWidget(textField);
  }
};


MTextField::MTextField() {
  fontPath=asset::plugin(pluginInstance,"res/FreeMonoBold.ttf");
}

int MTextField::_bndIconLabelTextPosition(NVGcontext *ctx,float x,float y,float w,float h,float fontsize,const char *label,int px,int py) {
  std::shared_ptr<window::Font> font=APP->window->loadFont(fontPath);
  float bounds[4];
  float pleft=BND_TEXT_RADIUS;
  if(!label)
    return -1;

  if(font&&font->handle>=0) {

    x+=pleft;
    y+=BND_WIDGET_HEIGHT-BND_TEXT_PAD_DOWN;

    nvgFontFaceId(ctx,font->handle);
    nvgFontSize(ctx,fontsize);
    nvgTextAlign(ctx,NVG_ALIGN_LEFT|NVG_ALIGN_BASELINE);

    w-=BND_TEXT_RADIUS+pleft;

    float asc,desc,lh;

    int nrows=nvgTextBreakLines(ctx,label,NULL,w,rows,_BND_MAX_ROWS);
    if(nrows==0)
      return 0;
    nvgTextBoxBounds(ctx,x,y,w,label,NULL,bounds);
    nvgTextMetrics(ctx,&asc,&desc,&lh);

    // calculate vertical position
    int row=_clamp((int)((float)(py-bounds[1])/lh),0,nrows-1);
    // search horizontal position
    int nglyphs=nvgTextGlyphPositions(ctx,x,y,rows[row].start,rows[row].end+1,glyphs,_BND_MAX_GLYPHS);
    int col,p=0;
    for(col=0;col<nglyphs&&glyphs[col].x<px;++col)
      p=glyphs[col].str-label;
    // see if we should move one character further
    if(col>0&&col<nglyphs&&glyphs[col].x-px<px-glyphs[col-1].x)
      p=glyphs[col].str-label;
    return p;
  } else {
    return -1;
  }
}

void MTextField::_bndCaretPosition(NVGcontext *ctx,float x,float y,float desc,float lineHeight,const char *caret,NVGtextRow *rows,int nrows,int *cr,float *cx,float *cy) {
  static NVGglyphPosition glyphs[_BND_MAX_GLYPHS];
  int r,nglyphs;
  for(r=0;r<nrows-1&&rows[r].end<caret;++r);
  *cr=r;
  *cx=x;
  *cy=y-lineHeight-desc+r*lineHeight;
  if(nrows==0)
    return;
  *cx=rows[r].minx;
  nglyphs=nvgTextGlyphPositions(ctx,x,y,rows[r].start,rows[r].end+1,glyphs,_BND_MAX_GLYPHS);
  for(int i=0;i<nglyphs;++i) {
    *cx=glyphs[i].x;
    if(glyphs[i].str==caret)
      break;
  }
}
void MTextField::getCursorPosition(float &px, float& py) {
  std::shared_ptr<window::Font> font=APP->window->loadFont(fontPath);
  float pleft=BND_TEXT_RADIUS;
  int cbegin=std::min(cursor,selection);
  int cend=std::max(cursor,selection);
  float x=0; float y=0; float w=box.size.x;
  NVGcontext *ctx = APP->window->vg;
  if(font&&font->handle>=0) {

    x+=pleft;
    y+=BND_WIDGET_HEIGHT-BND_TEXT_PAD_DOWN;

    nvgFontFaceId(ctx,font->handle);
    nvgFontSize(ctx,fontSize);
    nvgTextAlign(ctx,NVG_ALIGN_LEFT|NVG_ALIGN_BASELINE);

    w-=BND_TEXT_RADIUS+pleft;

    if(cend>=cbegin) {
      int c0r,c1r;
      float c0x,c0y,c1x,c1y;
      float desc,lh;
      static NVGtextRow rows[_BND_MAX_ROWS];
      int nrows=nvgTextBreakLines(ctx,text.c_str(),text.c_str()+cend+1,w,rows,_BND_MAX_ROWS);
      nvgTextMetrics(ctx,NULL,&desc,&lh);

      _bndCaretPosition(ctx,x,y,desc,lh,text.c_str()+cbegin,rows,nrows,&c0r,&c0x,&c0y);
      px=c0x;
      py=c0y;
    }
  }
}
void MTextField::_bndIconLabelCaret(NVGcontext *ctx,float x,float y,float w,float h,NVGcolor color,float fontsize,const char *label,NVGcolor caretcolor,int cbegin,int cend) {
  std::shared_ptr<window::Font> font=APP->window->loadFont(fontPath);
  float pleft=BND_TEXT_RADIUS;
  if(!label)
    return;

  if(font&&font->handle>=0) {

    x+=pleft;
    y+=BND_WIDGET_HEIGHT-BND_TEXT_PAD_DOWN;

    nvgFontFaceId(ctx,font->handle);
    nvgFontSize(ctx,fontsize);
    nvgTextAlign(ctx,NVG_ALIGN_LEFT|NVG_ALIGN_BASELINE);

    w-=BND_TEXT_RADIUS+pleft;

    if(cend>=cbegin) {
      int c0r,c1r;
      float c0x,c0y,c1x,c1y;
      float desc,lh;
      static NVGtextRow rows[_BND_MAX_ROWS];
      int nrows=nvgTextBreakLines(ctx,label,label+cend+1,w,rows,_BND_MAX_ROWS);
      nvgTextMetrics(ctx,NULL,&desc,&lh);

      _bndCaretPosition(ctx,x,y,desc,lh,label+cbegin,rows,nrows,&c0r,&c0x,&c0y);
      _bndCaretPosition(ctx,x,y,desc,lh,label+cend,rows,nrows,&c1r,&c1x,&c1y);

      nvgBeginPath(ctx);
      if(cbegin==cend) {
        nvgFillColor(ctx,nvgRGBf(0.337,0.502,0.761));
        nvgRect(ctx,c0x-1,c0y,2,lh+1);
      } else {
        nvgFillColor(ctx,caretcolor);
        if(c0r==c1r) {
          nvgRect(ctx,c0x-1,c0y,c1x-c0x+1,lh+1);
        } else {
          int blk=c1r-c0r-1;
          nvgRect(ctx,c0x-1,c0y,x+w-c0x+1,lh+1);
          nvgRect(ctx,x,c1y,c1x-x+1,lh+1);
          if(blk)
            nvgRect(ctx,x,c0y+lh,w,blk*lh+1);
        }
      }
      nvgFill(ctx);
    }

    nvgBeginPath(ctx);
    nvgFillColor(ctx,color);
    nvgTextBox(ctx,x,y,w,label,NULL);
  }
}


void MTextField::draw(const DrawArgs &args) {
  nvgScissor(args.vg,RECT_ARGS(args.clipBox));

  int begin=std::min(cursor,selection);
  int end=std::max(cursor,selection);

  _bndIconLabelCaret(args.vg,0.0,0.0,box.size.x,box.size.y,nvgRGB(128,255,128),fontSize,text.c_str(),nvgRGBA(128,128,128,128),begin,end);

  // Draw placeholder text
  if(text.empty()) {
    _bndIconLabelCaret(args.vg,0.0,0.0,box.size.x,box.size.y,bndGetTheme()->textFieldTheme.itemColor,fontSize,placeholder.c_str(),bndGetTheme()->textFieldTheme.itemColor,0,-1);
  }

  nvgResetScissor(args.vg);
}

void MTextField::onDragHover(const DragHoverEvent &e) {
  OpaqueWidget::onDragHover(e);

  if(e.origin==this) {
    int pos=getTextPosition(e.pos);
    cursor=pos;
  }
}

void MTextField::onButton(const ButtonEvent &e) {
  OpaqueWidget::onButton(e);

  if(e.action==GLFW_PRESS&&e.button==GLFW_MOUSE_BUTTON_LEFT) {
    cursor=selection=getTextPosition(e.pos);
  }

  if(e.action==GLFW_PRESS&&e.button==GLFW_MOUSE_BUTTON_RIGHT) {
    createContextMenu();
    e.consume(this);
  }
}

void MTextField::onSelectText(const SelectTextEvent &e) {
  if(e.codepoint<128) {
    std::string newText(1,(char)e.codepoint);
    insertText(newText);
  }
  e.consume(this);
}

void MTextField::split() {
  char prev=0;
  int rk=0;
  int start=0;
  for(int k=0;k<text.size();k++) {
    if((text[k]=='\r'||text[k]=='\n')) {
      if((prev=='\r'&& text[k]=='\n')||(prev=='\n'&& text[k]=='\r')) {
        start++;
      } else {
        textRows[rk].begin=start;
        textRows[rk++].end=k;
        start=k+1;
      }
    }
    prev=text[k];
  }
  if(rk>0) {
    textRows[rk].begin=text.size();
    textRows[rk++].end=text.size();
  }
  currentMax=rk;
}

void MTextField::onSelectKey(const SelectKeyEvent &e) {
  if(e.action==GLFW_PRESS||e.action==GLFW_REPEAT) {
    // Backspace
    if(e.key==GLFW_KEY_BACKSPACE&&(e.mods&RACK_MOD_MASK)==0) {
      if(cursor==selection) {
        cursor=std::max(cursor-1,0);
      }
      insertText("");
      e.consume(this);
    }
    // Ctrl+Backspace
    if(e.key==GLFW_KEY_BACKSPACE&&(e.mods&RACK_MOD_MASK)==RACK_MOD_CTRL) {
      if(cursor==selection) {
        cursorToPrevWord();
      }
      insertText("");
      e.consume(this);
    }
    // Delete
    if(e.key==GLFW_KEY_DELETE&&(e.mods&RACK_MOD_MASK)==0) {
      if(cursor==selection) {
        cursor=std::min(cursor+1,(int)text.size());
      }
      insertText("");
      e.consume(this);
    }
    // Ctrl+Delete
    if(e.key==GLFW_KEY_DELETE&&(e.mods&RACK_MOD_MASK)==RACK_MOD_CTRL) {
      if(cursor==selection) {
        cursorToNextWord();
      }
      insertText("");
      e.consume(this);
    }
    // Left
    if(e.key==GLFW_KEY_LEFT) {
      if((e.mods&RACK_MOD_MASK)==RACK_MOD_CTRL) {
        cursorToPrevWord();
      } else {
        cursor=std::max(cursor-1,0);
      }
      if(!(e.mods&GLFW_MOD_SHIFT)) {
        selection=cursor;
      }
      e.consume(this);
    }
    // Right
    if(e.key==GLFW_KEY_RIGHT) {
      if((e.mods&RACK_MOD_MASK)==RACK_MOD_CTRL) {
        cursorToNextWord();
      } else {
        cursor=std::min(cursor+1,(int)text.size());
      }
      if(!(e.mods&GLFW_MOD_SHIFT)) {
        selection=cursor;
      }
      e.consume(this);
    }

    if(e.key==GLFW_KEY_UP) {
      if(cursor>0) {
        //INFO("cursor=%d",cursor);
        split();
        for(int k=0;k<currentMax;k++) {
          //INFO("%d %d",textRows[k].begin,textRows[k].end);
        }
        for(int k=0;k<currentMax;k++) {
          if(cursor>=textRows[k].begin&&cursor<=textRows[k].end) {
            //INFO("row=%d",k);
            if(k>0) {
              int pos=std::min(cursor-textRows[k].begin,textRows[k-1].end-textRows[k-1].begin);
              cursor=textRows[k-1].begin+pos;
            }
            break;
          }
        }

        if(!(e.mods&GLFW_MOD_SHIFT)) {
          selection=cursor;
        }
      }
      e.consume(this);
    }

    if(e.key==GLFW_KEY_DOWN) {
      if(cursor<text.size()) {
        split();
        for(int k=0;k<currentMax;k++) {
          //INFO("%d %d",textRows[k].begin,textRows[k].end);
        }
        for(int k=0;k<currentMax;k++) {
          if(cursor>=textRows[k].begin&&cursor<=textRows[k].end) {
            //INFO("row=%d",k);
            if(k<currentMax-1) {
              int pos=std::min(cursor-textRows[k].begin,textRows[k+1].end-textRows[k+1].begin);
              cursor=textRows[k+1].begin+pos;
            }
            break;
          }
        }
      }

      if(!(e.mods&GLFW_MOD_SHIFT)) {
        selection=cursor;
      }
      e.consume(this);
    }

    // Home
    if(e.key==GLFW_KEY_HOME&&(e.mods&RACK_MOD_MASK)==0) {
      selection=cursor=0;
      e.consume(this);
    }
    // Shift+Home
    if(e.key==GLFW_KEY_HOME&&(e.mods&RACK_MOD_MASK)==GLFW_MOD_SHIFT) {
      cursor=0;
      e.consume(this);
    }
    // End
    if(e.key==GLFW_KEY_END&&(e.mods&RACK_MOD_MASK)==0) {
      selection=cursor=text.size();
      e.consume(this);
    }
    // Shift+End
    if(e.key==GLFW_KEY_END&&(e.mods&RACK_MOD_MASK)==GLFW_MOD_SHIFT) {
      cursor=text.size();
      e.consume(this);
    }
    // Ctrl+V
    if(e.keyName=="v"&&(e.mods&RACK_MOD_MASK)==RACK_MOD_CTRL) {
      pasteClipboard(false);
      e.consume(this);
    }
    // Ctrl+X
    if(e.keyName=="x"&&(e.mods&RACK_MOD_MASK)==RACK_MOD_CTRL) {
      cutClipboard(false);
      e.consume(this);
    }
    // Ctrl+C
    if(e.keyName=="c"&&(e.mods&RACK_MOD_MASK)==RACK_MOD_CTRL) {
      copyClipboard(false);
      e.consume(this);
    }
    // Ctrl+A
    if(e.keyName=="a"&&(e.mods&RACK_MOD_MASK)==RACK_MOD_CTRL) {
      selectAll();
      e.consume(this);
    }
    // Enter
    if((e.key==GLFW_KEY_ENTER||e.key==GLFW_KEY_KP_ENTER)&&(e.mods&RACK_MOD_MASK)==0) {
      if(multiline) {
        insertText("\n");
      } else {
        ActionEvent eAction;
        onAction(eAction);
      }
      e.consume(this);
    }
    // Tab
    if(e.key==GLFW_KEY_TAB&&(e.mods&RACK_MOD_MASK)==0) {
      if(nextField)
        APP->event->setSelectedWidget(nextField);
      e.consume(this);
    }
    // Shift-Tab
    if(e.key==GLFW_KEY_TAB&&(e.mods&RACK_MOD_MASK)==GLFW_MOD_SHIFT) {
      if(prevField)
        APP->event->setSelectedWidget(prevField);
      e.consume(this);
    }
    // Consume all printable keys
    if(e.keyName!="") {
      e.consume(this);
    }
    float px,py;
    getCursorPosition(px,py);
    //INFO("%f %f %f %f %f",px,py,scroll->offset.x,scroll->offset.y,scroll->box.size.y);
    while(py>scroll->box.size.y-24+scroll->offset.y) scroll->offset.y+=10;
    while(py<scroll->offset.y) scroll->offset.y-=10;
    if(scroll->offset.y<0) scroll->offset.y = 0;
    assert(0<=cursor);
    assert(cursor<=(int)text.size());
    assert(0<=selection);
    assert(selection<=(int)text.size());
  }
}

int MTextField::getTextPosition(math::Vec mousePos) {
  return _bndIconLabelTextPosition(APP->window->vg,0.0,0.0,box.size.x,box.size.y,fontSize,text.c_str(),mousePos.x,mousePos.y);
}

std::string MTextField::getText() {
  return text;
}

void MTextField::setText(std::string text) {
  if(this->text!=text) {
    this->text=text;
    // ChangeEvent
    ChangeEvent eChange;
    onChange(eChange);
  }
  selection=cursor=text.size();
}

void MTextField::selectAll() {
  cursor=text.size();
  selection=0;
}

std::string MTextField::getSelectedText() {
  int begin=std::min(cursor,selection);
  int len=std::abs(selection-cursor);
  return text.substr(begin,len);
}

void MTextField::insertText(std::string text) {
  bool changed=false;
  if(cursor!=selection) {
    // Delete selected text
    int begin=std::min(cursor,selection);
    int len=std::abs(selection-cursor);
    this->text.erase(begin,len);
    cursor=selection=begin;
    changed=true;
  }
  if(!text.empty()) {
    this->text.insert(cursor,text);
    cursor+=text.size();
    selection=cursor;
    changed=true;
  }
  if(changed) {
    ChangeEvent eChange;
    onChange(eChange);
  }
}

void MTextField::copyClipboard(bool menu) {
  if(cursor==selection)
    return;
  glfwSetClipboardString(APP->window->win,getSelectedText().c_str());
}

void MTextField::cutClipboard(bool menu) {
  copyClipboard();
  insertText("");
}

void MTextField::pasteClipboard(bool menu) {
  const char *newText=glfwGetClipboardString(APP->window->win);
  if(!newText)
    return;
  insertText(newText);
}

void MTextField::cursorToPrevWord() {
  size_t pos=text.rfind(' ',std::max(cursor-2,0));
  if(pos==std::string::npos)
    cursor=0;
  else
    cursor=std::min((int)pos+1,(int)text.size());
}

void MTextField::cursorToNextWord() {
  size_t pos=text.find(' ',std::min(cursor+1,(int)text.size()));
  if(pos==std::string::npos)
    pos=text.size();
  cursor=pos;
}

void MTextField::createContextMenu() {
  menu=createMenu();

  MTextFieldCutItem *cutItem=new MTextFieldCutItem;
  cutItem->text="Cut";
  cutItem->rightText=RACK_MOD_CTRL_NAME
  "+X";
  cutItem->textField=this;
  menu->addChild(cutItem);

  MTextFieldCopyItem *copyItem=new MTextFieldCopyItem;
  copyItem->text="Copy";
  copyItem->rightText=RACK_MOD_CTRL_NAME
  "+C";
  copyItem->textField=this;
  menu->addChild(copyItem);

  MTextFieldPasteItem *pasteItem=new MTextFieldPasteItem;
  pasteItem->text="Paste";
  pasteItem->rightText=RACK_MOD_CTRL_NAME
  "+V";
  pasteItem->textField=this;
  menu->addChild(pasteItem);

  MTextFieldSelectAllItem *selectAllItem=new MTextFieldSelectAllItem;
  selectAllItem->text="Select all";
  selectAllItem->rightText=RACK_MOD_CTRL_NAME
  "+A";
  selectAllItem->textField=this;
  menu->addChild(selectAllItem);
}
