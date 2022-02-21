//
// Created by docb on 2/20/22.
//

#ifndef DBCAEMODULES_FASTFORMULA_HPP
#define DBCAEMODULES_FASTFORMULA_HPP

#include "exprtk.hpp"
#include "rnd.h"
#include "textfield.hpp"
#include <thread>
typedef exprtk::symbol_table<float> symbol_table_t;
typedef exprtk::expression<float> expression_t;
typedef exprtk::parser<float> parser_t;
typedef exprtk::parser_error::type err_t;

template<typename T,size_t S>
struct RB {
  T data[S]={};
  size_t pos=0;
  size_t len=128;

  RB() {
    clear();
  }

  void setLen(int l) {
    if(l>1&&l<S)
      len=l;
  }

  void push(T t) {
    data[pos]=t;
    pos++;
    if(pos>=len)
      pos=0;
  }

  T get(int relPos) {
    int idx=pos+relPos;
    while(idx<0)
      idx+=len;
    return data[idx%len];
  }

  void clear() {
    pos=0;
    len=1024;
    for(size_t k=0;k<S;k++)
      data[k]=0;
  }
};

template<typename T,size_t S,size_t K>
struct RBuffers {
  RB<T,S> buffers[K];

  void setLen(int nr,int l) {
    if(nr<0||nr>=K)
      return;
    buffers[nr].setLen(l);
  }

  void push(int nr,T t) {
    if(nr<0||nr>=K)
      return;
    buffers[nr].push(t);
  }

  T get(int nr,int relPos) {
    if(nr<0||nr>=K)
      return T(0);
    return buffers[nr].get(relPos);
  }

  void clear() {
    for(size_t j=0;j<K;j++) {
      buffers[j].clear();
    }
  }
};


template<typename T,size_t S>
struct Buffer {
  T data[S]={};

  Buffer() {
    clear();
  }

  void write(int idx, T t) {
    if(idx<0||idx>=S)
      return;
    data[idx]=t;
  }

  T read(int idx) {
    if(idx<0||idx>=S)
      return T(0);
    return data[idx];
  }

  void clear() {
    for(size_t k=0;k<S;k++)
      data[k]=0;
  }
};

template<typename T>
struct Random : public exprtk::ifunction<T> {
  using exprtk::ifunction<T>::operator();
  RND rnd;

  Random() : exprtk::ifunction<T>(0) {
  }

  inline T operator()() {
    return T(rnd.nextDouble());
  }
};

template<typename T>
struct STrigger : public exprtk::ifunction<T> {
  using exprtk::ifunction<T>::operator();
  dsp::SchmittTrigger st[16];

  STrigger() : exprtk::ifunction<T>(2) {
  }

  inline T operator()(const T &nr,const T &v) {
    int n=int(nr);
    if(n>=0&&n<16)
      return st[n].process(v)?1.f:0.f;
    return T(0);
  }
};

template<typename T>
struct DCBlock : public exprtk::ifunction<T> {
  using exprtk::ifunction<T>::operator();
  DCBlocker<T> dcb[16];

  DCBlock() : exprtk::ifunction<T>(2) {
  }

  inline T operator()(const T &nr,const T &v) {
    int n=int(nr);
    if(n>=0&&n<16)
      return dcb[n].process(v);
    return T(0);
  }

  void reset() {
    for(int k=0;k<16;k++) {
      dcb[k].reset();
    }
  }
};

template<typename T,size_t S,size_t K>
struct RBSetLength : public exprtk::ifunction<T> {
  using exprtk::ifunction<T>::operator();
  RBuffers<T,S,K> *buffers=nullptr;

  RBSetLength() : exprtk::ifunction<T>(2) {
  }

  inline T operator()(const T& nr,const T &v) {
    if(buffers)
      buffers->setLen(int(nr),int(v));
    return v;
  }
};

template<typename T,size_t S, size_t K>
struct RBPush : public exprtk::ifunction<T> {
  using exprtk::ifunction<T>::operator();
  RBuffers<T,S,K> *buffers=nullptr;

