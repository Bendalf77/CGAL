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

#ifndef CGAL_SHAPE_DETECTION_REGION_GROWING_POLYGON_MESH_LEAST_SQUARES_PLANE_FIT_REGION_H
#define CGAL_SHAPE_DETECTION_REGION_GROWING_POLYGON_MESH_LEAST_SQUARES_PLANE_FIT_REGION_H

#include <CGAL/license/Shape_detection.h>

// Internal includes.
#include <CGAL/Shape_detection/Region_growing/internal/property_map.h>

namespace CGAL {
namespace Shape_detection {
namespace Polygon_mesh {

  /*!
    \ingroup PkgShapeDetectionRGOnMesh

    \brief Region type based on the quality of the least squares plane
    fit applied to faces of a polygon mesh.

    This class fits a plane, using \ref PkgPrincipalComponentAnalysisDRef "PCA",
    to chunks of faces in a polygon mesh and controls the quality of this fit.
    If all quality conditions are satisfied, the chunk is accepted as a valid region,
    otherwise rejected.

    \tparam GeomTraits
    a model of `Kernel`

    \tparam PolygonMesh
    a model of `FaceListGraph`

    \tparam FaceRange
    a model of `ConstRange` whose iterator type is `RandomAccessIterator` and
    value type is the face type of a polygon mesh

    \tparam VertexToPointMap
    a model of `ReadablePropertyMap` whose key type is the vertex type of a polygon mesh and
    value type is `Kernel::Point_3`

    \cgalModels `RegionType`
  */
  template<
  typename GeomTraits,
  typename PolygonMesh,
  typename FaceRange = typename PolygonMesh::Face_range,
  typename VertexToPointMap = typename property_map_selector<PolygonMesh, CGAL::vertex_point_t>::const_type>
  class Least_squares_plane_fit_region {

  public:
    /// \name Types
    /// @{

    /// \cond SKIP_IN_MANUAL
    using Traits = GeomTraits;
    using Face_graph = PolygonMesh;
    using Face_range = FaceRange;
    using Vertex_to_point_map = VertexToPointMap;

    using Face_to_index_map = internal::Item_to_index_property_map<Face_range>;
    using Face_to_region_index_map = internal::Item_to_region_index_map<Face_to_index_map>;

    struct Face_to_region_map : public Face_to_region_index_map {
      template<typename Regions>
      Face_to_region_map(const Face_range& face_range, const Regions& regions) :
      Face_to_region_index_map(face_range, Face_to_index_map(face_range), regions)
      { }
    };
    /// \endcond

    /// Number type.
    typedef typename GeomTraits::FT FT;

    #ifdef DOXYGEN_NS
      /*!
        a model of `ReadablePropertyMap` whose key type is `face_descriptor`
        and value type is `std::size_t`. This map associates each face of the input mesh
        to the index of the planar region it belongs to.
      */
      typedef unspecified_type Face_to_region_map;
    #endif

    /// @}

  private:
    using Point_3 = typename Traits::Point_3;
    using Vector_3 = typename Traits::Vector_3;
    using Plane_3 = typename Traits::Plane_3;

    using Squared_length_3 = typename Traits::Compute_squared_length_3;
    using Squared_distance_3 = typename Traits::Compute_squared_distance_3;
    using Scalar_product_3 = typename Traits::Compute_scalar_product_3;
    using Cross_product_3 = typename Traits::Construct_cross_product_vector_3;

  public:
    /// \name Initialization
    /// @{

