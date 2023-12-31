/**
 * @file approx_novelty_siw_plus.hxx
 * @author Anubhav Singh (anubhav.singh.er@pm.me)
 * @author Miquel Ramirez (miquel.ramirez@unimelb.edu.au)
 * @author Nir Lipovetzky (nirlipo@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2023-03-28
 * 
 * @copyright Copyright (c) 2023
 * 
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

#ifndef __SIW_Plus_Approx_Novelty__
#define __SIW_Plus_Approx_Novelty__

#include <aptk/search_prob.hxx>
#include <aptk/resources_control.hxx>
#include <aptk/closed_list.hxx>
// #include <aptk/serialized_search.hxx>
#include "rp_approximate_novelty.hxx"
#include "approximate_novelty_serialized_search.hxx"
#include <landmark_graph.hxx>
#include <vector>
#include <algorithm>
#include <iostream>

namespace aptk
{

	namespace search
	{

		namespace novelty_spaces
		{

			template <typename Search_Model>
			class Approximate_SIW_Plus_Search : public Serialized_Search_Sampling<Search_Model,
																																						RP_IW_Sampling<Search_Model,
																																													 aptk::agnostic::Novelty_Partition<Search_Model, Node<aptk::State>>,
																																													 aptk::agnostic::Relaxed_Plan_Heuristic<Search_Model, aptk::agnostic::H1_Heuristic<Search_Model, aptk::agnostic::H_Add_Evaluation_Function>>>,
																																						Node<aptk::State>>
			{

			public:
				typedef Node<aptk::State> Search_Node;
				typedef aptk::agnostic::Landmarks_Graph Landmarks_Graph;
				typedef aptk::agnostic::Relaxed_Plan_Heuristic<Search_Model, aptk::agnostic::H1_Heuristic<Search_Model, aptk::agnostic::H_Add_Evaluation_Function>> RP_Heuristic;

				Approximate_SIW_Plus_Search(const Search_Model &search_problem, float sample_factor,
																		std::string sampling_strategy, unsigned rand_seed, unsigned min_k4sample)
						: Serialized_Search_Sampling<Search_Model, RP_IW_Sampling<Search_Model, aptk::agnostic::Novelty_Partition<Search_Model, Search_Node>, RP_Heuristic>, Search_Node>(search_problem, sample_factor, sampling_strategy, rand_seed, min_k4sample), m_sum_exp_count(0), m_sum_gen_count(0), m_pruned_sum_B_count(0), m_sum_B_count(0), m_max_B_count(0), m_iw_calls(0), m_max_bound(std::numeric_limits<unsigned>::max())
				{
					m_goal_agenda = NULL;
				}

				virtual ~Approximate_SIW_Plus_Search()
				{
				}
				void set_goal_agenda(Landmarks_Graph *lg) { m_goal_agenda = lg; }

				virtual bool find_solution(float &cost, std::vector<Action_Idx> &plan)
				{

					unsigned gsize = this->problem().task().goal().size();
					Search_Node *end = NULL;
					State *new_init_state = NULL;
					this->m_goals_achieved.clear();
					this->m_goal_candidates.clear();

					cost = 0;

					std::cout << std::endl
										<< "Caption\n{#goals, #UNnachieved,  #Achieved} -> IW(max_w)" << std::endl;

					if (m_goal_agenda)
					{
						m_goal_agenda->get_leafs(this->m_goal_candidates);
					}
					else
					{
						this->m_goal_candidates.insert(this->m_goal_candidates.begin(),
																					 this->problem().task().goal().begin(), this->problem().task().goal().end());
					}

					do
					{

						std::cout << std::endl
											<< "{" << gsize << "/" << this->m_goal_candidates.size() << "/" << this->m_goals_achieved.size() << "}:IW(" << this->bound() << ") -> ";
						end = this->do_search();
						m_pruned_sum_B_count += this->pruned_by_bound();
						m_sum_exp_count += this->expanded();
						m_sum_gen_count += this->generated();

						if (end == NULL)
						{

							/**
							 * If no partial plan to achieve any goal is  found,
							 * throw IW(b+1) from same root node
							 *
							 * If no state has been pruned by bound, then IW is in a dead-end,
							 * return NO-PLAN
							 */
							if (this->pruned_by_bound() == 0)
								return false;

							new_init_state = new State(this->problem().task());
							new_init_state->set(this->m_root->state()->fluent_vec());
							new_init_state->update_hash();

							if (this->bound() > this->max_bound()) // Hard cap on width exceeded
								return false;

							this->set_bound(this->bound() + 1);
							this->start(new_init_state);

							// Memory exceeded to reserve data structures for novelty
							if (this->m_novelty->arity() != this->bound())
								return false;
						}
						else
						{

							/**
							 * If a partial plan extending the achieved goals set is found,
							 * IW(1) is thrown from end_state
							 */

							m_max_B_count = m_max_B_count < this->bound() ? this->bound() : m_max_B_count;
							m_sum_B_count += this->bound();
							m_iw_calls++;

							std::vector<Action_Idx> partial_plan;
							float partial_cost = 0.0f;
							this->extract_plan(this->m_root, end, partial_plan, partial_cost);
							plan.insert(plan.end(), partial_plan.begin(), partial_plan.end());
							cost += partial_cost;

							new_init_state = new State(this->problem().task());
							new_init_state->set(end->state()->fluent_vec());
							new_init_state->update_hash();

							this->set_bound(1);
							this->start(new_init_state);

							if (m_goal_agenda)
							{
								for (Fluent_Vec::iterator it = this->m_goals_achieved.begin(); it != this->m_goals_achieved.end(); it++)
									m_goal_agenda->consume_node(*it);

								this->m_goal_candidates.clear();
								m_goal_agenda->get_leafs(this->m_goal_candidates);
							}

							// this->debug_info( new_init_state, this->m_goal_candidates );
						}

					} while (!this->problem().goal(*new_init_state));

					return true;
				}

				unsigned sum_expanded() const { return m_sum_exp_count; }
				unsigned sum_generated() const { return m_sum_gen_count; }
				unsigned sum_pruned_by_bound() const { return m_pruned_sum_B_count; }
				float avg_B() const { return (float)(m_sum_B_count) / m_iw_calls; }
				unsigned max_B() const { return m_max_B_count; }
				void set_max_bound(unsigned v) { m_max_bound = v; }
				unsigned max_bound() { return m_max_bound; }

			protected:
				unsigned m_sum_exp_count;
				unsigned m_sum_gen_count;
				unsigned m_pruned_sum_B_count;
				unsigned m_sum_B_count;
				unsigned m_max_B_count;
				unsigned m_iw_calls;
				Landmarks_Graph *m_goal_agenda;
				unsigned m_max_bound;
			};

		}

	}

}

#endif // siw.hxx