  RBPush() : exprtk::ifunction<T>(2) {
  }

  inline T operator()(const T& nr,const T &v) {
    if(buffers) {
      buffers->push(int(nr),v);
      return v;
    }
    return T(0);
  }
};

template<typename T,size_t S,size_t K>
struct RBGet : public exprtk::ifunction<T> {
  using exprtk::ifunction<T>::operator();
  RBuffers<T,S,K> *buffers=nullptr;

  RBGet() : exprtk::ifunction<T>(2) {
  }

  inline T operator()(const T& nr,const T &v) {
    if(buffers)
      return buffers->get(int(nr),int(v));
    return T(0);
  }
};

template<typename T,size_t S>
struct BufferWrite : public exprtk::ifunction<T> {
  using exprtk::ifunction<T>::operator();
  Buffer<T,S> *buffer=nullptr;

  BufferWrite() : exprtk::ifunction<T>(2) {
  }

  inline T operator()(const T &idx,const T &v) {
    if(buffer) {
      buffer->write(int(idx),v);
    }
    return v;
  }
};

template<typename T,size_t S>
struct BufferRead : public exprtk::ifunction<T> {
  using exprtk::ifunction<T>::operator();
  Buffer<T,S> *buffer=nullptr;

  BufferRead() : exprtk::ifunction<T>(1) {
  }

  inline T operator()(const T &pos) {
    return buffer?buffer->read((int)pos):0.f;
  }
};


struct FormulaOne : Module {

  enum ParamId {
    A_PARAM,B_PARAM,C_PARAM,D_PARAM,E_PARAM,PARAMS_LEN
  };
  enum InputId {
    W_INPUT,X_INPUT,Y_INPUT,Z_INPUT,P_INPUT,INPUTS_LEN
  };
  enum OutputId {
    V_OUTPUT,V1_OUTPUT,V2_OUTPUT,OUTPUTS_LEN
  };
  enum LightId {
    ERROR_LIGHT,OK_LIGHT,LIGHTS_LEN
  };
  symbol_table_t symbol_table;
  expression_t expression;
  parser_t parser;

  float x=0;
  float y=0;
  float z=0;
  float t=0;
  float w=0;
  float a=0;
  float b=0;
  float c=0;
  float d=0;
  float e=0;
  float v1=0;
  float v2=0;
  float v3=0;
  float v4=0;
  float v5=0;
  float v6=0;
  float v7=0;
  float v8=0;
  float out1=0;
  float out2=0;
  float channel=0;
  std::string formula;
  bool compiled=false;
  bool dirty=false;
  bool extDirty=false;
  float blinkPhase=0.0f;
  float sr;
  float smpTime;
  RBuffers<float,48000,16> rBuffers;
  RBPush<float,48000,16> bufPush;
  RBGet<float,48000,16> bufGet;
  RBSetLength<float,48000,16> bufSetLength;

  Buffer<float,4096> buf[4];
  BufferWrite<float,4096> bufWrite[4];
  BufferRead<float,4096> bufRead[4];

  Random<float> random;
  STrigger<float> strigger[4];
  DCBlock<float> dcb;
  DCBlock<float> dcb2;

