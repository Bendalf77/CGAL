// Copyright (c) 2013-2015  The University of Western Sydney, Australia.
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
// You can redistribute it and/or modify it under the terms of the GNU
// General Public License as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// Licensees holding a valid commercial license may use this file in
// accordance with the commercial license agreement provided with the software.
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// $URL$
// $Id$
//
//
// Authors: Weisheng Si, Quincy Tse

/*! \file Construct_yao_graph_2.h
 *
 * This header implements the functor for constructing Yao graphs.
 */

#ifndef CGAL_CONSTRUCT_YAO_GRAPH_2_H
#define CGAL_CONSTRUCT_YAO_GRAPH_2_H

#include <iostream>
#include <cstdlib>
#include <utility>
#include <CGAL/Compute_cone_boundaries_2.h>
#include <CGAL/Cone_spanners_2/Less_by_direction_2.h>

#include <boost/config.hpp>
#include <boost/graph/adjacency_list.hpp>

namespace CGAL {

/*! \ingroup PkgConeBasedSpanners

  \brief A template functor for constructing Yao graphs with a given set of 2D points and
         a given initial direction for the cone boundaries.

  For the meaning and use of its template parameters, please refer to the concept
    `ConstructConeBasedSpanner_2`.

  \cgalModels `ConstructConeBasedSpanner_2`
 */
template <typename Traits, typename Graph_>
class Construct_yao_graph_2 {

public:
	/*! Indicate the type of the \cgal kernel. */
    typedef Traits      Kernel_type;
	/*! Indicate the specific type of `boost::adjacency_list`. */
    typedef Graph_                        Graph_type;

private:
    typedef typename Traits::Direction_2           Direction_2;
    typedef typename Traits::Point_2               Point_2;
    typedef typename Traits::Line_2                Line_2;
    typedef Less_by_direction_2<Traits, Graph_>    Less_by_direction;
    // a type for the set to store vertices sorted by a direction
    typedef std::set<typename Graph_::vertex_descriptor, Less_by_direction> Point_set;

    /* Store the number of cones.  */
    unsigned int  cone_number;

    /* Store the directions of the rays dividing the plane. The initial direction will be
       stored in rays[0].  */
    std::vector<Direction_2>   rays;

public:
    /*! 
	  \brief    Constructor.

      \param k     Number of cones to divide space into
      \param initial_direction  A direction denoting one of the rays dividing the
                   cones. This allows arbitary rotations of the rays that divide
                   the plane.  (default: positive x-axis)
     */
    Construct_yao_graph_2 (unsigned int k,
                           Direction_2 initial_direction = Direction_2(1,0) 
						  ): cone_number(k), rays(std::vector<Direction_2>(k))

    {
        if (k<2) {
            std::cout << "The number of cones must be larger than 1!" << std::endl;
            std::exit(1);
        }

        /* Initialize a functor, specialization will happen here depending on the kernel type to
         compute the cone boundaries either exactly or inexactly */
        Compute_cone_boundaries_2<Traits> compute_cones;
        // compute the rays using the functor
        compute_cones(k, initial_direction, rays.begin());
    }

    /* \brief Copy constructor. Not needed.
       \param x  another Construct_yao_graph_2 object to copy from.

    Construct_yao_graph_2 (const Construct_yao_graph_2& x) : cone_number(x.cone_number), rays(x.rays) {}
    */

    /*! 
	  \brief Function operator to construct a Yao graph.

      \details This operator implements the algorithm for constructing the Yao graph.
         The algorithm implemented is an adaptation from the algorithm for constructing Theta graph. 
		 For more details, please refer to the user manual.
     
      \param[in] start An iterator pointing to the first vertex of the input.
      \param[in]  end  An iterator pointing to the past-the-end location of the input.
      \param[out]  g   The constructed graph object.
     */
    template <typename PointInputIterator>
    Graph_& operator()(const PointInputIterator& start,
                       const PointInputIterator& end,
                       Graph_& g) {

        // add vertices into the graph
        for (PointInputIterator curr = start; curr != end; ++curr) {
            g[boost::add_vertex(g)] = *curr;
        }

        unsigned int i;   // ray index of the cw ray
        unsigned int j;   // index of the ccw ray

        // add edges into the graph for every cone
        for (i = 0; i < cone_number; i++) {
            j = (i+1) % cone_number;
            add_edges_in_cone(rays[i], rays[j], g);
        }

        return g;
    }

