// Copyright (c) 2018 INRIA Sophia-Antipolis (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL$
// $Id$
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Florent Lafarge, Simon Giraudot, Thien Hoang, Dmitry Anisimov
//

#ifndef CGAL_SHAPE_DETECTION_REGION_GROWING_POLYGON_MESH_LEAST_SQUARES_PLANE_FIT_SORTING_H
#define CGAL_SHAPE_DETECTION_REGION_GROWING_POLYGON_MESH_LEAST_SQUARES_PLANE_FIT_SORTING_H

#include <CGAL/license/Shape_detection.h>

// Internal includes.
#include <CGAL/Shape_detection/Region_growing/internal/property_map.h>

namespace CGAL {
namespace Shape_detection {
namespace Polygon_mesh {

  /*!
    \ingroup PkgShapeDetectionRGOnMesh

    \brief Sorting of polygon mesh faces with respect to the local plane fit quality.

    Indices of faces in a polygon mesh are sorted with respect to the quality of the
    least squares plane fit applied to the vertices of incident faces of each face.

    \tparam GeomTraits
    a model of `Kernel`

    \tparam PolygonMesh
    a model of `FaceListGraph`

    \tparam NeighborQuery
    a model of `NeighborQuery`

    \tparam FaceRange
    a model of `ConstRange` whose iterator type is `RandomAccessIterator` and
    value type is the face type of a polygon mesh

    \tparam VertexToPointMap
    a model of `ReadablePropertyMap` whose key type is the vertex type of a polygon mesh and
    value type is `Kernel::Point_3`
  */
  template<
  typename GeomTraits,
  typename PolygonMesh,
  typename NeighborQuery,
  typename FaceRange = typename PolygonMesh::Face_range,
  typename VertexToPointMap = typename property_map_selector<PolygonMesh, CGAL::vertex_point_t>::const_type>
  class Least_squares_plane_fit_sorting {

  public:
    /// \name Types
    /// @{

    /// \cond SKIP_IN_MANUAL
    using Traits = GeomTraits;
    using Face_graph = PolygonMesh;
    using Neighbor_query = NeighborQuery;
    using Face_range = FaceRange;
    using Vertex_to_point_map = VertexToPointMap;
    using Seed_map = internal::Seed_property_map;
    /// \endcond

    #ifdef DOXYGEN_NS
      /*!
        a model of `ReadablePropertyMap` whose key and value type is `std::size_t`.
        This map provides an access to the ordered indices of input faces.
      */
      typedef unspecified_type Seed_map;
    #endif

    /// @}

  private:
    using FT = typename Traits::FT;
    using Compare_scores = internal::Compare_scores<FT>;

  public:
    /// \name Initialization
    /// @{

    /*!
      \brief initializes all internal data structures.

      \tparam NamedParameters
      a sequence of \ref bgl_namedparameters "Named Parameters"

      \param pmesh
      an instance of `PolygonMesh` that represents a polygon mesh

      \param neighbor_query
      an instance of `NeighborQuery` that is used internally to
      access face's neighbors

      \param np
      a sequence of \ref bgl_namedparameters "Named Parameters"
      among the ones listed below

      \cgalNamedParamsBegin
        \cgalParamNBegin{vertex_point_map}
          \cgalParamDescription{an instance of `VertexToPointMap` that maps a polygon mesh
          vertex to `Kernel::Point_3`}
          \cgalParamDefault{`boost::get(CGAL::vertex_point, pmesh)`}
        \cgalParamNEnd
        \cgalParamNBegin{geom_traits}
          \cgalParamDescription{an instance of `GeomTraits`}
          \cgalParamDefault{`GeomTraits()`}
        \cgalParamNEnd
      \cgalNamedParamsEnd

      \pre `faces(pmesh).size() > 0`
    */
    template<typename NamedParameters>
    Least_squares_plane_fit_sorting(
      const PolygonMesh& pmesh,
      NeighborQuery& neighbor_query,
      const NamedParameters& np) :
    m_face_graph(pmesh),
    m_neighbor_query(neighbor_query),
    m_face_range(faces(m_face_graph)),
    m_vertex_to_point_map(parameters::choose_parameter(parameters::get_parameter(
      np, internal_np::vertex_point), get_const_property_map(CGAL::vertex_point, pmesh))),
    m_traits(parameters::choose_parameter(parameters::get_parameter(
      np, internal_np::geom_traits), GeomTraits())) {

      CGAL_precondition(m_face_range.size() > 0);
      m_order.resize(m_face_range.size());
      std::iota(m_order.begin(), m_order.end(), 0);
      m_scores.resize(m_face_range.size());
    }