  FormulaOne() {
    config(PARAMS_LEN,INPUTS_LEN,OUTPUTS_LEN,LIGHTS_LEN);
    configInput(X_INPUT,"x");
    configInput(Y_INPUT,"y");
    configInput(Z_INPUT,"z");
    configInput(W_INPUT,"w");
    configInput(P_INPUT,"t - Phase ([-5,5] -> [0,1])");
    configOutput(V_OUTPUT,"CV");
    configParam(A_PARAM,-1,1,0,"a");
    configParam(B_PARAM,-1,1,0,"b");
    configParam(C_PARAM,-1,1,0,"c");
    configParam(D_PARAM,-1,1,0,"d");
    symbol_table.add_variable("y",y);
    symbol_table.add_variable("z",z);
    symbol_table.add_variable("x",x);
    symbol_table.add_variable("w",w);
    symbol_table.add_variable("t",t);
    symbol_table.add_variable("a",a);
    symbol_table.add_variable("b",b);
    symbol_table.add_variable("c",c);
    symbol_table.add_variable("d",d);
    symbol_table.add_variable("e",e);
    symbol_table.add_variable("v1",v1);
    symbol_table.add_variable("v2",v2);
    symbol_table.add_variable("v3",v3);
    symbol_table.add_variable("v4",v4);
    symbol_table.add_variable("v5",v5);
    symbol_table.add_variable("v6",v6);
    symbol_table.add_variable("v7",v7);
    symbol_table.add_variable("v8",v8);
    symbol_table.add_variable("out1",out1);
    symbol_table.add_variable("out2",out2);
    symbol_table.add_variable("chn",channel);
    symbol_table.add_variable("sr",sr);
    symbol_table.add_variable("stim",smpTime);
    for(int k=0;k<4;k++) {
      bufWrite[k].buffer=&buf[k];
      bufRead[k].buffer=&buf[k];
    }
    bufGet.buffers=&rBuffers;
    bufPush.buffers=&rBuffers;
    bufSetLength.buffers=&rBuffers;
    symbol_table.add_function("rblen",bufSetLength);
    symbol_table.add_function("rbget",bufGet);
    symbol_table.add_function("rbpush",bufPush);
    symbol_table.add_function("bufr",bufRead[0]);
    symbol_table.add_function("bufw",bufWrite[0]);
    symbol_table.add_function("buf1r",bufRead[1]);
    symbol_table.add_function("buf1w",bufWrite[1]);
    symbol_table.add_function("buf2r",bufRead[2]);
    symbol_table.add_function("buf2w",bufWrite[2]);
    symbol_table.add_function("buf3r",bufRead[3]);
    symbol_table.add_function("buf3w",bufWrite[3]);

    symbol_table.add_function("st",strigger[0]);
    symbol_table.add_function("st1",strigger[1]);
    symbol_table.add_function("st2",strigger[2]);
    symbol_table.add_function("st3",strigger[3]);
    symbol_table.add_function("rnd",random);
    symbol_table.add_function("dcb",dcb);
    symbol_table.add_function("dcb2",dcb2);
    symbol_table.add_constants();
    expression.register_symbol_table(symbol_table);
  }

  void clear() {
    for(int k=0;k<4;k++) {
      buf[k].clear();
    }
    v1=0;
    v2=0;
    v3=0;
    v4=0;
    v5=0;
    v6=0;
    v7=0;
    v8=0;
    dcb.reset();
  }

  void compile() {
    clear();
    compiled=false;
    // wait until the process cycle has completed
    std::this_thread::sleep_for(std::chrono::duration<double>(100e-6));
    compiled=parser.compile(formula,expression);
    if(!compiled) {
      INFO("Error: %s\n",parser.error().c_str());

      for(std::size_t i=0;i<parser.error_count();++i) {
        err_t error=parser.get_error(i);

        INFO("Error: %02d Position: %02d Type: [%14s] Msg: %s\tExpression: %s\n",i,error.token.position,exprtk::parser_error::to_str(error.mode).c_str(),error.diagnostic.c_str(),formula.c_str());
      }
    }

  }

  void process(const ProcessArgs &args) override;

  void onReset(const ResetEvent &e) override {
    formula="";
    compile();
    dirty=true;
  }

  json_t *dataToJson() override {
    json_t *rootJ=json_object();
    json_object_set_new(rootJ,"formula",json_string(formula.c_str()));
    return rootJ;
  }

  void dataFromJson(json_t *rootJ) override {
    json_t *textJ=json_object_get(rootJ,"formula");
    if(textJ)
      formula=json_string_value(textJ);
    compile();
    dirty=true;
  }
};

#endif //DBCAEMODULES_FASTFORMULA_HPP
