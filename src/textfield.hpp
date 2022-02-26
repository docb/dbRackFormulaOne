#pragma once

// From Rack ui but modified to have more possibilities to customize.
// Uses also some functions of the blender utilities directly to provide more lines in the text.
// Adds the feature to move up and down using the arrow keys.

#include "plugin.hpp"
#define _BND_MAX_ROWS 100
#define _BND_MAX_GLYPHS 10240
struct TextRow {
  int begin;
  int end;
};
struct MTextField : OpaqueWidget {
  ScrollWidget *scroll;
  std::string text;
  std::string placeholder;
  /** Masks text with "*". */
  bool password=false;
  bool multiline=false;
  int fontSize = 10;
  /** The index of the text cursor */
  int cursor=0;
  /** The index of the other end of the selection.
  If nothing is selected, this is equal to `cursor`.
  */
  int selection=0;

  int _bndIconLabelTextPosition(NVGcontext *ctx,float x,float y,float w,float h,float fontsize,const char *label,int px,int py);

  void _bndCaretPosition(NVGcontext *ctx,float x,float y,float desc,float lineHeight,const char *caret,NVGtextRow *rows,int nrows,int *cr,float *cx,float *cy);

  void _bndIconLabelCaret(NVGcontext *ctx,float x,float y,float w,float h,NVGcolor color,float fontsize,const char *label,NVGcolor caretcolor,int cbegin,int cend);

  /** For Tab and Shift-Tab focusing.
  */
  widget::Widget *prevField=NULL;
  widget::Widget *nextField=NULL;
  ui::Menu *menu;
  std::basic_string<char> fontPath;
  NVGtextRow rows[_BND_MAX_ROWS];
  TextRow textRows[_BND_MAX_ROWS];
  NVGglyphPosition glyphs[_BND_MAX_GLYPHS];
  float _clamp(float v,float mn,float mx) {
    return (v>mx)?mx:(v<mn)?mn:v;
  }
  void split();
  int currentMax=0;
  MTextField();

  virtual void draw(const DrawArgs &args) override;

  void onDragHover(const DragHoverEvent &e) override;

  void onButton(const ButtonEvent &e) override;

  void onSelectText(const SelectTextEvent &e) override;

  virtual void onSelectKey(const SelectKeyEvent &e) override;

  virtual int getTextPosition(math::Vec mousePos);

  std::string getText();

  /** Replaces the entire text */
  void setText(std::string text);

  void selectAll();

  std::string getSelectedText();

  /** Inserts text at the cursor, replacing the selection if necessary */
  void insertText(std::string text);

  virtual void copyClipboard(bool menu=true);

  virtual void cutClipboard(bool menu=true);

  virtual void pasteClipboard(bool menu=true);

  void cursorToPrevWord();

  void cursorToNextWord();

  virtual void createContextMenu();

  void getCursorPosition(float &px,float &py);
};
