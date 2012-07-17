/*
 *   Repast for High Performance Computing (Repast HPC)
 *
 *   Copyright (c) 2010 Argonne National Laboratory
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with
 *   or without modification, are permitted provided that the following
 *   conditions are met:
 *
 *     Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *
 *     Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *
 *     Neither the name of the Argonne National Laboratory nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 *   PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE TRUSTEES OR
 *   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 *   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 *   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 *   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 *   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 *   EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *
 *  Context.h
 *
 *  Created on: May 22, 2009
 *      Author: nick
 */

#ifndef CONTEXT_H_
#define CONTEXT_H_

#include <vector>
#include <set>

#include <boost/unordered_map.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/iterator/filter_iterator.hpp>
#include <boost/function.hpp>

#include "AgentId.h"
#include "AgentRequest.h"
#include "Random.h"
#include "ValueLayer.h"
#include "Projection.h"
#include "RepastErrors.h"

namespace repast {

// Unary function used in the transform_iterator that allows context iterators
// to return the agent maps values.
template<typename T>
struct SecondElement: public std::unary_function<typename boost::unordered_map<AgentId, boost::shared_ptr<T> >::value_type, boost::shared_ptr<T> > {
  boost::shared_ptr<T> operator()(const typename boost::unordered_map<AgentId, boost::shared_ptr<T> >::value_type& value) const {
    const boost::shared_ptr<T>& ptr = value.second;
    return ptr;
  }
};



/**
 * Collection of agents of type T with set semantics. Object identity and equality
 * is determined by their AgentId.
 *
 * @tparam the type objects contained by the Context. The T must extends repast::Agent.
 */
template<typename T>
class Context {

private:

	typedef typename std::vector<Projection<T>*>::iterator ProjPtrIter;
	typedef typename boost::unordered_map<AgentId, boost::shared_ptr<T>, HashId> AgentMap;

	typedef typename AgentMap::iterator AgentMapIterator;
	typedef typename AgentMap::const_iterator AgentMapConstIterator;

	AgentMap agents;
	std::vector<Projection<T> *> projections;
	std::map<std::string, BaseValueLayer*> valueLayers;

public:

	typedef typename boost::transform_iterator<SecondElement<T> , typename AgentMap::const_iterator> const_iterator;
	typedef typename boost::filter_iterator<IsAgentType<T> , typename Context<T>::const_iterator> const_bytype_iterator;

	Context();

	/**
	 * Destroys this context and the projections it contains.
	 */
	virtual ~Context();

	/**
	 * Adds the agent to the context. Returns true if the
	 * agent is added, or false if an agent with the same
	 * id is already in the context.
	 *
	 * @param agent the agent to add
	 *
	 * @return true if the agent is successfully dded, or false if an agent with the same
	 * id is already in the context.
	 */
	bool addAgent(T* agent);

	/**
	 * Adds the specified projection to this context. All the agents in this
	 * context will be added to the Projection. Any agents subsequently added
	 * to this context will also be added to the Projection.
	 *
	 * @param projection the projection to add
	 */
	void addProjection(Projection<T>* projection);

	/**
	 * Get the named Projection.
	 *
	 * @param the name of the projection to get
	 *
	 * @return the named Projection or 0 if no such Projection is found.
	 */
	Projection<T>* getProjection(const std::string& name);

	/**
	 * Removes the specified agent from this context.
	 *
	 * @param id the id of the agent to remove
	 */
	void removeAgent(const AgentId id);

	/**
	 * Removes the specified agent from this context.
	 */
	void removeAgent(T* agent);

	/**
	 * Gets the specified agent.
	 *
	 * @param the id of the agent to get.
	 */
	T* getAgent(const AgentId& id);

	/**
	 * Gets at random the specified count of agents and returns them
	 * in the agents vector.
	 *
	 * @param count the number of agents to get
	 * @param [out] agents a vector where the agents will be returned
	 */
	void getRandomAgents(const int count, std::vector<T*>& agents);

	/**
	 * Gets the start of iterator over the agents in this context.
	 * The iterator derefrences into shared_ptr<T>. The actual
	 * agent can be accessed by derefrenceing the iter: (*iter)->getId() for example.
	 *
	 * @return the start of iterator over the agents in this context.
	 */
	const_iterator begin() const {
		return const_iterator(agents.begin());
	}

	/**
	 * Gets the end of an iterator over the agents in this context. The iterator derefrences
	 * into shared_ptr<T>. The actual agent can be accessed by derefrenceing the iter:
	 * (*iter)->getId() for example.
	 *
	 * @return  the end of an iterator over the agents in this context
	 */
	const_iterator end() const {
		return const_iterator(agents.end());
	}