    /*! \brief returns the number of cones.
     */
    const unsigned int number_of_cones() const {
        return cone_number;
    }

    /*! \brief outputs the directions in the vector rays to the iterator `result`.
	       
        \return `result`
    */
    template<class DirectionOutputIterator>
    DirectionOutputIterator directions(DirectionOutputIterator result) {
    typename std::vector<Direction_2>::iterator it;
    for (it=rays.begin(); it!=rays.end(); it++) {
            *result++ = *it;
       }
       return result;
    }

protected:

    /* Construct edges in one cone bounded by two directions.

     \param cwBound      The direction of the clockwise boundary of the cone.
     \param ccwBound     The direction of the counter-clockwise boundary.
     \param g            The Yao graph to be built.
    */
    void add_edges_in_cone(const Direction_2& cwBound, const Direction_2& ccwBound, Graph_& g) {
        if (ccwBound == cwBound) {
            // Degenerate case,  not allowed.
            throw std::out_of_range("The cw boundary and the ccw boundary shouldn't be same!");
        }

        // Ordering
        // here D1 is the reverse of D1 in the book, we find this is easier to implement
        const Less_by_direction  orderD1 (g, ccwBound);
        const Less_by_direction  orderD2 (g, cwBound);

        typename Graph_::vertex_iterator vit, ve;
        boost::tie(vit, ve) = boost::vertices(g);

        // Step 1: Sort S according to order induced by D1
        std::vector<typename Graph_::vertex_descriptor> S(vit, ve);
        std::sort(S.begin (), S.end (), orderD1);

        // Step 2: Initialise an empty set to store vertices sorted by orderD2
        Point_set pst(orderD2);

        // Step 3: visit S in orderD1
        //         insert 'it' into pst
        //         search the min in pst
        for (typename std::vector<typename Graph_::vertex_descriptor>::const_iterator
                it = S.begin(); it != S.end(); ++it) {
            Less_euclidean_distance comp(g[*it], g);

            pst.insert(*it);
            // Find the last added node - O(logn)
            typename Point_set::iterator it2 = pst.find(*it);
            // Find minimum in pst from last ended node - O(n)
            typename Point_set::iterator min = std::min_element(++it2, pst.end(), comp);
			// add an edge
            if (min != pst.end()) {
                typename Graph_::edge_descriptor existing_e;
			    bool                    existing;
				// check whether the edge already exists
				boost::tie(existing_e, existing)=boost::edge(*it, *min, g);
				if (!existing)
                    boost::add_edge(*it, *min, g);
                //else
				 //   std::cout << "Edge " << *it << ", " << *min << " already exists!" << std::endl;
			}

        } // end of for

    };     // end of add edges in cone


    /* Functor for comparing Euclidean distances of two vertices in a graph g to a given vertex.
      It is implemented by encapsulating the CGAL::has_smaller_distance_to_point() function.
     */
    struct Less_euclidean_distance {
        const Point_2& p;
        const Graph_& g;

		// constructor
        Less_euclidean_distance(const Point_2&p, const Graph_& g) : p(p), g(g) {}

		// operator
        bool operator() (const typename Point_set::iterator::value_type& i, const typename Point_set::iterator::value_type& j) {
            const Point_2& p1 = g[i];
            const Point_2& p2 = g[j];
            return has_smaller_distance_to_point(p, p1, p2);
        }
    };

};      // class Construct_yao_graph_2


}  // namespace CGAL


#endif
