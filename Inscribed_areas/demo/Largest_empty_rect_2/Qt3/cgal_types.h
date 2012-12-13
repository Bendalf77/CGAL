// Copyright (c) 2002  Tel-Aviv University (Israel).
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
// Author(s)     : Radu Ursu

#include <CGAL/basic.h>
#include <CGAL/Cartesian.h>
#include <CGAL/point_generators_2.h>
#include <CGAL/Largest_empty_iso_rectangle_2.h>

typedef double                      Coord_type;
typedef CGAL::Cartesian<Coord_type> Rep;

typedef Rep::Point_2             Point_2;
typedef Rep::Segment_2           Segment;
typedef Rep::Iso_rectangle_2     Iso_rectangle_2;
typedef CGAL::Largest_empty_iso_rectangle_2<Rep>
                                 Largest_empty_rect;
