#include "not_null.hpp"

#include <memory>

int main()
{
  auto p = cpp::assume_not_null(std::make_shared<int>(0));

  return *p;
}
