#ifndef CGAL_SHAPE_DETECTION_EXAMPLES_UTILS_H
#define CGAL_SHAPE_DETECTION_EXAMPLES_UTILS_H

// STL includes.
#include <string>
#include <vector>
#include <utility>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <cassert>

// CGAL includes.
#include <CGAL/array.h>
#include <CGAL/memory.h>
#include <CGAL/IO/PLY.h>
#include <CGAL/IO/OFF.h>
#include <CGAL/Real_timer.h>
#include <CGAL/property_map.h>
#include <CGAL/IO/write_ply_points.h>
#include <CGAL/IO/Color.h>
#include <CGAL/Random.h>

// Define how a color should be stored.
namespace CGAL {
  template<class F>
  struct Output_rep< ::std::array<unsigned char, 3>, F > {
    const ::std::array<unsigned char, 3>& c;
    static const bool is_specialized = true;
    Output_rep(const ::std::array<unsigned char, 3>& c) : c(c) { }
    std::ostream& operator()(std::ostream& out) const {
      if (is_ascii(out)) { out << int(c[0]) << " " << int(c[1]) << " " << int(c[2]);
      } else { out.write(reinterpret_cast<const char*>(&c), sizeof(c)); }
      return out;
    }
  };
} // namespace CGAL

namespace utils {

template<typename Kernel, typename Input_range, typename Point_map>
void save_point_regions_2(
  const Input_range& input_range,
  const std::vector< std::vector<std::size_t> >& regions,
  const std::string fullpath,
  const Point_map point_map = Point_map()) {

  using Point_3          = typename Kernel::Point_3;
  using Color            = std::array<unsigned char, 3>;
  using Point_with_color = std::pair<Point_3, Color>;
  using PLY_Point_map    = CGAL::First_of_pair_property_map<Point_with_color>;
  using PLY_Color_map    = CGAL::Second_of_pair_property_map<Point_with_color>;

  std::vector<Point_with_color> pwc;
  srand(static_cast<unsigned int>(time(NULL)));

  // Iterate through all regions.
  for (const auto& region : regions) {

    // Generate a random color.
    const Color color =
      CGAL::make_array(
        static_cast<unsigned char>(rand() % 256),
        static_cast<unsigned char>(rand() % 256),
        static_cast<unsigned char>(rand() % 256));

    // Iterate through all region items.
    for (const auto index : region) {
      const auto& key = *(input_range.begin() + index);
      const auto& point = get(point_map, key);
      pwc.push_back(std::make_pair(Point_3(point.x(), point.y(), 0), color));
    }
  }

  std::ofstream out(fullpath);
  CGAL::IO::set_ascii_mode(out);
  CGAL::IO::write_PLY_with_properties(
    out, pwc,
    CGAL::make_ply_point_writer(PLY_Point_map()),
      std::make_tuple(
        PLY_Color_map(),
        CGAL::PLY_property<unsigned char>("red"),
        CGAL::PLY_property<unsigned char>("green"),
        CGAL::PLY_property<unsigned char>("blue")));
  out.close();
}

template<typename Kernel, typename Input_range, typename Point_map>
void save_point_regions_3(
  const Input_range& input_range,
  const std::vector< std::vector<std::size_t> >& regions,
  const std::string fullpath,
  const Point_map point_map = Point_map()) {

  using Point_3          = typename Kernel::Point_3;
  using Color            = std::array<unsigned char, 3>;
  using Point_with_color = std::pair<Point_3, Color>;
  using PLY_Point_map    = CGAL::First_of_pair_property_map<Point_with_color>;
  using PLY_Color_map    = CGAL::Second_of_pair_property_map<Point_with_color>;

  std::vector<Point_with_color> pwc;
  srand(static_cast<unsigned int>(time(NULL)));

  // Iterate through all regions.
  for (const auto& region : regions) {

    // Generate a random color.
    const Color color =
      CGAL::make_array(
        static_cast<unsigned char>(rand() % 256),
        static_cast<unsigned char>(rand() % 256),
        static_cast<unsigned char>(rand() % 256));

    // Iterate through all region items.
    for (const auto index : region) {
      const auto& key = *(input_range.begin() + index);
      const auto& point = get(point_map, key);
      pwc.push_back(std::make_pair(point, color));
    }
  }

  std::ofstream out(fullpath);
  CGAL::IO::set_ascii_mode(out);
  CGAL::IO::write_PLY_with_properties(
    out, pwc,
    CGAL::make_ply_point_writer(PLY_Point_map()),
      std::make_tuple(
        PLY_Color_map(),
        CGAL::PLY_property<unsigned char>("red"),
        CGAL::PLY_property<unsigned char>("green"),
        CGAL::PLY_property<unsigned char>("blue")));
  out.close();
}

template<typename Kernel, typename Input_range, typename Segment_map>
void save_segment_regions_2(
  const Input_range& input_range,
  const std::vector< std::vector<std::size_t> >& regions,
  const std::string fullpath,
  const Segment_map segment_map = Segment_map()) {

  using Point_3          = typename Kernel::Point_3;
  using Color            = std::array<unsigned char, 3>;
  using Point_with_color = std::pair<Point_3, Color>;
  using PLY_Point_map    = CGAL::First_of_pair_property_map<Point_with_color>;
  using PLY_Color_map    = CGAL::Second_of_pair_property_map<Point_with_color>;

  std::vector<Point_with_color> pwc;
  srand(static_cast<unsigned int>(time(NULL)));

  // Iterate through all regions.
  for (const auto& region : regions) {

    // Generate a random color.
    const Color color =
      CGAL::make_array(
        static_cast<unsigned char>(rand() % 256),
        static_cast<unsigned char>(rand() % 256),
        static_cast<unsigned char>(rand() % 256));

    // Iterate through all region items.
    for (const auto index : region) {
      const auto& key = *(input_range.begin() + index);
      const auto& segment = get(segment_map, key);
      const auto& s = segment.source();
      const auto& t = segment.target();
      pwc.push_back(std::make_pair(Point_3(s.x(), s.y(), 0), color));
      pwc.push_back(std::make_pair(Point_3(t.x(), t.y(), 0), color));
    }
  }

  std::ofstream out(fullpath);
  CGAL::IO::set_ascii_mode(out);
  CGAL::IO::write_PLY_with_properties(
    out, pwc,
    CGAL::make_ply_point_writer(PLY_Point_map()),
      std::make_tuple(
        PLY_Color_map(),
        CGAL::PLY_property<unsigned char>("red"),
        CGAL::PLY_property<unsigned char>("green"),
        CGAL::PLY_property<unsigned char>("blue")));
  out.close();
}

template<typename Kernel, typename Input_range, typename Segment_map>
void save_segment_regions_3(
  const Input_range& input_range,
  const std::vector< std::vector<std::size_t> >& regions,
  const std::string fullpath,
  const Segment_map segment_map = Segment_map()) {

  using Point_3          = typename Kernel::Point_3;
  using Color            = std::array<unsigned char, 3>;
  using Point_with_color = std::pair<Point_3, Color>;
  using PLY_Point_map    = CGAL::First_of_pair_property_map<Point_with_color>;
  using PLY_Color_map    = CGAL::Second_of_pair_property_map<Point_with_color>;

  std::vector<Point_with_color> pwc;
  srand(static_cast<unsigned int>(time(NULL)));

  // Iterate through all regions.
  for (const auto& region : regions) {

    // Generate a random color.
    const Color color =
      CGAL::make_array(
        static_cast<unsigned char>(rand() % 256),
        static_cast<unsigned char>(rand() % 256),
        static_cast<unsigned char>(rand() % 256));

    // Iterate through all region items.
    for (const auto index : region) {
      const auto& key = *(input_range.begin() + index);
      const auto& segment = get(segment_map, key);
      pwc.push_back(std::make_pair(segment.source(), color));
      pwc.push_back(std::make_pair(segment.target(), color));
    }
  }

  std::ofstream out(fullpath);
  CGAL::IO::set_ascii_mode(out);
  CGAL::IO::write_PLY_with_properties(
    out, pwc,
    CGAL::make_ply_point_writer(PLY_Point_map()),
      std::make_tuple(
        PLY_Color_map(),
        CGAL::PLY_property<unsigned char>("red"),
        CGAL::PLY_property<unsigned char>("green"),
        CGAL::PLY_property<unsigned char>("blue")));
  out.close();
}

// Define an insert iterator.
template<
typename Input_range,
typename Output_range,
typename Point_map>
struct Insert_point_colored_by_region_index {
  using argument_type = std::vector<std::size_t>;
  using result_type   = void;
  using Color_map     =
    typename Output_range:: template Property_map<unsigned char>;

