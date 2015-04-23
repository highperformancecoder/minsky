/*
 @copyright Steve Keen 2012
 @author Michael Roy
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

#ifndef _PROPERTIESDLG_H
#define _PROPERTIESDLG_H

#include "guiDefs.h"
#include <vector>
#include <Wt/Ext/Dialog>
#include <Wt/WString>
#include <Wt/WContainerWidget>
#include <Wt/WHBoxLayout>
#include <Wt/WLineEdit>
#include <Wt/WText>
#include <Wt/WCheckBox>
#include <boost/function.hpp>
#include <memory>

namespace minsky { namespace gui {


/**
 * Basic edit control
 */
class AbstractPropertyControl
  : public Wt::WContainerWidget
{
private:
  typedef Wt::WContainerWidget _base_class;

public:

  virtual ~AbstractPropertyControl()
  { }

  virtual bool getValue(void*) = 0;

  virtual bool setValue(const void*) = 0;

  virtual void create(const Wt::WString& label, Wt::WLayout* parentLayout) = 0;
};

/**
 * Abstract templated control.
 */
template <class T>
class PropertyControl
  : public AbstractPropertyControl
{
private:
  typedef AbstractPropertyControl _base_class;

protected:
  Wt::WWebWidget* valueWidget; ///< Value widget, pointer belongs to Wt.

public:

  virtual ~PropertyControl()
  { }

  virtual void create(const Wt::WString& label, Wt::WLayout* parentLayout) = 0;

  virtual bool setValue(const T& val)  = 0;

  virtual bool getValue(T& val) = 0;

private:
  virtual bool getValue(void* val)
  {
    return getValue(*static_cast<T*>(val));
  }

  virtual bool setValue(const void* val)
  {
    return setValue(*static_cast<const T*>(val));
  }
};

/**
 * Simple text edit control.
 */
template <class T>
class LineEditPropertyControl
  : public PropertyControl<T>
{
private:
  typedef PropertyControl<T> _base_class;

public:

  virtual ~LineEditPropertyControl()
  { }

  virtual void create(const Wt::WString& label, Wt::WLayout* parentLayout)
  {
    _base_class::setMinimumSize(200, 25);
    _base_class::setSelectable(false);

    Wt::WText* labelWidget  = new Wt::WText(label);
    _base_class::valueWidget = new Wt::WLineEdit;

    labelWidget->setVerticalAlignment(Wt::AlignMiddle);
    dynamic_cast<Wt::WLineEdit*>( _base_class::valueWidget)->setVerticalAlignment(Wt::AlignMiddle);

//    addChild(labelWidget);
//    addChild(valueWidget);

    Wt::WHBoxLayout* layout = new Wt::WHBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(3, 3, 3, 3);
    layout->addWidget(labelWidget);
    layout->addStretch(1);
    layout->addWidget(_base_class::valueWidget);

    parentLayout->addWidget(this);

    _base_class::valueWidget->resize(100, Wt::WLength::Auto);
  }

  virtual bool setValue(const T& val)
  {
    std::wstringstream str;
    str << val;
    dynamic_cast<Wt::WLineEdit*>(_base_class::valueWidget)->setText(str.str());
    return true;
  }

  virtual bool getValue(T& val)
  {
    std::wstringstream str(dynamic_cast<Wt::WLineEdit*>(_base_class::valueWidget)->text());
    str >> val;
    return true;
  }
};

template<>
bool LineEditPropertyControl<Wt::WString>::setValue(const Wt::WString& val);

template<>
bool LineEditPropertyControl<Wt::WString>::getValue(Wt::WString& val);

class CheckBoxPropertyControl
    : public PropertyControl<bool>
{
public:
  virtual ~CheckBoxPropertyControl()
  { }

  virtual void create(const Wt::WString& label, Wt::WLayout* parentLayout)
  {
    setMinimumSize(200, 25);
    setSelectable(false);

    Wt::WText* labelWidget  = new Wt::WText(label);
    valueWidget = new Wt::WCheckBox;

    labelWidget->setVerticalAlignment(Wt::AlignMiddle);
    valueWidget->setVerticalAlignment(Wt::AlignMiddle);

//    addChild(labelWidget);
//    addChild(valueWidget);

    Wt::WHBoxLayout* layout = new Wt::WHBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(3, 3, 3, 3);
    layout->addWidget(labelWidget);
    layout->addStretch(1);
    layout->addWidget(valueWidget);

    parentLayout->addWidget(this);

    labelWidget->resize(200, Wt::WLength::Auto);
  }

  virtual bool setValue(const bool& val)
  {
    dynamic_cast<Wt::WCheckBox*>(valueWidget)->setCheckState((val) ? Wt::Checked : Wt::Unchecked);
    return true;
  }

  virtual bool getValue(bool& val)
  {
    val = (dynamic_cast<Wt::WCheckBox*>(valueWidget)->checkState() == Wt::Checked);
    return true;
  }
};

/**
 * Basic validator
 */
class AbstractValidator
{
public:
  virtual ~AbstractValidator() { }

  /**
   * Performs simultaneous validation and formatting.
   */
  virtual bool validate(void*) = 0;
};

template <class T>
class Validator
    : public AbstractValidator
{
public:
  virtual ~Validator() { }

  /**
   * Performs simultaneous validation and formatting.
   */
  virtual bool validate(T&) = 0;

private:
  virtual bool validate(void* value)
  {
    return validate(*static_cast<T*>(value));
  }
};

/**
 * NULL validator
 */
class NullValidator
    : public AbstractValidator
{
public:
  virtual ~NullValidator()
  { }

  virtual bool validate(void*)
  {
    return true;
  }
};

/**
 *  Range validator.
 */
