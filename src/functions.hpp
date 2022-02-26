//
// Created by docb on 2/24/22.
//

#ifndef DBRACKFORMULAONE_FUNCTIONS_HPP
#define DBRACKFORMULAONE_FUNCTIONS_HPP

template<typename T>
T chebyshev(T in,std::vector<T> coeff) {
  int len=coeff.size();
  if(len<2)
    return 0;
  T v1[len*2];
  T v2[len*2];
  v1[0]=v1[1]=T(1);

  for(int i=2;i<len*2;++i)
    v1[i]=0;
  v2[0]=coeff[0];
  v2[1]=coeff[1];
  for(int i=2;i<len*2;++i)
    v2[i]=0;
  for(int i=2;i<len;i+=2) {
    v1[0]=-v1[0];
    v2[0]+=v1[0]*coeff[i];
    for(int j=2;j<len;j+=2) {
      v1[j]=2*v1[j-1]-v1[j];
      v2[j]+=v1[j]*coeff[i];
    }
    if(len-1>i) {
      for(int j=1;j<len;j+=2) {
        v1[j]=2*v1[j-1]-v1[j];
        v2[j]+=v1[j]*coeff[i+1];
      }
    }
  }
  T sum = v2[len-1];
  for (int i = len-2; i >= 0 ; --i) {
    sum *= in;
    sum += v2[i];
  }
  return sum;
}
template<typename T>
T linseg(T in,const std::vector<T> &params) {
  int len = params.size();
  if(len==0) return T(0);
  if(len==1) return params[0];
  float s=0;
  float pre=0;
  int j=1;
  for(;j<len;j+=2) {
    pre=s;
    s+=params[j];
    if(in<s) break;
  }
  if(j>=len) return params[len%2==1?len-1:0];
  float pct=params[j]==0?1:(in-pre)/params[j];
  return params[j-1]+pct*(params[(j+1)<len?j+1:0]-params[j-1]);
}
template<typename T>
T expseg(T in,const std::vector<T> &params) {
  int len = params.size();
  if(len<4 || (len-1)%3!=0) return T(0);
  float s=0;
  float pre=0;
  int j=1;
  for(;j<len;j+=3) {
    pre=s;
    s+=params[j];
    if(in<s) break;
  }
  if(j>=len) return params[len-1];
  float pct=params[j]==0?1:(in-pre)/params[j];
  float f=params[j+1]==0?pct:(1 - std::exp( pct*params[j+1])) / (1 - std::exp(params[j+1]));
  return params[j-1]+f*(params[j+2]-params[j-1]);
}

template<typename T>
T spline(T phs,const std::vector<T> &pts) {
  int len = pts.size();
  if(phs>=1.f) phs=0.99999f;
  if(phs<0.f) phs = 0.f;
  int idx0 = int(phs*float(len))%len;
  int idx1 = (idx0+1)%len;
  int idx2 = (idx0+2)%len;
  int idx3 = (idx0+3)%len;
  float a0=pts[idx3]-pts[idx2]-pts[idx0]+pts[idx1];
  float a1=pts[idx0]-pts[idx1]-a0;
  float a2=pts[idx2]-pts[idx0];
  float a3=pts[idx1];
  float start = float(idx0)/float(len);
  float stepPhs = (phs - start)*len;
  return ((a0*stepPhs+a1)*stepPhs+a2)*stepPhs+a3;
}

template<typename T>
T poly(T in,const std::vector<T> &coeff) {
  if(coeff.size()==0)
    return 0;
  int last=coeff.size()-1;
  T sum=coeff[last];
  for(int i=last;i>=0;--i) {
    sum*=in;
    sum+=coeff[i];
  }
  return sum;
}
template<typename T>
T scale(T v,T min, T max, T newmin, T newmax) {
  T pct = (v-min)/(max-min);
  return newmin+pct*(newmax-newmin);
}

template<typename T>
T saw(T v) {
  return v-std::floor(v+0.5);
}

template<typename T>
T tri(T v) {
  return 2*std::fabs(2*saw(v))-1;
}

template<typename T>
T pls(T v) {
  T t=tri(v);
  if(t>0)
    return 1;
  if(t<0)
    return -1;
  return 0;
}

#endif //DBRACKFORMULAONE_FUNCTIONS_HPP