	/**
	 * Gets the start of an iterator over agents in this context of the specified type. The type
	 * corresponds to the type component of an agent's AgentId.
	 *
	 * @param typeId the type of the agent. Only Agents whose agentId.agentType() is equal to
	 * this typeId will be included in the iterator
	 *
	 * @return the start of an iterator over agents in this context of the specified type.
	 */
	const_bytype_iterator byTypeBegin(int typeId) const {
		return const_bytype_iterator(IsAgentType<T> (typeId), Context<T>::begin(), Context<T>::end());
	}

	/**
	 * Gets the end of an iterator over agents in this context of the specified type. The type
	 * corresponds to the type component of an agent's AgentId.
	 *
	 * @param typeId the type of the agent. Only Agents whose agentId.agentType() is equal to
	 * this typeId will be included in the iterator
	 *
	 * @return the end of an iterator over agents in this context of the specified type.
	 */
	const_bytype_iterator byTypeEnd(int typeId) const {
		return const_bytype_iterator(IsAgentType<T> (typeId), Context<T>::end(), Context<T>::end());
	}

	/**
	 * Returns true if the specified agent is in this context, otherwise false.
	 */
	bool contains(const AgentId& id);

	/**
	 * Gets the size (number of agents) in this context.
	 */
	int size() const {
		return agents.size();
	}

	/**
	 * Adds a value layer to this context.
	 *
	 * @param valueLayer the value layer to add
	 */
	void addValueLayer(BaseValueLayer* valueLayer);

	/**
	 * Gets the named discrete value layer from this Context. The value layer must have been
	 * previously added.
	 *
	 * @param valueLayerName the name of the value layer to get
	 *
	 * @tparam ValueType the numeric type contained by the value layer
	 * @tparam Borders the Border type of the value layer
	 *
	 * @return the named discrete value layer from this Context.
	 */
	template<typename ValueType, typename Borders>
	DiscreteValueLayer<ValueType, Borders>* getDiscreteValueLayer(const std::string& valueLayerName);

	/**
	 * Gets the named continuous value layer from this Context. The value layer must have been
	 * previously added.
	 *
	 * @param valueLayerName the name of the value layer to get
	 *
	 * @tparam ValueType the numeric type contained by the value layer
	 * @tparam Borders the Border type of the value layer
	 *
	 * @return the named continuous value layer from this Context.
	 */
	template<typename ValueType, typename Borders>
	ContinuousValueLayer<ValueType, Borders>* getContinuousValueLayer(const std::string& valueLayerName);

	/**
	 * Creates a filtered iterator over the set of agents
	 * in this context and returns it with a value equal
	 * to the beginning of the list.
	 *
	 * The struct can be any user-defined structure that implements
	 * a unary operator (see IsAgentType) that can be passed and
	 * which will become a filter to sort across the agent
     * list, e.g.:
     *
     * struct filter {
     *    bool operator()(const boost::shared_ptr<T>& ptr){ return (ptr->getAgentValue() == targetValue;) }
     * }
     *
     * This should allow filtering of agents by any attribute.
     *
     * @param fStruct an instance of the struct to be used as the filter
	 *
	 * @tparam filterStruct the type of the filter to be applied to the agents
	 *
	 * @return an iterator positioned at the beginning of the list of
	 * agents meeting the filter's criteria
	 */
	template<typename filterStruct>
	boost::filter_iterator<filterStruct, typename Context<T>::const_iterator> filteredBegin(const filterStruct& fStruct);
	
	/**
	 * Creates a filtered iterator over the set of agents
	 * in this context and returns it with a value equal
	 * to one step past end of the list.
	 *
	 * The struct can be any user-defined structure that implements
	 * a unary operator (see IsAgentType) that can be passed and
	 * which will become a filter to sort across the agent
	 * list, e.g.:
	 *
	 * struct filter {
	 *    bool operator()(const boost::shared_ptr<T>& ptr){ return (ptr->getAgentValue() == targetValue;) }
	 * }
	 *
	 * This should allow filtering of agents by any attribute.
	 *
	 * @param fStruct an instance of the struct to be used as the filter
	 *
	 * @tparam filterStruct the type of the filter to be applied to the agents
	 *
	 * @return an iterator positioned at one past the end of the list of
	 * agents meeting the filter's criteria
	 */
    template<typename filterStruct>
    boost::filter_iterator<filterStruct, typename Context<T>::const_iterator> filteredEnd(const filterStruct& fStruct);
	
	
	/**
	 * Creates a filtered iterator over the set of agents
	 * in this context of the specified type (per their AgentId
	 * values), and returns it with a value equal
	 * to the beginning of the list.
	 *
	 * The struct can be any user-defined structure that implements
	 * a unary operator (see IsAgentType) that can be passed and
	 * which will become a filter to sort across the agent
	 * list, e.g.:
	 *
	 * struct filter {
	 *    bool operator()(const boost::shared_ptr<T>& ptr){ return (ptr->getAgentValue() == targetValue;) }
	 * }
	 *
	 * This should allow filtering of agents by type and on any attribute.
	 *
	 * @param fStruct an instance of the struct to be used as the filter
	 * @param type the numeric type of agents to be included in the list
	 *
	 * @tparam filterStruct the type of the filter to be applied to the agents
	 *
	 * @return an iterator positioned at the beginning of the list of
	 * agents meeting the filter's criteria
	 */
    template<typename filterStruct>
    boost::filter_iterator<filterStruct, typename Context<T>::const_bytype_iterator> byTypeFilteredBegin(const int type, const filterStruct& fStruct);
	
