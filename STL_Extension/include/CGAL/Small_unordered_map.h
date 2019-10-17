// Copyright (c) 2019  GeometryFactory (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org); you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; either version 3 of the License,
// or (at your option) any later version.
//
// Licensees holding a valid commercial license may use this file in
// accordance with the commercial license agreement provided with the software.
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// $URL$
// $Id$
// SPDX-License-Identifier: LGPL-3.0+
//
// Author(s)     : Andreas Fabri

#ifndef CGAL_SMALL_UNORDERED_MAP_H
#define CGAL_SMALL_UNORDERED_MAP_H

#include <array>
#include <iostream>

//#define CGAL_SMALL_UNORDERED_MAP_STATS
namespace CGAL {
  
template <typename K, typename T, typename H, unsigned int N>
class Small_unordered_map{
#ifdef    CGAL_SMALL_UNORDERED_MAP_STATS
  std::array<int,10> collisions = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
#endif
  static const unsigned int M = N<<3;
  int head = -2;
  mutable std::array<int, M>           occupied;
  std::array<int, M>           unfreelist; 
  std::array<std::pair<K,T>, M> data;
  const H hash = {};

  
public:
  Small_unordered_map()
  {
    occupied.fill(-1);
  }

#ifdef CGAL_SMALL_UNORDERED_MAP_STATS  
  ~Small_unordered_map()
  {
    for(int i = 0; i < 10; i++){
      std::cout << i << " " << collisions[i] << std::endl;
    }
  }
#endif
  
  /// Set only once for a key and not more than N 
  void set(const K& k, const T& t)
  {
    unsigned int h  = hash(k)%M;
    unsigned i = h;
#ifdef CGAL_SMALL_UNORDERED_MAP_STATS    
    int collision = 0;
#endif     
    do {
      if(occupied[i]== -1){
        occupied[i] = 1;
        data[i].first = k;
        data[i].second = t;
        unfreelist[i] = head;
        head = i;
#ifdef  CGAL_SMALL_UNORDERED_MAP_STATS        
        if(collision>9){
          std::cerr << collision << " collisions" << std::endl;
        }
        ++collisions[collision];
#endif        
        return;
      }
      i = (i+1)%M;
#ifdef CGAL_SMALL_UNORDERED_MAP_STATS   
      ++collision;
#endif      
    }while(i != h);
    CGAL_error();
  }

  
  const T& get(const K& k) const
  {
    unsigned int h  = hash(k)%M;
    int i = h;
    do{
      if((occupied[i] == 1) && (data[i].first == k)){
        occupied[i] = -1;
        return data[i].second;
      }
      i = (i+1)%M;
    }while(i != h);
    CGAL_error();
  }

  void clear()
  {
    head = -2;
    //    occupied.fill(-1);
  }

  struct iterator {
    const Small_unordered_map& map;
    int pos;

    iterator(const Small_unordered_map& map)
      : map(map),pos(-2)
    {}

    iterator(const Small_unordered_map& map, int pos)
      :  map(map), pos(pos)
    {}

    bool operator==(const iterator& other) const
    {
      return pos == other.pos;
    }

    bool operator!=(const iterator& other) const
    {
      return pos != other.pos;
    }
    iterator operator++()
    {
      pos = map.unfreelist[pos];
      return *this;
    }
    
    const std::pair<K,T>& operator*()const
    {
      return map.data[pos];
    }
  };

  iterator begin() const
  {
    return iterator(*this,head);
  }

  iterator end() const
  {
    return iterator(*this);
  }

  void clear(const iterator it)
  {
    occupied[it.pos] = -1;
  }
  
  friend struct iterator;
};

} // namespace CGAL
#endif // CGAL_SMALL_UNORDERED_MAP_H
