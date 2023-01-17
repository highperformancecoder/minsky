namespace classdesc
{
  // specialise for VariableValues to give it an associative container flavour.
  // defining is_associative_container type attribute doesn't work
  template <> inline
  void RESTProcess(RESTProcess_t& t, const string& d, minsky::VariableValues& a)
  {t.add(d, new RESTProcessAssociativeContainer<minsky::VariableValues>(a));}
}