	/**
	 * Creates a filtered iterator over the set of agents
	 * in this context of the specified type (per their AgentId
	 * values), and returns it with a value equal
	 * to one past the end of the list.
	 *
	 * The struct can be any user-defined structure that implements
	 * a unary operator (see IsAgentType) that can be passed and
	 * which will become a filter to sort across the agent
	 * list, e.g.:
	 *
	 * struct filter {
	 *    bool operator()(const boost::shared_ptr<T>& ptr){ return (ptr->getAgentValue() == targetValue;) }
	 * }
	 *
	 * This should allow filtering of agents by type and on any attribute.
	 *
	 * @param fStruct an instance of the struct to be used as the filter
	 * @param type the numeric type of agents to be included in the list
	 *
	 * @tparam filterStruct the type of the filter to be applied to the agents
	 *
	 * @return an iterator positioned at one past the end of the list of
	 * agents meeting the filter's criteria
	 */
    template<typename filterStruct>
    boost::filter_iterator<filterStruct, typename Context<T>::const_bytype_iterator> byTypeFilteredEnd(const int type, const filterStruct& fStruct);
	
	/**
	 * Gets a set of pointers to all agents in this context.
	 *
	 * If the 'remove' parameter is set to true, any elements in the
	 * original set will be removed before the method returns.
	 *
	 * @param [out] selectedAgents a set into which the pointers to the agents will be placed
	 * @param remove if true, remove any elements originally in the set before the
	 * set is returned (default is false)
	 */
	void selectAgents(std::set<T*>& selectedAgents, bool remove = false);

	/**
	 * Gets a randomly ordered vector of pointers to all agents in this context.
	 *
	 * If the 'remove' parameter is set to true, any elements in the
	 * original vector will be removed before the method returns.
	 *
	 * @param [out] selectedAgents a vector into which the pointers to the agents will be placed
	 * @param remove if true, remove any elements originally in the set before the
	 * set is returned (default is false)
	 */
	void selectAgents(std::vector<T*>& selectedAgents, bool remove = false);

	/**
	 * Gets a set of pointers to a specified number of randomly selected agents.
	 *
	 * If the set passed contains any elements when this method is called,
	 * the agents pointed to by those elements will be omitted from the selection.
	 *
	 * If the 'remove' parameter is set to true, any elements in the
	 * original set will be removed before the method returns.
	 *
	 * @param count the number of agents to be selected. If this exceeds the number
	 * that can possibly be selected, all possible agents will be selected
	 * @param [out] selectedAgents a set into which the pointers to the agents will be placed
	 * @param remove if true, remove any elements originally in the set before the
	 * set is returned (default is false)
	 */
	void selectAgents(int count, std::set<T*>& selectedAgents, bool remove = false);

	/**
	 * Gets a randomly ordered vector of pointers to a specified number of randomly selected agents.
	 *
	 * If the vector passed contains any elements when this method is called,
	 * the agents pointed to by those elements will be omitted from the selection.
	 *
	 * If the 'remove' parameter is set to true, any elements in the
	 * original vector will be removed before the method returns.
	 *
	 * @param count the number of agents to be selected. If this exceeds the number
	 * that can possibly be selected, all possible agents will be selected
	 * @param [out] selectedAgents a vector into which the pointers to the agents will be placed
	 * @param remove if true, remove any elements originally in the set before the
	 * set is returned (default is false)
	 */
	void selectAgents(int count, std::vector<T*>& selectedAgents, bool remove = false);
	
	/**
	 * Gets a set of pointers to all agents in this context
	 * of a specified type (per their AgentId values).
	 *
	 * If the 'remove' parameter is set to true, any elements in the
	 * original set will be removed before the method returns.
	 *
	 * The popSize parameter is used when the method is repeatedly called
	 * on a population whose size is known. Calls to this method typically begin
	 * by determining the size of the (valid) population to be sampled; if this is known,
	 * it can be provided here, improving performance.
	 *
	 * @param [out] selectedAgents a set into which the pointers to the agents will be placed
	 * @param type numeric type of agent to be selected
	 * @param remove if true, remove any elements originally in the set before the
	 * set is returned (default is false)
	 * @param popSize size of the population from which the sample will be drawn
	 */
    void selectAgents(std::set<T*>& selectedAgents, int type, bool remove = false, int popSize = -1);