    /*!
      \brief initializes all internal data structures.

      \tparam NamedParameters
      a sequence of \ref bgl_namedparameters "Named Parameters"

      \param pmesh
      an instance of `PolygonMesh` that represents a polygon mesh

      \param np
      a sequence of \ref bgl_namedparameters "Named Parameters"
      among the ones listed below

      \cgalNamedParamsBegin
        \cgalParamNBegin{maximum_distance}
          \cgalParamDescription{the maximum distance from the furthest vertex of a face to a plane}
          \cgalParamType{`GeomTraits::FT`}
          \cgalParamDefault{1}
        \cgalParamNEnd
        \cgalParamNBegin{maximum_angle}
          \cgalParamDescription{the maximum angle in degrees between
          the normal of a face and the normal of a plane}
          \cgalParamType{`GeomTraits::FT`}
          \cgalParamDefault{25 degrees}
        \cgalParamNEnd
        \cgalParamNBegin{cosine_value}
          \cgalParamDescription{the cos value computed as `cos(maximum_angle * PI / 180)`,
          this parameter can be used instead of the `maximum_angle`}
          \cgalParamType{`GeomTraits::FT`}
          \cgalParamDefault{`cos(25 * PI / 180)`}
        \cgalParamNEnd
        \cgalParamNBegin{minimum_region_size}
          \cgalParamDescription{the minimum number of faces a region must have}
          \cgalParamType{`std::size_t`}
          \cgalParamDefault{1}
        \cgalParamNEnd
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
      \pre `maximum_distance >= 0`
      \pre `maximum_angle >= 0 && maximum_angle <= 90`
      \pre `cosine_value >= 0 && cosine_value <= 1`
      \pre `minimum_region_size > 0`
    */
    template<typename NamedParameters>
    Least_squares_plane_fit_region(
      const PolygonMesh& pmesh,
      const NamedParameters& np) :
    m_face_graph(pmesh),
    m_face_range(faces(m_face_graph)),
    m_vertex_to_point_map(parameters::choose_parameter(parameters::get_parameter(
      np, internal_np::vertex_point), get_const_property_map(CGAL::vertex_point, pmesh))),
    m_traits(parameters::choose_parameter(parameters::get_parameter(
      np, internal_np::geom_traits), GeomTraits())),
    m_squared_length_3(m_traits.compute_squared_length_3_object()),
    m_squared_distance_3(m_traits.compute_squared_distance_3_object()),
    m_scalar_product_3(m_traits.compute_scalar_product_3_object()),
    m_cross_product_3(m_traits.construct_cross_product_vector_3_object()) {

      CGAL_precondition(m_face_range.size() > 0);
      const FT max_distance = parameters::choose_parameter(
        parameters::get_parameter(np, internal_np::maximum_distance), FT(1));
      CGAL_precondition(max_distance >= FT(0));
      m_distance_threshold = max_distance;

      const FT max_angle = parameters::choose_parameter(
        parameters::get_parameter(np, internal_np::maximum_angle), FT(25));
      CGAL_precondition(max_angle >= FT(0) && max_angle <= FT(90));

      m_min_region_size = parameters::choose_parameter(
        parameters::get_parameter(np, internal_np::minimum_region_size), 1);
      CGAL_precondition(m_min_region_size > 0);

      const FT default_cos_value = static_cast<FT>(std::cos(CGAL::to_double(
        (max_angle * static_cast<FT>(CGAL_PI)) / FT(180))));
      const FT cos_value = parameters::choose_parameter(
        parameters::get_parameter(np, internal_np::cosine_value), default_cos_value);
      CGAL_precondition(cos_value >= FT(0) && cos_value <= FT(1));
      m_cos_value_threshold = cos_value;
    }

    #if !defined(CGAL_NO_DEPRECATED_CODE) || defined(DOXYGEN_RUNNING)

    /*!
      \brief initializes all internal data structures.

      \deprecated This constructor is deprecated since the version 5.4 of \cgal.

      \param pmesh
      an instance of `PolygonMesh` that represents a polygon mesh

      \param distance_threshold
      the maximum distance from the furthest vertex of a face to a plane. %Default is 1.

      \param angle_threshold
      the maximum accepted angle in degrees between the normal of a face and
      the normal of a plane. %Default is 25 degrees.

      \param min_region_size
      the minimum number of faces a region must have. %Default is 1.

      \param vertex_to_point_map
      an instance of `VertexToPointMap` that maps a polygon mesh
      vertex to `Kernel::Point_3`

      \param traits
      an instance of `GeomTraits`

      \pre `faces(pmesh).size() > 0`
      \pre `distance_threshold >= 0`
      \pre `angle_threshold >= 0 && angle_threshold <= 90`
      \pre `min_region_size > 0`
    */
    CGAL_DEPRECATED_MSG("This constructor is deprecated since the version 5.4 of CGAL!")
    Least_squares_plane_fit_region(
      const PolygonMesh& pmesh,
      const FT distance_threshold = FT(1),
      const FT angle_threshold = FT(25),
      const std::size_t min_region_size = 1,
      const VertexToPointMap vertex_to_point_map = VertexToPointMap(),
      const GeomTraits traits = GeomTraits()) :
    Least_squares_plane_fit_region(
      pmesh, CGAL::parameters::
    maximum_distance(distance_threshold).
    maximum_angle(angle_threshold).
    minimum_region_size(min_region_size).
    vertex_point_map(vertex_to_point_map).
    geom_traits(traits))
    { }

    #endif // CGAL_NO_DEPRECATED_CODE

