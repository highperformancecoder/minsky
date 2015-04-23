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

#include "database.h"
#include "ecolab_epilogue.h"
using namespace soci;
using namespace schema1;

namespace minsky
{
//  namespace
//  {
//    // quote a string so it can be used as a SQL literal ' and \ are escaped
//    string SQLquote(const string& s)
//    {
//      string r="'";
//      for (int i=0; i<s.length(); ++i)
//        switch (s[i])
//          {
//          case '\'':
//            r+="''";
//            break;
////          case '\\':
////            r+="\\\\";
////            break;
//          default:
//            r+=s[i];
//            break;
//          }
//      return r+"'";
//    }
//  }

  int Database::createElement(int modelId, Item& x) 
  {
    transaction tr(db);
    int maxItemId;
    db << "select maxelem from models where id="<<modelId, into(maxItemId);
    db << "update models set maxelem="<<(++maxItemId)<<" where id="<<modelId;
    x.id=maxItemId;

    string type=x.type(), data=x.json();

    db << "insert into elements (modelId, id, type, data) "
      "values (:modelId,:id,:type,:data)",use(modelId),use(maxItemId),use(type),use(data);

    // insert a default Layout
    UnionLayout l;
    type=l.type(); data=l.json();
    db << "insert into layouts (modelId, id, type, data) "
      "values (:modelId,:id,:type,:data)",use(modelId),use(maxItemId),use(type),use(data);
     
    tr.commit();
    return maxItemId;
  }

  unique_ptr<Item> Database::readElement(int modelId, int id)
  {
    string type, elemData;
    db << "select type, data from elements where id="<<id<<" and modelId="
       << modelId, into(type), into(elemData);
    unique_ptr<Item> r(factory<Item>(type));
    r->json(elemData);
    return r;
  }

  void Database::updateElement(int modelId, const Item& x) {}

  void Database::deleteElement(int modelId, const Item& x) {}

  auto_ptr<Layout> Database::readLayout(int modelId, int id) 
  {return auto_ptr<Layout>();}

  void Database::updateLayout(int modelId, const Layout& x) {}
   
  namespace
  {
    // copy a vector into a base polymorphic vector, where T is a subclass of B
    template <class T>
    void moveInto(vector<int>& ids, vector<string>& types, 
                  vector<string>& data, vector<T>& y)
    {
      for (typename vector<T>::const_iterator i=y.begin(); i!=y.end(); ++i)
        {
          const T& ii=*i;
          ids.push_back(i->id);
          types.push_back(i->typeName());
          data.push_back(i->json());
        }
      y.clear();
    }
  }


  int Database::createModel(const string& name, const string& owner, 
                            schema1::Minsky m) 
  {
    // extract elements and layouts for later storage in their tables
    vector<string> elementType, elementData, layoutType, layoutData;
    vector<int> ids;
    moveInto(ids, elementType, elementData, m.model.ports);
    moveInto(ids, elementType, elementData, m.model.wires);
    moveInto(ids, elementType, elementData, m.model.operations);
    moveInto(ids, elementType, elementData, m.model.variables);
    moveInto(ids, elementType, elementData, m.model.plots);
    moveInto(ids, elementType, elementData, m.model.groups);
    moveInto(ids, elementType, elementData, m.model.godleys);

    // prepare type and data vectors ready for the insert
    int maxElementId=0;
    for (size_t i=0; i<ids.size(); ++i)
      if (maxElementId < ids[i])
        maxElementId=ids[i];

    // firstly, store the layout data in a map, so we can map to same
    // order as elements
    map<int, shared_ptr<Layout> > layouts;
    for (size_t i=0; i<m.layout.size(); ++i)
      layouts.insert(make_pair(m.layout[i]->id, m.layout[i]));
    m.layout.clear();

    for (vector<int>::const_iterator i=ids.begin(); i!=ids.end(); ++i)
      if (layouts[*i])
        {
          layoutType.push_back(layouts[*i]->type());
          layoutData.push_back(layouts[*i]->json());
        }
      else
        {
          layoutType.push_back(UnionLayout().type());
          layoutType.push_back(UnionLayout().json());
        }

    int modelId;
    string modelData=json(m);
    indicator ind;
    transaction tr(db);
    db << "insert into models (name, owner, maxelem, data) values ('"<< 
      name <<"','"<< owner <<"',"<< maxElementId <<",:modelData)",use(modelData); 
    // retrieve automatically allocated modelId
    db << "select max(id) from models",into(modelId);
    db << "insert into elements (modelid, id, type, data) values ("
       <<modelId<<", :id, :type, :data)",
      use(ids), use(elementType),use(elementData);
    db << "insert into layouts (modelid, id, type, data) values ("<<
      modelId << ", :id, :type, :data)",
      use(ids), use(layoutType),use(layoutData);

    tr.commit();
    return modelId;
  }

  void Database::readModel(int modelId, schema1::Minsky& model) 
  {
    string modelData;
    db << "select data from models where id="<<modelId, into(modelData);
    json(model, modelData);

    // retrieve elements
    int count;
    db << "select count(id) from elements where modelid="<<modelId,into(count);
    vector<string> type(count), data(count);
    db << "select type,data from elements where modelid="<<modelId, into(type), into(data);

    for (size_t i=0; i<type.size(); ++i)
      {
        unique_ptr<Item> r(factory<Item>(type[i]));
        r->json(data[i]);
        if (schema1::Operation* op=dynamic_cast<schema1::Operation*>(r.get()))
          model.model.operations.push_back(*op);
        // TODO: the other types
      }

    // TODO: layouts

  }
  void Database::updateModel(int modelId, const schema1::Minsky&) {}
  void Database::deleteModel(int modelId) {}

  
  void Database::listModels(const string& user, ModelList& models)
  {
    // TODO - select based on user field and shares
    // bloody soci requires presizing the input vectors! grrrr
    unsigned count;
    db << "select count(id) from models",into(count);
    if (count==0) return;
    vector<int> ids(count);
    vector<string> names(count);
    db << "select id, name from models",into(ids),into(names);
    for (size_t i=0; i<ids.size(); ++i)
      models.push_back(ModelDescriptor(ids[i], names[i], false));
  }

}