	/**
	 * Gets a randomly ordered vector of pointers to all agents in this context
	 * of a specified type (per their AgentId values).
	 *
	 * If the 'remove' parameter is set to true, any elements in the
	 * original vector will be removed before the method returns.
	 *
	 * The popSize parameter is used when the method is repeatedly called
	 * on a population whose size is known. Calls to this method typically begin
	 * by determining the size of the (valid) population to be sampled; if this is known,
	 * it can be provided here, improving performance.
	 *
	 * @param [out] selectedAgents a vector into which the pointers to the agents will be placed
	 * @param type numeric type of agent to be selected
	 * @param remove if true, remove any elements originally in the set before the
	 * set is returned (default is false)
	 * @param popSize size of the population from which the sample will be drawn
	 */
	void selectAgents(std::vector<T*>& selectedAgents, int type, bool remove = false, int popSize = -1);

	/**
	 * Gets a set of pointers to a specified number of randomly selected agents
	 * of a specified type (per their AgentId values).
	 *
	 * If the set passed contains any elements when this method is called,
	 * the agents pointed to by those elements will be omitted from the selection.
	 *
	 * If the 'remove' parameter is set to true, any elements in the
	 * original set will be removed before the method returns.
	 *
	 * The popSize parameter is used when the method is repeatedly called
	 * on a population whose size is known. Calls to this method typically begin
	 * by determining the size of the (valid) population to be sampled; if this is known,
	 * it can be provided here, improving performance.
	 *
	 * @param count the number of agents to be selected. If this exceeds the number
	 * that can possibly be selected, all possible agents will be selected
	 * @param [out] selectedAgents a set into which the pointers to the agents will be placed
	 * @param type numeric type of agent to be selected
	 * @param remove if true, remove any elements originally in the set before the
	 * set is returned (default is false)
	 * @param popSize size of the population from which the sample will be drawn
	 */
	void selectAgents(int count, std::set<T*>& selectedAgents, int type, bool remove = false, int popSize = -1);

	/**
	 * Gets a randomly ordered vector of pointers to a specified number of randomly selected agents
	 * of a specified type (per their AgentId values).
	 *
	 * If the vector passed contains any elements when this method is called,
	 * the agents pointed to by those elements will be omitted from the selection.
	 *
	 * If the 'remove' parameter is set to true, any elements in the
	 * original vector will be removed before the method returns.
	 *
	 * The popSize parameter is used when the method is repeatedly called
	 * on a population whose size is known. Calls to this method typically begin
	 * by determining the size of the (valid) population to be sampled; if this is known,
	 * it can be provided here, improving performance.
	 *
	 * @param count the number of agents to be selected. If this exceeds the number
	 * that can possibly be selected, all possible agents will be selected
	 * @param [out] selectedAgents a vector into which the pointers to the agents will be placed
	 * @param type numeric type of agent to be selected
	 * @param remove if true, remove any elements originally in the set before the
	 * set is returned (default is false)
	 * @param popSize size of the population from which the sample will be drawn
	 */
	void selectAgents(int count, std::vector<T*>& selectedAgents, int type, bool remove = false, int popSize = -1);
	
	/**
	 * Gets a set of pointers to all agents in this context matching a user-defined filter.
	 *
	 * If the 'remove' parameter is set to true, any elements in the
	 * original set will be removed before the method returns.
	 *
	 * The popSize parameter is used when the method is repeatedly called
	 * on a population whose size is known. Calls to this method typically begin
	 * by determining the size of the (valid) population to be sampled; if this is known,
	 * it can be provided here, improving performance.
	 *
	 * that can possibly be selected, all possible agents will be selected
	 * @param [out] selectedAgents a set into which the pointers to the agents will be placed
	 * @param filter user-defined filter specifying any criteria agents to be selected
	 * must meet
	 * @param remove if true, remove any elements originally in the set before the
	 * set is returned (default is false)
	 * @param popSize size of the population from which the sample will be drawn
	 *
	 * @tparam filterStruct the type of the filter to be applied to the agents
	 */
    template<typename filterStruct>
    void selectAgents(std::set<T*>& selectedAgents, filterStruct& filter, bool remove = false, int popSize = -1);

	/**
	 * Gets a randomly ordered vector of pointers to all agents in this context
	 * matching a user-defined filter.
	 *
	 * If the 'remove' parameter is set to true, any elements in the
	 * original vector will be removed before the method returns.
	 *
	 * The popSize parameter is used when the method is repeatedly called
	 * on a population whose size is known. Calls to this method typically begin
	 * by determining the size of the (valid) population to be sampled; if this is known,
	 * it can be provided here, improving performance.
	 *
	 * @param [out] selectedAgents a vector into which the pointers to the agents will be placed
	 * @param filter user-defined filter specifying any criteria agents to be selected
	 * must meet
	 * @param remove if true, remove any elements originally in the set before the
	 * set is returned (default is false)
	 * @param popSize size of the population from which the sample will be drawn
	 *
	 * @tparam filterStruct the type of the filter to be applied to the agents
	 */
	template<typename filterStruct>
    void selectAgents(std::vector<T*>& selectedAgents, filterStruct& filter, bool remove = false, int popSize = -1);

