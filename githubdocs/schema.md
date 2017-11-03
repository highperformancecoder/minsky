The format for the Minksy file (.mky extension) is documented as an XML schema at <http://minsky.sf.net/minsky.xsd>. The schema version used by Minsky 1.x is 1, and Minsky 2.x is 2, thus the schema version is currently tracking the Minsky version. This will not always be the case, however, but if the schema version is bumped, then so will the Minsky version. 

A UML representation of the schema can be found ![here](../MinskySchema1.svg).


By following this schema, you can create other tools that can interact with Minsky.

Notes:

* Minsky does not care what order elements are presented in, although schema validators (eg xmllint) do

* Minsky does not care if elements are missing (it substitutes default values), and ignores additional elements

* Leading and trailing whitespace on element fields are trimmed.

* If an element is missing, but an attribute of the same name is present, the Minsky will use that. So the following two code snippets are handled identically:

~~~~~~~~~~~~~~~~
    <Minsky schemaVersion="1">
    </Minsky>
~~~~~~~~~~~~~~~~
------------
~~~~~~~~~~~~~~~~
    <Minsky>
      <schemaVersion>1</schemaVersion>
    </Minsky>
~~~~~~~~~~~~~~~~

* The Schema 1 Layout vector contains optional visual layout information, so tools that do not care about layout can ignore this. In the fullness of time, Minsky will provide an auto-layout functionality to allow the import of Minsky files with partial, or even no layout information. In Schema 2, the layout information is incorporated into the items and wires themselves, but are entirely optional fields.

* The layout vector elements have entirely optional elements, except for the one mandatory element id, which refers to the id of the minskyModel element the layout element is describing. 
