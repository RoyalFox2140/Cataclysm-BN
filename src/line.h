#pragma once
#ifndef CATA_SRC_LINE_H
#define CATA_SRC_LINE_H

#include <cmath>
#include <functional>
#include <string>
#include <vector>
#include <algorithm>

#include "math_defines.h"
#include "point.h"
#include "cached_options.h"

/** Converts degrees to radians */
constexpr double deg2rad( double v )
{
    return v * M_PI / 180;
}

/** Converts degrees to radians */
constexpr double rad2deg( double v )
{
    return v * 180 / M_PI;
}

/** Converts arc minutes to radians */
constexpr double degmin2rad( double v )
{
    return deg2rad( v ) / 60;
}

/**
 * Calculate base of an isosceles triangle
 * @param distance one of the equal lengths
 * @param vertex the unequal angle expressed in MoA
 * @returns base in equivalent units to distance
 */
inline double iso_tangent( double distance, double vertex )
{
    // we can use the cosine formula (a² = b² + c² - 2bc⋅cosθ) to calculate the tangent
    return std::sqrt( 2 * std::pow( distance, 2 ) * ( 1 - std::cos( degmin2rad( vertex ) ) ) );
}

//! This compile-time usable function combines the sign of each (x, y, z) component into a single integer
//! to allow simple runtime and compile-time mapping of (x, y, z) tuples to @ref direction enumerators.
//! Specifically, (0, -, +) => (0, 1, 2); a base-3 number.
//! This only works correctly for inputs between -1,-1,-1 and 1,1,1.
//! For numbers outside that range, use make_xyz().
inline constexpr unsigned make_xyz_unit( const tripoint &p ) noexcept
{
    return ( ( p.x > 0 ) ? 2u : ( p.x < 0 ) ? 1u : 0u ) * 1u +
           ( ( p.y > 0 ) ? 2u : ( p.y < 0 ) ? 1u : 0u ) * 3u +
           ( ( p.z > 0 ) ? 2u : ( p.z < 0 ) ? 1u : 0u ) * 9u;
}

// This more general version of this function gives correct values for larger inputs.
unsigned make_xyz( const tripoint & );

enum class direction : unsigned {
    ABOVENORTHWEST = make_xyz_unit( tripoint_above + tripoint_north_west ),
    NORTHWEST      = make_xyz_unit( tripoint_north_west ),
    BELOWNORTHWEST = make_xyz_unit( tripoint_below + tripoint_north_west ),
    ABOVENORTH     = make_xyz_unit( tripoint_above + tripoint_north ),
    NORTH          = make_xyz_unit( tripoint_north ),
    BELOWNORTH     = make_xyz_unit( tripoint_below + tripoint_north ),
    ABOVENORTHEAST = make_xyz_unit( tripoint_above + tripoint_north_east ),
    NORTHEAST      = make_xyz_unit( tripoint_north_east ),
    BELOWNORTHEAST = make_xyz_unit( tripoint_below + tripoint_north_east ),

    ABOVEWEST      = make_xyz_unit( tripoint_above + tripoint_west ),
    WEST           = make_xyz_unit( tripoint_west ),
    BELOWWEST      = make_xyz_unit( tripoint_below + tripoint_west ),
    ABOVECENTER    = make_xyz_unit( tripoint_above ),
    CENTER         = make_xyz_unit( tripoint_zero ),
    BELOWCENTER    = make_xyz_unit( tripoint_below ),
    ABOVEEAST      = make_xyz_unit( tripoint_above + tripoint_east ),
    EAST           = make_xyz_unit( tripoint_east ),
    BELOWEAST      = make_xyz_unit( tripoint_below + tripoint_east ),

    ABOVESOUTHWEST = make_xyz_unit( tripoint_above + tripoint_south_west ),
    SOUTHWEST      = make_xyz_unit( tripoint_south_west ),
    BELOWSOUTHWEST = make_xyz_unit( tripoint_below + tripoint_south_west ),
    ABOVESOUTH     = make_xyz_unit( tripoint_above + tripoint_south ),
    SOUTH          = make_xyz_unit( tripoint_south ),
    BELOWSOUTH     = make_xyz_unit( tripoint_below + tripoint_south ),
    ABOVESOUTHEAST = make_xyz_unit( tripoint_above + tripoint_south_east ),
    SOUTHEAST      = make_xyz_unit( tripoint_south_east ),
    BELOWSOUTHEAST = make_xyz_unit( tripoint_below + tripoint_south_east ),
};

template< class T >
constexpr inline direction operator%( const direction &lhs, const T &rhs )
{
    return static_cast<direction>( static_cast<T>( lhs ) % rhs );
}

template< class T >
constexpr inline T operator+( const direction &lhs, const T &rhs )
{
    return static_cast<T>( lhs ) + rhs;
}

template< class T >
constexpr inline bool operator==( const direction &lhs, const T &rhs )
{
    return static_cast<T>( lhs ) == rhs;
}

template< class T >
constexpr inline bool operator==( const T &lhs, const direction &rhs )
{
    return operator==( rhs, lhs );
}

template< class T >
constexpr inline bool operator!=( const T &lhs, const direction &rhs )
{
    return !operator==( rhs, lhs );
}

template< class T >
constexpr inline bool operator!=( const direction &lhs, const T &rhs )
{
    return !operator==( lhs, rhs );
}

direction direction_from( const point &p ) noexcept;
direction direction_from( const tripoint &p ) noexcept;
direction direction_from( const point &p1, const point &p2 ) noexcept;
direction direction_from( const tripoint &p, const tripoint &q );