template<class T>
class MinMaxValidator: public Validator<T>
{
private:
  T minVal;
  T maxVal;

public:
  MinMaxValidator(const T& _minVal, const T& _maxVal) :
      minVal(_minVal), maxVal(_maxVal)
  { }

  virtual ~MinMaxValidator()
  { }

  virtual bool validate(T& val)
  {
    return ((minVal <= val) && (val <= maxVal));
  }
};

/**
 * Custom  validator.
 */
template <class T>
class ProcValidator : public Validator<T>
{
public:
  typedef boost::function<bool(T&)> Procedure;

private:
  Procedure userProc;

public:
  ProcValidator(Procedure _userProc)
    : userProc(_userProc) { }

  virtual ~ProcValidator()
  { }

  virtual bool validate(T& val)
  {
    return userProc(val);
  }
};

/**
 * Basic property
 */
class AbstractProperty
{
public:
  Wt::WString label;
  AbstractValidator*        validator;
  AbstractPropertyControl*  editor;   ///<  pointer belongs to Wt

protected:
  AbstractProperty(const Wt::WString& _label,
                   AbstractValidator* _validator,
                   AbstractPropertyControl*  _editor)
    : label(_label),
      validator(_validator),
      editor(_editor)
  { }

public:
  virtual ~AbstractProperty()
  {
    delete validator;
  }

  virtual bool validate() = 0;

  virtual void createControl(Wt::WLayout* parentLayout) = 0;

  virtual void save() = 0;
};

/**
 * Property for basic types.
 */
template<class T>
class Property: public AbstractProperty
{
public:
  T& value;

  Property(const Wt::WString& _label, T& _value,
           AbstractValidator* _validator,
           AbstractPropertyControl*  _editor)
    : AbstractProperty(_label, _validator, _editor), value(_value)
  { }

  PropertyControl<T>* widget()
  {
    return dynamic_cast<PropertyControl<T>*>(editor);
  }

  virtual bool validate()
  {
    T newValue;
    bool result;
    widget()->getValue(newValue);
    result = validator->validate(&newValue);
    if (result)
    {
      widget()->setValue(newValue);
    }
    return result;
  }

  virtual void createControl(Wt::WLayout* parentLayout)
  {
    widget()->create(label, parentLayout);
    widget()->setValue(value);
  }

  virtual void save()
  {
    widget()->getValue(value);
  }
};

/**
 *	Basic Properties dialog.  
 *
 *	Sets properties for an item on the canvas.
 *
 */
class PropertiesDlg: public Wt::Ext::Dialog
{
private:
  typedef Wt::Ext::Dialog _base_class;

protected:
  typedef std::shared_ptr<AbstractProperty> PropertyPtr;

  typedef std::vector<PropertyPtr> Properties;

private:
  Properties properties;

public:
  /**
   *	Constructor.
   *
   *  Set the default dialog behaviour.
   *
   *	@param parent Parent widget for the dialog. Defaults to the canvas 
   *                if NULL.
   */
  PropertiesDlg(WObject* parent = 0);

  /**
   * Shows the dialog (modeless).
   */
  void show();

  /**
   * Shows the dialog (modal).
   */
  DialogCode exec();

  /**
   * Adds an editable property to the dialog.
   */
  /** @{ */
  void addProperty(const Wt::WString& label, bool& value, AbstractPropertyControl* widget = NULL)
  {
    if (!widget)
    {
      widget = new CheckBoxPropertyControl;
    }
    PropertyPtr prop(new Property<bool> (label,
                                         value,
                                         new NullValidator,
                                         widget));
    properties.push_back(prop);
  }

  template<class T>
  void addProperty(const Wt::WString& label, T& value, AbstractPropertyControl* widget)
  {
    PropertyPtr prop(new Property<T> (label,
                                      value,
                                      new NullValidator,
                                      widget));

    properties.push_back(prop);
  }

  template<class T>
  void addProperty(const Wt::WString& label, T& value)
  {
    AbstractPropertyControl* widget = new LineEditPropertyControl<T>;
    PropertyPtr prop(new Property<T> (label,
                                      value,
                                      new NullValidator,
                                      widget));

    properties.push_back(prop);
  }

  template<class T>
  void addProperty(const Wt::WString& label, T& value,
                   boost::function<bool(T&)> validator, AbstractPropertyControl* widget = NULL)
  {
    if (!widget)
    {
      widget = new LineEditPropertyControl<T>(this);
    }
    PropertyPtr prop(new Property<T> (label,
                                      value,
                                      new ProcValidator<T>(validator),
                                      widget));

    properties.push_back(prop);
  }

  template<class T>
  void addProperty(const Wt::WString& label, T& value,
                   AbstractValidator* validator, AbstractPropertyControl* widget = NULL)
  {
    if (!widget)
    {
      widget = new LineEditPropertyControl<T>;
    }
    PropertyPtr prop(new Property<T> (label,
                                      value,
                                      validator,
                                      widget));

    properties.push_back(prop);
  }

  template<class T>
  void addProperty(const Wt::WString& label, T& value, const T& minVal, const T& maxVal, AbstractPropertyControl* widget = NULL)
  {
    addProperty(label, value, new MinMaxValidator<T>(minVal, maxVal), widget);
  }
  /** @} */
protected:
  /**
   * Creates dialog contents
   */
  virtual void initialize();

  /**
   * Validates user input
   */
  virtual bool validate();

  /**
   * Handler for Ok button
   */
  virtual void onOk();

  /**
   * Handler for Cancel button
   */
  virtual void onCancel();

private:
  /**
   * Handler for Ok button
   */
  void onOkStub();

  /**
   * Handler for Cancel button
   */
  void onCancelStub();

  void onFinished(DialogCode exitCode);
};

}}  // namespace minsky::gui

#endif