  const Input_range& m_input_range;
  const   Point_map  m_point_map;
       Output_range& m_output_range;
        std::size_t& m_number_of_regions;
  Color_map m_red, m_green, m_blue;

  Insert_point_colored_by_region_index(
    const Input_range& input_range,
    const   Point_map  point_map,
         Output_range& output_range,
          std::size_t& number_of_regions) :
  m_input_range(input_range),
  m_point_map(point_map),
  m_output_range(output_range),
  m_number_of_regions(number_of_regions) {
    m_red   = m_output_range.template add_property_map<unsigned char>("red", 0).first;
    m_green = m_output_range.template add_property_map<unsigned char>("green", 0).first;
    m_blue  = m_output_range.template add_property_map<unsigned char>("blue", 0).first;
  }

  result_type operator()(const argument_type& region) {

    CGAL::Random rand(static_cast<unsigned int>(m_number_of_regions));
    const unsigned char r = static_cast<unsigned char>(64 + rand.get_int(0, 192));
    const unsigned char g = static_cast<unsigned char>(64 + rand.get_int(0, 192));
    const unsigned char b = static_cast<unsigned char>(64 + rand.get_int(0, 192));

    for (const std::size_t index : region) {
      const auto& key = *(m_input_range.begin() + index);
      const auto& point = get(m_point_map, key);
      const auto it = m_output_range.insert(point);

      m_red[*it]   = r;
      m_green[*it] = g;
      m_blue[*it]  = b;
    }
    ++m_number_of_regions;
  }
};

template<typename Polygon_mesh>
void save_polygon_mesh_regions(
  Polygon_mesh& polygon_mesh,
  const std::vector< std::vector<std::size_t> >& regions,
  const std::string fullpath) {

  using Color      = CGAL::Color;
  using Face_index = typename Polygon_mesh::Face_index;
  using size_type  = typename Polygon_mesh::size_type;
  srand(static_cast<unsigned int>(time(NULL)));

  bool created;
  typename Polygon_mesh::template Property_map<Face_index, Color> face_color;
  boost::tie(face_color, created) =
    polygon_mesh.template add_property_map<Face_index, Color>(
      "f:color", Color(0, 0, 0));
  if (!created) return;

  // Iterate through all regions.
  std::ofstream out(fullpath);
  for (const auto& region : regions) {

    // Generate a random color.
    const Color color(
      static_cast<unsigned char>(rand() % 256),
      static_cast<unsigned char>(rand() % 256),
      static_cast<unsigned char>(rand() % 256));

    // Iterate through all region items.
    for (const std::size_t index : region) {
      face_color[Face_index(static_cast<size_type>(index))] = color;
    }
  }
  CGAL::IO::write_PLY(out, polygon_mesh);
}

} // namespace utils

#endif // CGAL_SHAPE_DETECTION_EXAMPLES_UTILS_H