point direction_XY( direction dir );
std::string direction_name( direction dir );
std::string direction_name_short( direction dir );

/* Get suffix describing vector from p to q (e.g. 1NW, 2SE) or empty string if p == q */
std::string direction_suffix( const tripoint &p, const tripoint &q );

/**
 * The actual Bresenham algorithm in 2D and 3D, everything else should call these
 * and pass in an interact functor to iterate across a line between two points.
 */
void bresenham( const point &p1, const point &p2, int t,
                const std::function<bool( const point & )> &interact );
void bresenham( const tripoint &loc1, const tripoint &loc2, int t, int t2,
                const std::function<bool( const tripoint & )> &interact );

tripoint move_along_line( const tripoint &loc, const std::vector<tripoint> &line,
                          int distance );
// The "t" value decides WHICH Bresenham line is used.
std::vector<point> line_to( const point &p1, const point &p2, int t = 0 );
// t and t2 decide which Bresenham line is used.
std::vector<tripoint> line_to( const tripoint &loc1, const tripoint &loc2, int t = 0, int t2 = 0 );
// sqrt(dX^2 + dY^2)

inline float trig_dist( const tripoint &loc1, const tripoint &loc2 )
{
    return std::sqrt( static_cast<double>( ( loc1.x - loc2.x ) * ( loc1.x - loc2.x ) ) +
                      ( ( loc1.y - loc2.y ) * ( loc1.y - loc2.y ) ) +
                      ( ( loc1.z - loc2.z ) * ( loc1.z - loc2.z ) ) );
}
inline float trig_dist( const point &loc1, const point &loc2 )
{
    return trig_dist( tripoint( loc1, 0 ), tripoint( loc2, 0 ) );
}

// Roguelike distance; maximum of dX and dY
inline int square_dist( const tripoint &loc1, const tripoint &loc2 )
{
    const tripoint d = ( loc1 - loc2 ).abs();
    return std::max( { d.x, d.y, d.z } );
}
inline int square_dist( const point &loc1, const point &loc2 )
{
    const point d = ( loc1 - loc2 ).abs();
    return std::max( d.x, d.y );
}

// Choose between the above two according to the "circular distances" option
inline int rl_dist( const tripoint &loc1, const tripoint &loc2 )
{
    if( trigdist ) {
        return trig_dist( loc1, loc2 );
    }
    return square_dist( loc1, loc2 );
}
inline int rl_dist( const point &a, const point &b )
{
    return rl_dist( tripoint( a, 0 ), tripoint( b, 0 ) );
}

/**
 * Helper type for the return value of dist_fast().
 *
 * This lets us delay the sqrt() call of trigdist until the actual length is needed.
 */
struct FastDistanceApproximation {
    private:
        int value;
    public:
        inline FastDistanceApproximation( int value ) : value( value ) { }
        template<typename T>
        inline bool operator<=( const T &rhs ) const {
            if( trigdist ) {
                return value <= rhs * rhs;
            }
            return value <= rhs;
        }
        template<typename T>
        inline bool operator>=( const T &rhs ) const {
            if( trigdist ) {
                return value >= rhs * rhs;
            }
            return value >= rhs;
        }
        inline operator int() const {
            if( trigdist ) {
                return std::sqrt( value );
            }
            return value;
        }
};

inline FastDistanceApproximation trig_dist_fast( const tripoint &loc1, const tripoint &loc2 )
{
    return ( loc1.x - loc2.x ) * ( loc1.x - loc2.x ) +
           ( loc1.y - loc2.y ) * ( loc1.y - loc2.y ) +
           ( loc1.z - loc2.z ) * ( loc1.z - loc2.z );
}
inline FastDistanceApproximation square_dist_fast( const tripoint &loc1, const tripoint &loc2 )
{
    const tripoint d = ( loc1 - loc2 ).abs();
    return std::max( { d.x, d.y, d.z } );
}
inline FastDistanceApproximation rl_dist_fast( const tripoint &loc1, const tripoint &loc2 )
{
    if( trigdist ) {
        return trig_dist_fast( loc1, loc2 );
    }
    return square_dist_fast( loc1, loc2 );
}
inline FastDistanceApproximation rl_dist_fast( const point &a, const point &b )
{
    return rl_dist_fast( tripoint( a, 0 ), tripoint( b, 0 ) );
}

float rl_dist_exact( const tripoint &loc1, const tripoint &loc2 );
// Sum of distance in both axes
int manhattan_dist( const point &loc1, const point &loc2 );

// get angle of direction represented by point (in radians or degrees)
double atan2( const point & );
double atan2_degrees( const point & );

// Get the magnitude of the slope ranging from 0.0 to 1.0
float get_normalized_angle( const point &start, const point &end );
std::vector<tripoint> continue_line( const std::vector<tripoint> &line, int distance );
std::vector<point> squares_in_direction( const point &p1, const point &p2 );
// Returns a vector of squares adjacent to @from that are closer to @to than @from is.
// Currently limited to the same z-level as @from.
std::vector<tripoint> squares_closer_to( const tripoint &from, const tripoint &to );
void calc_ray_end( int angle, int range, const tripoint &p, tripoint &out );
/**
 * Calculates the horizontal angle between the lines from (0,0,0) to @p a and
 * the line from (0,0,0) to @p b.
 * Returned value is in degree and in the range 0....360.
 * Example: if @p a is (0,1) and @p b is (1,0), the result will 90 degree
 * The function currently ignores the z component.
 */
double coord_to_angle( const tripoint &a, const tripoint &b );

#endif // CATA_SRC_LINE_H