	/**
	 * Gets a set of pointers to a specified number of randomly selected agents
	 * matching a user-defined filter.
	 *
	 * If the set passed contains any elements when this method is called,
	 * the agents pointed to by those elements will be omitted from the selection.
	 *
	 * If the 'remove' parameter is set to true, any elements in the
	 * original set will be removed before the method returns.
	 *
	 * The popSize parameter is used when the method is repeatedly called
	 * on a population whose size is known. Calls to this method typically begin
	 * by determining the size of the (valid) population to be sampled; if this is known,
	 * it can be provided here, improving performance.
	 *
	 * @param count the number of agents to be selected. If this exceeds the number
	 * that can possibly be selected, all possible agents will be selected
	 * @param [out] selectedAgents a set into which the pointers to the agents will be placed
	 * @param filter user-defined filter specifying any criteria agents to be selected
	 * must meet
	 * @param remove if true, remove any elements originally in the set before the
	 * set is returned (default is false)
	 * @param popSize size of the population from which the sample will be drawn
	 *
	 * @tparam filterStruct the type of the filter to be applied to the agents
	 */
	template<typename filterStruct>
    void selectAgents(int count, std::set<T*>& selectedAgents, filterStruct& filter, bool remove = false, int popSize = -1);

	/**
	 * Gets a randomly ordered vector of pointers to a specified number of randomly selected agents
	 * matching a user-defined filter.
	 *
	 * If the vector passed contains any elements when this method is called,
	 * the agents pointed to by those elements will be omitted from the selection.
	 *
	 * If the 'remove' parameter is set to true, any elements in the
	 * original vector will be removed before the method returns.
	 *
	 * The popSize parameter is used when the method is repeatedly called
	 * on a population whose size is known. Calls to this method typically begin
	 * by determining the size of the (valid) population to be sampled; if this is known,
	 * it can be provided here, improving performance.
	 *
	 * @param count the number of agents to be selected. If this exceeds the number
	 * that can possibly be selected, all possible agents will be selected
	 * @param [out] selectedAgents a vector into which the pointers to the agents will be placed
	 * @param filter user-defined filter specifying any criteria agents to be selected
	 * must meet
	 * @param remove if true, remove any elements originally in the set before the
	 * set is returned (default is false)
	 * @param popSize size of the population from which the sample will be drawn
	 *
	 * @tparam filterStruct the type of the filter to be applied to the agents
	 */
	template<typename filterStruct>
    void selectAgents(int count, std::vector<T*>& selectedAgents, filterStruct& filter, bool remove = false, int popSize = -1);
	
	/**
	 * Gets a set of pointers to all agents in this context
	 * of a specified type (per their AgentId values) and matching a user-defined filter.
	 *
	 * If the 'remove' parameter is set to true, any elements in the
	 * original set will be removed before the method returns.
	 *
	 * The popSize parameter is used when the method is repeatedly called
	 * on a population whose size is known. Calls to this method typically begin
	 * by determining the size of the (valid) population to be sampled; if this is known,
	 * it can be provided here, improving performance.
	 *
	 * @param [out] selectedAgents a set into which the pointers to the agents will be placed
	 * @param type numeric type of agent to be selected
	 * @param filter user-defined filter specifying any criteria agents to be selected
	 * must meet
	 * @param remove if true, remove any elements originally in the set before the
	 * set is returned (default is false)
	 * @param popSize size of the population from which the sample will be drawn
	 *
	 * @tparam filterStruct the type of the filter to be applied to the agents
	 */
    template<typename filterStruct>
    void selectAgents(std::set<T*>& selectedAgents, int type, filterStruct& filter, bool remove = false, int popSize = -1);

	/**
	 * Gets a randomly ordered vector of pointers to all agents in this context
	 * of a specified type (per their AgentId values) and matching a user-defined filter.
	 *
	 * If the 'remove' parameter is set to true, any elements in the
	 * original vector will be removed before the method returns.
	 *
	 * The popSize parameter is used when the method is repeatedly called
	 * on a population whose size is known. Calls to this method typically begin
	 * by determining the size of the (valid) population to be sampled; if this is known,
	 * it can be provided here, improving performance.
	 *
	 * @param [out] selectedAgents a vector into which the pointers to the agents will be placed
	 * @param type numeric type of agent to be selected
	 * @param filter user-defined filter specifying any criteria agents to be selected
	 * must meet
	 * @param remove if true, remove any elements originally in the set before the
	 * set is returned (default is false)
	 * @param popSize size of the population from which the sample will be drawn
	 *
	 * @tparam filterStruct the type of the filter to be applied to the agents
	 */
	template<typename filterStruct>
    void selectAgents(std::vector<T*>& selectedAgents, int type, filterStruct& filter, bool remove = false, int popSize = -1);