    #if !defined(CGAL_NO_DEPRECATED_CODE) || defined(DOXYGEN_RUNNING)

    /*!
      \brief initializes all internal data structures.

      \deprecated This constructor is deprecated since the version 5.4 of \cgal.

      \param pmesh
      an instance of `PolygonMesh` that represents a polygon mesh

      \param neighbor_query
      an instance of `NeighborQuery` that is used internally to
      access face's neighbors

      \param vertex_to_point_map
      an instance of `VertexToPointMap` that maps a polygon mesh
      vertex to `Kernel::Point_3`

      \pre `faces(pmesh).size() > 0`
    */
    CGAL_DEPRECATED_MSG("This constructor is deprecated since the version 5.4 of CGAL!")
    Least_squares_plane_fit_sorting(
      const PolygonMesh& pmesh,
      NeighborQuery& neighbor_query,
      const VertexToPointMap vertex_to_point_map = VertexToPointMap()) :
    Least_squares_plane_fit_sorting(
      pmesh, neighbor_query, CGAL::parameters::
    vertex_point_map(vertex_to_point_map))
    { }

    #endif // CGAL_NO_DEPRECATED_CODE

    /// \cond SKIP_IN_MANUAL
    // TODO: Should be off until the deprecated code is removed.
    // Least_squares_plane_fit_sorting(
    //   const PolygonMesh& pmesh,
    //   NeighborQuery& neighbor_query) :
    // Least_squares_plane_fit_sorting(
    //   pmesh, neighbor_query, CGAL::parameters::all_default())
    // { }
    /// \endcond

    /// @}

    /// \name Sorting
    /// @{

    /*!
      \brief sorts indices of input faces.
    */
    void sort() {

      compute_scores();
      CGAL_precondition(m_scores.size() > 0);
      Compare_scores cmp(m_scores);
      std::sort(m_order.begin(), m_order.end(), cmp);
    }

    /// @}

    /// \name Access
    /// @{

    /*!
      \brief returns an instance of `Seed_map` to access the ordered indices
      of input faces.
    */
    Seed_map seed_map() {
      return Seed_map(m_order);
    }

    /// @}

  private:
    const Face_graph& m_face_graph;
    Neighbor_query& m_neighbor_query;
    const Face_range m_face_range;
    const Vertex_to_point_map m_vertex_to_point_map;
    const Traits m_traits;
    std::vector<std::size_t> m_order;
    std::vector<FT> m_scores;

    void compute_scores() {

      std::vector<std::size_t> neighbors;
      for (std::size_t i = 0; i < m_face_range.size(); ++i) {
        neighbors.clear();
        m_neighbor_query(i, neighbors);
        neighbors.push_back(i);
        m_scores[i] = internal::create_plane_from_faces(
          m_face_graph, m_face_range, m_vertex_to_point_map, neighbors, m_traits).second;
      }
    }
  };

} // namespace Polygon_mesh
} // namespace Shape_detection
} // namespace CGAL

#endif // CGAL_SHAPE_DETECTION_REGION_GROWING_POLYGON_MESH_LEAST_SQUARES_PLANE_FIT_SORTING_H
