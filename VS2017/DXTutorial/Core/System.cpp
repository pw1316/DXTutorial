#include <stdafx.h>
#include "System.hpp"
#include <stdio.h>

Naiive::Core::SystemClass& Naiive::Core::System()
{
    static SystemClass obj;
    return obj;
}
