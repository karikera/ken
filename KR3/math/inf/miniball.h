#pragma once

#include <cassert>
#include <algorithm>
#include <list>
#include <ctime>
#include <limits>

#include "../vertexlist.h"

namespace kr
{
	
	// Class Declaration
	// =================

	class Miniball
	{
	public:
		// The iterator type to go through the support points

		// PRE:  [begin, end) is a nonempty range
		// POST: computes the smallest enclosing ball of the points in the range
		//       [begin, end); the functor a maps a point iterator to an iterator 
		//       through the d coordinates of the point  
		Miniball(const VertexList & vl);

		// POST: returns a pointer to the first element of an array that holds
		//       the d coordinates of the center of the computed ball  
		const vec3& getCenter() const;

		// POST: returns the squared radius of the computed ball  
		float getRadiusSq() const;
	
		// POST: returns the maximum excess of any input point w.r.t. the computed 
		//       ball, divided by the squared radius of the computed ball. The 
		//       excess of a point is the difference between its squared distance
		//       from the center and the squared radius; Ideally, the return value 
		//       is 0. subopt is set to the absolute value of the most negative 
		//       coefficient in the affine combination of the support points that 
		//       yields the center. Ideally, this is a convex combination, and there 
		//       is no negative coefficient in which case subopt is set to 0.  
		float relative_error(float& subopt) const;

		// POST: return true if the relative error is at most tol, and the
		//       suboptimality is 0; the default tolerance is 10 times the
		//       coordinate type's machine epsilon  
		bool is_valid(float tol = float(10) * std::numeric_limits<float>::epsilon()) const;
	
		// POST: deletes dynamically allocated arrays
		~Miniball();

	private:
		void mtf_mb(std::list<const vec3*>::iterator n);
		void mtf_move_to_front(std::list<const vec3*>::iterator j);
		void pivot_mb(VertexList::Iterator n);
		void pivot_move_to_front(const vec3* j);
		float excess(const vec3& pit) const;
		void pop();
		bool push(const vec3& pit);
		float suboptimality() const;

	private:
		const VertexList & m_vl;

		//...for the algorithms
		std::list<const vec3*> L;
		std::list<const vec3*>::iterator support_end;
		//typedef std::list<const vec3*>::const_iterator SupportPointIterator;
		int m_fsize;   // number of forced points                                   
		int ssize;   // number of support points                               

					 // ...for the ball updates
		vec3 * m_current_c; // ptrlized
		float  m_current_sqr_r;
		vec3 m_c[4];
		vec3 m_v[4];
		vec3 m_a[4];
		float m_z[4];
		float m_f[4];
		float m_sqr_r[4];

		// helper arrays
		vec3 m_q0;

	};


}
