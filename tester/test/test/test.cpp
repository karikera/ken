#include <cbs/alignedarray.h>
#include <cbs/char.h>
#include <cbs/datalist.h>
#include <cbs/deltams.h>
#include <cbs/indexmaker.h>
#include <cbs/serializer.h>

using namespace cbs;

struct Test
{
	int a, b, c;
	std::string d;
	std::vector<int> e;

	template <typename S>
	void serialize(S &s)
	{
		s & a;
		s & b;
		s & c;
		s & d;
		s & e;
	}
};

int main()
{
	Test t1 = { 1,2,3, "test", {0,1,2,3,4,4,4,4,4} };
	Serializer s;
	s << t1;

	std::string str = s.str();

	Test t2;
	Deserializer d(s.str());
	d >> t2;
}