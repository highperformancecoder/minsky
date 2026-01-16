/*
  Test for scalar division issue - ticket #1886
*/

#include "variableType.h"
#include "evalOp.h"
#include "minskyTensorOps.h"
#include "minsky.h"
#include "minsky_epilogue.h"
#include <iostream>
#include <cassert>

using namespace minsky;
using namespace std;

int main()
{
  Minsky dummyM;
  LocalMinsky lm{dummyM};
  
  // Create a vector [1, 2, 3, 4, 5]
  VariablePtr vec(VariableType::flow, "vec");
  vec->init("iota(5)+1");  // [1, 2, 3, 4, 5]
  
  // Create a scalar value 2
  VariablePtr scalar(VariableType::flow, "scalar");
  scalar->init("2");
  
  // Create result variable
  VariablePtr result(VariableType::flow, "result");
  
  // Create division operation
  OperationPtr divOp(OperationType::divide);
  
  // Wire them up: vec / scalar
  GroupPtr g(new Group);
  g->self = g;
  g->addItem(vec);
  g->addItem(scalar);
  g->addItem(result);
  g->addItem(divOp);
  
  Wire w1(vec->ports(0), divOp->ports(1));
  Wire w2(scalar->ports(0), divOp->ports(2));
  Wire w3(divOp->ports(0), result->ports(1));
  
  // Evaluate
  auto ev = make_shared<EvalCommon>();
  TensorsFromPort tp(ev);
  TensorEval eval(result->vValue(), ev, TensorOpFactory().create(divOp, tp));
  eval.eval(ValueVector::flowVars.data(), ValueVector::flowVars.size(), ValueVector::stockVars.data());
  
  // Check results
  cout << "Vector: ";
  for (size_t i = 0; i < vec->vValue()->size(); ++i)
    cout << (*vec->vValue())[i] << " ";
  cout << "\nScalar: " << scalar->vValue()->value() << "\n";
  cout << "Result: ";
  for (size_t i = 0; i < result->vValue()->size(); ++i)
    cout << (*result->vValue())[i] << " ";
  cout << "\n";
  
  cout << "Scalar rank: " << scalar->vValue()->rank() << "\n";
  cout << "Scalar size: " << scalar->vValue()->size() << "\n";
  
  // Expected: [0.5, 1.0, 1.5, 2.0, 2.5]
  bool success = true;
  for (size_t i = 0; i < result->vValue()->size(); ++i)
  {
    double expected = (i + 1.0) / 2.0;
    double actual = (*result->vValue())[i];
    if (abs(expected - actual) > 1e-10)
    {
      cout << "ERROR: result[" << i << "] = " << actual << ", expected " << expected << "\n";
      success = false;
    }
  }
  
  if (success)
    cout << "TEST PASSED\n";
  else
    cout << "TEST FAILED\n";
  
  return success ? 0 : 1;
}