	/**
	 * Gets a set of pointers to a specified number of randomly selected agents
	 * of a specified type (per their AgentId values) and matching a user-defined filter.
	 *
	 * If the set passed contains any elements when this method is called,
	 * the agents pointed to by those elements will be omitted from the selection.
	 *
	 * If the 'remove' parameter is set to true, any elements in the
	 * original set will be removed before the method returns.
	 *
	 * The popSize parameter is used when the method is repeatedly called
	 * on a population whose size is known. Calls to this method typically begin
	 * by determining the size of the (valid) population to be sampled; if this is known,
	 * it can be provided here, improving performance.
	 *
	 * @param count the number of agents to be selected. If this exceeds the number
	 * that can possibly be selected, all possible agents will be selected
	 * @param [out] selectedAgents a set into which the pointers to the agents will be placed
	 * @param type numeric type of agent to be selected
	 * @param filter user-defined filter specifying any criteria agents to be selected
	 * must meet
	 * @param remove if true, remove any elements originally in the set before the
	 * set is returned (default is false)
	 * @param popSize size of the population from which the sample will be drawn
	 *
	 * @tparam filterStruct the type of the filter to be applied to the agents
	 */
	template<typename filterStruct>
    void selectAgents(int count, std::set<T*>& selectedAgents, int type, filterStruct& filter, bool remove = false, int popSize = -1);

