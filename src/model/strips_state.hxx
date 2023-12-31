
/*
Lightweight Automated Planning Toolkit

Copyright 2022
Miquel Ramirez <miquel.ramirez@unimelb.edu.au>Nir Lipovetzky <nirlipo@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files
(the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge,
publish, distribute, sublicense, and/or sell copies of the Software,
and to permit persons to whom the Software is furnished to do so, subject
 to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef __APTK_STATE__
#define __APTK_STATE__

#include <strips_prob.hxx>
#include <types.hxx>
#include <fluent.hxx>
#include <iostream>

namespace aptk
{

	class Action;

	class State
	{
	public:
		State(const STRIPS_Problem &p);
		~State();

		Fluent_Vec &fluent_vec() { return m_fluent_vec; }
		Fluent_Set &fluent_set() { return m_fluent_set; }
		const Fluent_Vec &fluent_vec() const { return m_fluent_vec; }
		const Fluent_Set &fluent_set() const { return m_fluent_set; }

		unsigned value_for_var(unsigned var) const { return 0 == m_fluent_set.isset(var) ? 0 : 1; }

		void set(unsigned f);
		void unset(unsigned f);
		void set(const Fluent_Vec &fv);
		void unset(const Fluent_Vec &fv);
		void reset();
		bool entails(unsigned f) const { return fluent_set().isset(f); }
		bool entails(const State &s) const;
		bool entails(const Fluent_Vec &fv) const;
		bool entails(const Fluent_Vec &fv, unsigned &num_unsat) const;
		size_t hash() const;
		void update_hash();

		State *progress_through(const Action &a, Fluent_Vec *added = NULL, Fluent_Vec *deleted = NULL) const;

		State *progress_through_df(const Action &a) const;

		State *regress_through(const Action &a) const;

		void progress_lazy_state(const Action *a, Fluent_Vec *added = NULL, Fluent_Vec *deleted = NULL);

		void regress_lazy_state(const Action *a, Fluent_Vec *added = NULL, Fluent_Vec *deleted = NULL);

		const STRIPS_Problem &problem() const;

		bool operator==(const State &a) const;

		void print(std::ostream &os) const;

	protected:
		Fluent_Vec m_fluent_vec;
		Fluent_Set m_fluent_set;
		const STRIPS_Problem &m_problem;
		size_t m_hash;
	};

	inline size_t State::hash() const
	{
		return m_hash;
	}

	inline bool State::operator==(const State &a) const
	{
		return fluent_set() == a.fluent_set();
	}

	inline const STRIPS_Problem &State::problem() const
	{
		return m_problem;
	}

	inline void State::set(unsigned f)
	{
		if (entails(f))
			return;
		m_fluent_vec.push_back(f);
		m_fluent_set.set(f);
	}

	inline void State::set(const Fluent_Vec &f)
	{

		for (unsigned i = 0; i < f.size(); i++)
		{
			if (!entails(f[i]))
			{
				m_fluent_vec.push_back(f[i]);
				m_fluent_set.set(f[i]);
			}
		}
	}

	inline void State::unset(unsigned f)
	{
		if (!entails(f))
			return;

		for (unsigned k = 0; k < m_fluent_vec.size(); k++)
			if (m_fluent_vec[k] == f)
			{
				for (unsigned l = k + 1; l < m_fluent_vec.size(); l++)
					m_fluent_vec[l - 1] = m_fluent_vec[l];
				m_fluent_vec.resize(m_fluent_vec.size() - 1);
				break;
			}

		m_fluent_set.unset(f);
	}

	inline void State::unset(const Fluent_Vec &f)
	{

		for (unsigned i = 0; i < f.size(); i++)
		{
			if (!entails(f[i]))
				continue;
			for (unsigned k = 0; k < m_fluent_vec.size(); k++)
				if (m_fluent_vec[k] == f[i])
				{
					for (unsigned l = k + 1; l < m_fluent_vec.size(); l++)
						m_fluent_vec[l - 1] = m_fluent_vec[l];
					m_fluent_vec.resize(m_fluent_vec.size() - 1);
					break;
				}
			m_fluent_set.unset(f[i]);
		}
	}

	inline void State::reset()
	{
		m_fluent_vec.clear();
		m_fluent_set.reset();
	}

	inline bool State::entails(const State &s) const
	{
		return entails(s.fluent_vec());
	}

	inline std::ostream &operator<<(std::ostream &os, State &s);
	inline std::ostream &operator<<(std::ostream &os, const State &s);

	inline bool State::entails(const Fluent_Vec &fv) const
	{
		for (unsigned i = 0; i < fv.size(); i++)
			if (!fluent_set().isset(fv[i]))
			{
				return false;
			}
		return true;
	}

	inline bool State::entails(const Fluent_Vec &fv, unsigned &num_unsat) const
	{
		num_unsat = 0;
		for (unsigned i = 0; i < fv.size(); i++)
			if (!fluent_set().isset(fv[i]))
				num_unsat++;
		return num_unsat == 0;
	}

	inline std::ostream &operator<<(std::ostream &os, State &s)
	{
		for (unsigned i = 0; i < s.fluent_vec().size(); i++)
		{
			os << s.problem().fluents()[s.fluent_vec()[i]]->signature();
			os << ", ";
		}
		os << std::endl;
		return os;
	}

	inline std::ostream &operator<<(std::ostream &os, const State &s)
	{
		for (unsigned i = 0; i < s.fluent_vec().size(); i++)
		{
			os << s.problem().fluents()[s.fluent_vec()[i]]->signature();
			os << ", ";
		}
		os << std::endl;
		return os;
	}

}

#endif // State.hxx