    /// \cond SKIP_IN_MANUAL
    // TODO: Should be off until the deprecated code is removed.
    // Least_squares_plane_fit_region(
    //   const PolygonMesh& pmesh) :
    // Least_squares_plane_fit_region(
    //   pmesh, CGAL::parameters::all_default())
    // { }
    /// \endcond

    /// @}

    /// \name Access
    /// @{

    /*!
      \brief implements `RegionType::is_part_of_region()`.

      This function controls if a face with the index `query_index` is within
      the `maximum_distance` from the corresponding plane and if the angle between
      its normal and the plane's normal is within the `maximum_angle`. If both conditions
      are satisfied, it returns `true`, otherwise `false`.

      \param query_index
      index of the query face

      The first and third parameters are not used in this implementation.

      \return Boolean `true` or `false`

      \pre `query_index < faces(pmesh).size()`
    */
    bool is_part_of_region(
      const std::size_t,
      const std::size_t query_index,
      const std::vector<std::size_t>&) const {

      CGAL_precondition(query_index < m_face_range.size());
      const auto face = *(m_face_range.begin() + query_index);

      const FT squared_distance_to_fitted_plane = get_max_squared_distance(face);
      if (squared_distance_to_fitted_plane < FT(0)) return false;
      const FT squared_distance_threshold =
        m_distance_threshold * m_distance_threshold;

      const Vector_3 face_normal = get_face_normal(face);
      const FT cos_value = m_scalar_product_3(face_normal, m_normal_of_best_fit);
      const FT squared_cos_value = cos_value * cos_value;

      FT squared_cos_value_threshold =
        m_cos_value_threshold * m_cos_value_threshold;
      squared_cos_value_threshold *= m_squared_length_3(face_normal);
      squared_cos_value_threshold *= m_squared_length_3(m_normal_of_best_fit);

      return (
        ( squared_distance_to_fitted_plane <= squared_distance_threshold ) &&
        ( squared_cos_value >= squared_cos_value_threshold ));
    }

    /*!
      \brief implements `RegionType::is_valid_region()`.

      This function controls if the `region` contains at least `minimum_region_size` faces.

      \param region
      indices of faces included in the region

      \return Boolean `true` or `false`
    */
    inline bool is_valid_region(const std::vector<std::size_t>& region) const {
      return (region.size() >= m_min_region_size);
    }

    /*!
      \brief implements `RegionType::update()`.

      This function fits the least squares plane to all vertices of the faces
      from the `region`.

      \param region
      indices of faces included in the region

      \return Boolean `true` if the plane fitting succeeded and `false` otherwise

      \pre `region.size() > 0`
    */
    bool update(const std::vector<std::size_t>& region) {

      CGAL_precondition(region.size() > 0);
      if (region.size() == 1) { // create new reference plane and normal
        const std::size_t face_index = region[0];
        CGAL_precondition(face_index < m_face_range.size());

        // The best fit plane will be a plane through this face centroid with
        // its normal being the face's normal.
        const auto face = *(m_face_range.begin() + face_index);
        const Point_3 face_centroid = get_face_centroid(face);
        const Vector_3 face_normal = get_face_normal(face);
        if (face_normal == CGAL::NULL_VECTOR) return false;

        CGAL_precondition(face_normal != CGAL::NULL_VECTOR);
        m_plane_of_best_fit = Plane_3(face_centroid, face_normal);
        m_normal_of_best_fit = m_plane_of_best_fit.orthogonal_vector();

      } else { // update reference plane and normal
        CGAL_precondition(region.size() >= 2);
        std::tie(m_plane_of_best_fit, m_normal_of_best_fit) =
          get_plane_and_normal(region);
      }
      return true;
    }

    /// @}

