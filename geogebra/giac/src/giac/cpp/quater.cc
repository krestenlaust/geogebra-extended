// -*- mode:C++ ; compile-command: "g++-3.4 -I.. -g -c quater.cc -DHAVE_CONFIG_H -DIN_GIAC" -*-
#include "giacPCH.h"
/*
 *  Copyright (C) 2001,2014 B. Parisse, Institut Fourier, 38402 St Martin d'Heres
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "quater.h"
#include "unary.h"
#include "sym2poly.h"
#include "usual.h"
#include "intg.h"
#include "subst.h"
#include "derive.h"
#include "lin.h"
#include "vecteur.h"
#include "gausspol.h"
#include "plot.h"
#include "prog.h"
#include "modpoly.h"
#include "series.h"
#include "tex.h"
#include "ifactor.h"
#include "risch.h"
#include "solve.h"
#include "modfactor.h"
#include "giacintl.h"
using namespace std;

#ifndef NO_NAMESPACE_GIAC
namespace giac {
#endif // ndef NO_NAMESPACE_GIAC

#ifdef NO_RTTI
  extern const unary_function_ptr * const  at_quaternion=0; // user-level quaterni
#else
  static const char _quaternion_s []="quaternion";
  gen _quaternion(const gen & args,GIAC_CONTEXT){
    if ( args.type==_STRNG && args.subtype==-1) return  args;
    if (args.type!=_VECT)
      return quaternion(args);
    vecteur v(*args._VECTptr);
    if (v.size()==1)
      return quaternion(v.front());
    if (v.size()!=4)
      return gensizeerr(gettext("Quaternion has 1 or 4 arguments"));
    return quaternion(v[0],v[1],v[2],v[3]);
  }
  static define_unary_function_eval (__quaternion,&_quaternion,_quaternion_s);
  define_unary_function_ptr5( at_quaternion ,alias_at_quaternion,&__quaternion,0,true); // auto-register
  
  string quaternion::print(GIAC_CONTEXT) const {
    return string(_quaternion_s)+"("+r.print()+","+i.print()+","+j.print()+","+k.print()+")";
    this->dbgprint();
  }

  quaternion::quaternion(const gen & g){
    if (g.type==_USER){
      const quaternion  * q =dynamic_cast<const quaternion *>(g._USERptr);
      if (q)
	*this=*q;
    }
    else {
      r=g;
      i=zero;
      j=zero;
      k=zero;
    }
  };

  static const char _galois_field_s []="GF";

  void lrdm(modpoly & p,int n); // form intg.cc

  // Is the polynomial v irreducible and primitive modulo p?
  // If it is only irreducible, returns 2 and sets vmin to a primitive poly mod p if primitive is true
  int is_irreducible_primitive(const modpoly & v,const gen & p,modpoly & vmin,bool primitive,GIAC_CONTEXT){
    vmin=v;
    int m=int(v.size())-1;
    if (m<2)
      return 0; // setsizeerr(gettext("irreducibility: degree too short")+gen(v).print());
    gen gpm=pow(p,m);
    int pm=gpm.type==_INT_?gpm.val:RAND_MAX; // max number of tries
    environment E;
    environment * env=&E;
    env->modulo=p;
    env->pn=env->modulo;
    env->moduloon=true;
    vecteur polyx(2),g;
    polyx[0]=1;
    vecteur test(2);
    test[0]=1;
    // Irreducible: v must be prime with x^(p^k)-x, for k<=m/2
    for (int k=1;k<=m/2;k++){
      test=powmod(test,p,v,env);
      gcdmodpoly(operator_minus(test,polyx,env),v,env,g);
      if (!is_one(g)){
	return 0;
      }
    }
    if (!primitive){
      *logptr(contextptr) << gettext("Warning, minimal polynomial is only irreducible, not necessarily primitive") << endl;
      return 3;
    }
    // Primi: must not divide x^[(p^m-1)/d]-1 for any prime divisor d of p^m-1
    gen pm_minus_1=gpm-1;
    vecteur vp(pfacprem(pm_minus_1,true,context0));
    int ntest=int(vp.size());
    for (int i=0;i<ntest;i+=2){
      // Compute x^[(p^m-1)/d] mod v, mod p, is it 1?
      gen pm_d=(gpm-1)/vp[i];
      test=powmod(polyx,pm_d,v,env);
      if (is_one(test)){
	vecteur cyclic;
	// Find a cyclic element in GF, [1,0] does not work
	for (int k=p.val+1;k<pm;k++){
	  cyclic=vecteur(m);
#if 0
	  // decompose k in base p -> REPLACED by random init
	  int k1=k;
	  for (int j=0;j<m;++j){
	    cyclic[j]=k1%p.val;
	    k1/=p.val;
	  }
#else
	  for (int j=0;j<m;++j)
	    cyclic[j]=giac_rand(context0) % p.val;
#endif
	  cyclic=trim(cyclic,0);
	  // ?cyclic
	  for (int i=0;i<ntest;i+=2){
	    gen pm_over_d=(gpm-1)/vp[i];
	    test=powmod(cyclic,pm_over_d,v,env);
	    if (is_one(test))
	      break; // not cyclic
	  }
	  if (!is_one(test)) // cyclic! 
	    break;
	}
	// cyclic is cyclic, find it's minimal polynomial
	// Compute 1,cyclic, ..., cyclic^m and find kernel 
	matrice minmat(m+1);
	minmat[0]=vecteur(1,1);
	for (int i=1;i<=m;++i)
	  minmat[i]=operator_mod(operator_times(cyclic,*minmat[i-1]._VECTptr,env),v,env);
	for (int i=0;i<=m;++i)
	  lrdm(*minmat[i]._VECTptr,m-1);
	minmat=mtran(minmat);
	matrice minred,pivots; gen det;
	if (!modrref(minmat,minred,pivots,det,0,m,0,m+1,true,0,p,false,0))
	  return 0;
	// Extract kernel from last column
	vmin=vecteur(m+1,1);
	for (int i=1;i<=m;++i)
	  vmin[i]=-minred[m-i][m];
	// vecteur tmpv;
	// COUT << is_irreducible_primitive(vmin,p,tmpv) << endl;
	return 2;
      }
      /* vecteur test(pm_d+1);
	 test[0]=1;
	 test[pm_d]=-1; 
      if (is_zero(operator_mod(test,v,env))){
	return false;
      }
      */
    }
    return 1;
  }

  vecteur find_irreducible_primitive(const gen & p_,int m,bool primitive,GIAC_CONTEXT){
    if (p_.type==_ZINT){
#ifdef HAVE_LIBPARI
      gen pari=pari_ffinit(p_,m);
      pari=unmod(pari);
      if (pari.type==_VECT)
	return *pari._VECTptr;
#else
      return vecteur(1,gensizeerr("Compile with PARI for characteristic>2^31"));
#endif
    }
    int p=p_.val;
    // First check M random polynomials
    int M=100*m;
    // start coefficients near the end if only irreducible minpoly required
    // this make division by minpoly faster
    int start=primitive?1:m-2,nextpow2=1;
    for (int k=0;k<M;++k){
      if (start>1 && k==nextpow2){
	--start;
	nextpow2 *=2;
      }
      vecteur test(m+1),test2;
      test[0]=1;
      // random
      for (int j=start;j<=m;++j){
	if (p==2)
	  test[j]=(giac_rand(contextptr)>>29)%2;
	else
	  test[j]=giac_rand(contextptr)%p;
      }
      // *logptr(contextptr) << test << endl;
      if (is_irreducible_primitive(test,p,test2,primitive,contextptr))
	return test2;
    }
    *logptr(contextptr) << gettext("Warning, random search for irreducible polynomial did not work, starting exhaustive search") << endl;
    // Now test all possible coeffs for test[k] until it's irreducible
    double pm=std::pow(double(p),double(m));
    for (int k=0;k<pm;k++){
      vecteur test(m+1),test2;
      test[0]=1;
      // decompose k in base p
      for (int j=1,k1=k;j<=m;++j){
	test[j]=k1%p;
	k1/=p;
      }
      if (is_irreducible_primitive(test,p,test2,primitive,contextptr))
	return test2;
    }
    return vecteur(1,gensizeerr(gettext("No irreducible primitive polynomial found")));
  }
  bool make_free_variable(gen & g,GIAC_CONTEXT,bool warn,gen k,gen K){
    if (g.type!=_IDNT)
      return false;
    string s(g.print(contextptr));
    while (g==k || g==K || eval(g,1,contextptr)!=g){
      if (warn)
	*logptr(contextptr) << g << gettext(" already assigned. Trying ");
      autoname_plus_plus(s);
      if (warn)
	*logptr(contextptr) << s << endl;
      g=identificateur(s);
    }
    return true;
  }
  int gfsize(const gen & P){
    if (P.type!=_VECT)
      return 0;
    return P._VECTptr->size()-1;
  }

  gen _galois_field(const gen & args,GIAC_CONTEXT){
    if ( args.type==_STRNG && args.subtype==-1) return  args;
    vecteur v;
    if (is_integer(args)){ // must be a power of a prime
      if (_isprime(args,contextptr)!=0){
	return gensizeerr(gettext("GF is used for non-prime finite field. Use % or mod for prime fields, e.g. 1 % ")+args.print(contextptr)+'.');
      }
      v.push_back(args);
    }
    else {
      if (args.type!=_VECT){
	vecteur lv=lvar(args);
	if (!lv.empty()){
	  gen tmp=_e2r(makesequence(args,lv[0]),contextptr),modulo;
	  if (tmp.type==_VECT){
	    if (has_mod_coeff(tmp,modulo)){
	      return _galois_field(makesequence(modulo,args),contextptr);
	    }
	  }
	}
	return galois_field(args,true,contextptr);
      }
      v=*args._VECTptr;
    }
    int s=int(v.size());
    if (s<1 || !is_integer(v[0]))
      return gensizeerr(contextptr);
    if (_isprime(v[0],contextptr)==0){
      gen pm=abs(v[0],contextptr); // ok
      vecteur u(pfacprem(pm,true,contextptr));
      if (u.size()!=2)
	return gensizeerr(gettext("Not a power of a prime"));
      v[0]=u[0];
      v.insert(v.begin()+1,u[1]);
      ++s;
    }
    if (s==3 && v[1].type!=_INT_){
      v.push_back(undef);
      ++s;
    }
    bool primitive=true;
    if (s>=3 && is_integer(v[0]) && is_zero(v.back())){
      primitive=false;
      v.pop_back();
      --s;
    }
    if (s==2 && v[1].type!=_INT_){
      vecteur l=lvar(v[1]);
      if (l.size()!=1)
	return gensizeerr(contextptr);
      v.push_back(l.front());
      primitive=false;
      ++s;
    }
    if (s==2){
#ifdef GIAC_HAS_STO_38
      gen k(identificateur("v")),g(identificateur("g")),K(identificateur("k"));
#else
      gen k(k__IDNT_e),g(g__IDNT_e),K(identificateur("K"));
#endif
      make_free_variable(k,contextptr,false,0,0);
      make_free_variable(K,contextptr,false,k,0);
      make_free_variable(g,contextptr,true,k,K);
      v.push_back(makevecteur(k,K,g));
      *logptr(contextptr) << gettext("Setting ") << g << gettext(" as generator for Galois field ") << K << endl << gettext("(auxiliary polynomial variable for addition representation ") << k << ")" << endl;
      ++s;
    }
    if (s==3){
      gen fieldvalue;
      if (v[2].type==_IDNT){
	gen k(k__IDNT_e),g(v[2]),K(identificateur("K"));
	if (k==g)
	  k=l__IDNT_e;
	make_free_variable(k,contextptr,false,0,0);
	make_free_variable(K,contextptr,false,k,0);
	// make_free_variable(g,contextptr,true,k,K);
	if (v[1].type==_SYMB){
	  gen P=unmod(_e2r(makesequence(v[1],v[2]),contextptr));
	  if (P.type==_VECT){
	    vecteur vmin; // not used
	    int res=is_irreducible_primitive(*P._VECTptr,v[0],vmin,false,contextptr);
	    if (res==0)
	      return gensizeerr("Polynomial "+v[1].print(contextptr)+" is not irreducible modulo "+v[0].print(contextptr));
	    fieldvalue=galois_field(v[0],P,v[2],undef);
	  }
	}
	v[2]=makevecteur(k,K,g);
	if (fieldvalue.type==_USER){
	  galois_field *gf=dynamic_cast<galois_field *>( fieldvalue._USERptr);
	  gf->x=v[2];
	}
      }
      if (fieldvalue.type!=_USER)
	fieldvalue=galois_field(gen(v,args.subtype),primitive,contextptr);
      if (v.back().type==_VECT && v.back()._VECTptr->size()==3 && fieldvalue.type==_USER){
	// assign field and generator
	gen K=(*v.back()._VECTptr)[1];
	gen g=(*v.back()._VECTptr)[2];
	gen k=(*v.back()._VECTptr)[0];
	galois_field *gf=dynamic_cast<galois_field *>( fieldvalue._USERptr);
	*logptr(contextptr) << gettext("Assigning variables ") << g << gettext(" and ") << K << endl;
	*logptr(contextptr) << gettext("Now e.g. ") << g << gettext("^200+1 will build an element of ") << K << endl;
	sto(fieldvalue,K,contextptr);
	gen gene=galois_field(gf->p,gf->P,gf->x,makevecteur(1,0));
	sto(gene,g,contextptr);
	gen_context_t genec={gene,(context *)contextptr};
	gf_list()[pow(gf->p,gfsize(gf->P),contextptr)]=genec;
	return fieldvalue;
      }
      return fieldvalue;
    }
    if (s!=4)
      return gensizeerr(gettext("galois_field has 1 or 4 arguments (charac p, irred poly P, var name x, value as a poly of x or as a vector)"));
    vecteur a,P,vmin;
    gen & x= v[2];
    gen xid(x);
    if (x.type==_VECT && !x._VECTptr->empty())
      xid=x._VECTptr->front();
    if (!is_undef(v[3]) && v[3].type!=_VECT)
      v[3]=_e2r(makesequence(v[3],xid),contextptr); // ok
    if (v[1].type!=_VECT)
      v[1]=_e2r(makesequence(v[1],xid),contextptr); // ok
    v[1]=unmod(v[1]);
    if (v[1].type!=_VECT)
      return gensizeerr();
    if (is_undef(v[3])){
      int res=is_irreducible_primitive(*v[1]._VECTptr,v[0],vmin,primitive,contextptr);
      if (!res)
	return gensizeerr(gettext("Not irreducible or not primitive polynomial")+args.print());
      if (res==2)
	*logptr(contextptr) << gettext("Warning ") << symb_horner(*v[1]._VECTptr,xid) << gettext(" is irreducible but not primitive. You could use ") << symb_horner(vmin,xid) << gettext(" instead ") << endl;
    }
    return galois_field(v[0],v[1],v[2],v[3]);
  }
  static define_unary_function_eval (__galois_field,&_galois_field,_galois_field_s);
  define_unary_function_ptr5( at_galois_field ,alias_at_galois_field,&__galois_field,0,true); // auto-register
  
  string galois_field::print(GIAC_CONTEXT) const {
    gen xid(x);
    if (x.type==_VECT && x._VECTptr->size()>=2){
      xid=x._VECTptr->front();
      if (!is_undef(a)){
	if (x._VECTptr->size()==3 && a.type==_VECT){
	  if (a._VECTptr->size()==1)
	    return makemod(a._VECTptr->front(),p).print(contextptr);
	  gen tmp=symb_horner(*a._VECTptr,x._VECTptr->back());
	  if (tmp.is_symb_of_sommet(at_plus))
	    return '('+tmp.print(contextptr)+')';
	  else
	    return tmp.print(contextptr);
	}
	return x._VECTptr->back().print()+"("+r2e(a,xid,contextptr).print()+")";      
      }
    }
    return string(_galois_field_s)+"("+p.print()+","+r2e(unmod(P),xid,contextptr).print()+","+x.print()+","+r2e(a,xid,contextptr).print()+")";
    this->dbgprint(); // not reached, it's for the debugger
    return "";
  }

  string galois_field::texprint(GIAC_CONTEXT) const {
    gen xid(x);
    if (x.type==_VECT && x._VECTptr->size()>=2){
      xid=x._VECTptr->front();
      if (!is_undef(a)){
	if (x._VECTptr->size()==3 && a.type==_VECT){
	  if (a._VECTptr->size()==1)
	    return makemod(a._VECTptr->front(),p).print(contextptr);
	  gen tmp=symb_horner(*a._VECTptr,x._VECTptr->back());
	  if (tmp.is_symb_of_sommet(at_plus))
	    return '('+gen2tex(tmp,contextptr)+')';
	  else
	    return gen2tex(tmp,contextptr);
	}
	return gen2tex(x._VECTptr->back(),contextptr)+"("+gen2tex(r2e(a,xid,contextptr),contextptr)+")";      
      }
    }
    return string(_galois_field_s)+"("+gen2tex(p,contextptr)+","+gen2tex(r2e(P,xid,contextptr),contextptr)+","+gen2tex(x,contextptr)+","+gen2tex(r2e(a,xid,contextptr),contextptr)+")";
  }

  galois_field::galois_field(const gen & g,bool primitive,GIAC_CONTEXT){
    if (g.type==_USER){
      const galois_field  * q =dynamic_cast<const galois_field *>(g._USERptr);
      if (q)
	*this=*q;
      else {
	P=gensizeerr(gettext("Unable to convert user type to galois field"));
      }
    }
    else {
      if (g.type!=_VECT || g._VECTptr->size()<2 || !is_integer(g._VECTptr->front()) || (*g._VECTptr)[1].type!=_INT_)
	P=gensizeerr(gettext("Expecting characteristic p, integer m"));
      else {
	gen p0=g._VECTptr->front(); // max(absint(),2);
	if (is_greater(1,p0,contextptr))
	  P=gensizeerr(gettext("Bad characteristic: ")+p0.print(contextptr));
	else {
	  int m0=(*g._VECTptr)[1].val; // max(absint(),2);
	  if (m0<2)
	    P=gensizeerr(gettext("Exponent must be >=2: ")+print_INT_(m0));
	  else {
	    p=p0;
	    P=find_irreducible_primitive(p0,m0,primitive,contextptr);
	    if (g._VECTptr->size()>2)
	      x=(*g._VECTptr)[2];
	    else {
	      x=vx_var;
	    }
	    a=undef;
	  }
	}
      }
    }
  };
  
  void galois_field::reduce(){
    if (!is_undef(a)){
      a = smod(a,p);
      if (a.type!=_VECT)
	a=gen(vecteur(1,a),_POLY1__VECT);
    }
  }

  galois_field::galois_field(const gen p_,const gen & P_,const gen & x_,const gen & a_,bool doreduce):p(p_),P(P_),x(x_),a(a_) {
    if (doreduce)
      reduce();
  }

  galois_field::galois_field(const galois_field & q,bool doreduce):p(q.p),P(q.P),x(q.x),a(q.a) { 
    if (doreduce)
      reduce();
  }

  // find common GF, returns 0 if impossible, 1 if same, 2 if extension created
  int common_gf(galois_field & a,galois_field & b){
    if (a.p!=b.p || is_undef(a.P) || is_undef(b.P))
      return 0;
    if (a.P==b.P)
      return 1;
    gfmap & l=gf_list();
    gen_context_t agc=l[pow(a.p,gfsize(a.P),context0)],bgc=l[pow(b.p,gfsize(b.P),context0)];
    if (agc.ptr!=bgc.ptr || !equalposcomp(context_list(),agc.ptr))
      return false;
    gen ag=agc.g, bg=bgc.g;
    context * contextptr=agc.ptr;
    if (ag.type==_USER && bg.type==_USER){
      galois_field * agptr=dynamic_cast<galois_field *>(ag._USERptr);
      galois_field * bgptr=dynamic_cast<galois_field *>(bg._USERptr);
      if (!agptr || !bgptr) 
	return 0;
      galois_field * tmpptr=agptr;
      if (agptr->P!=bgptr->P){
	// build a common field extension
	int as=gfsize(a.P),bs=gfsize(b.P);
	int cs=lcm(as,bs).val;
	gen tmp;
	if (cs==as)
	  tmp=ag;
	if (cs==bs)
	  tmp=bg;
	if (cs!=as && cs!=bs){
	  // GF(a.p,cs), then factor a.P and b.P over GF 
	  // select one root as new generators for a.GF and b.GF
	  // update gf_list, a and b
	  *logptr(contextptr) << "Creating common field extension GF(" << a.p << "," << cs << ")" << endl;
	  tmp=_galois_field(makesequence(a.p,cs),contextptr);
	}
	if (tmp.type!=_USER || !(tmpptr=dynamic_cast<galois_field *>(tmp._USERptr)))
	  return 0;     
	*logptr(contextptr) << "Minimal polynomial of field generator " << symb_horner(*tmpptr->P._VECTptr,tmpptr->x[2]) << endl;
	tmp=galois_field(tmpptr->p,tmpptr->P,tmpptr->x,makevecteur(1,0)); // field generator
	if (agptr->P.type==_VECT && bgptr->P.type==_VECT){
	  polynome A; factorization f;
	  if (cs>as){
	    poly12polynome(*agptr->P._VECTptr,1,A,1);
	    gen af=tmpptr->polyfactor(A,f);
	    A=f.front().fact;
	    if (A.lexsorted_degree()!=1)
	      return 0;
	    ag=-A.coord.back().value/A.coord.front().value;
	    *logptr(contextptr) << "GF(" << a.p << "," << gfsize(a.P) << ") generator maps to " << ag << endl;
	    agc.g=ag;
	    l[pow(a.p,gfsize(a.P),context0)]=agc;
	    f.clear();
	  }
	  if (cs>bs){
	    poly12polynome(*bgptr->P._VECTptr,1,A,1);
	    gen af=tmpptr->polyfactor(A,f);
	    A=f.front().fact;
	    if (A.lexsorted_degree()!=1)
	      return 0;
	    bg=-A.coord.back().value/A.coord.front().value;
	    *logptr(contextptr) << "GF(" << b.p << "," << gfsize(b.P) << ") generator maps to " << bg << endl;
	    bgc.g=bg;
	    l[pow(b.p,gfsize(b.P),context0)]=bgc;
	  }
	}
      }
      ag=horner(a.a,ag);
      if (ag.type!=_USER)
	ag=galois_field(tmpptr->p,tmpptr->P,tmpptr->x,vecteur(1,ag));
      bg=horner(b.a,bg);
      if (bg.type!=_USER)
	bg=galois_field(tmpptr->p,tmpptr->P,tmpptr->x,vecteur(1,bg));
      if (ag.type==_USER && bg.type==_USER){
	agptr=dynamic_cast<galois_field *>(ag._USERptr);
	bgptr=dynamic_cast<galois_field *>(bg._USERptr);
	a=*agptr;
	b=*bgptr;
	return 2;
      }     
    }
    return 0;
  }

  void gf_add(const vecteur & a,const vecteur &b,int p,vecteur & c){
    int n=int(a.size()),m=int(b.size());
    if (n<m){
      gf_add(b,a,p,c);
      return;
    }
    c.clear(); c.reserve(n);
    const_iterateur it=a.begin(),itend=a.end(),jt=b.begin();
    if (n>m){
      for (;n>m;--n,++it)
	c.push_back(*it);
    }
    else {
      for (;it!=itend;++it,++jt){
	int j=it->val+jt->val;
	j += (unsigned(j)>>31)*p; // make positive
	j -= (unsigned((p>>1)-j)>>31)*p;
	if (j){
	  c.push_back(j);
	  ++it;++jt;
	  break;
	}
      }
    }
    for (;it!=itend;++it,++jt){
      int j=it->val+jt->val;
      j += (unsigned(j)>>31)*p; // make positive
      j -= (unsigned((p>>1)-j)>>31)*p;
      c.push_back(j);
    }
  }

  // remove % p in MOD elements of g
  gen cleanup(const gen & p,const gen & g){
    if (g.type==_VECT){
      vecteur v=*g._VECTptr;
      for (int i=0;i<v.size();++i)
	v[i]=cleanup(p,v[i]);
      return gen(v,g.subtype);
    }
    if (g.type==_MOD){
      if (p!=*(g._MODptr+1))
	return gensizeerr(gettext("Incompatible characteristics"));
      return *g._MODptr;
    }
    if (g.type==_SYMB)
      return symbolic(g._SYMBptr->sommet,cleanup(p,g._SYMBptr->feuille));
    return g;
  }
  inline gen cleanup(const galois_field & gf,const gen & g){
    return cleanup(gf.p,g);
  }

  gen galois_field::operator + (const gen & g) const { 
    if (is_integer(g))
      return galois_field(p,P,x,a+g);
    if (g.type==_MOD){
      if (*(g._MODptr+1)!=p)
	return gensizeerr(gettext("Incompatible characteristics"));
      return galois_field(p,P,x,a+*g._MODptr);
    }
    if (g.type!=_USER)
      return sym_add(*this,cleanup(*this,g),context0); // ok symbolic(at_plus,makesequence(g,*this));
    if (galois_field * gptr=dynamic_cast<galois_field *>(g._USERptr)){
      // if (gptr->p!=p || gptr->P!=P || is_undef(P) || is_undef(gptr->P)) return gensizeerr();
      if (a.type==_VECT && gptr->a.type==_VECT){
	galois_field * gfptr=new galois_field(p,P,x,new ref_vecteur(*a._VECTptr));
	int tst=common_gf(*gfptr,*gptr);
	if (!tst)
	  return gensizeerr();
	if (tst==1 && p.type==_INT_){
	  ref_gen_user * resptr=new ref_gen_user(*gfptr);
	  gf_add(*a._VECTptr,*gptr->a._VECTptr,p.val,*gfptr->a._VECTptr);
	  delete gfptr;
	  return resptr;
	}
	environment env;
	env.modulo=p;
	env.pn=env.modulo;
	env.moduloon=true;
	addmodpoly(*gfptr->a._VECTptr,*gptr->a._VECTptr,&env,*gfptr->a._VECTptr);
	// CERR << gfptr->P << " " << gfptr->a << endl;	
	gen res=*gfptr;
	delete gfptr;
	return res;
      }
      return galois_field(p,P,x,a+gptr->a);
    }
    else
      return gensizeerr();
  }

  gen galois_field::operator - (const gen & g) const { 
    if (is_integer(g)){
      gen tmp=a-g;
      if (is_exactly_zero(tmp))
	return tmp;
      return galois_field(p,P,x,tmp);
    }
    if (g.type==_MOD){
      if (*(g._MODptr+1)!=p)
	return gensizeerr(gettext("Incompatible characteristics"));
      return galois_field(p,P,x,a-*g._MODptr);
    }
    if (g.type!=_USER)
      return sym_add(*this,cleanup(*this,-g),context0); // ok symbolic(at_plus,makesequence(-g,*this));
    if (galois_field * gptr=dynamic_cast<galois_field *>(g._USERptr)){
      return *this+(-g);
      if (gptr->p!=p || gptr->P!=P || is_undef(P) || is_undef(gptr->P)) return gensizeerr();
      if (a.type==_VECT && gptr->a.type==_VECT){
	vecteur res;
	environment env;
	env.modulo=p;
	env.pn=env.modulo;
	env.moduloon=true;
	submodpoly(*a._VECTptr,*gptr->a._VECTptr,&env,res);
	return galois_field(p,P,x,res,false);
      }
      return galois_field(p,P,x,a-gptr->a);
    }
    else
      return gensizeerr();
  }

  gen galois_field::operator - () const { 
    return galois_field(p,P,x,-a,true);
  }

  gen galois_field::operator / (const gen & g) const { 
    if (is_integer(g)){
      gen tmp=invmod(g,p);
      return (*this)*tmp;
    }
    gen tmp=g.inverse(context0);
    return (*this)*tmp;
  }

  gen galois_field::operator * (const gen & g) const { 
    if (is_integer(g)){
      gen tmp=smod(g,p);
      if (is_exactly_zero(tmp))
	return zero;
      return galois_field(p,P,x,g*a);
    }
    if (g.type==_MOD){
      if (*(g._MODptr+1)!=p)
	return gensizeerr(gettext("Incompatible characteristics"));
      return *this*(*g._MODptr);
    }
    if (g.type==_VECT){
      vecteur v=*g._VECTptr;
      int s=int(v.size());
      for (int i=0;i<s;++i)
	v[i]=*this*v[i];
      return gen(v,g.subtype);
    }
    if (g.type!=_USER)
      return sym_mult(*this,g,context0); // ok symbolic(at_prod,makesequence(g,*this));
    if (galois_field * gptr=dynamic_cast<galois_field *>(g._USERptr)){
      // if (gptr->p!=p || gptr->P!=P || P.type!=_VECT || is_undef(P) || is_undef(gptr->P)) return gensizeerr();
      if (a.type==_VECT && gptr->a.type==_VECT){
	galois_field * gfptr=new galois_field(p,P,x,new ref_vecteur(*a._VECTptr));
	int tst=common_gf(*gfptr,*gptr);
	if (!tst)
	  return gensizeerr();
	if (tst==1 && p.type==_INT_){
	  ref_gen_user * resptr=new ref_gen_user(*gfptr);
	  int m=p.val;
	  vector<int> amod,bmod,ab,pmod;
	  vecteur2vector_int(*a._VECTptr,0,amod);
	  vecteur2vector_int(*gptr->a._VECTptr,0,bmod);
	  vecteur2vector_int(*P._VECTptr,0,pmod);
	  mulext(amod,bmod,pmod,m,ab);
	  int absize=int(ab.size());
	  int * i=&ab.front(),*iend=i+absize;
	  for (;i<iend;++i){
	    int j=*i;
	    //j =smod(j,m);
	    j += (unsigned(j)>>31)*m; // make positive
	    j -= (unsigned((m>>1)-j)>>31)*m;
	    *i=j;
	  }
	  // if (absize==1) return ab.front()?zero:makemod(ab.front(),p); // does not work chk_fhan12
	  vector_int2vecteur(ab,*gfptr->a._VECTptr);
	  delete gfptr;
	  return resptr; // galois_field(p,P,x,resdbg,false);
	}
	environment env;
	env.modulo=p;
	env.pn=env.modulo;
	env.moduloon=true;
	vecteur resdbg,quo;
	mulmodpoly(*gfptr->a._VECTptr,*gptr->a._VECTptr,&env,resdbg);
	DivRem(resdbg,*gptr->P._VECTptr,&env,quo,*gfptr->a._VECTptr);
	gen res=*gfptr;
	delete gfptr;
	return res;
      }
      return galois_field(p,P,x,a*gptr->a);
    }
    else
      return gensizeerr();
  }

  gen galois_field::inv () const {
    if (a.type!=_VECT || P.type!=_VECT || is_undef(P))
      return gensizeerr(gettext("galois field inv"));
    vecteur & A = *a._VECTptr;
    if (A.empty())
      return galois_field(p,P,x,undef);
    modpoly u,v,d;
    environment * env=new environment;
    env->modulo=p;
    env->pn=env->modulo;
    env->moduloon=true;
    egcd(A,*P._VECTptr,env,u,v,d);
    delete env;
    // d should be [1]
    if (d!=vecteur(1,1))
      return gensizeerr(gettext("GF inv internal bug"));
    return galois_field(p,P,x,u);
  }

  bool galois_field::operator == (const gen & g) const {
    if (is_zero())
      return is_exactly_zero(g);
    if (g.type!=_USER)
      return a==vecteur(1,g);
    if (galois_field * gptr=dynamic_cast<galois_field *>(g._USERptr)){
      if (gptr->p!=p || gptr->P!=P)
	return false;
      return gptr->a==a;
    }
    return false;
  }

  bool galois_field::is_zero () const {
    return a.type==_VECT && ( a._VECTptr->empty() || (a._VECTptr->size()==1 && a._VECTptr->front()==0) );
  }

  bool galois_field::is_one () const {
    return a.type==_VECT && a._VECTptr->size()==1 && a._VECTptr->front()==1;
  }

  bool galois_field::is_minus_one () const {
    return a.type==_VECT && a._VECTptr->size()==1 && smod(a._VECTptr->front(),p)==-1;
  }

  gen galois_field::operator () (const gen & g,GIAC_CONTEXT) const {
    if (is_undef(a)){
      gen res;
      if (g.type==_VECT){
	vecteur v(*g._VECTptr);
	for (unsigned i=0;i<v.size();++i){
	  v[i]=(*this)(v[i],contextptr);
	}
	return gen(v,g.subtype);
      }
      else {
	gen xid(x);
	if (x.type==_VECT && !x._VECTptr->empty())
	  xid=x._VECTptr->front();
	vecteur v(1,xid);
	lvar(g,v);
	if (v.size()>1){
	  if (g.type==_IDNT)
	    return g;
	  else {
	    if (g.type==_SYMB){
	      if (g._SYMBptr->sommet==at_plus)
		return _plus((*this)(g._SYMBptr->feuille,contextptr),contextptr);
	      if (g._SYMBptr->sommet==at_neg)
		return -(*this)(g._SYMBptr->feuille,contextptr);
	      if (g._SYMBptr->sommet==at_prod)
		return _prod((*this)(g._SYMBptr->feuille,contextptr),contextptr);
	      if (g._SYMBptr->sommet==at_inv){
		gen tmp=(*this)(g._SYMBptr->feuille,contextptr);
		return _inv(tmp,contextptr);
	      }
	      if (g._SYMBptr->sommet==at_pow && g._SYMBptr->feuille.type==_VECT && g._SYMBptr->feuille._VECTptr->size()==2)
		return pow((*this)(g._SYMBptr->feuille[0],contextptr),g._SYMBptr->feuille[1],contextptr);
	      return gensizeerr(x[1].print()+"("+g.print()+gettext(") invalid, works only for integers or polynomials depending on ")+xid.print());
	    }
	  }
	}
	else
	  res=_e2r(makesequence(g,xid),contextptr);
      }
      if (res.type==_VECT){
	environment env;
	env.modulo=p;
	env.pn=env.modulo;
	env.moduloon=true;
	res=smod(res,p);
	res=operator_mod(*res._VECTptr,*P._VECTptr,&env);
      }
      return galois_field(p,P,x,res);
    }
    return *this;
  }

  gen galois_field::operator [] (const gen & g) {
    if (g.type==_INT_){
      int i= g.val;
      if (array_start(context0)) --i;
      switch (i){
      case 0:
	return p;
      case 1:
	return P;
      case 2:
	return x;
      case 3:
	return a;
      }
    }
    return undef;
  }

  gen galois_field::operator >(const gen & g) const {
    if (g.type!=_USER)
      return undef;
    galois_field * gf=dynamic_cast<galois_field *>(g._USERptr);
    if (!gf)
      return undef;
    return is_strictly_positive(p-gf->p,context0); // ok
  }

  gen galois_field::operator <(const gen & g) const {
    if (g.type!=_USER)
      return undef;
    galois_field * gf=dynamic_cast<galois_field *>(g._USERptr);
    if (!gf)
      return undef;
    return is_strictly_positive(gf->p-p,context0); // ok
  }

  gen galois_field::operator <=(const gen & g) const {
    if (g.type!=_USER)
      return undef;
    galois_field * gf=dynamic_cast<galois_field *>(g._USERptr);
    if (!gf)
      return undef;
    return is_positive(gf->p-p,context0); // ok
  }

  gen galois_field::operator >=(const gen & g) const {
    if (g.type!=_USER)
      return undef;
    galois_field * gf=dynamic_cast<galois_field *>(g._USERptr);
    if (!gf)
      return undef;
    return is_positive(p-gf->p,0);
  }

  gen galois_field::sqrt(GIAC_CONTEXT) const {
    unsigned m=unsigned(P._VECTptr->size())-1;
    // K* has cardinal p^m-1, a has a sqrt iff a^((p^m-1)/2)==1
    environment env;
    env.modulo=p;
    gen gpm=pow(p,int(m),contextptr);
    // if (gpm.type!=_INT_) return gensizeerr(gettext("Field too large"));
    // int pn=gpm.val;
    env.moduloon=true;
    modpoly & A=*a._VECTptr;
    if (p!=2){
      modpoly test(powmod(A,(gpm-1)/2,*P._VECTptr,&env));
      if (test.size()!=1 || test.front()!=1) 
	return undef;
      // if p^m=3 [4], return A^((p^m+1)/4)
      if (smod(gpm,4)==-1){
	test=powmod(A,(gpm+1)/4,*P._VECTptr,&env);
	if (is_positive(-test.front(),contextptr))
	  test=-test;
	return galois_field(p,P,x,test);
      }
    }
    env.moduloon=false;
    env.coeff=*this;
    env.pn=gpm;
    modpoly X(3);
    X[0]=1;
    X[2]=-*this;
    polynome px(unmodularize(X));
    factorization sqff_f(squarefree_fp(px,env.modulo.val,m)),f;
    if (p!=2){
      if (!sqff_ffield_factor(sqff_f,env.modulo.val,&env,f) || f.size()!=2)
	return undef;
      sqff_f.swap(f);
    }
    gen tmp=sqff_f.front().fact.coord.back().value;
    if (tmp.type==_USER){
      if (galois_field * gf=dynamic_cast<galois_field *>(tmp._USERptr)){
	if (is_positive(-gf->a._VECTptr->front(),contextptr))
	  tmp=-tmp;
      }
    }
    return tmp;
  }

  polynome galois_field::poly_reduce(const polynome & q) const {
    polynome res(q.dim);
    vector< monomial<gen> >::const_iterator it=q.coord.begin(),itend=q.coord.end();
    for (;it!=itend;++it){
      gen g=it->value;
      if (is_integer(g)){
	gen tmp=smod(g,p);
	if (is_exactly_zero(tmp))
	  continue;
	g=galois_field(p,P,x,vecteur(1,g));
      }
      res.coord.push_back(monomial<gen>(g,it->index));
    }
    return res;
  }

  gen galois_field::polygcd(const polynome & p,const polynome & q,polynome & res) const {
    res=Tgcdpsr(poly_reduce(p),poly_reduce(q));
    if (!res.coord.empty())
      res=res/res.coord.front().value;
    return 0;
  }

  gen galois_field::makegen(int i) const {
    if (p.type==_ZINT)
      return galois_field(p,P,x,vecteur(1,i));
    if (P.type!=_VECT || p.type!=_INT_)
      return gendimerr();
    unsigned n=unsigned(P._VECTptr->size())-1;
    //    i += pow(p,int(n)).val;
    vecteur res;
    for (unsigned j=0;j<n;++j){
      if (!i)
	break;
      res.push_back(gen(i%p.val));
      i=i/p.val;
    }
    reverse(res.begin(),res.end());
    return galois_field(p,P,x,res);
  }

  gen galois_field::polyfactor (const polynome & p0,factorization & f) const {
    f.clear();
    polynome p(p0.dim);
    // p0 may contain null coefficients if we multiply a polynomial over Z with a galois_field
    // because of the non-0 characteristic
    vector<monomial<gen> >::const_iterator it=p0.coord.begin(),itend=p0.coord.end();
    for (;it!=itend;++it){
      if (it->value!=0)
	p.coord.push_back(*it);
    }
    if (p.coord.empty())
      return 0;
    gen lcoeff=p.coord.front().value;
    p=lcoeff.inverse(context0)*p;
    if (p.dim!=1){
#if 1
      CERR << gettext("Warning: multivariate GF factorization is experimental and may fail") << endl;
#else
      return gendimerr(gettext("Multivariate GF factorization not yet implemented"));
#endif
    }
    if (P.type!=_VECT || this->p.type!=_INT_)
      return gensizeerr(gettext("GF polyfactor"));
    environment env;
    env.moduloon=true; // false;
    env.coeff=*this;
    env.modulo=this->p.to_int();
    int exposant=int(this->P._VECTptr->size())-1;
    env.pn=pow(this->p,exposant);
    factorization sqff_f(squarefree_fp(p,env.modulo.val,exposant));
    if (!sqff_ffield_factor(sqff_f,env.modulo.val,&env,f))
      return gensizeerr(gettext("GF polyfactor"));
    if (!is_exactly_one(lcoeff))
      f.push_back(facteur<polynome>(
				    polynome(
					     monomial<gen>(lcoeff,0,p.dim)
					     ),
				    1));
    return 0;
  }

  gen galois_field::rand (GIAC_CONTEXT) const {
    int c=p.type==_INT_?p.val:RAND_MAX;
    int m=int(P._VECTptr->size())-1;
    vecteur v(m);
    for (int i=0;i<m;++i){
      if (c==2)
	v[i]=(giac_rand(contextptr) >> 29)%2;
      else
	v[i]=giac_rand(contextptr) % c;
    }
    v=trim(v,0);
    return galois_field(p,P,x,v);
  }

  gfmap & gf_list(){
    static gfmap * ans= new gfmap;
    return *ans;
  }

  bool has_gf_coeff(const vecteur & v,gen & p, gen & pmin){
    const_iterateur it=v.begin(),itend=v.end();
    for (;it!=itend;++it){
      if (has_gf_coeff(*it,p,pmin))
	return true;
    }
    return false;
  }

  bool has_gf_coeff(const polynome & P,gen & p, gen & pmin){
    vector< monomial<gen> >::const_iterator it=P.coord.begin(),itend=P.coord.end();
    for (;it!=itend;++it){
      if (has_gf_coeff(it->value,p,pmin))
	return true;
    }
    return false;
  }

  bool has_gf_coeff(const gen & e,gen & p, gen & pmin){
    switch (e.type){
    case _USER:
      if (galois_field * ptr=dynamic_cast<galois_field *>(e._USERptr)){
	p = ptr->p;
	pmin=ptr->P;
	return true;
      }
      return false;
    case _SYMB:
      return has_gf_coeff(e._SYMBptr->feuille,p,pmin);
    case _VECT:
      return has_gf_coeff(*e._VECTptr,p,pmin);
    case _POLY:
      return has_gf_coeff(*e._POLYptr,p,pmin);
    default:
      return false;
    }
  }

#endif // NO_RTTI

#ifndef NO_NAMESPACE_GIAC
} // namespace giac
#endif // ndef NO_NAMESPACE_GIAC

