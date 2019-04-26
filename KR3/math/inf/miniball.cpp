#include "stdafx.h"
#include "miniball.h"
#include "../coord.h"

// Global Functions
// ================

using namespace kr;

inline float mb_sqr(float r) { return r*r; }

kr::Miniball::Miniball(const VertexList & vl)
	: m_vl(vl),
	L(),
	support_end(L.begin()),
	m_fsize(0),
	ssize(0),
	m_current_c(nullptr),
	m_current_sqr_r(float(-1))
{
	_assert(vl.size() != 0);

	// set initial center
	m_c[0] = { 0, 0, 0 };
	m_current_c = &m_c[0];

	// compute miniball
	pivot_mb(m_vl.end());
}
kr::Miniball::~Miniball()
{
}

const kr::vec3& kr::Miniball::getCenter() const
{
	return *m_current_c;
}
float kr::Miniball::getRadiusSq() const
{
	return m_current_sqr_r;
}
float kr::Miniball::relative_error(float& subopt) const
{
	float e, max_e = 0.f;
	// compute maximum absolute excess of support points
	for (auto it = L.begin(); it != support_end; ++it) 
	{
		e = excess(**it);
		if (e < 0.f) e = -e;
		if (e > max_e) {
			max_e = e;
		}
	}
	// compute maximum excess of any point
	for (const vec3 & v : m_vl)
	{
		if ((e = excess(v)) > max_e)
			max_e = e;
	}

	subopt = suboptimality();
	_assert(m_current_sqr_r > 0.f || max_e == 0.f);
	return (m_current_sqr_r == 0.f ? 0.f : max_e / m_current_sqr_r);
}
bool kr::Miniball::is_valid(float tol) const
{
	float suboptimality;
	return ((relative_error(suboptimality) <= tol) && (suboptimality == 0));
}
void kr::Miniball::mtf_mb(std::list<const vec3*>::iterator n)
{
	// Algorithm 1: mtf_mb (L_{n-1}, B), where L_{n-1} = [L.begin, n)  
	// B: the set of forced points, defining the current ball
	// S: the superset of support points computed by the algorithm
	// --------------------------------------------------------------
	// from B. Gaertner, Fast and Robust Smallest Enclosing Balls, ESA 1999,
	// http://www.inf.ethz.ch/personal/gaertner/texts/own_work/esa99_final.pdf  

	//   PRE: B = S  
	_assert(m_fsize == ssize);

	support_end = L.begin();
	if ((m_fsize) == 4) return;

	// incremental construction
	for (auto i = L.begin(); i != n;)
	{
		// INV: (support_end - L.begin() == |S|-|B|)
		_assert(std::distance(L.begin(), support_end) == ssize - m_fsize);

		auto j = i++;
		if (excess(**j) > 0.f)
			if (push(**j)) {          // B := B + p_i
				mtf_mb(j);            // mtf_mb (L_{i-1}, B + p_i)
				pop();                 // B := B - p_i
				mtf_move_to_front(j);
			}
	}
	// POST: the range [L.begin(), support_end) stores the set S\B
}
void kr::Miniball::mtf_move_to_front(std::list<const vec3*>::iterator j)
{
	if (support_end == j)
		support_end++;
	L.splice(L.begin(), L, j);
}
void kr::Miniball::pivot_mb(VertexList::Iterator n)
{
	// Algorithm 2: pivot_mb (L_{n-1}), where L_{n-1} = [L.begin, n)  
	// --------------------------------------------------------------
	// from B. Gaertner, Fast and Robust Smallest Enclosing Balls, ESA 1999,
	// http://www.inf.ethz.ch/personal/gaertner/texts/own_work/esa99_final.pdf  
	float          old_sqr_r;
	do {
		old_sqr_r = m_current_sqr_r;
		float sqr_r = m_current_sqr_r;

		VertexList::Iterator pivot = m_vl.begin();
		float max_e = 0.f;
		for (VertexList::Iterator k = m_vl.begin(); k != n; ++k) 
		{
			float e = -sqr_r;
			e += (*k - *m_current_c).length_sq();

			if (e > max_e)
			{
				max_e = e;
				pivot = k;
			}
		}

		if (max_e > 0.f) {
			// check if the pivot is already contained in the support set
			if (std::find(L.begin(), support_end, &*pivot) == support_end) {
				_assert(m_fsize == 0);
				if (push(*pivot)) {
					mtf_mb(support_end);
					pop();
					pivot_move_to_front(&*pivot);
				}
			}
		}
	}
	while (old_sqr_r < m_current_sqr_r);
}
void kr::Miniball::pivot_move_to_front(const vec3* j)
{
	L.push_front(j);
	if (std::distance(L.begin(), support_end) == 5)
		support_end--;
}
inline float kr::Miniball::excess(const vec3& pit) const
{
	float e = -m_current_sqr_r;
	e += (pit - *m_current_c).length_sq();
	return e;
}
void kr::Miniball::pop()
{
	--m_fsize;
}
bool kr::Miniball::push(const vec3& pit)
{
	float eps = std::numeric_limits<float>::epsilon();
	eps *= eps;

	if (m_fsize == 0) 
	{
		m_q0 = pit;
		m_c[0] = m_q0;
		m_sqr_r[0] = 0.f;
	}
	else 
	{
		m_v[m_fsize] = pit - m_q0;

		// compute the a_{m_fsize,i}, i< m_fsize
		for (int i = 1; i < m_fsize; ++i) 
		{
			m_a[m_fsize][i] = dot(m_v[i],m_v[m_fsize]);
			m_a[m_fsize][i] *= (2 / m_z[i]);
		}

		// update v_fsize to Q_fsize-\bar{Q}_fsize
		for (int i = 1; i < m_fsize; ++i) 
		{
			m_v[m_fsize] -= m_v[i] * m_a[m_fsize][i];
		}

		// compute z_fsize
		m_z[m_fsize] = m_v[m_fsize].length_sq();
		m_z[m_fsize] *= 2;

		// reject push if z_fsize too small
		if (m_z[m_fsize] < eps*m_current_sqr_r) 
		{
			return false;
		}

		// update m_c, m_sqr_r
		float e = -m_sqr_r[m_fsize - 1];
		e += (pit - m_c[m_fsize - 1]).length_sq();
		m_f[m_fsize] = e / m_z[m_fsize];
		m_c[m_fsize] = m_c[m_fsize - 1] + m_v[m_fsize] * m_f[m_fsize];
		m_sqr_r[m_fsize] = m_sqr_r[m_fsize - 1] + e*m_f[m_fsize] / 2;
	}
	m_current_c = &m_c[m_fsize];
	m_current_sqr_r = m_sqr_r[m_fsize];
	ssize = ++m_fsize;
	return true;
}
float kr::Miniball::suboptimality() const
{
	float l[4];
	float min_l = 0.f;
	l[0] = 1.f;
	for (int i = ssize - 1; i > 0; --i) 
	{
		l[i] = m_f[i];
		for (int k = ssize - 1; k > i; --k)
			l[i] -= m_a[k][i] * l[k];
		if (l[i] < min_l) min_l = l[i];
		l[0] -= l[i];
	}
	if (l[0] < min_l) min_l = l[0];
	if (min_l < 0.f)
		return -min_l;
	return 0.f;
}