	/**
	 * Gets a randomly ordered vector of pointers to a specified number of randomly selected agents
	 * of a specified type (per their AgentId values) and matching a user-defined filter.
	 *
	 * If the vector passed contains any elements when this method is called,
	 * the agents pointed to by those elements will be omitted from the selection.
	 *
	 * If the 'remove' parameter is set to true, any elements in the
	 * original vector will be removed before the method returns.
	 *
	 * The popSize parameter is used when the method is repeatedly called
	 * on a population whose size is known. Calls to this method typically begin
	 * by determining the size of the (valid) population to be sampled; if this is known,
	 * it can be provided here, improving performance.
	 *
	 * @param count the number of agents to be selected. If this exceeds the number
	 * that can possibly be selected, all possible agents will be selected
	 * @param [out] selectedAgents a vector into which the pointers to the agents will be placed
	 * @param type numeric type of agent to be selected
	 * @param filter user-defined filter specifying any criteria agents to be selected
	 * must meet
	 * @param remove if true, remove any elements originally in the set before the
	 * set is returned (default is false)
	 * @param popSize size of the population from which the sample will be drawn
	 *
	 * @tparam filterStruct the type of the filter to be applied to the agents
	 */
	template<typename filterStruct>
    void selectAgents(int count, std::vector<T*>& selectedAgents, int type, filterStruct& filter, bool remove = false, int popSize = -1);
	

};





template<typename T>
Context<T>::Context() {
}

template<typename T>
Context<T>::~Context() {
	agents.erase(agents.begin(), agents.end());
	for (ProjPtrIter iter = projections.begin(); iter != projections.end(); ++iter) {
		Projection<T>* proj = *iter;
		delete proj;
	}
	projections.clear();

	for (std::map<std::string, BaseValueLayer*>::iterator iter = valueLayers.begin(); iter != valueLayers.end(); ++iter) {
		BaseValueLayer* layer = iter->second;
		delete layer;
	}
	valueLayers.clear();
}

template<typename T>
void Context<T>::addProjection(Projection<T>* projection) {
	if (find(projections.begin(), projections.end(), projection) != projections.end())
      throw Repast_Error_9(projection->name()); // Projection with specified name already in context

	for (const_iterator iter = begin(); iter != end(); ++iter) {
		projection->addAgent(*iter);
	}
	projections.push_back(projection);
}

template<typename T>
Projection<T>* Context<T>::getProjection(const std::string& name) {
	for (ProjPtrIter iter = projections.begin(); iter != projections.end(); ++iter) {
		Projection<T>* proj = *iter;
		if (proj->name() == name)
			return proj;
	}
	return NULL;
}

template<typename T>
T* Context<T>::getAgent(const AgentId& id) {
	AgentMapIterator iter = agents.find(id);
	if (iter == agents.end())
		return 0;
	return iter->second.get();
}

template<typename T>
void Context<T>::getRandomAgents(const int count, std::vector<T*>& agents) {
	for (int i = 0; i < count; i++) {
		IntUniformGenerator rnd = Random::instance()->createUniIntGenerator(0, size() - 1);
		bool found = false;
		while (!found) {
			const_iterator iter = begin();
			for (int j = 0, n = rnd.next(); j < n; iter++, j++)
				;
			T* agent = iter->get();
			if (find(agents.begin(), agents.end(), agent) == agents.end()) {
				agents.push_back(agent);
				found = true;
			}
		}
	}
}

template<typename T>
bool Context<T>::addAgent(T* agent) {
	const AgentId& id = agent->getId();
	if (agents.find(id) != agents.end())
		return false;

	boost::shared_ptr<T> ptr(agent);
	agents[id] = ptr;

	for (ProjPtrIter iter = projections.begin(); iter != projections.end(); ++iter) {
		Projection<T>* proj = *iter;
		proj->addAgent(ptr);
	}

	return true;
}

template<typename T>
void Context<T>::removeAgent(T* agent) {
	removeAgent(agent->getId());
}

template<typename T>
void Context<T>::removeAgent(const AgentId id) {
	const AgentMapIterator iter = agents.find(id);
	if (iter != agents.end()) {
		boost::shared_ptr<T>& ptr = iter->second;
		for (ProjPtrIter pIter = projections.begin(); pIter != projections.end(); ++pIter) {
			Projection<T>* proj = *pIter;
			proj->removeAgent(ptr.get());
		}
		agents.erase(iter);
	}
}

template<typename T>
bool Context<T>::contains(const AgentId& id) {
	return agents.find(id) != agents.end();
}

template<typename T>
void Context<T>::addValueLayer(BaseValueLayer* layer) {
	valueLayers[layer->name()] = layer;
}

template<typename T>
template<typename ValueType, typename Borders>
DiscreteValueLayer<ValueType, Borders>* Context<T>::getDiscreteValueLayer(const std::string& valueLayerName) {

	std::map<std::string, BaseValueLayer*>::iterator iter = valueLayers.find(valueLayerName);
	if (iter == valueLayers.end())
		return 0;
	return static_cast<DiscreteValueLayer<ValueType, Borders>*> (iter->second);
}

template<typename T>
template<typename ValueType, typename Borders>
ContinuousValueLayer<ValueType, Borders>* Context<T>::getContinuousValueLayer(const std::string& valueLayerName) {

	std::map<std::string, BaseValueLayer*>::iterator iter = valueLayers.find(valueLayerName);
	if (iter == valueLayers.end())
		return 0;
	return static_cast<ContinuousValueLayer<ValueType, Borders>*> (iter->second);
}

	

	
template<typename T>
template<typename filterStruct>
boost::filter_iterator<filterStruct, typename Context<T>::const_iterator> Context<T>::filteredBegin(const filterStruct& fStruct){
	return boost::filter_iterator<filterStruct , typename Context<T>::const_iterator> (fStruct, Context<T>::begin(), Context<T>::end());
}
	
template<typename T>
template<typename filterStruct>
boost::filter_iterator<filterStruct, typename Context<T>::const_iterator> Context<T>::filteredEnd(const filterStruct& fStruct){
	return boost::filter_iterator<filterStruct , typename Context<T>::const_iterator> (fStruct, Context<T>::end(), Context<T>::end());
}
	
	
template<typename T>
template<typename filterStruct>
boost::filter_iterator<filterStruct, typename Context<T>::const_bytype_iterator> Context<T>::byTypeFilteredBegin(const int type, const filterStruct& fStruct){
	return boost::filter_iterator<filterStruct , typename Context<T>::const_bytype_iterator> (fStruct, Context<T>::byTypeBegin(type), Context<T>::byTypeEnd(type));
}
	
template<typename T>
template<typename filterStruct>
boost::filter_iterator<filterStruct, typename Context<T>::const_bytype_iterator> Context<T>::byTypeFilteredEnd(const int type, const filterStruct& fStruct){
	return boost::filter_iterator<filterStruct , typename Context<T>::const_bytype_iterator> (fStruct, Context<T>::byTypeEnd(type), Context<T>::byTypeEnd(type));
}
	
	

	
	
	
	/* ****************************** */
	
// Selection
	
template<typename T>
void Context<T>::selectAgents(std::set<T*>& selectedAgents, bool remove){
	selectNElementsAtRandom(begin(), size(), size(), selectedAgents, remove);
}
	
template<typename T>
void Context<T>::selectAgents(std::vector<T*>& selectedAgents, bool remove){
	selectNElementsInRandomOrder(begin(), size(), size(), selectedAgents, remove);
}
	
template<typename T>
void Context<T>::selectAgents(int count, std::set<T*>& selectedAgents, bool remove){
	selectNElementsAtRandom(begin(), size(), count, selectedAgents, remove);
}
	
template<typename T>
void Context<T>::selectAgents(int count, std::vector<T*>& selectedAgents, bool remove){
	selectNElementsInRandomOrder(begin(), end(), count, selectedAgents, remove);
}
	
template<typename T>
void Context<T>::selectAgents(std::set<T*>& selectedAgents, int type, bool remove, int popSize){
	if(popSize <= -1) selectNElementsAtRandom(byTypeBegin(type), byTypeEnd(type), size(), selectedAgents, remove);
	else              selectNElementsAtRandom(byTypeBegin(type), popSize, size(), selectedAgents, remove);
}
	
template<typename T>
void Context<T>::selectAgents(std::vector<T*>& selectedAgents, int type, bool remove, int popSize){
	if(popSize <= -1) selectNElementsInRandomOrder(byTypeBegin(type), byTypeEnd(type), size(), selectedAgents, remove);
	else              selectNElementsInRandomOrder(byTypeBegin(type), popSize, size(), selectedAgents, remove);
}
	
template<typename T>
void Context<T>::selectAgents(int count, std::set<T*>& selectedAgents, int type, bool remove, int popSize){
	if(popSize <= -1) selectNElementsAtRandom(byTypeBegin(type), byTypeEnd(type), count, selectedAgents, remove);
	else              selectNElementsAtRandom(byTypeBegin(type), popSize, count, selectedAgents, remove);
}
	
template<typename T>
void Context<T>::selectAgents(int count, std::vector<T*>& selectedAgents, int type, bool remove, int popSize){
	if(popSize <= -1) selectNElementsInRandomOrder(byTypeBegin(type), byTypeEnd(type), count, selectedAgents, remove);
	else              selectNElementsInRandomOrder(byTypeBegin(type), popSize, count, selectedAgents, remove);
}
	
template<typename T>
template<typename filterStruct>
void Context<T>::selectAgents(std::set<T*>& selectedAgents, filterStruct& filter, bool remove, int popSize){
	if(popSize <= -1) selectNElementsAtRandom(filteredBegin<filterStruct>(filter), filteredEnd<filterStruct>(filter), size(), selectedAgents, remove);
	else              selectNElementsAtRandom(filteredBegin<filterStruct>(filter), popSize, size(), selectedAgents, remove);
}
	
template<typename T>
template<typename filterStruct>
void Context<T>::selectAgents(std::vector<T*>& selectedAgents, filterStruct& filter, bool remove, int popSize){
	if(popSize <= -1) selectNElementsInRandomOrder(filteredBegin(filter), filteredEnd(filter), size(), selectedAgents, remove);
	else              selectNElementsInRandomOrder(filteredBegin(filter), popSize, size(), selectedAgents, remove);
}
	
template<typename T>
template<typename filterStruct>
void Context<T>::selectAgents(int count, std::set<T*>& selectedAgents, filterStruct& filter, bool remove, int popSize){
	if(popSize <= -1) selectNElementsAtRandom(filteredBegin<filterStruct>(filter), filteredEnd<filterStruct>(filter), count, selectedAgents, remove);
	else              selectNElementsAtRandom(filteredBegin<filterStruct>(filter), popSize, count, selectedAgents, remove);
}
	
template<typename T>
template<typename filterStruct>
void Context<T>::selectAgents(int count, std::vector<T*>& selectedAgents, filterStruct& filter, bool remove, int popSize){
	if(popSize <= -1) selectNElementsInRandomOrder(filteredBegin(filter), filteredEnd(filter), count, selectedAgents, remove);
	else              selectNElementsInRandomOrder(filteredBegin(filter), popSize, count, selectedAgents, remove);
}
	
template<typename T>
template<typename filterStruct>
void Context<T>::selectAgents(std::set<T*>& selectedAgents, int type, filterStruct& filter, bool remove, int popSize){
	if(popSize <= -1) selectNElementsAtRandom(byTypeFilteredBegin(type, filter), byTypeFilteredEnd(type, filter), size(), selectedAgents, remove);
	else              selectNElementsAtRandom(byTypeFilteredBegin(type, filter), popSize, size(), selectedAgents, remove);
}
	
template<typename T>
template<typename filterStruct>
void Context<T>::selectAgents(std::vector<T*>& selectedAgents, int type, filterStruct& filter, bool remove, int popSize){
	if(popSize <= -1) selectNElementsInRandomOrder(byTypeFilteredBegin(type, filter), byTypeFilteredEnd(type, filter), size(), selectedAgents, remove);
	else              selectNElementsInRandomOrder(byTypeFilteredBegin(type, filter), popSize, size(), selectedAgents, remove);
}
	
template<typename T>
template<typename filterStruct>
void Context<T>::selectAgents(int count, std::set<T*>& selectedAgents, int type, filterStruct& filter, bool remove, int popSize){
	if(popSize <= -1) selectNElementsAtRandom(byTypeFilteredBegin(type, filter), byTypeFilteredEnd(type, filter), count, selectedAgents, remove);
	else              selectNElementsAtRandom(byTypeFilteredBegin(type, filter), popSize, count, selectedAgents, remove);
}
	
template<typename T>
template<typename filterStruct>
void Context<T>::selectAgents(int count, std::vector<T*>& selectedAgents, int type, filterStruct& filter, bool remove, int popSize){
	if(popSize <= -1) selectNElementsInRandomOrder(byTypeFilteredBegin(type, filter), byTypeFilteredEnd(type, filter), count, selectedAgents, remove);
	else              selectNElementsInRandomOrder(byTypeFilteredBegin(type, filter), popSize, count, selectedAgents, remove);
}
	
	
	

}
#endif /* CONTEXT_H_ */
