namespace classdesc
{
  // specialise for VariableValues to give it an associative container flavour.
  // defining is_associative_container type attribute doesn't work
  template <> inline
  void RESTProcess(RESTProcess_t& t, const string& d, minsky::VariableValues& a)
  {t.add(d, new RESTProcessAssociativeContainer<minsky::VariableValues>(a));}
}

//namespace classdesc_access
//{
//  namespace cd=classdesc;
//  // specialise for VariableValues to give it an associative container flavour.
//  // defining is_associative_container type attribute doesn't work
//  template <>
//  struct access_RESTProcess<minsky::VariableValues>
//  {
//    void operator()(cd::RESTProcess_t& t, const cd::string& d, minsky::VariableValues& a)
//    {
//      t.add(d, new cd::RESTProcessAssociativeContainer<minsky::VariableValues>(a));
//    }
//  };
//}