    /// \cond SKIP_IN_MANUAL
    std::pair<Plane_3, Vector_3> get_plane_and_normal(
      const std::vector<std::size_t>& region) const {

      // The best fit plane will be a plane fitted to all vertices of all
      // region faces with its normal being perpendicular to the plane.
      // Given that the points, and no normals, are used in estimating
      // the plane, the estimated normal will point into an arbitray
      // one of the two possible directions.
      // We flip it into the correct direction (the one that the majority
      // of faces agree with) below.
      // This fix is proposed by nh2:
      // https://github.com/CGAL/cgal/pull/4563
      const Plane_3 unoriented_plane_of_best_fit =
        internal::create_plane_from_faces(
          m_face_graph, m_face_range, m_vertex_to_point_map, region, m_traits).first;
      const Vector_3 unoriented_normal_of_best_fit =
        unoriented_plane_of_best_fit.orthogonal_vector();

      // Compute actual direction of plane's normal sign
      // based on faces, which belong to that region.
      // Approach: each face gets one vote to keep or flip the current plane normal.
      long votes_to_keep_normal = 0;
      for (const std::size_t face_index : region) {
        const auto face = *(m_face_range.begin() + face_index);
        const Vector_3 face_normal = get_face_normal(face);
        const bool agrees =
          m_scalar_product_3(face_normal, unoriented_normal_of_best_fit) > FT(0);
        votes_to_keep_normal += (agrees ? 1 : -1);
      }
      const bool flip_normal = (votes_to_keep_normal < 0);

      const Plane_3 plane_of_best_fit = flip_normal
        ? unoriented_plane_of_best_fit.opposite()
        : unoriented_plane_of_best_fit;
      const Vector_3 normal_of_best_fit = flip_normal
        ? (-1 * unoriented_normal_of_best_fit)
        : unoriented_normal_of_best_fit;

      return std::make_pair(plane_of_best_fit, normal_of_best_fit);
    }
    /// \endcond

  private:
    const Face_graph& m_face_graph;
    const Face_range m_face_range;
    const Vertex_to_point_map m_vertex_to_point_map;
    const Traits m_traits;

    FT m_distance_threshold;
    FT m_cos_value_threshold;
    std::size_t m_min_region_size;

    const Squared_length_3 m_squared_length_3;
    const Squared_distance_3 m_squared_distance_3;
    const Scalar_product_3 m_scalar_product_3;
    const Cross_product_3 m_cross_product_3;

    Plane_3 m_plane_of_best_fit;
    Vector_3 m_normal_of_best_fit;

    // Compute centroid of the face.
    template<typename Face>
    Point_3 get_face_centroid(const Face& face) const {

      const auto hedge = halfedge(face, m_face_graph);
      const auto vertices = vertices_around_face(hedge, m_face_graph);
      CGAL_precondition(vertices.size() > 0);

      FT sum = FT(0), x = FT(0), y = FT(0), z = FT(0);
      for (const auto vertex : vertices) {
        const Point_3& point = get(m_vertex_to_point_map, vertex);
        x += point.x();
        y += point.y();
        z += point.z();
        sum += FT(1);
      }
      CGAL_precondition(sum > FT(0));
      x /= sum;
      y /= sum;
      z /= sum;
      return Point_3(x, y, z);
    }

    // Compute normal of the face.
    template<typename Face>
    Vector_3 get_face_normal(const Face& face) const {

      const auto hedge = halfedge(face, m_face_graph);
      const auto vertices = vertices_around_face(hedge, m_face_graph);
      CGAL_precondition(vertices.size() >= 3);

      auto vertex = vertices.begin();
      const Point_3& point1 = get(m_vertex_to_point_map, *vertex); ++vertex;
      const Point_3& point2 = get(m_vertex_to_point_map, *vertex); ++vertex;
      const Point_3& point3 = get(m_vertex_to_point_map, *vertex);

      const Vector_3 u = point2 - point1;
      const Vector_3 v = point3 - point1;
      const Vector_3 face_normal = m_cross_product_3(u, v);
      return face_normal;
    }

    // The maximum squared distance from the vertices of the face
    // to the best fit plane.
    template<typename Face>
    FT get_max_squared_distance(const Face& face) const {

      FT max_squared_distance = -FT(1);
      const FT a = CGAL::abs(m_plane_of_best_fit.a());
      const FT b = CGAL::abs(m_plane_of_best_fit.b());
      const FT c = CGAL::abs(m_plane_of_best_fit.c());
      const FT d = CGAL::abs(m_plane_of_best_fit.d());
      if (a == FT(0) && b == FT(0) && c == FT(0) && d == FT(0))
        return max_squared_distance;

      const auto hedge = halfedge(face, m_face_graph);
      const auto vertices = vertices_around_face(hedge, m_face_graph);
      CGAL_precondition(vertices.size() > 0);

      for (const auto vertex : vertices) {
        const Point_3& point = get(m_vertex_to_point_map, vertex);
        const FT squared_distance = m_squared_distance_3(point, m_plane_of_best_fit);
        max_squared_distance = (CGAL::max)(squared_distance, max_squared_distance);
      }
      CGAL_precondition(max_squared_distance >= FT(0));
      return max_squared_distance;
    }
  };

} // namespace Polygon_mesh
} // namespace Shape_detection
} // namespace CGAL

#endif // CGAL_SHAPE_DETECTION_REGION_GROWING_POLYGON_MESH_LEAST_SQUARES_PLANE_FIT_REGION_H
