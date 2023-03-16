#include <Arduino.h>
#include <cstdlib>
#include <iostream>

void setup()
{
  // put your setup code here, to run once:
  std::cout << std::getenv("TEST_STRING") << std::endl;
}

void loop()
{
  // put your main code here, to run repeatedly:
}