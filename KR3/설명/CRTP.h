
#pragma once

// Curiously recurring template pattern
// https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern

// The Curiously Recurring Template Pattern (CRTP)
template<class T>
class Base
{
	// methods within Base can use template to access members of Derived
};
class Derived : public Base<Derived>
{
	// ...
};