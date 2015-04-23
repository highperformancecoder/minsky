/*
  @copyright Steve Keen 2013
  @author Russell Standish
  This file is part of Minsky.

  Minsky is free software: you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Minsky is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Minsky.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "latexMarkup.h"
#include <map>
#include <vector>
using namespace std;

#include <ctype.h>
#include <string.h>

namespace
{
  //  pair<string, string> 
  struct Symbol
  {
    const char* first, *second;
  }
    symbolData[]={
      {"{","{"},
      {"}","}"},
    {"alpha","α"},
    {"beta","β"},
    {"gamma","γ"},
    {"delta","δ"},
    {"epsilon","∈"},
    {"varepsilon","ε"},
    {"zeta","ζ"},
    {"eta","η"},
    {"theta","θ"},
    {"iota","ι"},
    {"kappa","κ"},
    {"lambda","λ"},
    {"mu","μ"},
    {"nu","ν"},
    {"xi","ξ"},
    {"pi","π"},
    {"varpi","ϖ"},
    {"rho","ρ"},
    {"varrho","ϱ"},
    {"sigma","σ"},
    {"varsigma","ς"},
    {"tau","τ"},
    {"upsilon","υ"},
    {"phi","φ"},
    {"varphi","ϕ"},
    {"chi","χ"},
    {"omega","ω"},
    {"Gamma","Γ"},
    {"Delta","Δ"},
    {"Theta","Θ"},
    {"Lambda","Λ"},
    {"Xi","Ξ"},
    {"Pi","Π"},
    {"Sigma","Σ"},
    {"Upsilon","Υ"},
    {"Phi","Φ"},
    {"Psi","Ψ"},
    {"Omega","Ω"},
    {"dag","†"},
    {"ddag","‡"},
    {"S","§"},
    {"P","¶"},
    {"copyright","©"},
    {"pounds","£"},
    {"ldots","…"},
    {"cdots","⋯"},
    {"vdots","⋮"},
    {"ddots","⋱"},
    {"pm","±"},
    {"mp","∓"},
    {"times","×"},
    {"div","÷"},
    {"ast","∗"},
    {"star","⋆"},
    {"circ","∘"},
    {"bullet","•"},
    {"cdot","·"},
    {"cap","∩"},
    {"cup","∪"},
    {"uplus","⊎"},
    {"sqcap","⊓"},
    {"sqcup","⊔"},
    {"vee","∨"},
    {"wedge","∧"},
    {"setminus","∖"},
    {"wr","≀"},
    {"diamond","⋄"},
    {"bigtriangleup","△"},
    {"bigtriangledown","▽"},
    {"triangleleft","◃"},
    {"triangleright","▹"},
    {"lhd","◁"},
    {"rhd","▷"},
    {"oplus","⊕"},
    {"ominus","⊖"},
    {"otimes","⊗"},
    {"oslash","⊘"},
    {"odot","⊙"},
    {"bigcirc","◯"},
    {"dagger","†"},
    {"ddagger","‡"},
    {"amalg","∐"},
    {"leq","≤"},
    {"prec","≺"},
    {"preceq","≼"},
    {"ll","≪"},
    {"subset","⊂"},
    {"subseteq","⊆"},
    {"sqsubset","⊏"},
    {"sqsubseteq","⊑"},
    {"in","∈"},
    {"vdash","⊢"},
    {"geq","≥"},
    {"succ","≻"},
    {"succeq","≽"},
    {"gg","≫"},
    {"supset","⊃"},
    {"supseteq","⊇"},
    {"sqsupset","⊐"},
    {"sqsupseteq","⊒"},
    {"ni","∋"},
    {"dashv","⊣"},
    {"equiv","≡"},
    {"sim","∼"},
    {"simeq","≃"},
    {"asymp","≍"},
    {"approx","≈"},
    {"cong","≅"},
    {"neq","≠"},
    {"doteq","≐"},
    {"notin","∉"},
    {"models","⊧"},
    {"perp","⊥"},
    {"mid","∣"},
    {"parallel","∥"},
    {"bowtie","⋈"},
    {"Join","⋈"},
    {"smile","⌣"},
    {"frown","⌢"},
    {"propto","∝"},
    {"leftarrow","←"},
    {"Leftarrow","⇐"},
    {"rightarrow","→"},
    {"Rightarrow","⇒"},
    {"leftrightarrow","↔"},
    {"Leftrightarrow","⇔"},
    {"mapsto","↦"},
    {"hookleftarrow","↩"},
    {"leftharpoonup","↼"},
    {"leftharpoondown","↽"},
    {"rightleftharpoons","⇌"},
    {"longleftarrow","←"},
    {"Longleftarrow","⇐"},
    {"longrightarrow","→"},
    {"Longrightarrow","⇒"},
    {"longleftrightarrow","↔"},
    {"Longleftrightarrow","⇔"},
    {"longmapsto","↦"},
    {"hookrightarrow","↪"},
    {"rightharpoonup","⇀"},
    {"rightharpoondown","⇁"},
    {"leadsto","↝"},
    {"uparrow","↑"},
    {"Uparrow","⇑"},
    {"downarrow","↓"},
    {"Downarrow","⇓"},
    {"updownarrow","↕"},
    {"Updownarrow","⇕"},
    {"nearrow","↗"},
    {"searrow","↘"},
    {"swarrow","↙"},
    {"nwarrow","↖"},
    {"aleph","ℵ"},
    {"imath","ı"},
    {"ell","ℓ"},
    {"wp","℘"},
    {"Re","ℜ"},
    {"Im","ℑ"},
    {"mho","℧"},
    {"prime","′"},
    {"emptyset","∅"},
    {"nabla","∇"},
    {"surd","√"},
    {"top","⊤"},
    {"bot","⊥"},
    {"angle","∠"},
    {"forall","∀"},
    {"exists","∃"},
    {"neg","¬"},
    {"flat","♭"},
    {"natural","♮"},
    {"sharp","♯"},
    {"backslash","\\"},
    {"partial","∂"},
    {"infty","∞"},
    {"Box","□"},
    {"Diamond","◇"},
    {"triangle","▵"},
    {"clubsuit","♣"},
    {"diamondsuit","♢"},
    {"heartsuit","♥"},
    {"spadesuit","♠"},
    {"sum","∑"},
    {"prod","∏"},
    {"coprod","∐"},
    {"int","∫"},
    {"oint","∮"},
    {"bigcap","⋂"},
    {"bigcup","⋃"},
    {"bigvee","⋁"},
    {"bigwedge","⋀"},
    {"`a","à"},
    {"`{a}","à"},
    {"`A","À"},
    {"`{A}","À"},
    {"`e","è"},
    {"`{e}","è"},
    {"`E","È"},
    {"`{E}","È"},
    {"`i","ì"},
    {"`{i}","ì"},
    {"`I","Ì"},
    {"`{I}","Ì"},
    {"`n","ǹ"},
    {"`{n}","ǹ"},
    {"`N","Ǹ"},
    {"`{N}","Ǹ"},
    {"`o","ò"},
    {"`{o}","ò"},
    {"`O","Ò"},
    {"`{O}","Ò"},
    {"`u","ù"},
    {"`{u}","ù"},
    {"`U","Ù"},
    {"`{U}","Ù"},
    {"`w","ẁ"},
    {"`{w}","ẁ"},
    {"`W","Ẁ"},
    {"`{W}","Ẁ"},
    {"`y","ỳ"},
    {"`{y}","ỳ"},
    {"`Y","Ỳ"},
    {"`{Y}","Ỳ"},
    {"'a","á"},
    {"'{a}","á"},
    {"'A","Á"},
    {"'{A}","Á"},
    {"'c","ć"},
    {"'{c}","ć"},
    {"'C","Ć"},
    {"'{C}","Ć"},
    {"'e","é"},
    {"'{e}","é"},
    {"'E","É"},
    {"'{E}","É"},
    {"'g","ǵ"},
    {"'{g}","ǵ"},
    {"'G","Ǵ"},
    {"'{G}","Ǵ"},
    {"'i","í"},
    {"'{i}","í"},
    {"'I","Í"},
    {"'{I}","Í"},
    {"'k","ḱ"},
    {"'{k}","ḱ"},
    {"'K","Ḱ"},
    {"'{K}","Ḱ"},
    {"'l","ĺ"},
    {"'{l}","ĺ"},
    {"'L","Ĺ"},
    {"'{L}","Ĺ"},
    {"'m","ḿ"},
    {"'{m}","ḿ"},
    {"'M","Ḿ"},
    {"'{M}","Ḿ"},
    {"'n","ń"},
    {"'{n}","ń"},
    {"'N","Ń"},
    {"'{N}","Ń"},
    {"'o","ó"},
    {"'{o}","ó"},
    {"'O","Ó"},
    {"'{O}","Ó"},
    {"'p","ṕ"},
    {"'{p}","ṕ"},
    {"'P","Ṕ"},
    {"'{P}","Ṕ"},
    {"'r","ŕ"},
    {"'{r}","ŕ"},
    {"'R","Ŕ"},
    {"'{R}","Ŕ"},
    {"'s","ś"},
    {"'{s}","ś"},
    {"'S","Ś"},
    {"'{S}","Ś"},
    {"'u","ú"},
    {"'{u}","ú"},
    {"'U","Ú"},
    {"'{U}","Ú"},
    {"'w","ẃ"},
    {"'{w}","ẃ"},
    {"'W","Ẃ"},
    {"'{W}","Ẃ"},
    {"'y","ý"},
    {"'{y}","ý"},
    {"'Y","Ý"},
    {"'{Y}","Ý"},
    {"'z","ź"},
    {"'{z}","ź"},
    {"'Z","Ź"},
    {"'{Z}","Ź"},
    {"^a","â"},
    {"^{a}","â"},
    {"^A","Â"},
    {"^{A}","Â"},
    {"^c","ĉ"},
    {"^{c}","ĉ"},
    {"^C","Ĉ"},
    {"^{C}","Ĉ"},
    {"^e","ê"},
    {"^{e}","ê"},
    {"^E","Ê"},
    {"^{E}","Ê"},
    {"^g","ĝ"},
    {"^{g}","ĝ"},
    {"^G","Ĝ"},
    {"^{G}","Ĝ"},
    {"^h","ĥ"},
    {"^{h}","ĥ"},
    {"^H","Ĥ"},
    {"^{H}","Ĥ"},
    {"^i","î"},
    {"^{i}","î"},
    {"^I","Î"},
    {"^{I}","Î"},
    {"^j","ĵ"},
    {"^{j}","ĵ"},
    {"^J","Ĵ"},
    {"^{J}","Ĵ"},
    {"^o","ô"},
    {"^{o}","ô"},
    {"^O","Ô"},
    {"^{O}","Ô"},
    {"^s","ŝ"},
    {"^{s}","ŝ"},
    {"^S","Ŝ"},
    {"^{S}","Ŝ"},
    {"^u","û"},
    {"^{u}","û"},
    {"^U","Û"},
    {"^{U}","Û"},
    {"^w","ŵ"},
    {"^{w}","ŵ"},
    {"^W","Ŵ"},
    {"^{W}","Ŵ"},
    {"^y","ŷ"},
    {"^{y}","ŷ"},
    {"^Y","Ŷ"},
    {"^{Y}","Ŷ"},
    {"^z","ẑ"},
    {"^{z}","ẑ"},
    {"^Z","Ẑ"},
    {"^{Z}","Ẑ"},
    {"\"a","ä"},
    {"\"{a}","ä"},
    {"\"A","Ä"},
    {"\"{A}","Ä"},
    {"\"e","ë"},
    {"\"{e}","ë"},
    {"\"E","Ë"},
    {"\"{E}","Ë"},
    {"\"h","ḧ"},
    {"\"{h}","ḧ"},
    {"\"H","Ḧ"},
    {"\"{H}","Ḧ"},
    {"\"i","ï"},
    {"\"{i}","ï"},
    {"\"I","Ï"},
    {"\"{I}","Ï"},
    {"\"o","ö"},
    {"\"{o}","ö"},
    {"\"O","Ö"},
    {"\"{O}","Ö"},
    {"\"t","ẗ"},
    {"\"{t}","ẗ"},
    {"\"u","ü"},
    {"\"{u}","ü"},
    {"\"U","Ü"},
    {"\"{U}","Ü"},
    {"\"w","ẅ"},
    {"\"{w}","ẅ"},
    {"\"W","Ẅ"},
    {"\"{W}","Ẅ"},
    {"\"x","ẍ"},
    {"\"{x}","ẍ"},
    {"\"X","Ẍ"},
    {"\"{X}","Ẍ"},
    {"\"y","ÿ"},
    {"\"{y}","ÿ"},
    {"\"Y","Ÿ"},
    {"\"{Y}","Ÿ"},
    {"~a","ã"},
    {"~{a}","ã"},
    {"~A","Ã"},
    {"~{A}","Ã"},
    {"~e","ẽ"},
    {"~{e}","ẽ"},
    {"~E","Ẽ"},
    {"~{E}","Ẽ"},
    {"~i","ĩ"},
    {"~{i}","ĩ"},
    {"~I","Ĩ"},
    {"~{I}","Ĩ"},
    {"~n","ñ"},
    {"~{n}","ñ"},
    {"~N","Ñ"},
    {"~{N}","Ñ"},
    {"~o","õ"},
    {"~{o}","õ"},
    {"~O","Õ"},
    {"~{O}","Õ"},
    {"~u","ũ"},
    {"~{u}","ũ"},
    {"~U","Ũ"},
    {"~{U}","Ũ"},
    {"~v","ṽ"},
    {"~{v}","ṽ"},
    {"~V","Ṽ"},
    {"~{V}","Ṽ"},
    {"~y","ỹ"},
    {"~y","ỹ"},
    {"~{Y}","Ỹ"},
    {"~{Y}","Ỹ"},
    {"=a","ā"},
    {"={a}","ā"},
    {"=A","Ā"},
    {"={A}","Ā"},
    {"=e","ē"},  
    {"={e}","ē"},
    {"=E","Ē"},  
    {"={E}","Ē"},
    {"=g","ḡ"},
    {"={g}","ḡ"},
    {"=G","Ḡ"},
    {"={G}","Ḡ"},
    {"=i","ī"},
    {"={i}","ī"},
    {"=I","Ī"},
    {"={I}","Ī"},
    {"=o","ō"},
    {"={o}","ō"},
    {"=O","Ō"},
    {"={O}","Ō"},
    {"=u","ū"},
    {"={u}","ū"},
    {"=U","Ū"},
    {"={U}","Ū"},
    {"=y","ȳ"},
    {"={y}","ȳ"},
    {"=Y","Ȳ"},
    {"={Y}","Ȳ"},
    {".a","ȧ"},
    {".{a}","ȧ"},
    {".A","Ȧ"},
    {".{A}","Ȧ"},
    {".b","ḃ"},
    {".{b}","ḃ"},
    {".B","Ḃ"},
    {".{B}","Ḃ"},
    {".c","ċ"},
    {".{c}","ċ"},
    {".C","Ċ"},
    {".{C}","Ċ"},
    {".d","ḋ"},
    {".{d}","ḋ"},
    {".D","Ḋ"},
    {".{D}","Ḋ"},
    {".e","ė"},
    {".{e}","ė"},
    {".E","̇E"},
    {".{E}","Ė"},
    {".f","ḟ"},
    {".{f}","ḟ"},
    {".F","Ḟ"},
    {".{F}","Ḟ"},
    {".g","ġ"},
    {".{g}","ġ"},
    {".G","Ġ"},
    {".{G}","Ġ"},
    {".h","ḣ̇"},
    {".{h}","ḣ̇"},
    {".H","Ḣ̇"},
    {".{H}","Ḣ̇"},
    {".i","i"},
    {".{i}","i"},
    {".I","I"},
    {".{I}","I"},
    {".m","ṁ"},
    {".{m}","ṁ"},
    {".M","Ṁ"},
    {".{M}","Ṁ"},
    {".n","ṅ"},
    {".{n}","ṅ"},
    {".N","Ṅ"},
    {".{N}","Ṅ"},
    {".o","ȯ"},
    {".{o}","ȯ"},
    {".O","Ȯ"},
    {".{O}","Ȯ"},
    {".p","ṗ"},
    {".{p}","ṗ"},
    {".P","Ṗ"},
    {".{P}","Ṗ"},
    {".r","ṙ"},
    {".{r}","ṙ"},
    {".R","Ṙ"},
    {".{R}","Ṙ"},
    {".s","ṡ"},
    {".{s}","ṡ"},
    {".S","Ṡ"},
    {".{S}","Ṡ"},
    {".t","ṫ"},
    {".{t}","ṫ"},
    {".T","Ṫ"},
    {".{T}","Ṫ"},
    {".w","ẇ"},
    {".{w}","ẇ"},
    {".W","Ẇ"},
    {".{W}","Ẇ"},
    {".x","ẋ"},
    {".{x}","ẋ"},
    {".X","Ẋ"},
    {".{X}","Ẋ"},
    {".y","ẏ"},
    {".{y}","ẏ"},
    {".Y","Ẏ"},
    {".{Y}","Ẏ"},
    {".z","ż"},
    {".{z}","ż"},
    {".Z","Ż"},
    {".{Z}","Ż"},
    {"u{a}","ă"},
    {"u{A}","Ă"},
    {"u{e}","ĕ"},
    {"u{E}","Ĕ"},
    {"u{g}","ğ"},
    {"u{G}","Ğ"},
    {"u{i}","ĭ"},
    {"u{I}","Ĭ"},
    {"u{o}","ŏ"},
    {"u{O}","Ŏ"},
    {"u{u}","ŭ"},
    {"u{U}","Ŭ"},
    {"v{a}","ǎ"},
    {"v{A}","Ǎ"},
    {"v{c}","č"},
    {"v{C}","Č"},
    {"v{d}","ď"},
    {"v{D}","Ď"},
    {"v{e}","ě"},
    {"v{E}","Ě"},
    {"v{g}","ǧ"},
    {"v{G}","Ǧ"},
    {"v{h}","ȟ"},
    {"v{H}","Ȟ"},
    {"v{i}","ǐ"},
    {"v{I}","Ǐ"},
    {"v{j}","ǰ"},
    {"v{J}","ǰ"},
    {"v{k}","ǩ"},
    {"v{K}","Ǩ"},
    {"v{n}","ň"},
    {"v{N}","Ň"},
    {"v{o}","ǒ"},
    {"v{O}","Ǒ"},
    {"v{r}","ř"},
    {"v{R}","Ř"},
    {"v{s}","š"},
    {"v{S}","Š"},
    {"v{t}","ť"},
    {"v{T}","Ť"},
    {"v{u}","ǔ"},
    {"v{U}","Ǔ"},
    {"v{z}","ž"},
    {"v{Z}","Ž"},
    {"H{u}","ű"},
    {"H{U}","Ű"},
    {"H{o}","ő"},
    {"H{O}","Ő"},
    {"c{c}","ç"},
    {"c{C}","Ç"},
    {"c{e}","ȩ"},
    {"c{E}","Ȩ"},
    {"c{g}","ģ"},
    {"c{G}","Ģ"},
    {"c{k}","ķ"},
    {"c{K}","Ķ"},
    {"c{l}","ļ"},
    {"c{L}","Ļ"},
    {"c{n}","ņ"},
    {"c{N}","Ņ"},
    {"c{r}","ŗ"},
    {"c{R}","Ŗ"},
    {"c{s}","ş"},
    {"c{S}","Ş"},
    {"c{t}","ţ"},
    {"c{T}","Ţ"},
    {"oe","œ"},
    {"OE","Œ"},
    {"ae","æ"},
    {"AE","Æ"},
    {"aa","å"},
    {"AA","Å"},
    {"o","ø"},
    {"O","Ø"},
    {"l","ł"},
    {"L","Ł"},
    {"ss","ß"},
    {"th","þ"},
    {"TH","Þ"},
    {"dh","ð"},
    {"DH","Ð"}
  };
    
  map<string,string> populateSymbols(Symbol symbs[], size_t nSyms)
  {
    map<string,string> r;
    for (Symbol* s=symbs; s<symbs+nSyms; ++s)
      r[s->first]=s->second;
    return r;
  }

  map<string,string> latexSymbols=populateSymbols
    (symbolData, sizeof(symbolData)/sizeof(symbolData[0]));

  // extracts a LaTeX token from input, which points to the character
  // after the '\' leadin character. input is adjusted to refer to the
  // next character in the input
  string parseLaTeXSym(const char*& input)
  {
    string r;
    // treat some accented characters as a single symbol (brace form not accepted)
    if (strchr("`'^\"~=.",*input) && *(input+1)!='{') 
      {
        r.assign(input,2);
        input+=2;
        return r;
      }

    // lettered accents with braces
    if (strchr("`'^\"~=.uvHtcdb",*input) && *(input+1)=='{')
      {
        const char* end=strchr(input, '}');
        if (end!=NULL)
          {
            r.assign(input,end-input+1);
            input=end+1;
          }
        return r;
      }

    // normal LaTeX token processing
    while (*input!='\0' && isalpha(*input))
      r+=*input++;
    // if the next character is a whitespace, swallow the character;
    if (isspace(*input)) input++;
    return r;
  }

  // stucture to represent the returned string as it is being built
  struct Result: public string
  {
    vector<string> stack;
    vector<bool> popMore; // if true, then pop will pop the next item of stack

    Result(const string& x=""): string(x) {}

    void push_back(const string& x,bool popAgain=false)
    {
      stack.push_back(x);
      popMore.push_back(popAgain);
    }

    // pushes x onto the stack iff the next character is a brace,
    // otherwise constructs the markup pair
    void push(const string& x, const char*& input)
    {
      if (x=="{")
        {
          push_back(x); // preserve bare braced pairs
          return;
        }
      else if (*input=='{')
        if (x=="rm") // fake an rm tag by deitalicising
          {
            *this+="<span style=\"normal\">";
            push_back("span");
          }
        else
          {
            if (!x.empty()) 
              *this+="<"+x+">";
            push_back(x);
          }
      else if (*input=='\\')
        {
          push_back(x,x!="{");
          if (!x.empty()) 
            *this+="<"+x+">";
          processLaTeX(input);
          //          pop();
          return;
        }
      else if (x=="rm") 
        *this+=string("<span style=\"normal\">")+*input+"</span>";
      else if (!x.empty())
        *this+=string("<")+x+">"+*input+"</"+x+">";
      input++;
    }
    
    void pop()
    {
      if (stack.empty()) return;
      do
        {
          if (!stack.back().empty() && stack.back()!="{")
            *this+="</"+stack.back()+">";
          stack.pop_back();
          popMore.pop_back();
        } while (!stack.empty() && !popMore.empty() && popMore.back());
    }

    void process1arg(const string& tag, const char*& input)
    {
      if (*input=='{')
        push(tag, input);
      else if (!tag.empty())
        push(tag, input);
        //        *this+="<"+tag+">"+*input+++"</"+tag+">";
      else
        *this+=*input++;
    }

    // extract the contents of an optional argument (contained with [])
    string parseOpt(const char*& input)
    {
      string r;
      if (*input=='[') ++input;
      for (; *input!=']' && *input!='\0'; ++input)
        r+=*input;
      if (*input==']') ++input;
      return r;
    }

    void processLaTeX(const char*& input)
    {
      string token=parseLaTeXSym(++input);
      map<string,string>::const_iterator repl=latexSymbols.find(token);
      if (repl!=latexSymbols.end())
        *this+=repl->second;
      else if (token=="mathit" || token=="mathcal")
        process1arg("i", input);
      else if (token=="mathrm" || token=="mathsf")
        process1arg("rm", input);
      else if (token=="mathbf")
        process1arg("b", input);
      else if (token=="mathtt")
        process1arg("tt", input);
      else if (token=="sqrt")
        {
          if (*input=='[')
            {
              string index=parseOpt(input);
              *this+="<small><sup>"+index+"</sup></small>";
            }
          *this+=latexSymbols["surd"];
          process1arg("", input);
        }
      else if (token=="verb")
        {
          // next character is the delimiter
          *this+="<tt>";
          char delim=*input++;
          for (; *input!='\0' && *input!=delim; ++input)
            switch (*input)
              {
              case '<':
                *this+="&lt;";
                break;
              case '>':
                *this+="&gt;";
                break;
              case '&':
                *this+="&amp;";
                break;
              case '\'':
                *this+="&apos;";
                break;
              case '\"':
                *this+="&quot;";
                break;
              default:
                *this+=*input;
                break;
              }
          *this+="</tt>";
          ++input;
        }
      else
        *this+="\\"+token; //unknown token, leave it as is
    }

  };
}

namespace minsky
{
  string latexToPango(const char* input)
  {
    Result r("<i>");
    while (*input!='\0')
      switch (*input)
        {
        case '\\':
          r.processLaTeX(input);
          break;
        case '_':
          r.push("sub", ++input);
          break;
        case '^':
          r.push("sup", ++input);
          break;
        case '{':
          r.push("{", ++input);
          break;  
        case '}':
          input++;
          r.pop();
          break;
        default:
          r+=*input++;
          break;
        }

    // take care of mismatched braces
    while (!r.stack.empty()) r.pop();
    return r+"</i>";
  }

}
